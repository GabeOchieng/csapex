/// HEADER
#include <csapex/serialization/packet_serializer.h>

/// PROJECT
#include <csapex/utility/assert.h>
#include <csapex/serialization/streamable.h>
#include <csapex/serialization/io/std_io.h>

/// SYSTEM
#include <iostream>
#include <sstream>

using namespace csapex;

Serializer::~Serializer()
{
}
SerializationBuffer PacketSerializer::serializePacket(const Streamable& packet)
{
    SerializationBuffer data;
    instance().serialize(packet, data);
    data.finalize();
    return data;
}
SerializationBuffer PacketSerializer::serializePacket(const StreamableConstPtr& packet)
{
    return serializePacket(*packet);
}

StreamablePtr PacketSerializer::deserializePacket(SerializationBuffer& serial)
{
    return instance().deserialize(serial);
}

void PacketSerializer::registerSerializer(uint8_t type, Serializer* serializer)
{
    instance().serializers_[type] = serializer;
}

void PacketSerializer::deregisterSerializer(uint8_t type)
{
    instance().serializers_.erase(type);
}
Serializer* PacketSerializer::getSerializer(uint8_t type)
{
    auto it = instance().serializers_.find(type);
    if (it != instance().serializers_.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void PacketSerializer::serialize(const Streamable& packet, SerializationBuffer& data)
{
    // determine packet type
    uint8_t type = packet.getPacketType();
    auto it = serializers_.find(type);
    if (it != serializers_.end()) {
        data << type;

        // defer serialization to the corresponding serializer
        Serializer* serializer = it->second;
        serializer->serialize(packet, data);
    }
}

StreamablePtr PacketSerializer::deserialize(const SerializationBuffer& data)
{
    // determine packet type
    uint8_t type;
    data >> type;

    auto it = serializers_.find(type);
    if (it != serializers_.end()) {
        // defer deserialization to the corresponding serializer
        Serializer* serializer = it->second;
        return serializer->deserialize(data);

    } else {
        std::cerr << "cannot deserialize packet of type: " << (int)type << std::endl;
    }

    return StreamablePtr();
}
