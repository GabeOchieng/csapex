#ifndef COMBINER_GRIDHEATMAP_HIST_H
#define COMBINER_GRIDHEATMAP_HIST_H

#include "combiner_gridcompare_hist.h"

namespace vision_evaluator {
class GridHeatMapHist : public GridCompareHist
{

    Q_OBJECT

public:
    GridHeatMapHist();

    /// TODO : override combine !

    /// MEMENTO
    void         setState(Memento::Ptr memento);
    Memento::Ptr getState() const;

protected Q_SLOTS:
    virtual void updateState(int value);

protected:
    virtual void addSliders(QBoxLayout *layout);

    QSlider *slide_width_add1_;
    QSlider *slide_height_add1_;

    /// fill with standard gui elements
    virtual void fill(QBoxLayout *layout);

    /// MEMENTO
    class State : public GridCompareHist::State {
    public:
        State();

        typedef boost::shared_ptr<State> Ptr;
        virtual void readYaml(const YAML::Node &node);
        virtual void writeYaml(YAML::Emitter &out) const;

    public:
        int grid_width_add1;
        int grid_height_add1;
        int grid_width_max_add1;
        int grid_height_max_add1;
    };

    State *private_state_ghm_;

};
}
#endif // COMBINER_GRIDHEATMAP_HIST_H
