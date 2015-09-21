#ifndef FLOW_CHART_H_
#define FLOW_CHART_H_

#include <QtGui/QGraphicsView>
#include <QtCore/QStateMachine>
#include <QtCore/QState>
#include <QtCore/QPropertyAnimation>
#include "step_item.h"

class Arrow;

class FlowChart : public QGraphicsView{
    Q_OBJECT
public:
    FlowChart();
    ~FlowChart();

    void SetExecutedStep(StepItemType type);

signals:
    void ItemSelected(StepItemType type);
    void ItemDoubleClicked(StepItemType type);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    StepItem* data_view_item_;
    StepItem* pre_process_item_;
    StepItem* uncertainty_item_;
    StepItem* rms_item_;
    StepItem* adjustment_item_;
    StepItem* rms_adjustment_item_;
    StepItem* event_result_item_;
    StepItem* rms_result_item_;
    StepItem* comparison_item_;

    Arrow* data_2_pre_arrow;
    Arrow* pre_2_uncertainty_arrow_;
    Arrow* pre_2_rms_arrow_;
    Arrow* ensemble_un_2_calibration_arrow_;
    Arrow* ensemble_calibration_2_result_arrow_;
    Arrow* ensemble_result_2_comparison_arrow_;
    Arrow* rms_2_calibration_arrow_;
    Arrow* rms_calibration_2_result_arrow_;
    Arrow* rms_result_2_comparison_arrow_;

    QStateMachine* machine;
    QState* group;

    void InitItems();

    private slots:
        
};

#endif