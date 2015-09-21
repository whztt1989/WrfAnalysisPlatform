#include "arrow.h"
#include <QtGui/QPainter>

Arrow::Arrow(const char* file_name, QPointF pos){
    original_image_ = QImage(file_name);
    QGraphicsWidget::setGeometry(pos.x(), pos.y(), 0, 0);
}

Arrow::~Arrow(){

}

void Arrow::SetExecuted(){
    emit AnimationTriggered();
}

void Arrow::setGeometry(const QRectF &rect){
    QGraphicsWidget::setGeometry(rect);
    image_ = original_image_.scaled(rect.size().toSize());
}

//QRectF Arrow::boundingRect() const{
//    float left, top;
//    left = start_p_.x() < end_p_.x() ? start_p_.x() : end_p_.x();
//    top = start_p_.y() < end_p_.y() ? start_p_.y() : end_p_.y();
//    return QRectF(left, top, abs(start_p_.x() - end_p_.x()), abs(start_p_.y() - end_p_.y()));
//}

//QPainterPath Arrow::shape() const {
//    QPainterPath path;
//
//    return path;
//}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setPen(Qt::black);
    painter->drawImage(QPoint(), image_);
}