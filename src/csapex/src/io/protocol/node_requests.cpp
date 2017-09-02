/// HEADER
#include <csapex/io/protcol/node_requests.h>

/// PROJECT
#include <csapex/serialization/request_serializer.h>
#include <csapex/serialization/serialization_buffer.h>
#include <csapex/io/feedback.h>
#include <csapex/utility/uuid_provider.h>
#include <csapex/model/graph_facade_local.h>
#include <csapex/serialization/parameter_serializer.h>
#include <csapex/command/command.h>
#include <csapex/model/graph.h>
#include <csapex/model/node_handle.h>
#include <csapex/io/session.h>
#include <csapex/io/raw_message.h>

/// SYSTEM
#include <iostream>

CSAPEX_REGISTER_REQUEST_SERIALIZER(NodeRequests)

using namespace csapex;

///
/// REQUEST
///
NodeRequests::NodeRequest::NodeRequest(NodeRequestType request_type, const AUUID &uuid)
    : RequestImplementation(0),
      request_type_(request_type),
      uuid_(uuid)
{

}

NodeRequests::NodeRequest::NodeRequest(uint8_t request_id)
    : RequestImplementation(request_id)
{

}

ResponsePtr NodeRequests::NodeRequest::execute(const SessionPtr &session, CsApexCore &core) const
{
    NodeHandle* nh = core.getRoot()->getGraph()->findNodeHandle(uuid_);
    switch(request_type_)
    {
    case NodeRequestType::AddClient:
        nh->remote_data_connection.connect([session](SerializableConstPtr data){
            if(RawMessageConstPtr msg = std::dynamic_pointer_cast<RawMessage const>(data)) {
                session->write(msg);
            } else {
                // TODO: what to do here? we need a uuid
            }
        });
        break;
    case NodeRequestType::RemoveClient:
        // TODO
        break;

    default:
        return std::make_shared<Feedback>(std::string("unknown node request type ") + std::to_string((int)request_type_),
                                          getRequestID());
    }

    return std::make_shared<NodeResponse>(request_type_, getRequestID(), uuid_);
}

void NodeRequests::NodeRequest::serialize(SerializationBuffer &data) const
{
    data << request_type_;
    data << uuid_;
}

void NodeRequests::NodeRequest::deserialize(SerializationBuffer& data)
{
    data >> request_type_;
    data >> uuid_;
}

///
/// RESPONSE
///

NodeRequests::NodeResponse::NodeResponse(NodeRequestType request_type, uint8_t request_id, const AUUID& uuid)
    : ResponseImplementation(request_id),
      request_type_(request_type),
      uuid_(uuid)
{

}

NodeRequests::NodeResponse::NodeResponse(uint8_t request_id)
    : ResponseImplementation(request_id)
{

}

void NodeRequests::NodeResponse::serialize(SerializationBuffer &data) const
{
    data << request_type_;
    data << uuid_;
}

void NodeRequests::NodeResponse::deserialize(SerializationBuffer& data)
{
    data >> request_type_;
    data >> uuid_;
}
