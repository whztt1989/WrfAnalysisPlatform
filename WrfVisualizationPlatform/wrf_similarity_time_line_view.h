#ifndef WRF_SIMILARITY_TIME_LINE_VIEW_H_
#define WRF_SIMILARITY_TIME_LINE_VIEW_H_

#include "gl/glew.h"
#include <QtGui/QColor>
#include <QtOpenGL/QGLWidget>
#include <vector>
#include <iostream>
#include <string>
#include "wrf_color_bar_element.h"

class WrfSimilarityTimeLineView : public QGLWidget {
    Q_OBJECT 
public:
    WrfSimilarityTimeLineView();
    ~WrfSimilarityTimeLineView();

    void SetData(std::vector< std::vector< float > >& similarity_values, std::vector< std::vector< int > >& time_values, std::vector< std::string >& years);

signals:
    void EventTimeTriggered(int time);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent* event);

	void mouseMoveEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);

private:
    std::vector< std::vector< float > > data_values_;
	std::vector< std::vector< int > > date_time_;
    std::vector< std::string > year_strings_;

    WrfColorBarElement* bar_element_;

	int current_selected_day_index_;
	int current_selected_year_index_;

	int border;
	int title_height;
	int margin;
	int year_text_height;
	int color_bar_width;
	int date_index_bar_width;
	int y_step, x_step;

    bool is_initialized_;
};

#endif