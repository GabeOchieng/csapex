#ifndef INPUT_H
#define INPUT_H

/// COMPONENT
#include <csapex/msg/msg_fwd.h>
#include <csapex/model/connectable.h>
#include <csapex/msg/message.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT Input : public Connectable
{
    friend class Output;

public:
    Input(const UUID& uuid, ConnectableOwnerWeakPtr owner = ConnectableOwnerWeakPtr());
    virtual ~Input();

    void setInputTransition(InputTransition* it);
    void removeInputTransition();

    virtual bool isInput() const override
    {
        return true;
    }

    int maxConnectionCount() const override;

    virtual ConnectorType getConnectorType() const override
    {
        return ConnectorType::INPUT;
    }

    virtual void setToken(TokenPtr message);
    virtual TokenPtr getToken() const;

    OutputPtr getSource() const;

    virtual void removeAllConnectionsNotUndoable() override;

    virtual bool isOptional() const override;
    void setOptional(bool optional);

    virtual bool hasMessage() const;
    virtual bool hasReceived() const;

    void free();
    void stop() override;

    virtual void enable() override;
    virtual void disable() override;

    virtual void notifyMessageAvailable(Connection* connection);

    virtual void reset() override;

public:
    slim_signal::Signal<void(Connectable*)> message_set;
    slim_signal::Signal<void(Connection*)> message_available;

protected:
    virtual void addStatusInformation(std::stringstream& status_stream) const override;

protected:
    InputTransition* transition_;

    mutable std::mutex message_mutex_;
    TokenPtr message_;

    bool optional_;
};

}  // namespace csapex

#endif  // INPUT_H
