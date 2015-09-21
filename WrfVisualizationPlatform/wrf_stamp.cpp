#include "wrf_stamp.h"
#include <QtGui/QPainter>
#include "wrf_grid_map_element.h"
#include "wrf_statistic_solver.h"

WrfStamp::WrfStamp(){
    forecast_image_ = QImage("./Resources/unknown.png");
    renalaysis_image_ = QImage("./Resources/unknown.png");

	datetime_ = -1;
    this->setGeometry(0, 0, 200, 140);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);

    item_radius = 15;
    is_scatter_on_ = false;
}

WrfStamp::WrfStamp(WrfGridValueMap* forecast_map, WrfGridValueMap* renalysis_map, MapRange& viewing_range, std::vector< QPointF >& brush_path){
    //WrfGridMapELement::GetRenderImage(100, 100, forecast_map, viewing_range, forecast_pixels_);
    //forecast_image_ = QImage((uchar*)forecast_pixels_.data(), 100, 100, 100 * 4, QImage::Format_ARGB32);

    if ( renalysis_map == NULL ){
        renalaysis_image_ = QImage("./Resources/unknown.png");
    } else {
        //WrfGridMapELement::GetRenderImage(100, 100, renalysis_map, viewing_range, reanalysis_pixels_);
        //renalaysis_image_ = QImage((uchar*)reanalysis_pixels_.data(), 100, 100, 100 * 4, QImage::Format_ARGB32);
    }

    this->setGeometry(0, 0, 200, 140);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
}

WrfStamp::~WrfStamp(){

}

void WrfStamp::SetValueMap(WrfGridValueMap* forecast_map, WrfGridValueMap* renalysis_map, MapRange& viewing_range){
    int height = (viewing_range.end_y - viewing_range.start_y) / (viewing_range.end_x - viewing_range.start_x) * 100;
    WrfGridMapELement::GetRenderImage(100, height, forecast_map, viewing_range, forecast_pixels_);
    forecast_image_ = QImage((uchar*)forecast_pixels_.data(), 100, height, 100 * 4, QImage::Format_ARGB32);

    if ( renalysis_map == NULL ){
        renalaysis_image_ = QImage("./Resources/unkown.png");
    } else {
        WrfGridMapELement::GetRenderImage(100, height, renalysis_map, viewing_range, reanalysis_pixels_);
        renalaysis_image_ = QImage((uchar*)reanalysis_pixels_.data(), 100, height, 100 * 4, QImage::Format_ARGB32);
    }

    forecast_map_ = forecast_map;
    observed_map_ = renalysis_map;

    this->setGeometry(0, 0, 200, 40 + 100);

    this->update();
}

void WrfStamp::SetData(std::vector< QString >& axis_names, int max_rank, QColor color){
    axis_names_ = axis_names;
    max_rank_ = max_rank;
    item_color_ = color;
}

void WrfStamp::SetScatterData(std::vector< int >& selected_index){
    WrfGridValueMap* converted_map = WrfStatisticSolver::Convert2Map(observed_map_, forecast_map_->map_range);

    max_forecast_ = -1e10;
    forecast_data_.resize(selected_index.size());
    observed_data_.resize(selected_index.size());
    for ( int i = 0; i < selected_index.size(); ++i ){
        float num_value = forecast_map_->values[selected_index[i]];
        float reana_value = converted_map->values[selected_index[i]];
        if ( num_value > 3000 || reana_value > 3000 || num_value < 0 || reana_value < 0) {
            num_value = 0;
            reana_value = 0;
        }
        forecast_data_[i] = num_value;
        observed_data_[i] = reana_value;

        if ( forecast_data_[i] > max_forecast_ ) max_forecast_ = forecast_data_[i];
        if ( observed_data_[i] > max_forecast_ ) max_forecast_ = observed_data_[i];
    }

    delete converted_map;
}

void WrfStamp::SetIsScatterOn(bool b){
    is_scatter_on_ = b;
    if ( is_scatter_on_ ){
        this->setGeometry(0, 0, 315, 155);
    } else {
        this->setGeometry(0, 0, 200, 40 + 100);
    }

    this->update();
}

void WrfStamp::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
	emit EventTimeTriggered(datetime_);
}

