#include "step_item.h"
#include <QtGui/QStyleOption>
#include <QtGui/QPainter>

StepItem::StepItem(StepItemType type, QString icon_path, QString step_name)
    : type_(type), icon_path_(icon_path), step_name_(step_name){
    image_ = QImage(icon_path);
    is_executed_ = false;
}

StepItem::~StepItem(){

}

void StepItem::SetExecuted(bool is_executed){
    is_executed_ = is_executed;

    this->update();
}

QRectF StepItem::boundingRect() const{
    return QRectF(0, 0, 80, 100);
}

QPainterPath StepItem::shape() const {
    QPainterPath path;
    path.addRect(0, 0, 80, 100);
    return path;
}

void StepItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	QFont normal_font;
	normal_font.setFamily("arial");
	normal_font.setBold(false);
	normal_font.setPixelSize(13);

    painter->drawImage(QRectF(5, 0, 70, 70), image_);
    painter->setPen(Qt::darkGray);
	painter->setFont(normal_font);
    if ( step_name_.length() > 12 ){
        QStringList strlist = step_name_.split(" ");
        for ( int i = 0; i < strlist.size(); ++i )
            painter->drawText(QRectF(-40, 70 + 15 * i, 160, 10), Qt::AlignCenter, strlist[i]);
    } else 
        painter->drawText(QRectF(-40, 70, 160, 10), Qt::AlignCenter, step_name_);
 
    if ( is_executed_ )
        painter->drawImage(QRectF(0, 0, 30, 30), QImage("./Resources/tick.png"));
}

QVariant StepItem::itemChange(GraphicsItemChange change, const QVariant &value){
    switch (change) {
    case ItemPositionHasChanged:
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void StepItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    emit ItemSelected(type_);

    this->update();

    QGraphicsItem::mousePressEvent(event);
}

void StepItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    this->update();

    QGraphicsItem::mouseReleaseEvent(event);
}

void StepItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    emit ItemDoubleClicked(type_);

    this->update();

    QGraphicsItem::mouseDoubleClickEvent(event);
}