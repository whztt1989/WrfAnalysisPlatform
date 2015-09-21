#include "wrf_radar_item.h"
#include <QtGui/QPainter>

WrfRadarItem::WrfRadarItem(){
    this->setGeometry(0, 0, 100, 100);
    date_index_ = -1;

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
}

WrfRadarItem::~WrfRadarItem(){

}

QRectF WrfRadarItem::boundingRect() const{
    return QRectF(-1 * item_radius, -1 * item_radius, 2 * item_radius, 2 * item_radius);
}

void WrfRadarItem::SetData(std::vector< QString >& axis_names, int max_rank, QColor color){
    axis_names_ = axis_names;
    max_rank_ = max_rank;
    item_color_ = color;
}

void WrfRadarItem::SetRadius(float radius){
    item_radius = radius;
    QPointF pos = this->pos();
    this->setGeometry(-1 * radius, -1 * radius, 2 * radius, 2 * radius);
    this->setPos(pos);
}

void WrfRadarItem::SetDateIndex(int index){
    date_index_ = index;
}

void WrfRadarItem::UpdateItem(){
    std::vector< QPointF > axis_direction_vec;
    axis_direction_vec.resize(axis_names_.size());

    float rotate_step = 360.0 / axis_names_.size();
    for ( int i = 0; i < axis_names_.size(); ++i ){
        axis_direction_vec[i].setX(cos(i * rotate_step / 180 *3.1415926));
        axis_direction_vec[i].setY(sin(i * rotate_step / 180 *3.1415926));
    }

    // render the view rank
    if ( axis_rank.size() != 0 && axis_rank.size() == axis_names_.size() ){
        local_axis_pos_.resize(axis_names_.size());
        for ( int i = 0; i < axis_names_.size(); ++i ){
            int current_rank = axis_rank[i];
            if ( current_rank > max_rank_ ){
                local_axis_pos_[i] = item_radius * axis_direction_vec[i] * (0.1 + 0.2 * exp(((float)max_rank_ - current_rank) / 100));
            } else {
                local_axis_pos_[i] = item_radius * axis_direction_vec[i] * (0.3 + 0.7 * (1.0 - (float)current_rank / max_rank_));
            }
        }
    }

    this->update();
}

void WrfRadarItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    if ( date_index_ != -1 )
        emit RadarItemSelected(date_index_);
}

void WrfRadarItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if ( axis_names_.size() == 0 ) return;

    QPen axis_pen;
    axis_pen.setColor(Qt::darkGray);
    axis_pen.setWidth(2.0);

    //painter->translate(item_radius, item_radius);
    painter->setPen(axis_pen);
    float rotate_step = 360.0 / axis_names_.size();
    for ( int i = 0; i < axis_names_.size(); ++i ){
        painter->rotate(i * rotate_step);
        painter->drawLine(0, 0, item_radius, 0);
        painter->rotate(-1 * i * rotate_step);
    }

    // render the view rank
    if ( axis_rank.size() != 0 && axis_rank.size() == axis_names_.size() && local_axis_pos_.size() != 0 ){
        QPen rank_pen;
        rank_pen.setColor(item_color_);
        rank_pen.setWidth(2.0);

        painter->setPen(rank_pen);
        QPainterPath path;
        path.moveTo(local_axis_pos_[0]);
        for ( int i = 1; i < local_axis_pos_.size(); ++i ){
            path.lineTo(local_axis_pos_[i]);
        }
        path.lineTo(local_axis_pos_[0]);

        painter->drawPath(path);
    }
}