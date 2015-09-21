#ifndef WRF_RADAR_COORDINATE_VIEW_H_
#define WRF_RADAR_COORDINATE_VIEW_H_

#include <QtGui/QGraphicsView>
#include <QtCore/QStateMachine>
#include <QtCore/QState>
#include <QtCore/QFinalState>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSignalTransition>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QTimer>
#include <vector>

class WrfStamp;

class WrfRadarCoordinateView : public QGraphicsView{
    Q_OBJECT
public:
    WrfRadarCoordinateView();
    ~WrfRadarCoordinateView();
    
    void SetViewData(std::vector< std::vector< int > >& record_rank, std::vector< QColor >& record_color);
    void SetData(std::vector< QString >& axis_names, int max_rank);
    void SetIsLineOn(bool b);
    void SetBrushOn();
    void SetBrushOff();

    QPointF center_point;
    std::vector< QPointF > axis_direction_vec;
    float axis_length;

    std::vector< QPointF > brush_path;
    
signals:
    void SelectionChanged();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void resizeEvent(QResizeEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    bool is_line_on_;
    bool is_brush_on_;
    std::vector< QString > axis_names_;
    int max_rank_;

    std::vector< std::vector< int > > record_rank_;
    std::vector< QColor > record_color_;
};

#endif