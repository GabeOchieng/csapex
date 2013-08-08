#ifndef COMMAND_DELETE_CONNECTION_H
#define COMMAND_DELETE_CONNECTION_H

/// COMPONENT
#include "command.h"

namespace csapex
{

class Connector;
class ConnectorIn;
class ConnectorOut;

namespace command
{

struct DeleteConnection : public Command {
    DeleteConnection(Connector* a, Connector* b);

protected:
    bool execute(Graph& graph);
    bool undo(Graph& graph);
    bool redo(Graph& graph);

    bool refresh(Graph &graph);

private:
    ConnectorOut* from;
    ConnectorIn* to;

    std::string from_uuid;
    std::string to_uuid;
};
}
}

#endif // COMMAND_DELETE_CONNECTION_H