void WrfStamp::UpdateItem(){
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
}

void WrfStamp::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QRectF rect = this->geometry();
    int w = 200;
    int h = 140;

    int date_height = 20;
    int title_height = 20;

    QPen axis_pen;
    axis_pen.setColor(Qt::darkGray);
    axis_pen.setWidth(2.0);

    QPen rank_pen;
    rank_pen.setColor(item_color_);
    rank_pen.setWidth(2.0);

    QFont normal_font;
    normal_font.setFamily("arial");
    normal_font.setBold(false);
    normal_font.setPixelSize(10);
    // render the date 
    painter->setFont(normal_font);
    painter->setPen(Qt::black);
    painter->drawText(QRectF(0, 0, w, date_height), Qt::AlignCenter, date_string_);

    // render the title
    painter->drawText(QRectF(0, date_height, w / 2, title_height), Qt::AlignCenter, QString("Forecast"));
    painter->drawText(QRectF(w / 2, date_height, w / 2, title_height), Qt::AlignCenter, QString("Observed"));

    // render the image
    int temp_height = 50 - forecast_image_.height() / 2;
    painter->drawImage(QRectF(0, date_height + title_height + temp_height, w / 2, forecast_image_.height()), forecast_image_);
    painter->drawImage(QRectF(w / 2, date_height + title_height + temp_height, w / 2, forecast_image_.height()), renalaysis_image_);

    QPen border_pen;
    border_pen.setBrush(Qt::lightGray);
    border_pen.setStyle(Qt::DashDotLine);
    border_pen.setWidth(2.0);
    painter->setPen(border_pen);
    painter->drawRect(QRectF(0, date_height + title_height, w / 2, h - date_height - title_height));
    painter->drawRect(QRectF(w / 2, date_height + title_height, w / 2, h - date_height - title_height));

    if ( axis_names_.size() != 0 ){
        painter->translate(item_radius, item_radius);

        //painter->translate(item_radius, item_radius);
        painter->setPen(axis_pen);
        float rotate_step = 360.0 / axis_names_.size();
        for ( int i = 0; i < axis_names_.size(); ++i ){
            painter->rotate(i * rotate_step);
            painter->drawLine(0, 0, item_radius, 0);
            painter->rotate(-1 * i * rotate_step);
        }

        painter->setPen(rank_pen);
        QPainterPath path;
        path.moveTo(local_axis_pos_[0]);
        for ( int i = 1; i < local_axis_pos_.size(); ++i ){
            path.lineTo(local_axis_pos_[i]);
        }
        path.lineTo(local_axis_pos_[0]);

        painter->drawPath(path);
        painter->translate(-1 * item_radius, -1 * item_radius);
    }

    if ( is_scatter_on_ ){
        painter->translate(200, 40);
        painter->setPen(axis_pen);
        painter->drawLine(15, 0, 15, 100);
        painter->drawLine(15, 0, 12, 5);
        painter->drawLine(15, 0, 18, 5);

        painter->drawLine(15, 100, 115, 100);
        painter->drawLine(115, 100, 110, 103);
        painter->drawLine(115, 100, 110, 97);

        axis_pen.setWidth(1.0);
        axis_pen.setStyle(Qt::DashDotLine);
        painter->setPen(axis_pen);
        painter->drawLine(15, 100, 115, 0);
        painter->translate(-200, -40);

        painter->translate(215, 140);
        painter->setPen(rank_pen);
        for ( int i = 0; i < forecast_data_.size(); ++i ){
            float x = forecast_data_[i] / max_forecast_ * 100;
            float y = observed_data_[i] / max_forecast_ * -100;
            painter->drawPoint(x, y);
        }

        painter->setFont(normal_font);
        painter->setPen(Qt::black);
        painter->rotate(-90);
        painter->drawText(QRectF(0, 0, 100, -15), Qt::AlignCenter, QString("Observed"));
        painter->rotate(90);
        painter->drawText(QRectF(0, 0, 100, 15), Qt::AlignCenter, QString("Forecast"));
        painter->translate(-215, -140);
    }
}

void WrfStamp::GenerateImage(WrfGridValueMap* map, MapRange& viewing_range, QImage& image){
}