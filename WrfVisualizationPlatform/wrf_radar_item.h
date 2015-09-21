#ifndef WRF_RADAR_ITEM_H_
#define WRF_RADAR_ITEM_H_

#include <QtGui/QGraphicsWidget>
#include "wrf_data_common.h"

class WrfRadarItem : public QGraphicsWidget{
    Q_OBJECT
public:
    WrfRadarItem();
    ~WrfRadarItem();

    void SetData(std::vector< QString >& axis_names, int max_rank, QColor color);
    void SetRadius(float radius);
    void SetDateIndex(int index);
    void UpdateItem();

    std::vector< int > axis_rank;
    std::vector< QPointF > axis_pos;
    float item_radius;

signals:
    void RadarItemSelected(int);

protected:
    QRectF boundingRect() const;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    int date_index_;

    QColor item_color_;
    int max_rank_;
    std::vector< QString > axis_names_;

    std::vector< QPointF > local_axis_pos_;
    
};

#endif