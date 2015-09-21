#ifndef WRF_WEIGHT_HISTOGRAM_H_
#define WRF_WEIGHT_HISTOGRAM_H_

#include "gl/glew.h"
#include <QtOpenGL/QGLWidget>

class WrfHistogramDataSet;

class WrfWeightHistogram : public QGLWidget
{
	Q_OBJECT

public :
	WrfWeightHistogram();
	~WrfWeightHistogram();

	void SetData(WrfHistogramDataSet* data);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	//void resizeEvent(QResizeEvent* event);

private:
	WrfHistogramDataSet* data_set_;

	std::vector< std::vector< std::vector< float > > > model_weight_histogram;
	std::vector< std::vector< std::vector< int > > > model_weight_barchart;
	std::vector< std::vector< float > > blend_model_weight_histogram;
	std::vector< float > control_bar_pos_percent;
	std::vector< float > barchart_height;
	std::vector< float > current_control_bar_pos;
	int max_bin_count_;

	//@gcdofree
	float max_weight_data;
	float min_weight_data;
	float segment_num;
	int select_control_bar;
	bool is_dragging_control_bar;

	float top_margin;
	float left_margin;
	float left_margin_timeline;
	float width_range;
	float height_range;
	float value_length;
	float width_padding;
	float left_padding;
	float top_padding;
	float min_value_horizon_offset;
	float min_value_verticle_offset;
	float max_value_horizon_offset;
	float max_value_verticle_offset;
	float timeline_offset;
	float timeline_title_offset;
	float barchart_height_per_num;
	float barchart_width;
	float barchart_left_margin;
	float barchart_left_padding;
	float control_offset;
	float control_bar_height;
	float control_bar_width;
};

#endif