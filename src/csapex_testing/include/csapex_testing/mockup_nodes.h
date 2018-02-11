#ifndef MOCKUP_NODES_H
#define MOCKUP_NODES_H

/// PROJECT
#include <csapex/model/node.h>
#include <csapex/msg/msg_fwd.h>
#include <csapex/model/node_modifier.h>
#include <csapex/msg/io.h>

namespace csapex
{

template <int factor>
class MockupStaticMultiplierNode
{
public:
    void setup(NodeModifier& node_modifier)
    {
        in = node_modifier.addInput<int>("input");
        out = node_modifier.addOutput<int>("output");
    }

    void setupParameters(Parameterizable& /*parameters*/)
    {

    }

    void process(NodeModifier& node_modifier, Parameterizable& /*parameters*/)
    {
        int val = msg::getValue<int>(in);
        val *= factor;


        msg::publish(out, val);
    }

private:
    Input* in;
    Output* out;
};


class MockupDynamicMultiplierNode
{
public:
    MockupDynamicMultiplierNode();

    void setup(csapex::NodeModifier& node_modifier);

    void setupParameters(Parameterizable& parameters);

    void process(NodeModifier& node_modifier, Parameterizable& parameters);

private:
    Input* input_a_;
    Input* input_b_;
    Output* output_;
};


class MockupSource : public Node
{
public:
    MockupSource();

    void setup(NodeModifier& node_modifier) override;

    void setupParameters(Parameterizable& parameters) override;

    void process() override;

    int getValue() const;

private:
    Output* out;
};

class MockupSink : public Node
{
public:
    MockupSink();

    void setup(NodeModifier& node_modifier) override;

    void setupParameters(Parameterizable& parameters) override;

    void process(NodeModifier& node_modifier, Parameterizable& parameters) override;

    int getValue() const;

    void abort();

private:
    Input* in;

    bool aborted;
    int value;
};


class AnySink : public Node
{
public:
    AnySink();

    void setup(NodeModifier& node_modifier) override;

    void process(NodeModifier& node_modifier, Parameterizable& parameters) override;

private:
    Input* in;
};

}

#endif // MOCKUP_NODES_H
