#ifndef WRF_STAMP_H_
#define WRF_STAMP_H_

#include <QtGui/QGraphicsWidget>
#include "wrf_data_common.h"

class WrfStamp : public QGraphicsWidget{
    Q_OBJECT
public:
    WrfStamp();
    WrfStamp(WrfGridValueMap* forecast_map, WrfGridValueMap* renalysis_map, MapRange& viewing_range, std::vector< QPointF >& brush_path);
    ~WrfStamp();

    void SetValueMap(WrfGridValueMap* forecast_map, WrfGridValueMap* renalysis_map, MapRange& viewing_range);
    void SetData(std::vector< QString >& axis_names, int max_rank, QColor color);
    void SetScatterData(std::vector< int >& selected_index);
    void SetIsScatterOn(bool b);
    void UpdateItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QImage* GetReanalysisImage() { return &renalaysis_image_; }
	QImage* GetFcstImage() { return &forecast_image_; }
    void SetDateString(QString& str) { date_string_ = str; }
	void SetDateTime(int time) { datetime_ = time; }

    std::vector< int > axis_rank;
    float item_radius;

signals:
	void EventTimeTriggered(int);

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    QImage forecast_image_;
    QImage renalaysis_image_;

    WrfGridValueMap* forecast_map_;
    WrfGridValueMap* observed_map_;

    std::vector< QRgb > forecast_pixels_;
    std::vector< QRgb > reanalysis_pixels_;

    std::vector< float > forecast_data_, observed_data_;
    float max_forecast_, max_observe_;
    bool is_scatter_on_;

    QString date_string_;
	int datetime_;

    QColor item_color_;
    int max_rank_;
    std::vector< QString > axis_names_;
    std::vector< QPointF > local_axis_pos_;

    void GenerateImage(WrfGridValueMap* map, MapRange& viewing_range, QImage& image);
};

#endif