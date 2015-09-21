#include "wrf_radar_coordinate_view.h"
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

WrfRadarCoordinateView::WrfRadarCoordinateView(){
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    this->setContentsMargins(0, 0, 0, 0);

    this->setMinimumSize(500, 500);

    this->viewport()->setContentsMargins(0, 0, 0, 0);

    is_line_on_ = false;
}

WrfRadarCoordinateView::~WrfRadarCoordinateView(){

}

void WrfRadarCoordinateView::SetData(std::vector< QString >& axis_names, int max_rank){
    axis_names_ = axis_names;
    max_rank_ = max_rank;

    axis_length = this->sceneRect().right() - this->sceneRect().left();
    if ( axis_length > this->sceneRect().bottom() - this->sceneRect().top() ) axis_length = this->sceneRect().bottom() - this->sceneRect().top();
    axis_length *= 0.4;

    center_point = QPointF((this->sceneRect().left() + this->sceneRect().right()) / 2, (this->sceneRect().top(), this->sceneRect().bottom()) / 2);
    axis_direction_vec.resize(axis_names_.size());

    float rotate_step = 2 * 3.1415926 / axis_names_.size();
    for ( int i = 0; i < axis_names_.size(); ++i ){
        axis_direction_vec[i].setX(cos(i * rotate_step));
        axis_direction_vec[i].setY(sin(i * rotate_step));
    }

    this->viewport()->update();

    this->update();
}

void WrfRadarCoordinateView::SetViewData(std::vector< std::vector< int > >& record_rank, std::vector< QColor >& record_color){
    record_rank_ = record_rank;
    record_color_ = record_color;

    this->update();
}

void WrfRadarCoordinateView::SetIsLineOn(bool b){
    is_line_on_ = b;

    this->update();
}

void WrfRadarCoordinateView::SetBrushOn(){
    is_brush_on_ = true;

    this->update();
}

void WrfRadarCoordinateView::SetBrushOff(){
    is_brush_on_ = false;

    this->update();
}

void WrfRadarCoordinateView::resizeEvent(QResizeEvent *event){
    this->scene()->setSceneRect(0, 0, this->width() - 20, this->height() - 20);

    axis_length = this->sceneRect().right() - this->sceneRect().left();
    if ( axis_length > this->sceneRect().bottom() - this->sceneRect().top() ) axis_length = this->sceneRect().bottom() - this->sceneRect().top();
    axis_length *= 0.4;

    center_point = QPointF((this->sceneRect().left() + this->sceneRect().right()) / 2, (this->sceneRect().top(), this->sceneRect().bottom()) / 2);
    axis_direction_vec.resize(axis_names_.size());

    float rotate_step = 2 * 3.1415926 / axis_names_.size();
    for ( int i = 0; i < axis_names_.size(); ++i ){
        axis_direction_vec[i].setX(cos(i * rotate_step));
        axis_direction_vec[i].setY(sin(i * rotate_step));
    }

    this->update();
}

