/// HEADER
#include <csapex/view/param/interval_param_adapter.h>

/// PROJECT
#include <csapex/view/utility/qwrapper.h>
#include <csapex/view/node/parameter_context_menu.h>
#include <csapex/view/utility/qt_helper.hpp>
#include <csapex/utility/assert.h>
#include <csapex/utility/type.h>
#include <csapex/command/update_parameter.h>
#include <csapex/view/widgets/doublespanslider.h>

/// SYSTEM
#include <QPointer>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <iostream>
#include <qxt5/qxtspanslider.h>

using namespace csapex;


IntervalParameterAdapter::IntervalParameterAdapter(param::IntervalParameter::Ptr p)
    : ParameterAdapter(std::dynamic_pointer_cast<param::Parameter>(p)), interval_p_(p),
      internal_layout(new QHBoxLayout)
{

}

void IntervalParameterAdapter::setup(QBoxLayout* layout, const std::string& display_name)
{
    QLabel* label = new QLabel(QString::fromStdString(display_name));
    if(context_handler) {
        label->setContextMenuPolicy(Qt::CustomContextMenu);
        context_handler->setParent(label);
        QObject::connect(label, SIGNAL(customContextMenuRequested(QPoint)), context_handler, SLOT(showContextMenu(QPoint)));
    }

    internal_layout->addWidget(label);

    if(interval_p_->is<std::pair<int, int> >()) {
        genericSetup<int, QxtSpanSlider, QWrapper::QSpinBoxExt>();
    } else if(interval_p_->is<std::pair<double, double> >()) {
        genericSetup<double, DoubleSpanSlider, QWrapper::QDoubleSpinBoxExt>();
    } else {
        layout->addWidget(new QLabel((display_name + "'s type is not yet implemented (range: " + type2name(p_->type()) + ")").c_str()));
    }

    for(int i = 0; i < internal_layout->count(); ++i) {
        QWidget* child = internal_layout->itemAt(i)->widget();
        child->setProperty("parameter", QVariant::fromValue(static_cast<void*>(static_cast<csapex::param::Parameter*>(p_.get()))));
    }

    layout->addLayout(internal_layout);
}

namespace {
template <typename Slider>
struct MakeSlider
{
    static Slider* makeSlider(Qt::Orientation orientation, int) {
        return new Slider(orientation);
    }
};

template <>
struct MakeSlider<DoubleSpanSlider>
{
    static DoubleSpanSlider* makeSlider(Qt::Orientation orientation, double step) {
        return new DoubleSpanSlider(orientation, step);
    }
};


template <typename T, typename Slider, typename Spinbox>
struct WidgetConnector
{
    static void connect(IntervalParameterAdapter* parent,
                        QPointer<Slider>& slider,
                        QPointer<Spinbox>& displayLower,
                        QPointer<Spinbox>& displayUpper)
    {
        parent->connect(displayLower.data(), static_cast<void(Spinbox::*)(T)>(&Spinbox::valueChanged),
                        slider.data(), &Slider::setLowerValue);
        parent->connect(displayUpper.data(), static_cast<void(Spinbox::*)(T)>(&Spinbox::valueChanged),
                        slider.data(), &Slider::setUpperValue);

    }
};

template <>
struct WidgetConnector<double, DoubleSpanSlider, QWrapper::QDoubleSpinBoxExt>
{
    static void connect(IntervalParameterAdapter* parent,
                        QPointer<DoubleSpanSlider>& slider,
                        QPointer<QWrapper::QDoubleSpinBoxExt>& displayLower,
                        QPointer<QWrapper::QDoubleSpinBoxExt>& displayUpper)
    {
        parent->connect(displayLower.data(), static_cast<void(QWrapper::QDoubleSpinBoxExt::*)(double)>(&QWrapper::QDoubleSpinBoxExt::valueChanged),
                        slider.data(), &DoubleSpanSlider::setLowerDoubleValue);
        parent->connect(displayUpper.data(), static_cast<void(QWrapper::QDoubleSpinBoxExt::*)(double)>(&QWrapper::QDoubleSpinBoxExt::valueChanged),
                        slider.data(), &DoubleSpanSlider::setUpperDoubleValue);
    }
};

template <typename T, typename Slider>
struct Extractor
{
    static std::pair<T,T> makePair(Slider* slider) {
        return std::make_pair(slider->lowerValue(), slider->upperValue());
    }
};

template <>
struct Extractor<double, DoubleSpanSlider>
{
    static std::pair<double, double> makePair(DoubleSpanSlider* slider) {
        return std::make_pair(slider->lowerDoubleValue(), slider->upperDoubleValue());
    }
};
}

template <typename T, typename Slider, typename Spinbox>
void IntervalParameterAdapter::genericSetup()
{
    const std::pair<T,T>& v = interval_p_->as<std::pair<T,T> >();

    T min = interval_p_->min<T>();
    T max = interval_p_->max<T>();
    T step = interval_p_->step<T>();

    apex_assert_hard(min<=max);
    apex_assert_hard(step > 0);

    QPointer<Slider> slider = MakeSlider<Slider>::makeSlider(Qt::Horizontal, step);
    slider->setRange(min, max);
    slider->setSpan(v.first, v.second);

    QPointer<Spinbox> displayLower = new Spinbox;
    displayLower->setRange(min, max);
    displayLower->setValue(v.first);

    QPointer<Spinbox> displayUpper = new Spinbox;
    displayUpper->setRange(min, max);
    displayUpper->setValue(v.second);


    internal_layout->addWidget(displayLower);
    internal_layout->addWidget(slider);

    internal_layout->addWidget(displayUpper);


    QObject::connect(slider.data(), &Slider::rangeChanged, displayLower.data(), &Spinbox::setRange);
    QObject::connect(slider.data(), &Slider::rangeChanged, displayUpper.data(), &Spinbox::setRange);
    QObject::connect(slider.data(), &Slider::lowerValueChanged, displayLower.data(), &Spinbox::setValue);
    QObject::connect(slider.data(), &Slider::upperValueChanged, displayUpper.data(), &Spinbox::setValue);

    WidgetConnector<T, Slider, Spinbox>::connect(this, slider, displayLower, displayUpper);

    auto cb = [this, slider]() {
        if(!interval_p_ || !slider) {
            return;
        }
        auto value = Extractor<T, Slider>::makePair(slider);
        command::UpdateParameter::Ptr update_parameter = std::make_shared<command::UpdateParameter>(AUUID(interval_p_->getUUID()), value);
        executeCommand(update_parameter);
    };

    connect(slider.data(), &Slider::lowerValueChanged, cb);
    connect(slider.data(), &Slider::upperValueChanged, cb);

    // model change -> ui
    connectInGuiThread(p_->parameter_changed, [this, slider, displayLower, displayUpper](){
        if(!interval_p_ || !slider) {
            return;
        }
        slider->blockSignals(true);
        displayLower->blockSignals(true);
        displayUpper->blockSignals(true);

        slider->setSpan(interval_p_->lower<T>(), interval_p_->upper<T>());

        slider->blockSignals(false);
        displayLower->blockSignals(false);
        displayUpper->blockSignals(false);
    });

    // parameter scope changed -> update slider interval
    connectInGuiThread(p_->scope_changed, [this, slider, displayLower, displayUpper](){
        if(!interval_p_ || !slider) {
            return;
        }
        slider->blockSignals(true);
        displayLower->blockSignals(true);
        displayUpper->blockSignals(true);

        slider->setRange(interval_p_->min<T>(), interval_p_->max<T>());

        slider->blockSignals(false);
        displayLower->blockSignals(false);
        displayUpper->blockSignals(false);
    });
}