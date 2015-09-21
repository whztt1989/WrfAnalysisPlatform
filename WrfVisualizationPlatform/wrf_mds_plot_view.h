#ifndef WRF_MDS_PLOT_VIEW_H_
#define WRF_MDS_PLOT_VIEW_H_

#include "gl/glew.h"
#include <vector>
#include <QtOpenGL/QGLWidget>

class WrfMdsPlotView : public QGLWidget{
    Q_OBJECT

public:
    WrfMdsPlotView();
    ~WrfMdsPlotView();

    void SetData(std::vector< std::vector< float > >& data);
    void SetTrackingPen(bool enabled);
    void GetSelectionIndex(std::vector< bool >& selection_index);
    void SetSelectionIndex(std::vector< bool >& selection_index);

signals:
    void SelectionUpdated();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent* event);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    float view_size_, view_left_, view_top_, view_right_, view_bottom_;
    bool is_tracking_pen_;

    std::vector< std::vector< float > > data_values_;
    std::vector< bool > is_selected_;

    std::vector< float > project_values_;

    std::vector< QPointF > selection_path_;

    void UpdateViewData();
    void UpdateSelection();
    void UpdatePointSelection(QPoint p);
};

#endif