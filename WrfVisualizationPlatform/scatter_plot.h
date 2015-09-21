#ifndef SCATTER_PLOT_H_
#define SCATTER_PLOT_H_

#include "gl/glew.h"
#include <QtGui/QColor>
#include <QtOpenGL/QGLWidget>
#include <vector>
#include <iostream>
#include <string>

class ScatterPlot : public QGLWidget{
    Q_OBJECT

public:
    ScatterPlot();
    ~ScatterPlot();

    void SetData(std::vector< float >& values);
    void SetAxisNames(std::string& x_name, std::string& y_name);
    void SetAxisValueRange(float x_min, float x_max, float y_min, float y_max);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent* event);

	void mouseMoveEvent(QMouseEvent *event);

private:
    std::vector< float > data_values_;
    std::string axis_names_[2];
    float axis_ranges_[2][2];

    std::vector< float > point_pos_;

    int view_left_, view_top_, view_size_;
    int view_border_, text_size_;

	QPoint mouse_pos_;
	bool is_floating_;

    void UpdateViewData();
};

#endif