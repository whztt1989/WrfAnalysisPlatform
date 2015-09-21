#include "wrf_stamp_viewer.h"
#include "wrf_stamp.h"
#include <iostream>

WrfStampViewer::WrfStampViewer(){
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    this->setContentsMargins(0, 0, 0, 0);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    timer_ = new QTimer;

    next_state_ = NULL;

    machine_ = new QStateMachine;
    machine_->setGlobalRestorePolicy(QStateMachine::DontRestoreProperties);

    current_state_ = new QState(machine_);
    idle_state_ = new QState(current_state_);
    current_state_->setInitialState(idle_state_);

    next_state_ = new QState(current_state_);
    animation_group = new QParallelAnimationGroup;
    QSignalTransition* trans = current_state_->addTransition(timer_, SIGNAL(timeout()), next_state_);
    trans->addAnimation(animation_group);

    machine_->setInitialState(current_state_);
    machine_->start();
}

WrfStampViewer::~WrfStampViewer(){

}

void WrfStampViewer::Clear(){

}

void WrfStampViewer::SetStamps(std::vector< WrfStamp* >& stamps){
    /*is_selected_.resize(viewing_stamps_.size());
    is_selected_.assign(is_selected_.size(), true);

    for ( int i = viewing_stamps_.size(); i < stamps.size(); ++i ){
        scene()->addItem(stamps[i]);
        stamps[i]->setPos(-400, -400);

        idle_state_->assignProperty(stamps[i], "pos", QPoint(-400, -400));
    }*/

    is_selected_.resize(stamps.size());
    is_selected_.assign(is_selected_.size(), false);

    for ( int i = 0; i < stamps.size(); ++i )
        if ( stamps[i] != NULL ) {
            stamps[i]->setPos(-400, -400);

            idle_state_->assignProperty(stamps[i], "pos", QPoint(-400, -400));
        }

    viewing_stamps_ = stamps;
}

void WrfStampViewer::SetSelectedIndex(std::vector< int >& index){
    is_selected_.resize(viewing_stamps_.size());
    is_selected_.assign(is_selected_.size(), false);

    for ( int i = 0; i < index.size(); ++i ) is_selected_[index[i]] = true;

    UpdateStampPos();
}

void WrfStampViewer::UpdateStampPos(){
    int stamp_width = 200;
    for ( int i = 0; i < viewing_stamps_.size(); ++i )
        if ( viewing_stamps_[i] != NULL ){
            stamp_width = viewing_stamps_[i]->rect().width();
            break;
        }
    int stamp_per_row = this->width() / (20 + stamp_width);
    if ( stamp_per_row <= 0 ) stamp_per_row = 1;
    int bias = (this->width() - stamp_per_row * stamp_width) / (stamp_per_row + 1);
    int accu_num = 0;
    animation_group->clear();
    int accu_count = 0;
    for ( int i = 0; i < is_selected_.size(); ++i )
        if ( viewing_stamps_[i] != NULL ) {
            if ( is_selected_[i] ){
                accu_count++;

                int left = bias + accu_num % stamp_per_row * (stamp_width + bias);
                int top = accu_num / stamp_per_row * (viewing_stamps_[i]->rect().height() + 10) + 40;
                next_state_->assignProperty(viewing_stamps_[i], "pos", QPoint(left, top));

                if ( top + 150 > this->scene()->sceneRect().height() ){
                    this->scene()->setSceneRect(0, 0, this->width() - 20, top + 150);
                    this->scene()->update();
                }

                QPropertyAnimation *anim = new QPropertyAnimation(viewing_stamps_[i], "pos");
                anim->setDuration(750 + accu_count * 25);
                anim->setEasingCurve(QEasingCurve::InOutBack);
                animation_group->addAnimation(anim);

                accu_num++;
            } else {
                next_state_->assignProperty(viewing_stamps_[i], "pos", QPoint(-400, -400));
                QPropertyAnimation *anim = new QPropertyAnimation(viewing_stamps_[i], "pos");
                anim->setDuration(750 + accu_count * 25);
                anim->setEasingCurve(QEasingCurve::InOutBack);
                animation_group->addAnimation(anim);
            }
        }

    timer_->setSingleShot(true);
    timer_->start(250);
}

void WrfStampViewer::resizeEvent(QResizeEvent *event){
    this->scene()->setSceneRect(0, 0, this->width() - 20, this->height() - 20);
    //UpdateStampPos();
}

void WrfStampViewer::drawBackground(QPainter* painter, const QRectF& rect){
	QFont title_font;
	title_font.setFamily("arial");
	title_font.setBold(true);
	title_font.setPixelSize(16);

	painter->setRenderHint(QPainter::Antialiasing);
	painter->setPen(Qt::black);

    if ( title_.length() != 0 ){
        painter->setFont(title_font);
        painter->drawText(QRectF(0, 0, this->width(), 40), Qt::AlignCenter, title_);
    }
}