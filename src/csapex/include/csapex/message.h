#ifndef MESSAGE_H
#define MESSAGE_H

/// COMPONENT
#include <csapex/connection_type.h>

namespace csapex {
namespace connection_types {

struct Message : public ConnectionType
{
public:
    typedef boost::shared_ptr<Message> Ptr;

protected:
    Message(const std::string& name);
    virtual ~Message();

public:
    void writeYaml(YAML::Emitter& yaml);
    void readYaml(YAML::Node& node);
};

struct AnyMessage : public Message
{
public:
    typedef boost::shared_ptr<AnyMessage> Ptr;

protected:
    AnyMessage(const std::string& name);

public:
    virtual ConnectionType::Ptr clone() ;
    virtual ConnectionType::Ptr toType();

    static ConnectionType::Ptr make();

    bool canConnectTo(ConnectionType::Ptr other_side);
    bool acceptsConnectionFrom(ConnectionType* other_side);
};

template <class Type, class Instance>
struct MessageTemplate : public Message {
    typedef boost::shared_ptr<Instance> Ptr;

    MessageTemplate(const std::string& name)
        : Message(name)
    {}

    virtual ConnectionType::Ptr clone() {
        Ptr new_msg(new Instance);
        new_msg->value = value;
        return new_msg;
    }

    virtual ConnectionType::Ptr toType() {
        Ptr new_msg(new Instance);
        return new_msg;
    }

    bool acceptsConnectionFrom(ConnectionType* other_side) {
        return name() == other_side->name();
    }

    void writeYaml(YAML::Emitter& yaml) {
        yaml << YAML::Key << "value" << YAML::Value << "not implemented";
    }
    void readYaml(YAML::Node& node) {
    }

    Type value;
};


}
}

#endif // MESSAGE_H