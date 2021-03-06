/// HEADER
#include <csapex/msg/output_transition.h>

/// COMPONENT
#include <csapex/model/node_handle.h>
#include <csapex/msg/output.h>
#include <csapex/model/connection.h>
#include <csapex/utility/assert.h>
#include <csapex/msg/input.h>
#include <csapex/msg/no_message.h>
#include <csapex/utility/debug.h>

/// SYSTEM
#include <iostream>

using namespace csapex;

OutputTransition::OutputTransition(delegate::Delegate0<> activation_fn) : Transition(activation_fn), sequence_number_(-1)
{
}
OutputTransition::OutputTransition() : Transition(), sequence_number_(-1)
{
}

void OutputTransition::reset()
{
    std::unique_lock<std::recursive_mutex> lock(sync);
    for (const ConnectionPtr& connection : connections_) {
        connection->reset();
    }
    auto outputs_copy = outputs_;
    lock.unlock();

    for (const auto& pair : outputs_copy) {
        OutputPtr output = pair.second;
        output->reset();
    }
}

std::vector<UUID> OutputTransition::getOutputs() const
{
    std::vector<UUID> res;
    for (const auto& pair : outputs_) {
        res.push_back(pair.second->getUUID());
    }
    std::sort(res.begin(), res.end());
    return res;
}

OutputPtr OutputTransition::getOutput(const UUID& id) const
{
    return outputs_.at(id);
}

OutputPtr OutputTransition::getOutputNoThrow(const UUID& id) const noexcept
{
    auto pos = outputs_.find(id);
    if (pos == outputs_.end()) {
        return nullptr;
    }
    return pos->second;
}

void OutputTransition::addOutput(OutputPtr output)
{
    output->setOutputTransition(this);

    output->setSequenceNumber(sequence_number_);

    // remember the output
    outputs_[output->getUUID()] = output;

    // connect signals
    auto ca = output->connection_added.connect([this](const ConnectionPtr& connection) { addConnection(connection); });
    output_signal_connections_[output.get()].push_back(ca);

    auto cf = output->connection_faded.connect([this](const ConnectionPtr& connection) { removeConnection(connection); });
    output_signal_connections_[output.get()].push_back(cf);

    auto cp = output->message_processed.connect([this](const ConnectorPtr&) { tokenProcessed(); });
    output_signal_connections_[output.get()].push_back(cp);

    auto ce = output->connectionEnabled.connect([this](bool enabled) {
        if (enabled) {
            for (const auto& pair : outputs_) {
                OutputPtr output = pair.second;
                output->republish();
            }
        }
    });
    output_signal_connections_[output.get()].push_back(ce);

    //    auto cr = output->connection_removed_to.connect([this](Connectable* output) {
    //        if(output->isEnabled()) {
    //            publishNextMessage();
    //        }
    //    });
    //    output_signal_connections_[output].push_back(cr);
}

void OutputTransition::removeOutput(OutputPtr output)
{
    output->removeOutputTransition();

    // disconnect signals
    output_signal_connections_.erase(output.get());

    // forget the output
    outputs_.erase(output->getUUID());
}

void OutputTransition::setSequenceNumber(long seq_no)
{
    sequence_number_ = seq_no;

    for (const auto& pair : outputs_) {
        OutputPtr output = pair.second;
        output->setSequenceNumber(sequence_number_);
    }
}

long OutputTransition::getSequenceNumber() const
{
    return sequence_number_;
}

bool OutputTransition::isEnabled() const
{
    return canStartSendingMessages();
}

bool OutputTransition::canStartSendingMessages() const
{
    for (const auto& pair : outputs_) {
        const OutputPtr& output = pair.second;
        if (output->isEnabled() && output->isConnected()) {
            if (output->getState() != Output::State::IDLE) {
                // TRACE for(const auto& pair : outputs_) {
                // TRACE OutputPtr output = pair.second;
                // TRACE if(output->isEnabled() && output->isConnected()) {
                // TRACE std::cout << "output " << output->getUUID() << " is not idle" << std::endl;
                // TRACE }
                // TRACE }
                return false;
            }
        }
    }
    return areAllConnections(Connection::State::DONE, Connection::State::NOT_INITIALIZED);
}

