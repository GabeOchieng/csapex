/// HEADER
#include "selector_proxy.h"
#include "box.h"
#include "boxed_object.h"

/// PROJECT
#include <designer/box_manager.h>

using namespace vision_evaluator;

SelectorProxy::SelectorProxy(const std::string& name, BoxedObject* content, QWidget* parent)
    : QGraphicsView(parent), name_(name), box_(new vision_evaluator::Box(content))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSize size(80, 80);

    setScene(new QGraphicsScene(QRectF(QPoint(), size)));
    scene()->addPixmap(box_->makePixmap(name_));
}

SelectorProxy::~SelectorProxy()
{
}

void SelectorProxy::registerProxy(ProxyConstructor c)
{
    BoxManager::instance().register_box_type(c);
    \
}

void SelectorProxy::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        mimeData->setText(Box::MIME);
        mimeData->setParent(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(box_->makePixmap(name_));
        drag->exec();
    }
}

void SelectorProxy::spawnObject(QWidget* parent, const QPoint& pos)
{
    vision_evaluator::Box* object(new vision_evaluator::Box(makeContent(), parent));
    object->setObjectName(name_.c_str());
    object->init(pos);
    object->show();
}
