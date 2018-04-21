#ifndef TOKEN_DATA_H
#define TOKEN_DATA_H

/// COMPONENT
#include <csapex_core/csapex_core_export.h>
#include <csapex/serialization/streamable.h>

/// SYSTEM
#include <memory>
#include <string>

namespace csapex {

class CSAPEX_CORE_EXPORT TokenData : public Streamable
{
public:    
    static const uint8_t PACKET_TYPE_ID = 8;

    typedef std::shared_ptr<TokenData> Ptr;
    typedef std::shared_ptr<const TokenData> ConstPtr;

public:
    TokenData(const std::string &type_name);
    TokenData(const std::string &type_name, const std::string& descriptive_name);
    virtual ~TokenData();

    TokenData::Ptr toType() const;

    virtual bool isValid() const;

    virtual bool isContainer() const;
    virtual Ptr nestedType() const;
    virtual ConstPtr nestedValue(std::size_t i) const;
    virtual void addNestedValue(const ConstPtr& msg);
    virtual std::size_t nestedValueCount() const;

    virtual bool canConnectTo(const TokenData* other_side) const;
    virtual bool acceptsConnectionFrom(const TokenData *other_side) const;

    virtual std::string descriptiveName() const;
    std::string typeName() const;

    virtual void writeRaw(const std::string& file,  const std::string &base, const std::string &suffix) const;

    uint8_t getPacketType() const final override;

    void serialize(SerializationBuffer &data) const override;
    void deserialize(const SerializationBuffer& data) override;


protected:
    TokenData();
    void setDescriptiveName(const std::string& descriptiveName);

private:
    std::string type_name_;
    std::string descriptive_name_;
};

}

#endif // TOKEN_H