bool OutputTransition::sendMessages(bool is_active)
{
    std::unique_lock<std::recursive_mutex> lock(sync);

    apex_assert_hard(areAllConnections(Connection::State::NOT_INITIALIZED));

    bool has_sent_activator_message = false;

    for (const auto& pair : outputs_) {
        const OutputPtr& output = pair.second;
        if (output->isEnabled()) {
            has_sent_activator_message |= output->commitMessages(is_active);
        }
    }

    long seq_no = -1;
    for (const auto& pair : outputs_) {
        const OutputPtr& output = pair.second;
        long s = output->sequenceNumber();
        if (seq_no == -1) {
            seq_no = s;
        } else {
            apex_assert_soft(seq_no == s);
        }
    }

    setSequenceNumber(seq_no);

    fillConnections();

    if (!hasConnection()) {
        setOutputsIdle();
    }

    return has_sent_activator_message;
}

void OutputTransition::tokenProcessed()
{
    std::unique_lock<std::recursive_mutex> lock(sync);
    if (!areAllConnections(Connection::State::DONE)) {
        // if (!outputs_.empty()) {
        //     std::cerr << outputs_.begin()->second->getUUID() << ": cannot publish next, not all connections are done:" << std::endl;
        //     for (const ConnectionPtr& connection : connections_) {
        //         std::string s;
        //         switch (connection->getState()) {
        //             case Connection::State::DONE:
        //                 s = "DONE / NOT_INITIALIZED";
        //                 break;
        //             case Connection::State::UNREAD:
        //                 s = "UNREAD";
        //                 break;
        //             case Connection::State::READ:
        //                 s = "READ";
        //                 break;
        //         }
        //         std::cerr << outputs_.begin()->second->getUUID() << ": " << *connection << "-> " << s << std::endl;
        //     }
        // }
        return;
    }

    apex_assert_hard(areAllConnections(Connection::State::DONE));

    APEX_DEBUG_CERR << "all outputs are done" << std::endl;

    lock.unlock();

    // if (!outputs_.empty()) {
    //     std::cerr << outputs_.begin()->second->getUUID() << ": notify messages processed" << std::endl;
    // }

    messages_processed();
    checkIfEnabled();
}

bool OutputTransition::areOutputsIdle() const
{
    std::unique_lock<std::recursive_mutex> lock(sync);
    for (const auto& pair : outputs_) {
        OutputPtr output = pair.second;
        if (output->getState() != Output::State::IDLE) {
            return false;
        }
    }
    return true;
}

int OutputTransition::getPortCount() const
{
    return outputs_.size();
}

void OutputTransition::fillConnections()
{
    std::unique_lock<std::recursive_mutex> lock(sync);
    if (outputs_.empty()) {
        return;
    }
    if (areOutputsIdle()) {
        // no output has a message
        return;
    }

    apex_assert_hard(areAllConnections(Connection::State::NOT_INITIALIZED));

    for (const auto& pair : outputs_) {
        OutputPtr out = pair.second;
        apex_assert_hard(out);
        if (out->isEnabled()) {
            out->publish();
        }
    }
}

void OutputTransition::clearBuffer()
{
    std::unique_lock<std::recursive_mutex> lock(sync);
    for (const auto& pair : outputs_) {
        OutputPtr output = pair.second;
        output->clearBuffer();
    }
}

void OutputTransition::setOutputsIdle()
{
    // TRACE std::cout << "set outputs idle" << std::endl;
    std::unique_lock<std::recursive_mutex> lock(sync);
    for (const auto& pair : outputs_) {
        OutputPtr output = pair.second;
        output->setState(Output::State::IDLE);
    }
}
