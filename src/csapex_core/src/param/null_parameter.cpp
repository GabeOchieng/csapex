/// HEADER
#include <csapex/param/null_parameter.h>

/// PROJECT
#include <csapex/param/register_parameter.h>
#include <csapex/serialization/io/std_io.h>

/// SYSTEM
#include <yaml-cpp/yaml.h>

CSAPEX_REGISTER_PARAM(NullParameter)

using namespace csapex;
using namespace param;

NullParameter::NullParameter() : ParameterImplementation("null", ParameterDescription())
{
}

NullParameter::NullParameter(const std::string& name, const ParameterDescription& description) : ParameterImplementation(name, description)
{
}

NullParameter::~NullParameter()
{
}

bool NullParameter::hasState() const
{
    return false;
}

const std::type_info& NullParameter::type() const
{
    return typeid(void);
}

std::string NullParameter::toStringImpl() const
{
    return std::string("[null]");
}

void NullParameter::get_unsafe(boost::any& out) const
{
    throw std::logic_error("cannot use null parameters");
}

bool NullParameter::set_unsafe(const boost::any& /*v*/)
{
    throw std::logic_error("cannot use null parameters");
}

void NullParameter::doSerialize(YAML::Node& /*n*/) const
{
}

void NullParameter::doDeserialize(const YAML::Node& /*n*/)
{
}
