#ifndef NODE_FACADE_H
#define NODE_FACADE_H

/// COMPONENT
#include <csapex/model/model_fwd.h>
#include <csapex/model/observer.h>
#include <csapex/utility/notifier.h>
#include <csapex/utility/uuid.h>
#include <csapex/utility/slim_signal.hpp>
#include <csapex/profiling/profiling_fwd.h>
#include <csapex/model/activity_type.h>
#include <csapex/model/execution_state.h>
#include <csapex/model/connector_description.h>

namespace csapex
{

class CSAPEX_EXPORT NodeFacade : public Observer, public Notifier
{
public:
    NodeFacade(NodeHandlePtr nh);
    NodeFacade(NodeHandlePtr nh, NodeWorkerPtr nw);

    ~NodeFacade();

    std::string getType() const;
    UUID getUUID() const;

    bool isActive() const;
    bool isProcessingEnabled() const;

    bool isGraph() const;
    AUUID getSubgraphAUUID() const;

    bool isSource() const;
    bool isSink() const;

    bool isParameterInput(const UUID& id);
    bool isParameterOutput(const UUID& id);

    bool isVariadic() const;
    bool hasVariadicInputs() const;
    bool hasVariadicOutputs() const;
    bool hasVariadicEvents() const;
    bool hasVariadicSlots() const;

    std::vector<ConnectorDescription> getInputs() const;
    std::vector<ConnectorDescription> getOutputs() const;
    std::vector<ConnectorDescription> getEvents() const;
    std::vector<ConnectorDescription> getSlots() const;

    std::vector<ConnectorDescription> getInternalInputs() const;
    std::vector<ConnectorDescription> getInternalOutputs() const;
    std::vector<ConnectorDescription> getInternalEvents() const;
    std::vector<ConnectorDescription> getInternalSlots() const;

    NodeCharacteristics getNodeCharacteristics() const;

    bool isProfiling() const;
    void setProfiling(bool profiling);

    bool isError() const;
    ErrorState::ErrorLevel errorLevel() const;
    std::string errorMessage() const;

    ExecutionState getExecutionState() const;

    std::string getLabel() const;

    double getExecutionFrequency() const;
    double getMaximumFrequency() const;

    // Parameterizable
    bool hasParameter(const std::string& name) const;
    template <typename T>
    T readParameter(const std::string& name) const;
    template <typename T>
    void setParameter(const std::string& name, const T& value);

    // Debug Access
    std::string getDebugDescription() const;
    std::string getLoggerOutput(ErrorState::ErrorLevel level) const;

    // TODO: proxies
    ProfilerPtr getProfiler();
    NodeStatePtr getNodeState() const;
    NodeStatePtr getNodeStateCopy() const;
    GenericStateConstPtr getParameterState() const;

    // TODO: remove
    NodeHandlePtr getNodeHandle();

public:
    slim_signal::Signal<void(NodeFacade* facade)> start_profiling;
    slim_signal::Signal<void(NodeFacade* facade)> stop_profiling;


    slim_signal::Signal<void (ConnectablePtr)> connector_created;
    slim_signal::Signal<void (ConnectablePtr)> connector_removed;

    slim_signal::Signal<void (ConnectablePtr, ConnectablePtr)> connection_in_prograss;
    slim_signal::Signal<void (ConnectablePtr)> connection_done;
    slim_signal::Signal<void (ConnectablePtr)> connection_start;

    slim_signal::Signal<void()> messages_processed;

    slim_signal::Signal<void()> node_state_changed;
    slim_signal::Signal<void()> parameters_changed;

    slim_signal::Signal<void()> destroyed;

    slim_signal::Signal<void(NodeFacade* facade, ActivityType type, std::shared_ptr<const Interval> stamp)> interval_start;
    slim_signal::Signal<void(NodeFacade* facade, std::shared_ptr<const Interval> stamp)> interval_end;

private:
    NodeHandlePtr nh_;
    NodeWorkerPtr nw_;
};

}

#endif // NODE_FACADE_H