void WrfRadarCoordinateView::drawBackground(QPainter *painter, const QRectF &rect){
    // render the star plot
    QPen axis_pen;
    axis_pen.setColor(Qt::darkGray);
    axis_pen.setWidth(2.0);

    QPen circle_pen;
    circle_pen.setColor(Qt::lightGray);
    circle_pen.setWidth(1.0);
    circle_pen.setStyle(Qt::DashDotLine);

    QPen text_pen;
    text_pen.setColor(Qt::black);
    text_pen.setWidth(1.0);

    painter->translate(center_point);

    painter->setPen(circle_pen);
    painter->drawEllipse(QPointF(0, 0), axis_length * 0.1, axis_length * 0.1);
    painter->drawEllipse(QPointF(0, 0), axis_length * 0.3, axis_length * 0.3);
    painter->drawEllipse(QPointF(0, 0), axis_length, axis_length);

    float rotate_step = 360.0 / axis_names_.size();
    for ( int i = 0; i < axis_names_.size(); ++i ){
        painter->rotate(i * rotate_step);

        painter->setPen(axis_pen);
        painter->drawLine(0, 0, axis_length * 1.1, 0);
        painter->drawLine(axis_length * 1.1 - 5, 3, axis_length * 1.1, 0);
        painter->drawLine(axis_length * 1.1 - 5, -3, axis_length * 1.1, 0);
        // render the scale
        painter->drawLine(axis_length * 0.1, 0, axis_length * 0.1, -3);
        painter->drawLine(axis_length * 0.3, 0, axis_length * 0.3, -3);
        painter->drawLine(axis_length, 0, axis_length, -3);

        // render the text
        if ( i == 0 ){
            painter->setPen(text_pen);
            painter->drawText(QRectF(axis_length * 0.1 - 40, 3, 80, 20), Qt::AlignHCenter | Qt::AlignTop, QString("max"));
            painter->drawText(QRectF(axis_length * 0.3 - 40, 3, 80, 20), Qt::AlignHCenter | Qt::AlignTop, QString("%0").arg(max_rank_));
            painter->drawText(QRectF(axis_length - 40, 3, 80, 20), Qt::AlignHCenter | Qt::AlignTop, QString("1"));
        }

        painter->rotate(-1 * i * rotate_step);
    }

    for ( int i = 0; i < axis_names_.size(); ++i ){
        painter->setPen(text_pen);
        painter->drawText(QRectF(axis_direction_vec[i].rx() * (axis_length * 1.1 + 15) - 20, axis_direction_vec[i].ry() * (axis_length * 1.1 + 15) - 20, 40, 40), Qt::AlignCenter, axis_names_[i]);
    }
    

    painter->translate(-1 * center_point);
    
    if ( is_brush_on_ && brush_path.size() != 0 ){
        QPen path_pen;
        path_pen.setColor(Qt::red);
        path_pen.setWidth(2.0);

        QPainterPath path;
        path.moveTo(brush_path[0]);
        for ( int i = 1; i < brush_path.size(); ++i ) path.lineTo(brush_path[i]);
        path.lineTo(brush_path[0]);
        painter->setPen(path_pen);
        painter->drawPath(path);
    }
    // render the view rank

    if ( is_line_on_ ){
        painter->translate(center_point);

        std::vector< QPointF > local_axis_pos;
        local_axis_pos.resize(axis_names_.size());

        for ( int i = 0; i < record_rank_.size(); ++i ){
            QPen rank_pen;
            rank_pen.setColor(record_color_[i]);
            rank_pen.setWidth(2.0);

            painter->setPen(rank_pen);

            for ( int j = 0; j < axis_names_.size(); ++j ){
                int current_rank = record_rank_[i][j];
                if ( current_rank > max_rank_ ){
                    local_axis_pos[j] = axis_length * axis_direction_vec[j] * (0.1 + 0.2 * exp(((float)max_rank_ - current_rank) / 100));
                } else {
                    local_axis_pos[j] = axis_length * axis_direction_vec[j] * (0.3 + 0.7 * (1.0 - (float)current_rank / max_rank_));
                }
            }

            QPainterPath path;
            path.moveTo(local_axis_pos[0]);
            for ( int j = 1; j < local_axis_pos.size(); ++j ){
                path.lineTo(local_axis_pos[j]);
            }
            path.lineTo(local_axis_pos[0]);

            painter->drawPath(path);
        }
        painter->translate(-1 * center_point);
    }
}

void WrfRadarCoordinateView::mousePressEvent(QMouseEvent *event){
    if ( is_brush_on_ && event->buttons() & Qt::LeftButton) {
        brush_path.clear();
        
        brush_path.push_back(this->mapToScene(event->pos()));
    }

    if ( event->button() & Qt::RightButton ){
        brush_path.clear();

        emit SelectionChanged();

        this->viewport()->update();

    }
}

void WrfRadarCoordinateView::mouseMoveEvent(QMouseEvent *event){
    if ( is_brush_on_ && event->buttons() & Qt::LeftButton ) {
        brush_path.push_back(this->mapToScene(event->pos()));

        this->viewport()->update();
    }
}

void WrfRadarCoordinateView::mouseReleaseEvent(QMouseEvent *event){
    if ( is_brush_on_ ) {
        emit SelectionChanged();

        brush_path.clear();

        this->viewport()->update();
    }
}