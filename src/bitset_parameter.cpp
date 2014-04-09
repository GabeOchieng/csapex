/// HEADER
#include <utils_param/bitset_parameter.h>

using namespace param;

BitSetParameter::BitSetParameter()
    : Parameter("noname")
{
}


BitSetParameter::BitSetParameter(const std::string &name)
    : Parameter(name)
{
}

BitSetParameter::~BitSetParameter()
{

}

void BitSetParameter::setByName(const std::string &name)
{
    for(std::map<std::string, int>::iterator it = set_.begin(); it != set_.end(); ++it) {
        if(it->first == name) {
            value_ = it->second;
            triggerChange();
            return;
        }
    }

    throw std::runtime_error(std::string("no such parameter: ") + name);
}

void BitSetParameter::setBitSet(const std::map<std::string, int> &set) {
    set_ = set;
}

void BitSetParameter::clear()
{
    value_ = 0;
    triggerChange();
}

void BitSetParameter::setBits(const std::vector<std::string> &elements, bool silent)
{
    bool change = false;

    for(std::map<std::string, int>::iterator set_it = set_.begin(); set_it != set_.end(); ++set_it) {
        bool found = false;
        const std::string& e = set_it->first;
        for(std::vector<std::string>::const_iterator e_it = elements.begin(); e_it != elements.end(); ++e_it) {
            if(e == *e_it) {
                found = true;
                break;
            }
        }
        if(found) {
            if(!isSet(e)) {
                setBit(e, true);
                change = true;
            }
        } else {
            if(isSet(e)) {
                clearBit(e, true);
                change = true;
            }
        }
    }

    if(change && !silent) {
        triggerChange();
    }
}

void BitSetParameter::setBitTo(const std::string &element, bool set, bool silent)
{
    for(std::map<std::string, int>::iterator it = set_.begin(); it != set_.end(); ++it) {
        if(it->first == element) {
            if(set) {
                value_ |= it->second;
            } else {
                value_ &= ~(it->second);
            }

            if(!silent) {
                triggerChange();
            }
            return;
        }
    }
}

void BitSetParameter::setBit(const std::string &element, bool silent)
{
    setBitTo(element, true, silent);
}

void BitSetParameter::clearBit(const std::string &element, bool silent)
{
    setBitTo(element, false, silent);
}

bool BitSetParameter::isSet(const std::string &element) const
{
    for(std::map<std::string, int>::const_iterator it = set_.begin(); it != set_.end(); ++it) {
        if(it->first == element) {
            int target = it->second;

            return (value_ & target) == target;
        }
    }
    return false;
}

int BitSetParameter::noParameters() const
{
    return set_.size();
}

std::string BitSetParameter::getName(int idx) const
{
    std::map<std::string, int>::const_iterator i = set_.begin();
    std::advance(i, idx);
    return i->first;
}

std::string BitSetParameter::getName() const
{
    throw std::runtime_error("cannot get the name for parameter '" + name() + "'");
}


const std::type_info& BitSetParameter::type() const
{
    return typeid(int);
}

std::string BitSetParameter::toStringImpl() const
{
    return std::string("[bitset: ") +  "]";
}

boost::any BitSetParameter::get_unsafe() const
{
    return value_;
}


void BitSetParameter::set_unsafe(const boost::any &v)
{
    value_ = boost::any_cast<int>(v);
}


void BitSetParameter::doSetFrom(const Parameter &other)
{
    const BitSetParameter* range = dynamic_cast<const BitSetParameter*>(&other);
    if(range) {
        value_ = range->value_;
        triggerChange();
    } else {
        throw std::runtime_error("bad setFrom, invalid types");
    }
}

void BitSetParameter::doWrite(YAML::Emitter& e) const
{
    e << YAML::Key << "type" << YAML::Value << "bitset";
    e << YAML::Key << "int" << YAML::Value << boost::any_cast<int> (value_);
}

void BitSetParameter::doRead(const YAML::Node& n)
{
    if(!n.FindValue("name")) {
        return;
    }

    n["name"] >> name_;
    if(n.FindValue("int")) {
        n["int"] >> value_;
    }
}
