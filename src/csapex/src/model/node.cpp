/// HEADER
#include <csapex/model/node.h>

/// COMPONENT
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/signal/slot.h>
#include <csapex/signal/trigger.h>
#include <csapex/model/node_state.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/assert.h>
#include <csapex/core/settings.h>

/// SYSTEM


using namespace csapex;

Node::Node()
    : adebug(std::cout, ""), ainfo(std::cout, ""), awarn(std::cout, ""), aerr(std::cerr, ""),
      guard_(-1)
{
}

Node::~Node()
{
    apex_assert_hard(guard_ == -1);

    guard_ = 0xDEADBEEF;
}

void Node::initialize(csapex::NodeModifier* node_modifier, const UUID& uuid)
{
    uuid_ = uuid;
    node_modifier_ = node_modifier;
    parameters_ = this;

    parameter_state_->setParentUUID(uuid);

    std::string p = uuid.getFullName();
    adebug.setPrefix(p);
    ainfo.setPrefix(p);
    awarn.setPrefix(p);
    aerr.setPrefix(p);
}

UUID Node::getUUID() const
{
    return uuid_;
}

void Node::setupParameters(Parameterizable& )
{

}

void Node::stateChanged()
{

}

bool Node::isAsynchronous() const
{
    return false;
}

void Node::process(csapex::NodeModifier& node_modifier, csapex::Parameterizable& parameters,
                   std::function<void(std::function<void (csapex::NodeModifier&, csapex::Parameterizable&)>)> continuation)
{
    process(node_modifier, parameters);
    continuation([](csapex::NodeModifier&, csapex::Parameterizable&){});
}

void Node::process(csapex::NodeModifier& /*node_modifier*/, csapex::Parameterizable& /*parameters*/)
{
    process();
}


void Node::process()
{
    // default: do nothing, clients overwrite this
}


void Node::abort()
{
}
