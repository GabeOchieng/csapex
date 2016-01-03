/// HEADER
#include <csapex/utility/slim_signal.h>

/// PROJECT
#include <csapex/utility/assert.h>

/// SYSTEM
#include <algorithm>

using namespace csapex;
using namespace slim_signal;

SignalBase::SignalBase()
{

}

SignalBase::~SignalBase()
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    while(!connections_.empty()) {
        Connection* c = connections_.front();
        apex_assert_hard(c->parent_ == this);
        c->detach();
    }
}

void SignalBase::addConnection(Connection *connection)
{
    apex_assert_hard(connection->parent_ == this);

    std::unique_lock<std::recursive_mutex> lock(mutex_);

    connections_.push_back(connection);
}

void SignalBase::removeConnection(const Connection *connection)
{
    apex_assert_hard(connection->parent_ == this);

    std::unique_lock<std::recursive_mutex> lock(mutex_);

    for(auto it = connections_.begin(); it != connections_.end();) {
        if(*it == connection) {
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
}

void SignalBase::disconnectAll()
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    for(Connection* c : connections_) {
        c->disconnect();
    }
    connections_.clear();
}


Connection::Connection(SignalBase* parent, const Deleter& del)
    : parent_(parent), deleter_(del)
{
    apex_assert_hard(parent);
    parent_->addConnection(this);
}

Connection::Connection(const Connection& other)
    : parent_(other.parent_), deleter_(other.deleter_)
{
    if(parent_) {
        parent_->addConnection(this);
    }
}

Connection::Connection()
    : parent_(nullptr)
{
}

Connection::~Connection()
{
    if(parent_) {
        parent_->removeConnection(this);
    }
}

void Connection::detach() const
{
    parent_->removeConnection(this);
    parent_ = nullptr;
    detached_ = true;
}

void Connection::disconnect() const
{
    if(parent_ && deleter_) {
        deleter_();
        parent_->removeConnection(this);
        parent_ = nullptr;
    }
}



ScopedConnection::ScopedConnection(const Connection& c)
    : Connection(c)
{
}
ScopedConnection::ScopedConnection(ScopedConnection&& c)
    : Connection(c)
{
}
ScopedConnection::ScopedConnection()
{
}

ScopedConnection::~ScopedConnection()
{
    disconnect();
}


void ScopedConnection::operator = (const Connection& c)
{
    apex_assert_hard(c.parent_ != nullptr);
    disconnect();
    deleter_ = c.deleter_;
    parent_ = c.parent_;
    parent_->addConnection(this);
}

void ScopedConnection::operator = (ScopedConnection&& c)
{
    apex_assert_hard(c.parent_ != nullptr);
    disconnect();
    deleter_ = c.deleter_;
    c.parent_->removeConnection(&c);
    parent_ = c.parent_;
    parent_->addConnection(this);
}