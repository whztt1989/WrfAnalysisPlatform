#include "flow_chart.h"
#include "arrow.h"
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QAbstractTransition>
#include <QtCore/QSignalTransition>
#include <QtCore/QTimer>

FlowChart::FlowChart(){
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
	
	this->setBackgroundBrush(Qt::color0);
	this->autoFillBackground();
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    machine = new QStateMachine;
    machine->setGlobalRestorePolicy(QStateMachine::DontRestoreProperties);

    group = new QState(machine);
    group->setObjectName("group");
    QState *idleState = new QState(group);
    group->setInitialState(idleState);

    InitItems();

    machine->setInitialState(group);
    machine->start();
}

FlowChart::~FlowChart(){

}

void FlowChart::InitItems(){
    data_view_item_ = new StepItem(DATA_VIEW, "./Resources/data_view.jpg", "Data View");
	data_view_item_->setOpacity(0.5);
    pre_process_item_ = new StepItem(PRE_PROCESSING, "./Resources/pre_processing.jpg", "Post-Process");
	pre_process_item_->setOpacity(0.0);
    uncertainty_item_ = new StepItem(UNCERTAINTY, "./Resources/uncertainty_t.png", "Ensemble Forecast");
	uncertainty_item_->setOpacity(0.0);
    rms_item_ = new StepItem(RMS, "./Resources/reforecast_t.png", "Reforecast Rms");
	rms_item_->setOpacity(0.0);
    adjustment_item_ = new StepItem(ADJUSTMENT, "./Resources/adjustment_t.png", "Visual Calibration");;
	adjustment_item_->setOpacity(0.0);
    event_result_item_ = new StepItem(EVENT_RESULT, "./Resources/event_result_t.png", "Event Result");
	event_result_item_->setOpacity(0.0);
    rms_result_item_ = new StepItem(RMS_RESULT, "./Resources/calibrated_result_t.png", "Calibrated Result");
	rms_result_item_->setOpacity(0.0);
    comparison_item_ = new StepItem(COMPARISON, "./Resources/ensemble_icon_t.png", "Comparison");
	comparison_item_->setOpacity(0.0);

    scene()->addItem(data_view_item_);
    data_view_item_->setPos(70, 10);
    scene()->addItem(pre_process_item_);
    pre_process_item_->setPos(70, 130);

    data_2_pre_arrow = new Arrow("./Resources/ad_y_t.png", QPointF(110, 100));
    data_2_pre_arrow->setPos(110, 100);
    scene()->addItem(data_2_pre_arrow);

    QState *state = new QState(group);
    state->assignProperty(data_2_pre_arrow, "geometry", QRect(100, 100, 15, 40));
    QSignalTransition* trans = group->addTransition(data_2_pre_arrow, SIGNAL(AnimationTriggered()), state);
    trans->addAnimation(new QPropertyAnimation(data_2_pre_arrow, "geometry"));

    scene()->addItem(uncertainty_item_);
    uncertainty_item_->setPos(10, 250);
    scene()->addItem(rms_item_);
    rms_item_->setPos(130, 250);

    pre_2_uncertainty_arrow_ = new Arrow("./Resources/left_out_t.png", QPoint(50, 230));
    pre_2_uncertainty_arrow_->setPos(50, 230);
    scene()->addItem(pre_2_uncertainty_arrow_);

    pre_2_rms_arrow_ = new Arrow("./Resources/right_out_t.png", QPoint(110, 230));
    pre_2_rms_arrow_->setPos(170, 230);
    scene()->addItem(pre_2_rms_arrow_);

    state = new QState(group);
    state->assignProperty(pre_2_uncertainty_arrow_, "geometry", QRect(40, 180, 25, 65));
    group->addTransition(pre_2_uncertainty_arrow_, SIGNAL(AnimationTriggered()), state);
    machine->addDefaultAnimation(new QPropertyAnimation(pre_2_uncertainty_arrow_, "geometry"));

    state->assignProperty(pre_2_rms_arrow_, "geometry", QRect(150, 180, 25, 65));
    group->addTransition(pre_2_rms_arrow_, SIGNAL(AnimationTriggered()), state);
    machine->addDefaultAnimation(new QPropertyAnimation(pre_2_rms_arrow_, "geometry"));

    scene()->addItem(adjustment_item_);
    adjustment_item_->setPos(70, 370);

    ensemble_un_2_calibration_arrow_ = new Arrow("./Resources/left_in_t.png", QPoint(50, 350));
    ensemble_un_2_calibration_arrow_->setPos(50, 350);
    scene()->addItem(ensemble_un_2_calibration_arrow_);

	state = new QState(group);
	state->assignProperty(ensemble_un_2_calibration_arrow_, "geometry", QRect(40, 350, 25, 55));
	group->addTransition(ensemble_un_2_calibration_arrow_, SIGNAL(AnimationTriggered()), state);
	machine->addDefaultAnimation(new QPropertyAnimation(ensemble_un_2_calibration_arrow_, "geometry"));

    rms_2_calibration_arrow_ = new Arrow("./Resources/right_in_t.png", QPoint(170, 350));
    rms_2_calibration_arrow_->setPos(170, 350);
    scene()->addItem(rms_2_calibration_arrow_);

    state = new QState(group);
    state->assignProperty(rms_2_calibration_arrow_, "geometry", QRect(150, 350, 25, 55));
    group->addTransition(rms_2_calibration_arrow_, SIGNAL(AnimationTriggered()), state);
    machine->addDefaultAnimation(new QPropertyAnimation(rms_2_calibration_arrow_, "geometry"));

    scene()->addItem(event_result_item_);
    event_result_item_->setPos(10, 490);
    scene()->addItem(rms_result_item_);
    rms_result_item_->setPos(130, 490);

    ensemble_calibration_2_result_arrow_ = new Arrow("./Resources/left_out_t.png", QPoint(50, 470));
    ensemble_calibration_2_result_arrow_->setPos(50, 470);
    scene()->addItem(ensemble_calibration_2_result_arrow_);

    rms_calibration_2_result_arrow_ = new Arrow("./Resources/right_out_t.png", QPoint(110, 470));
    rms_calibration_2_result_arrow_->setPos(110, 470);
    scene()->addItem(rms_calibration_2_result_arrow_);

    state = new QState(group);
	state->assignProperty(ensemble_calibration_2_result_arrow_, "geometry", QRect(40, 420, 25, 65));
	group->addTransition(ensemble_calibration_2_result_arrow_, SIGNAL(AnimationTriggered()), state);
	machine->addDefaultAnimation(new QPropertyAnimation(ensemble_calibration_2_result_arrow_, "geometry"));
    state->assignProperty(rms_calibration_2_result_arrow_, "geometry", QRect(150, 420, 25, 65));
    group->addTransition(rms_calibration_2_result_arrow_, SIGNAL(AnimationTriggered()), state);
    machine->addDefaultAnimation(new QPropertyAnimation(rms_calibration_2_result_arrow_, "geometry"));

    scene()->addItem(comparison_item_);
    comparison_item_->setPos(70, 610);

    ensemble_result_2_comparison_arrow_ = new Arrow("./Resources/left_in_t.png", QPoint(50, 590));
    ensemble_result_2_comparison_arrow_->setPos(50, 590);
    scene()->addItem(ensemble_result_2_comparison_arrow_);

    rms_result_2_comparison_arrow_ = new Arrow("./Resources/right_in_t.png", QPoint(170, 590));
    rms_result_2_comparison_arrow_->setPos(170, 590);
    scene()->addItem(rms_result_2_comparison_arrow_);

    state = new QState(group);
    state->assignProperty(data_2_pre_arrow, "geometry", QRect(100, 100, 20, 25));
    state->assignProperty(pre_2_uncertainty_arrow_, "geometry", QRect(40, 180, 25, 65));
    state->assignProperty(pre_2_rms_arrow_, "geometry", QRect(150, 180, 25, 65));
    state->assignProperty(ensemble_un_2_calibration_arrow_, "geometry", QRect(40, 350, 25, 55));
    state->assignProperty(rms_2_calibration_arrow_, "geometry", QRect(150, 350, 25, 55));
    state->assignProperty(ensemble_calibration_2_result_arrow_, "geometry", QRect(40, 420, 25, 65));
    state->assignProperty(rms_calibration_2_result_arrow_, "geometry", QRect(150, 420, 25, 65));

    state->assignProperty(ensemble_result_2_comparison_arrow_, "geometry", QRect(40, 590, 25, 55));
    group->addTransition(ensemble_result_2_comparison_arrow_, SIGNAL(AnimationTriggered()), state);
    machine->addDefaultAnimation(new QPropertyAnimation(ensemble_result_2_comparison_arrow_, "geometry"));

    state->assignProperty(rms_result_2_comparison_arrow_, "geometry", QRect(150, 590, 25, 55));
    group->addTransition(rms_result_2_comparison_arrow_, SIGNAL(AnimationTriggered()), state);
    machine->addDefaultAnimation(new QPropertyAnimation(rms_result_2_comparison_arrow_, "geometry"));

    connect(data_view_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(pre_process_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(uncertainty_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(rms_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(adjustment_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(event_result_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(rms_result_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));
    connect(comparison_item_, SIGNAL(ItemSelected(StepItemType)), this, SIGNAL(ItemSelected(StepItemType)));

    connect(data_view_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(pre_process_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(uncertainty_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(rms_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(adjustment_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(event_result_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(rms_result_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
    connect(comparison_item_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SIGNAL(ItemDoubleClicked(StepItemType)));
}

void FlowChart::resizeEvent(QResizeEvent *event){
    this->scene()->setSceneRect(0, 0, this->width() - 20, this->height() - 20);
}

void FlowChart::SetExecutedStep(StepItemType type){
    switch ( type ){
    case DATA_VIEW:
        data_view_item_->SetExecuted(true);
		data_view_item_->setOpacity(1.0);
        data_2_pre_arrow->SetExecuted();
		pre_process_item_->setOpacity(0.3);
        break;
    case PRE_PROCESSING:
        pre_process_item_->SetExecuted(true);
		pre_process_item_->setOpacity(1.0);
        pre_2_uncertainty_arrow_->SetExecuted();
		uncertainty_item_->setOpacity(0.3);
		rms_item_->setOpacity(0.3);
        break;
    case UNCERTAINTY:
        uncertainty_item_->SetExecuted(true);
		uncertainty_item_->setOpacity(1.0);
        ensemble_un_2_calibration_arrow_->SetExecuted();
		adjustment_item_->setOpacity(0.3);
        break;
    case RMS:
        pre_2_rms_arrow_->SetExecuted();
        rms_item_->SetExecuted(true);
		rms_item_->setOpacity(1.0);
        rms_2_calibration_arrow_->SetExecuted();
		adjustment_item_->setOpacity(0.3);
        break;
    case ADJUSTMENT:
        adjustment_item_->SetExecuted(true);
		adjustment_item_->setOpacity(1.0);
        rms_calibration_2_result_arrow_->SetExecuted();
		ensemble_calibration_2_result_arrow_->SetExecuted();
		if ( event_result_item_->opacity() < 0.3 ) event_result_item_->setOpacity(0.3);
		if ( rms_result_item_->opacity() < 0.3 ) rms_result_item_->setOpacity(0.3);
        break;
    case EVENT_RESULT:
        event_result_item_->SetExecuted(true);
		event_result_item_->setOpacity(1.0);
        //ensemble_result_2_comparison_arrow_->SetExecuted();
        break;
    case RMS_RESULT:
        rms_result_item_->SetExecuted(true);
		rms_result_item_->setOpacity(1.0);
        //ensemble_result_2_comparison_arrow_->SetExecuted();
        break;
    case COMPARISON:
        comparison_item_->SetExecuted(true);
    default:
        break;
    }
}