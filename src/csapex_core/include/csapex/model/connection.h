#ifndef CONNECTION_H
#define CONNECTION_H

/// COMPONENT
#include <csapex/data/point.h>
#include <csapex/model/model_fwd.h>
#include <csapex/msg/msg_fwd.h>
#include <csapex/signal/signal_fwd.h>
#include <csapex/utility/slim_signal.hpp>
#include <csapex/model/token.h>
#include <csapex_core/csapex_core_export.h>
#include <csapex/model/connection_description.h>

/// SYSTEM
#include <memory>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>

namespace csapex
{
class CSAPEX_CORE_EXPORT Connection
{
    friend class GraphIO;
    friend class Graph;
    friend class Fulcrum;

public:
    typedef std::shared_ptr<Connection> Ptr;

    enum class State
    {
        NOT_INITIALIZED,
        UNREAD,
        READ,
        DONE = NOT_INITIALIZED
    };

public:
    friend std::ostream& operator<<(std::ostream& out, const Connection& c);

protected:
    Connection(OutputPtr from, InputPtr to);
    Connection(OutputPtr from, InputPtr to, int id);

public:
    static bool isCompatibleWith(Connector* from, Connector* to);

    static bool canBeConnectedTo(Connector* from, Connector* to);
    static bool targetsCanBeMovedTo(Connector* from, Connector* to);

    static bool areConnectorsConnected(Connector* from, Connector* to);

public:
    virtual ~Connection();

    void detach(Connector* c);
    bool isDetached() const;

    OutputPtr from() const;
    InputPtr to() const;

    ConnectorPtr source() const;
    ConnectorPtr target() const;

    int id() const;

    ConnectionDescription getDescription() const;

    bool contains(Connector* c) const;

    virtual void setToken(const TokenPtr& msg, const bool silent = false);

    TokenPtr getToken() const;
    void setTokenProcessed();

    /**
     * @brief readMessage retrieves the current message and marks the Connection read
     * @return
     */
    TokenPtr readToken();

    bool holdsToken() const;
    bool holdsActiveToken() const;

    bool isActive() const;
    void setActive(bool active);

    bool isEnabled() const;
    bool isSourceEnabled() const;
    bool isSinkEnabled() const;

    bool isPipelining() const;

    State getState() const;
    void setState(State s);

    int getSeq() const;

    void reset();

    void notifyMessageSet();
    void notifyMessageProcessed();

public:
    slim_signal::Signal<void()> deleted;

    slim_signal::Signal<void(bool)> source_enable_changed;
    slim_signal::Signal<void(bool)> sink_enabled_changed;
    slim_signal::Signal<void()> connection_changed;

    slim_signal::Signal<void(Fulcrum*)> fulcrum_added;
    slim_signal::Signal<void(Fulcrum*, bool dropped)> fulcrum_moved;
    slim_signal::Signal<void(Fulcrum*, bool dropped, int which)> fulcrum_moved_handle;
    slim_signal::Signal<void(Fulcrum*, int type)> fulcrum_type_changed;
    slim_signal::Signal<void(Fulcrum*)> fulcrum_deleted;

public:
    bool operator==(const Connection& c) const;

    std::vector<FulcrumPtr> getFulcrums() const;
    std::vector<Fulcrum> getFulcrumsCopy() const;

    int getFulcrumCount() const;
    FulcrumPtr getFulcrum(int fulcrum_id);

    void addFulcrum(int fulcrum_id, const Point& pos, int type, const Point& handle_in = Point(-10.0, 0.0), const Point& handle_out = Point(10.0, 0.0));
    void modifyFulcrum(int fulcrum_id, int type, const Point& handle_in = Point(-10.0, 0.0), const Point& handle_out = Point(10.0, 0.0));
    void moveFulcrum(int fulcrum_id, const Point& pos, bool dropped);
    void deleteFulcrum(int fulcrum_id);

protected:
    OutputPtr from_;
    InputPtr to_;
    int id_;

    bool active_;

    bool detached_;

    std::vector<FulcrumPtr> fulcrums_;

    State state_;
    TokenPtr message_;

    static int next_connection_id_;

    int seq_ = 0;

    mutable std::recursive_mutex sync;
};

std::ostream& operator<<(std::ostream& out, const Connection& c);

}  // namespace csapex

#endif  // CONNECTION_H
