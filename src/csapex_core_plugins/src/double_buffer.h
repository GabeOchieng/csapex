#ifndef DOUBLE_BUFFER_H
#define DOUBLE_BUFFER_H

/// PROJECT
#include <csapex/model/boxed_object.h>
#include <csapex/model/connection_type.h>

/// SYSTEM
#include <QMutex>

namespace csapex {

class DoubleBuffer : public BoxedObject
{
    Q_OBJECT

public:
    DoubleBuffer();

    virtual void fill(QBoxLayout* layout);
    virtual void messageArrived(ConnectorIn* source);
    virtual void tick();

    virtual Memento::Ptr getState() const;
    virtual void setState(Memento::Ptr memento);

protected:
    void checkIfDone();

private:
    void swapBuffers();

private:
    ConnectorIn* input_;
    ConnectorOut* output_;

    QMutex mutex_;

    bool dirty_;

    struct State : public Memento {
        ConnectionType::Ptr buffer_back_;
        ConnectionType::Ptr buffer_front_;
    };

    State state;
};

}

#endif // DOUBLE_BUFFER_H
