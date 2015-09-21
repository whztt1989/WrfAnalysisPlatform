#include "wrf_line_chart.h"
#include "wrf_common.h"
#include "wrf_data_manager.h"
#include <QtGui/QMouseEvent>

WrfLineChart::WrfLineChart(ViewMode mode){
	data_ = NULL;
	compared_data_ = NULL;
	view_mode_ = mode;

	data_type_ = WRF_RAIN;
	data_mode_ = NORMAL_MODE;

	compared_data_order_ = true;

	is_mean_selected_ = false;
	is_radius_selected_ = false;

	is_selection_on_ = false;

	x_border_width = 10;
	y_border_height = 10;
	x_text_width = 40;
	y_text_height = 20;
	identity_height_ = 30;

	this->setFixedWidth(600);
}

WrfLineChart::~WrfLineChart(){

}

void WrfLineChart::set_data(WrfLineChartDataSet* data){
	data_ = data;
	painting_order_vec_.resize(data->values.size());
	for ( int i = 0; i < painting_order_vec_.size(); ++i ) painting_order_vec_[i] = i;

	model_visibility_.resize(data->values.size());
	model_visibility_.assign(model_visibility_.size(), true);

	model_indentity_pos_.resize(data->values.size());
	for ( int i = 0; i < model_indentity_pos_.size(); ++i ) model_indentity_pos_[i].resize(4);

	mean_value_ = 0;
	float max_value = -1e10;
	float min_value = 1e10;
	for ( int i = 0; i < data_->values[data_->values.size() - 1].size(); ++i ){
		int temp_index = data_->values[data_->values.size() - 1][i]->values.size();
		float temp_value = data_->values[data_->values.size() - 1][i]->values[temp_index - 1];
		if ( temp_value > max_value ) max_value = temp_value;
		if ( temp_value < min_value ) min_value = temp_value;
		mean_value_ += temp_value;
	}
	mean_value_ /= data_->values[data_->values.size() - 1].size();
	value_radius_ = abs(max_value - mean_value_);
	if ( abs(min_value - mean_value_) < value_radius_ ) value_radius_ = abs(min_value - mean_value_);
	origin_mean_value_ = mean_value_;
	origin_value_radius_ = value_radius_;

	is_high_light_on_ = false;
	is_record_highlight_.resize(data_->values.size());
	for ( int i = 0; i < is_record_highlight_.size(); ++i ){
		is_record_highlight_[i].resize(data_->values[i].size());
		is_record_highlight_[i].assign(is_record_highlight_[i].size(), true);
	}

	highlight_index_.resize(WrfDataManager::GetInstance()->longitude_grid_number() * WrfDataManager::GetInstance()->latitude_grid_number());
	highlight_index_.assign(highlight_index_.size(), false);

	this->updateGL();
}

void WrfLineChart::set_compared_data(WrfLineChartDataSet* data){
	compared_data_ = data;

	this->updateGL();
}

void WrfLineChart::set_view_mode(ViewMode mode){
	if ( mode != view_mode_ ){
		view_mode_ = mode;

		OnSelectedAreaChanged();
	}
}

void WrfLineChart::set_data_mode(DataMode mode){
	data_mode_ = mode;

	this->updateGL();
}

void WrfLineChart::initializeGL(){
	if (glewInit() != GLEW_OK){
		std::cout << "Central viewer glewInit error!" << std::endl;
		exit(0);
	}

	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfLineChart::resizeGL(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2);
}

void WrfLineChart::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);

	if ( data_ == NULL ) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	PaintIdentities();
	if ( view_mode_ == BIAS_VIEW )
		PaintBiasAxis();
	else 
		PaintAbsoluteAxis();

	if ( compared_data_order_ ){
		if ( data_mode_ == NORMAL_MODE ){
			PaintLines(data_->values, 0.5, 0.5, 0.5, 1.0);
		} else if ( data_mode_ == CLUSTER_MODE ){
			glLineWidth(2.0);
			if ( !is_high_light_on_ )
				PaintLines(cluster_centers_, 0.5, 0.5, 0.5, 1.0);
			glLineWidth(1.0);
		} else if ( data_mode_ == COMPARED_MODE ){
			PaintLines(data_->values,  0.5, 0.5, 0.5, 1.0);
			if ( compared_data_ != NULL )
				PaintLines(compared_data_->values, 0.0, 1.0, 0.0, 1.0);
		}
	} else {
		if ( data_mode_ == CLUSTER_MODE ){
			glLineWidth(2.0);
			if ( !is_high_light_on_ ){
				PaintLines(cluster_centers_, 0.5, 0.5, 0.5, 1.0);
			}
			glLineWidth(1.0);
		} if ( data_mode_ == COMPARED_MODE && compared_data_ != NULL ){
			if ( compared_data_ != NULL )
				PaintLines(compared_data_->values, 0.0, 1.0, 0.0, 1.0);
			PaintLines(data_->values, 0.5, 0.5, 0.5, 1.0);
		} else if ( data_mode_ == NORMAL_MODE )
			PaintLines(data_->values, 0.5, 0.5, 0.5, 1.0);
	}

	if ( data_mode_ != COMPARED_MODE ) PaintAdjustController();
	glDisable(GL_BLEND);
}

void WrfLineChart::PaintIdentities(){
	float y_border_scale = x_border_width / this->height();
	float x_border_scale = y_border_height / this->width();

	float y_text_scale = y_text_height / this->height();
	float x_text_scale = x_text_width / this->width();

	float x_model_bias = (1.0 - 2 * x_border_scale) / data_->values.size();
	for ( int i = 0; i < data_->line_names.size(); ++i ){
		float line_pos_y = 1.0 - 2 * y_border_scale;
		float line_pos_x = x_border_scale + i * x_model_bias;
		float line_pos_x1 = x_border_scale + 20.0 / this->width() + x_model_bias * i;
		float text_pos_x = line_pos_x1 + x_border_scale / 2;
		glColor3f(data_->colors[i * 3], data_->colors[i * 3 + 1], data_->colors[i * 3 + 2]);
		glLineWidth(3.0);
		glBegin(GL_LINES);
		glVertex2f(line_pos_x, line_pos_y);
		glVertex2f(line_pos_x1, line_pos_y);
		glEnd();
		glColor3f(0.0, 0.0, 0.0);
		this->renderText(text_pos_x * this->width(), (1.0 - line_pos_y) * this->height() + 5, QString::fromLocal8Bit(data_->line_names[i].c_str()));

		model_indentity_pos_[i][0] = line_pos_x * this->width();
		model_indentity_pos_[i][1] = 0;
		model_indentity_pos_[i][2] = line_pos_x1 * this->width();
		model_indentity_pos_[i][3] = 2 * (1.0 - line_pos_y) * this->height();
	}
}

void WrfLineChart::PaintAbsoluteAxis(){
	float y_border_scale = x_border_width / this->height();
	float x_border_scale = y_border_height / this->width();

	float y_text_scale = y_text_height / this->height();
	float x_text_scale = x_text_width / this->width();

	// draw axis
	float arrow_bias = 10;
	float header_area_height_scale = identity_height_ / this->height(); 
	float arrow_area_length_scale = 30.0 / this->width();

	float marker_x_step = 1.0 / (data_->time_length - 1) * (1.0 - 2 * x_border_scale - 2 * x_text_scale);

	float temp_value_range = data_->max_value - data_->min_value;
	if ( data_mode_ == COMPARED_MODE && compared_data_ != NULL ){
		if ( compared_data_->max_value - compared_data_->min_value > temp_value_range ) 
			temp_value_range = compared_data_->max_value - compared_data_->min_value;
	}
	float step_range_array[12] = {0.02, 0.05, 0.1, 0.2, 0.5, 1, 5, 10, 15, 20, 50, 100};
	int range_k = 0;
	while ( temp_value_range / step_range_array[range_k] > 10 ) range_k++;

	// get the min max value
	line_min_value_ = data_->min_value - step_range_array[range_k];
	line_max_value_ = data_->max_value + step_range_array[range_k];
	if ( data_mode_ == COMPARED_MODE ){
		if ( compared_data_ != NULL ){
			if ( line_min_value_ > compared_data_->min_value - step_range_array[range_k] ) line_min_value_ = compared_data_->min_value - step_range_array[range_k];
			if ( line_max_value_ < compared_data_->max_value + step_range_array[range_k] ) line_max_value_ = compared_data_->max_value + step_range_array[range_k];
		}
	}

	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(3.0);
	float y_axis_x_pos = x_border_scale + x_text_scale;
	float y_axis_bottom_y = y_border_scale;
	float y_axis_top_y = 1.0 - y_border_scale - y_text_scale - header_area_height_scale;

	axis_bottom_y_value_ = y_axis_bottom_y;
	axis_top_y_value_ = y_axis_top_y;

	glBegin(GL_LINES);
	glVertex2f(y_axis_x_pos, y_axis_bottom_y);
	glVertex2f(y_axis_x_pos, y_axis_top_y);

	glVertex2f(y_axis_x_pos, y_axis_top_y);
	glVertex2f(y_axis_x_pos - arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());

	glVertex2f(y_axis_x_pos, y_axis_top_y);
	glVertex2f(y_axis_x_pos + arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());
	glEnd();

	float x_axis_left_x = x_border_scale + x_text_scale;
	float x_axis_right_x = y_axis_x_pos + marker_x_step * (data_->time_length - 1);
	mean_x_axis_ = x_axis_right_x;
	float x_pos_value = ((int)(data_->min_value / step_range_array[range_k])) * step_range_array[range_k];
	float x_axis_y_pos = ( x_pos_value - line_min_value_) / (line_max_value_ - line_min_value_) * (y_axis_top_y - y_axis_bottom_y) + y_axis_bottom_y;
	glBegin(GL_LINES);
	glVertex2f(x_axis_left_x, x_axis_y_pos);
	glVertex2f(x_axis_right_x, x_axis_y_pos);
	glEnd();

	glBegin(GL_LINES);
	glVertex2f(x_axis_right_x, y_axis_bottom_y);
	glVertex2f(x_axis_right_x, y_axis_top_y);

	glVertex2f(x_axis_right_x, y_axis_top_y);
	glVertex2f(x_axis_right_x - arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());

	glVertex2f(x_axis_right_x, y_axis_top_y);
	glVertex2f(x_axis_right_x + arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());
	glEnd();

	// draw y axis marker
	glLineWidth(1.0);
	float marker_scale = 5.0 / this->width();
	float marker_length = (int)(this->height() * (y_axis_top_y - y_axis_bottom_y) * step_range_array[range_k] / (line_max_value_ - line_min_value_));
	int max_marker_num = (int)(this->height() * (y_axis_top_y - x_axis_y_pos - arrow_area_length_scale) / marker_length);

	float marker_y_step = marker_length / this->height();
	float marker_y_pos = x_axis_y_pos;
	float marker_x_pos = x_border_scale + x_text_scale;
	float marker_value = x_pos_value;
	float right_marker_value = x_pos_value;
	this->renderText(x_border_scale * this->width(), (1.0 - x_axis_y_pos) * this->height(), QString("%0").arg(marker_value));
	this->renderText(x_axis_right_x * this->width() + 5, (1.0 - x_axis_y_pos) * this->height(), QString("%0").arg(right_marker_value));
	for ( int i = 0; i < max_marker_num; ++i ){
		marker_value += step_range_array[range_k];
		right_marker_value += step_range_array[range_k];
		marker_y_pos = (marker_value - line_min_value_) / (line_max_value_ - line_min_value_) * (y_axis_top_y - y_axis_bottom_y) + y_axis_bottom_y;
		this->renderText(x_border_scale * this->width(), (1.0 - marker_y_pos) * this->height(), QString("%0").arg(marker_value));
		this->renderText(x_axis_right_x * this->width() + 5, (1.0 - marker_y_pos) * this->height(), QString("%0").arg(right_marker_value));
		glBegin(GL_LINES);
		glVertex2f(marker_x_pos, marker_y_pos);
		glVertex2f(marker_x_pos + marker_scale, marker_y_pos);

		glVertex2f(x_axis_right_x, marker_y_pos);
		glVertex2f(x_axis_right_x - marker_scale, marker_y_pos);
		glEnd();
	}

	glEnable(GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	glColor4f(0.5, 0.5, 0.5, 1.0);
	marker_x_pos = x_border_scale + x_text_scale;
	marker_y_pos = x_axis_y_pos;
	axis_pos_.resize(data_->time_length - 1);
	axis_pos_[0] = marker_x_pos * this->width();
	for ( int i = 0; i < data_->time_length - 2; ++i ){
		marker_x_pos += marker_x_step;
		axis_pos_[i + 1] = marker_x_pos * this->width();
		glBegin(GL_LINES);
		glVertex2f(marker_x_pos, y_axis_bottom_y);
		glVertex2f(marker_x_pos, y_axis_top_y);
		glEnd();
	}
	glDisable(GL_LINE_STIPPLE);

	int current_time = WrfDataManager::GetInstance()->current_time();
	marker_x_pos = x_border_scale + x_text_scale;
	for ( int i = 0; i < data_->time_length; ++i ){
		this->renderText(marker_x_pos * this->width() - 27,  (1.0 - y_axis_top_y) * this->height() - 3, QString("%0th(Abs)").arg(current_time - data_->time_length + i + 1));
		marker_x_pos += marker_x_step;
	}
}

void WrfLineChart::PaintBiasAxis(){
	float y_border_scale = x_border_width / this->height();
	float x_border_scale = y_border_height / this->width();

	float y_text_scale = y_text_height / this->height();
	float x_text_scale = x_text_width / this->width();

	// draw axis
	float arrow_bias = 10;
	float header_area_height_scale = identity_height_ / this->height(); 
	float arrow_area_length_scale = 30.0 / this->width();

	float marker_x_step = 1.0 / (data_->time_length - 1) * (1.0 - 2 * x_border_scale - 2 * x_text_scale);

	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(3.0);
	float y_axis_x_pos = x_border_scale + x_text_scale;
	float y_axis_bottom_y = y_border_scale;
	float y_axis_top_y = 1.0 - y_border_scale - y_text_scale - header_area_height_scale;

	axis_bottom_y_value_ = y_axis_bottom_y;
	axis_top_y_value_ = y_axis_top_y;

	glBegin(GL_LINES);
	glVertex2f(y_axis_x_pos, y_axis_bottom_y);
	glVertex2f(y_axis_x_pos, y_axis_top_y);

	glVertex2f(y_axis_x_pos, y_axis_top_y);
	glVertex2f(y_axis_x_pos - arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());

	glVertex2f(y_axis_x_pos, y_axis_top_y);
	glVertex2f(y_axis_x_pos + arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());
	glEnd();

	float x_axis_left_x = x_border_scale + x_text_scale;
	float x_axis_right_x = y_axis_x_pos + marker_x_step * (data_->time_length - 1);
	mean_x_axis_ = x_axis_right_x;
	float x_axis_y_pos = 0.5 * (y_axis_bottom_y + y_axis_top_y);
	float temp_x = x_axis_left_x, temp_y;
	float length = marker_x_step * (data_->time_length - 2);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	glBegin(GL_LINES);
	glVertex2f(x_axis_left_x, x_axis_y_pos);
	glVertex2f(x_axis_left_x + length, x_axis_y_pos);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(x_axis_left_x + length, x_axis_y_pos);
	glVertex2f(x_axis_right_x, x_axis_y_pos);
	glEnd();

	glBegin(GL_LINES);
	glVertex2f(x_axis_right_x, y_axis_bottom_y);
	glVertex2f(x_axis_right_x, y_axis_top_y);

	glVertex2f(x_axis_right_x, y_axis_top_y);
	glVertex2f(x_axis_right_x - arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());

	glVertex2f(x_axis_right_x, y_axis_top_y);
	glVertex2f(x_axis_right_x + arrow_bias / this->width(), y_axis_top_y - arrow_bias / this->height());
	glEnd();

	// draw y axis marker
	float step_range_array[10] = {0.02, 0.05, 0.1, 0.2, 0.5, 1, 5, 10, 15, 20};
	glLineWidth(1.0);
	float marker_scale = 5.0 / this->width();
	float marker_length = 20.0;
	float max_bias_value = abs(data_->max_value);
	if ( abs(data_->min_value) > max_bias_value ) max_bias_value = abs(data_->min_value);
	if ( data_mode_ == COMPARED_MODE ){
		if ( compared_data_ != NULL ){
			if (abs(compared_data_->min_value) > max_bias_value) max_bias_value = abs(compared_data_->min_value);
			if (abs(compared_data_->max_value) > max_bias_value) max_bias_value = abs(compared_data_->max_value);
		}
	}
	int max_marker_num = (int)(this->height() * (y_axis_top_y - x_axis_y_pos - arrow_area_length_scale) / marker_length);
	int range_k = 0;
	while ( step_range_array[range_k] * max_marker_num < max_bias_value ) range_k++;
	max_marker_num = max_bias_value / step_range_array[range_k] + 1;
	marker_length = this->height() * (y_axis_top_y - x_axis_y_pos - arrow_area_length_scale) / max_marker_num;

	// Get the values of max and min
	line_max_value_ = step_range_array[range_k] * (y_axis_top_y - y_axis_bottom_y) * 0.5 * this->height() / marker_length;
	line_min_value_ = -1 * line_max_value_;

	float marker_y_pos = x_axis_y_pos;
	float marker_x_pos = x_border_scale + x_text_scale;
	float marker_value = 0.0;
	float right_marker_value = data_->center_absolute_value;
	this->renderText(x_border_scale * this->width(), (1.0 - x_axis_y_pos) * this->height(), QString("%0").arg(marker_value));
	this->renderText(x_axis_right_x * this->width() + 5, (1.0 - x_axis_y_pos) * this->height(), QString("%0").arg(right_marker_value));
	for ( int i = 0; i < max_marker_num; ++i ){
		marker_value += step_range_array[range_k];
		right_marker_value += step_range_array[range_k];
		marker_y_pos = (marker_value - line_min_value_) / (line_max_value_ - line_min_value_) * (y_axis_top_y - y_axis_bottom_y) + y_axis_bottom_y;
		this->renderText(x_border_scale * this->width(), (1.0 - marker_y_pos) * this->height(), QString("%0").arg(marker_value));
		this->renderText(x_axis_right_x * this->width() + 5, (1.0 - marker_y_pos) * this->height(), QString("%0").arg(right_marker_value));
		glBegin(GL_LINES);
		glVertex2f(marker_x_pos, marker_y_pos);
		glVertex2f(marker_x_pos + marker_scale, marker_y_pos);

		glVertex2f(x_axis_right_x, marker_y_pos);
		glVertex2f(x_axis_right_x - marker_scale, marker_y_pos);
		glEnd();
	}
	marker_y_pos = x_axis_y_pos;
	marker_value = 0.0f;
	right_marker_value = data_->center_absolute_value;
	for ( int i = 0; i < max_marker_num; ++i ){
		marker_value -= step_range_array[range_k];
		right_marker_value -= step_range_array[range_k];
		marker_y_pos = (marker_value - line_min_value_) / (line_max_value_ - line_min_value_) * (y_axis_top_y - y_axis_bottom_y) + y_axis_bottom_y;
		this->renderText(x_border_scale * this->width(), (1.0 - marker_y_pos) * this->height(), QString("%0").arg(marker_value));
		this->renderText(x_axis_right_x * this->width() + 5, (1.0 - marker_y_pos) * this->height(), QString("%0").arg(right_marker_value));
		glBegin(GL_LINES);
		glVertex2f(marker_x_pos, marker_y_pos);
		glVertex2f(marker_x_pos + marker_scale, marker_y_pos);

		glVertex2f(x_axis_right_x, marker_y_pos);
		glVertex2f(x_axis_right_x - marker_scale, marker_y_pos);
		glEnd();
	}


	glEnable(GL_LINE_STIPPLE);
	glLineStipple (1, 0x0F0F);
	glColor4f(0.5, 0.5, 0.5, 1.0);
	marker_x_pos = x_border_scale + x_text_scale;
	marker_y_pos = x_axis_y_pos;
	axis_pos_.resize(data_->time_length - 1);
	axis_pos_[0] = marker_x_pos * this->width();
	for ( int i = 0; i < data_->time_length - 2; ++i ){
		marker_x_pos += marker_x_step;
		axis_pos_[i + 1] = marker_x_pos * this->width();
		glBegin(GL_LINES);
		glVertex2f(marker_x_pos, y_axis_bottom_y);
		glVertex2f(marker_x_pos, y_axis_top_y);
		glEnd();
	}
	glDisable(GL_LINE_STIPPLE);


	int current_time = WrfDataManager::GetInstance()->current_time();
	marker_x_pos = x_border_scale + x_text_scale;
	for ( int i = 0; i < data_->time_length - 1; ++i ){
		this->renderText(marker_x_pos * this->width() - 27,  (1.0 - y_axis_top_y) * this->height() - 3, QString("%0th(Dev)").arg(current_time - data_->time_length + i + 1));
		marker_x_pos += marker_x_step;
	}
	this->renderText(marker_x_pos * this->width() - 27,  (1.0 - y_axis_top_y) * this->height() - 3, QString("%0th(Abs)").arg(current_time));
}

void WrfLineChart::PaintLines(std::vector< std::vector< LineChartRecord* > >& data_set, float r, float g, float b, float alpha_scale){
	float y_border_scale = x_border_width / this->height();
	float x_border_scale = y_border_height / this->width();

	float y_text_scale = y_text_height / this->height();
	float x_text_scale = x_text_width / this->width();

	float header_area_height_scale = identity_height_ / this->height(); 
	float arrow_area_length_scale = 30.0 / this->width();

	float marker_x_step = 1.0 / (data_->time_length - 1) * (1.0 - 2 * x_border_scale - 2 * x_text_scale);

	int curve_level = 3;
	// Generate the parameters for b-spline
	std::vector< float > t_values;
	t_values.resize(15, 0);
	for ( int i = 0; i < t_values.size(); ++i ) t_values[i] = (float)i * 5.99999 / (t_values.size() - 1);
	std::vector< float > point_t_values;
	point_t_values.resize(2 * curve_level + 7);
	for ( int i = 0; i < curve_level; ++i ) point_t_values[i] = 0;
	for ( int i = 0; i < 7; ++i ) point_t_values[curve_level + i] = i;
	for ( int i = 0; i < curve_level; ++i ) point_t_values[7 + curve_level + i] = 6;

	std::vector< int > t_range_vec;
	t_range_vec.resize(t_values.size());
	for ( int i = 0; i < t_values.size(); ++i )
		for ( int j = 0; j < point_t_values.size() - 1; ++j )
			if ( t_values[i] >= point_t_values[j] && t_values[i] <= point_t_values[j + 1] ) t_range_vec[i] = j;

	int time_step_num = data_->time_length;		
	float begin_pos = x_border_scale + x_text_scale;
	float end_pos = 1.0 - x_text_scale - x_border_scale;
	float step_size = (end_pos - begin_pos) / (time_step_num - 1);

	std::vector< float > axis_x;

	float x, y1, y2, y3, base, axis_length;
	x = begin_pos;
	base = y_border_scale;
	axis_length = 1.0 - 2 * y_border_scale - y_text_scale - header_area_height_scale;

	for ( int i = 0; i < time_step_num; ++i ){
		axis_x.push_back(x);
		x += step_size;
	}

	// Get the three median value of each segment of the parallel coordinate
	std::vector< std::vector< float > > median_value_set;
	for ( int i = 0; i < data_set.size(); ++i ){
		if ( data_set[i].size() <= 0 ) continue;
		std::vector< float > median_values;
		median_values.resize((data_set[0][0]->values.size() - 1) * 6, 0);
		for ( int j = 0; j < data_set[i].size(); ++j ){
			LineChartRecord* record = data_set[i][j];
			//glColor4f(record.r, record.g, record.b, record.alpha);
			y1 = (record->values[0] - line_min_value_) / (line_max_value_ - line_min_value_);
			y1 = base + y1 * axis_length;
			for ( int k = 1; k < record->values.size(); ++k ){
				y2 = (record->values[k] - line_min_value_) / (line_max_value_ - line_min_value_);
				y2 = base + y2 * axis_length;

				int temp_index = (k - 1) * 6;
				median_values[temp_index] += 0.33 * step_size + axis_x[k - 1];
				median_values[temp_index + 1] += y1 + 0.33 * (y2 - y1);
				median_values[temp_index + 2] += 0.5 * step_size + axis_x[k - 1];
				median_values[temp_index + 3] += y1 + 0.5 * (y2 - y1);
				median_values[temp_index + 4] += 0.67 * step_size + axis_x[k - 1];
				median_values[temp_index + 5] += y1 + 0.67 * (y2 - y1);

				y1 = y2;
			}
		}
		for ( int k = 0; k < median_values.size(); ++k )
			median_values[k] /= data_set[i].size();
		median_value_set.push_back(median_values);
	}

	float alpha1 = 0.5, alpha2 = 0.2;
	float median_y1, middle_y, median_y2;
	std::vector< float > control_point_pos;
	control_point_pos.resize(18, 0);
	for ( int i = 0; i < data_set.size(); ++i ){
		int current_index = painting_order_vec_[i];
		if ( !model_visibility_[current_index] ) continue;
		for ( int j = 0; j < data_set[current_index].size(); ++j ){
			LineChartRecord* record = data_set[current_index][j];

			if ( (data_mode_ == NORMAL_MODE || data_mode_ == CLUSTER_MODE) && is_high_light_on_ ){
				float alpha = 0.05 * record->alpha;
				if ( is_record_highlight_[current_index][j] ) alpha = record->alpha;
				if ( data_mode_ == NORMAL_MODE || data_mode_ == CLUSTER_MODE )
					glColor4f(data_->colors[current_index * 3], data_->colors[current_index * 3 + 1], data_->colors[current_index * 3 + 2], alpha);
				else
					glColor4f(r, g, b, alpha);
			} else {
				if ( data_mode_ == NORMAL_MODE || data_mode_ == CLUSTER_MODE )
					glColor4f(data_->colors[current_index * 3], data_->colors[current_index * 3 + 1], data_->colors[current_index * 3 + 2], record->alpha * alpha_scale);
				else 
					glColor4f(r, g, b, record->alpha * alpha_scale);
			}

			y1 = (record->values[0] - line_min_value_) / (line_max_value_ - line_min_value_);
			y1 = base + y1 * axis_length;
			y2 = (record->values[1] - line_min_value_) / (line_max_value_ - line_min_value_);
			y2 = base + y2 * axis_length;
			y3 = (record->values[2] - line_min_value_) / (line_max_value_ - line_min_value_);
			y3 = base + y3 * axis_length;

			// Add assisting point for each segment
			for ( int k = 0; k < 2; ++k ){
				control_point_pos[2 * k] = axis_x[0];
				control_point_pos[2 * k + 1] = y1;
			}
			control_point_pos[2 + 2] = (median_value_set[current_index][0] - axis_x[0]) / 0.33 * 0.1 + axis_x[0];
			control_point_pos[2 + 3] = (median_value_set[current_index][1] - y1) / 0.33 * 0.1 + y1;
			median_y1 = (y2 - y1) * 0.33 + y1;
			control_point_pos[2 + 4] = median_value_set[current_index][0];
			control_point_pos[2 + 5] = median_value_set[current_index][1] + (median_y1 - median_value_set[current_index][1]) * alpha1;
			middle_y = (y2 - y1) * 0.5 + y1;
			control_point_pos[2 + 6] = median_value_set[current_index][2];
			control_point_pos[2 + 7] = median_value_set[current_index][3] + (middle_y - median_value_set[current_index][3]) * alpha2;
			median_y2 = (y2 - y1) * 0.67 + y1;
			control_point_pos[2 + 8] = median_value_set[current_index][4];
			control_point_pos[2 + 9] = median_value_set[current_index][5] + (median_y2 - median_value_set[current_index][5]) * alpha1;
			control_point_pos[2 + 10] = (median_value_set[current_index][4] - median_value_set[current_index][6]) / 0.66 * 0.1 + axis_x[1];
			control_point_pos[2 + 11] = (median_value_set[current_index][5] - median_value_set[current_index][7]) / 0.66 * 0.1 + y2;
			for ( int k = 0; k < 2; ++k ){
				control_point_pos[2 + 12 + k * 2] = axis_x[1];
				control_point_pos[2 + 13 + k * 2] = y2;
			}

			// Draw segment
			DrawBsplineCurve(control_point_pos, t_values, point_t_values, t_range_vec, curve_level);

			for ( int k = 1; k < record->values.size() - 2; ++k ){
				y1 = y2;

				y2 = (record->values[k + 1] - line_min_value_) / (line_max_value_ - line_min_value_);
				y2 = base + y2 * axis_length;

				int temp_index = k * 6;
				control_point_pos[0] = axis_x[k];
				control_point_pos[1] = y1;
				control_point_pos[2] = axis_x[k];
				control_point_pos[3] = y1;
				control_point_pos[4] = (median_value_set[current_index][temp_index + 0] - median_value_set[current_index][temp_index - 2]) / 0.66 * 0.05 + axis_x[k];
				control_point_pos[5] = (median_value_set[current_index][temp_index + 1] - median_value_set[current_index][temp_index - 1]) / 0.66 * 0.05 + y1;
				median_y1 = (y2 - y1) * 0.33 + y1;
				control_point_pos[6] = median_value_set[current_index][temp_index + 0];
				control_point_pos[7] = median_value_set[current_index][temp_index + 1] + (median_y1 - median_value_set[current_index][temp_index + 1]) * alpha1;
				middle_y = (y2 - y1) * 0.5 + y1;
				control_point_pos[8] = median_value_set[current_index][temp_index + 2];
				control_point_pos[9] = median_value_set[current_index][temp_index + 3] + (middle_y - median_value_set[current_index][temp_index + 3]) * alpha2;
				median_y2 = (y2 - y1) * 0.67 + y1;
				control_point_pos[10] = median_value_set[current_index][temp_index + 4];
				control_point_pos[11] = median_value_set[current_index][temp_index + 5] + (median_y2 - median_value_set[current_index][temp_index + 5]) * alpha1;
				control_point_pos[12] = (median_value_set[current_index][temp_index + 4] - median_value_set[current_index][temp_index + 6]) / 0.66 * 0.05 + axis_x[k + 1];
				control_point_pos[13] = (median_value_set[current_index][temp_index + 5] - median_value_set[current_index][temp_index + 7]) / 0.66 * 0.05 + y2;
				control_point_pos[14] = axis_x[k + 1];
				control_point_pos[15] = y2;
				control_point_pos[16] = axis_x[k + 1];
				control_point_pos[17] = y2;

				DrawBsplineCurve(control_point_pos, t_values, point_t_values, t_range_vec, curve_level);
			}

			y1 = y2;
			y2 = (record->values[record->values.size() - 1] - line_min_value_) / (line_max_value_ - line_min_value_);
			y2 = base + y2 * axis_length;

			int temp_index = (record->values.size() - 2) * 6;
			control_point_pos[0] = axis_x[record->values.size() - 2];
			control_point_pos[1] = y1;
			control_point_pos[2] = axis_x[record->values.size() - 2];
			control_point_pos[3] = y1;
			control_point_pos[4] = (median_value_set[current_index][temp_index + 0] - median_value_set[current_index][temp_index - 2]) / 0.66 * 0.05 + axis_x[record->values.size() - 2];
			control_point_pos[5] = (median_value_set[current_index][temp_index + 1] - median_value_set[current_index][temp_index - 1]) / 0.66 * 0.05 + y1;
			median_y1 = (y2 - y1) * 0.33 + y1;
			control_point_pos[6] = median_value_set[current_index][temp_index + 0];
			control_point_pos[7] = median_value_set[current_index][temp_index + 1] + (median_y1 - median_value_set[current_index][temp_index + 1]) * alpha1;
			middle_y = (y2 - y1) * 0.5 + y1;
			control_point_pos[8] = median_value_set[current_index][temp_index + 2];
			control_point_pos[9] = median_value_set[current_index][temp_index + 3] + (middle_y - median_value_set[current_index][temp_index + 3]) * alpha2;
			median_y2 = (y2 - y1) * 0.67 + y1;
			control_point_pos[10] = median_value_set[current_index][temp_index + 4];
			control_point_pos[11] = median_value_set[current_index][temp_index + 5] + (median_y2 - median_value_set[current_index][temp_index + 5]) * alpha1;
			control_point_pos[12] = (median_value_set[current_index][temp_index + 4] - axis_x[record->values.size() - 1]) / 0.33 * 0.05 + axis_x[record->values.size() - 1];
			control_point_pos[13] = (median_value_set[current_index][temp_index + 5] - y2) / 0.33 * 0.05 + y2;
			control_point_pos[14] = axis_x[record->values.size() - 1];
			control_point_pos[15] = y2;
			control_point_pos[16] = axis_x[record->values.size() - 1];
			control_point_pos[17] = y2;

			DrawBsplineCurve(control_point_pos, t_values, point_t_values, t_range_vec, curve_level);
		}
	}

	float temp_scale = (mean_value_ - line_min_value_) / (line_max_value_ - line_min_value_);
	mean_height_ = base + temp_scale * axis_length;
	radius_length_ = value_radius_ / (line_max_value_ - line_min_value_) * axis_length;
}

void WrfLineChart::DrawBsplineCurve(std::vector< float >& point_pos, std::vector< float >& t_values, std::vector< float >& point_t_values, std::vector< int >& t_range_vec, int curve_level){
	std::vector< float > line_pos;
	line_pos.resize(t_values.size() * 2);
	std::vector< float > P;
	std::vector< float > temp_P;
	P.assign(point_pos.begin(), point_pos.end());
	temp_P.assign(point_pos.begin(), point_pos.end());

	for ( int t = 0; t < t_values.size(); ++t ){
		P.assign(point_pos.begin(), point_pos.end());

		for ( int r = 1; r < curve_level; ++r ){
			temp_P.assign(P.begin(), P.end());
			for ( int i = t_range_vec[t] - curve_level + r + 1; i <= t_range_vec[t]; ++i ){
				float scale = t_values[t] - point_t_values[i];
				scale /= (point_t_values[i + curve_level - r] - point_t_values[i]);
				P[2 * i] = scale * temp_P[2 * i] + (1.0 - scale) * temp_P[2 * (i - 1)];
				P[2 * i + 1] = scale * temp_P[2 * i + 1] + (1.0 - scale) * temp_P[2 * (i - 1) + 1];
			}
		}
		line_pos[t * 2] = P[2 * t_range_vec[t]];
		line_pos[t * 2 + 1] = P[2 * t_range_vec[t] + 1];
	}

	glBegin(GL_LINE_STRIP);
	for ( int i = 0; i < t_values.size(); ++i ) glVertex2f(line_pos[2 * i], line_pos[2 * i + 1]);
	glEnd();
}

void WrfLineChart::OnSelectedAreaChanged(){
	if ( data_mode_ == NORMAL_MODE){
		if ( view_mode_ == BIAS_VIEW )
			this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, false));
		else
			this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, true));
	} else if ( data_mode_ == COMPARED_MODE ){
		if ( view_mode_ == BIAS_VIEW )
			this->set_compared_data(WrfDataManager::GetInstance()->GetComparedLineChartDataSet(data_type_, false));
		else
			this->set_compared_data(WrfDataManager::GetInstance()->GetComparedLineChartDataSet(data_type_, true));
	} else if ( data_mode_ == CLUSTER_MODE ){
		data_mode_ = NORMAL_MODE;
		if ( view_mode_ == BIAS_VIEW )
			this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, false));
		else
			this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, true));
	}
	is_high_light_on_ = false;
}

void WrfLineChart::OnHighLightChanged(){
	is_high_light_on_ = true;

	WrfDataManager::GetInstance()->GetHighLightIndex(highlight_index_);
	for ( int i = 0; i < is_record_highlight_.size(); ++i ) is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);
	for ( int i = 0; i < data_->values.size(); ++i )
		for ( int j = 0; j < data_->values[i].size(); ++j )
			if ( highlight_index_[data_->values[i][j]->grid_index] ) is_record_highlight_[i][j] = true;
	this->updateGL();
}

void WrfLineChart::OnHighLightOff(){
	is_high_light_on_ = false;
	for ( int i = 0; i < is_record_highlight_.size(); ++i ) is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);

	this->updateGL();
}

void WrfLineChart::OnSelectedAttributeChanged(WrfGeneralDataStampType type){
	if ( type != data_type_ ){
		data_type_ = type;
		
		if ( data_mode_ == NORMAL_MODE){
			if ( view_mode_ == BIAS_VIEW )
				this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, false));
			else
				this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, true));
		} else if ( data_mode_ == COMPARED_MODE ){
			if ( view_mode_ == BIAS_VIEW )
				this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, false));
			else
				this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, true));

			if ( view_mode_ == BIAS_VIEW )
				this->set_compared_data(WrfDataManager::GetInstance()->GetComparedLineChartDataSet(data_type_, false));
			else
				this->set_compared_data(WrfDataManager::GetInstance()->GetComparedLineChartDataSet(data_type_, true));
		} else if ( data_mode_ == CLUSTER_MODE ){
			data_mode_ = NORMAL_MODE;
			if ( view_mode_ == BIAS_VIEW )
				this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, false));
			else
				this->set_data(WrfDataManager::GetInstance()->GetLineChartDataSet(data_type_, true));
		}
		is_high_light_on_ = false;
	} 
}

void WrfLineChart::DrawCirclePlane(float radius, float angle, float r, float g, float b, float a){
	glColor4f(r, g, b, a);

	float pixel_radius = radius * this->height();

	float pi = 3.14159;
	float theta = pi / 2.0;
	float theta_step = -1 * pi / 18.0;
	float x = 0, y = pixel_radius / this->height();
	float next_x, next_y;
	glBegin(GL_TRIANGLES);
	int step_num = (int)(angle * 36);
	for ( int i = 0; i < step_num; ++i ){
		theta += theta_step;
		next_x = pixel_radius * cos(theta) / this->width();
		next_y = pixel_radius * sin(theta) / this->height();

		glVertex2f(0, 0);
		glVertex2f(x, y);
		glVertex2f(next_x, next_y);

		x = next_x;
		y = next_y;
	}
	glEnd();
}

void WrfLineChart::mousePressEvent(QMouseEvent *event){
	int x = event->pos().x();
	int y = event->pos().y();
	move_begin_pos_ = event->pos();

	for ( int i = 0; i < model_indentity_pos_.size(); ++i ){
		if ( (x - model_indentity_pos_[i][0]) * (x - model_indentity_pos_[i][2]) < 0 && (y - model_indentity_pos_[i][1]) * (y - model_indentity_pos_[i][3]) < 0 ){
			if ( event->button() == Qt::LeftButton ){
				model_visibility_[i] = !model_visibility_[i];
				this->updateGL();
			} else if (event->button() == Qt::RightButton ){
				for ( int j = 0; j < painting_order_vec_.size(); ++j )
					if ( painting_order_vec_[j] == i ){
						for ( int k = j; k < painting_order_vec_.size() - 1; ++k ) painting_order_vec_[k] = painting_order_vec_[k + 1];
						painting_order_vec_[painting_order_vec_.size() - 1] = i;
						this->updateGL();
					}
			}
			return;
		}
	}

	if ( event->button() == Qt::RightButton ){
		if ( data_mode_ == COMPARED_MODE ){
			compared_data_order_ = !compared_data_order_;
			this->updateGL();
		}
	}

	float x_pos = (float)x / this->width();
	float y_pos = 1.0 - (float)y / this->height();
	float height_scale = 3.0 / this->height();
	float width_scale = 10.0 / this->width();
	float x0 = mean_x_axis_;
	float y0 = mean_height_;
	float y1 = mean_height_ + radius_length_;
	float y2 = mean_height_ - radius_length_;
	if ( (x_pos - x0 + width_scale) * (x_pos - x0 - width_scale) < 0 && (y_pos - y0 - height_scale) * (y_pos - y0 + height_scale) < 0 ){
		is_mean_selected_ = true;
	}
	if ( (x_pos - x0 + width_scale) * (x_pos - x0 - width_scale) < 0 && (y_pos - y1 - height_scale) * (y_pos - y1 + height_scale) < 0 ){
		is_radius_selected_ = true;
	}
	if ( (x_pos - x0 + width_scale) * (x_pos - x0 - width_scale) < 0 && (y_pos - y2 - height_scale) * (y_pos - y2 + height_scale) < 0 ){
		is_radius_selected_ = true;
	}
}

void WrfLineChart::mouseMoveEvent(QMouseEvent *event){
	int x = event->pos().x();
	int y = event->pos().y();
	float x_pos = (float)x / this->width();
	float y_pos = 1.0 - (float)y / this->height();

	float y_border_scale = y_border_height / this->height();
	float y_text_scale = y_text_height / this->height();
	float header_area_height_scale = identity_height_ / this->height(); 

	float base = (float)y_border_height / this->height();
	float axis_length = 1.0 - y_border_scale - y_text_scale - header_area_height_scale;

	if ( is_mean_selected_ ){
		mean_height_ = y_pos;
		mean_value_ = line_min_value_ + (y_pos - base) / axis_length * (line_max_value_ - line_min_value_);

		this->updateGL();
	}

	if ( is_radius_selected_ ){
		radius_length_ = abs(mean_height_ - y_pos);
		value_radius_ = radius_length_ / axis_length * (line_max_value_ - line_min_value_);

		this->updateGL();
	}

	if ( !is_selection_on_ ) {
		int x = event->pos().x();
		for ( int i = 0; i < axis_pos_.size(); ++i )
			if ( abs(x - axis_pos_[i]) < 20 ){
				is_high_light_on_ = true;
				selection_axis_index_ = i;
				is_selection_on_ = true;
				break;
			}
	}
}

void WrfLineChart::mouseReleaseEvent(QMouseEvent *event){
	is_radius_selected_ = false;
	is_mean_selected_ = false;

	if ( is_selection_on_ ){
		is_selection_on_ = false;
		int y_current = event->pos().y();
		int y_pre = move_begin_pos_.y();
		int low_y, high_y;
		if ( y_current < y_pre){
			low_y = y_pre;
			high_y = y_current;
		} else {
			low_y = y_current;
			high_y = y_pre;
		}

		highlight_index_.assign(highlight_index_.size(), false);
		float low_value = line_min_value_ + ((float)(this->height() - low_y) / this->height() - axis_bottom_y_value_) / (axis_top_y_value_ - axis_bottom_y_value_) * (line_max_value_ - line_min_value_);
		float high_value = line_min_value_ + ((float)(this->height() - high_y) / this->height() - axis_bottom_y_value_) / (axis_top_y_value_ - axis_bottom_y_value_) * (line_max_value_ - line_min_value_);
		if ( data_mode_ == NORMAL_MODE || data_mode_ == CLUSTER_MODE ){
			for ( int i = 0; i < data_->values.size(); ++i )
				if ( painting_order_vec_[painting_order_vec_.size() - 1] == i ){
					for ( int j = 0; j < data_->values[i].size(); ++j )
						if ( data_->values[i][j]->values[selection_axis_index_] >= low_value && data_->values[i][j]->values[selection_axis_index_] <= high_value) {
							is_record_highlight_[i][j] = true;
							highlight_index_[data_->values[i][j]->grid_index] = true;
						}
						else
							is_record_highlight_[i][j] = false;
				} else
					for ( int j = 0; j < data_->values[i].size(); ++j ) is_record_highlight_[i][j] = false;
		} else if ( data_mode_ == CLUSTER_MODE ){
			for ( int i = 0; i < cluster_centers_.size(); ++i )
				if ( painting_order_vec_[painting_order_vec_.size() - 1] == i ){
					for ( int j = 0; j < cluster_centers_[i].size(); ++j )
						if ( cluster_centers_[i][j]->values[selection_axis_index_] >= low_value && cluster_centers_[i][j]->values[selection_axis_index_] <= high_value) {
							for ( int k = 0; k < data_->values[i].size(); ++k )
								if ( belonging_[i][k] == cluster_flags_[i][j] ){
									is_record_highlight_[i][k] = true;
									highlight_index_[data_->values[i][k]->grid_index] = true;
								} else {
									is_record_highlight_[i][k] = false;
								}
						}
				} else
					for ( int j = 0; j < data_->values[i].size(); ++j ) is_record_highlight_[i][j] = false;
		}

		WrfDataManager::GetInstance()->SetHighLightIndex(highlight_index_);
		emit HighLightChanged();
		this->updateGL();
	}
}

void WrfLineChart::mouseDoubleClickEvent(QMouseEvent *event){
	if ( is_high_light_on_ ){
		is_high_light_on_ = false;
		highlight_index_.assign(highlight_index_.size(), false);

		for ( int i = 0; i < is_record_highlight_.size(); ++i ) is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);
		WrfDataManager::GetInstance()->SetHighLightIndex(highlight_index_);

		emit HighLightOff();

		this->updateGL();
	}
}

void WrfLineChart::PaintAdjustController(){
	float height_scale = 3.0 / this->height();
	float width_scale = 10.0 / this->width();

	glColor4f(0.0, 0.0, 0.0, 1.0);
	glRectf(mean_x_axis_ - width_scale, mean_height_ - height_scale, mean_x_axis_ + width_scale, mean_height_ + height_scale);
	glRectf(mean_x_axis_ - width_scale, mean_height_ - height_scale + radius_length_, mean_x_axis_ + width_scale, mean_height_ + height_scale + radius_length_);
	glRectf(mean_x_axis_ - width_scale, mean_height_ - height_scale - radius_length_, mean_x_axis_ + width_scale, mean_height_ + height_scale - radius_length_);
}

void WrfLineChart::HierarchicalCluster(std::vector< std::vector< LineChartRecord* > >& records, std::vector< std::vector< LineChartRecord* > >& centers){
	for ( int i = 0; i < centers.size(); ++i )
		for ( int j = 0; j < centers[i].size(); ++j ) delete centers[i][j];
	centers.clear();
	centers.resize(records.size());
	belonging_.clear();
	belonging_.resize(records.size());
	cluster_flags_.clear();
	cluster_flags_.resize(records.size());
	for ( int k = 0; k < records.size(); ++k ){
		std::vector< LineChartRecord* > temp_centers;
		belonging_[k].resize(records[k].size());
		for ( int i = 0; i < belonging_[k].size(); ++i ) belonging_[k][i] = i;
		temp_centers.resize(records[k].size());
		for ( int i = 0; i < temp_centers.size(); ++i ){
			temp_centers[i] = new LineChartRecord;
			temp_centers[i]->alpha = records[k][i]->alpha;
			temp_centers[i]->values.assign(records[k][i]->values.begin(), records[k][i]->values.end());
			temp_centers[i]->grid_index = records[k][i]->grid_index;
		}
		int cluster_count = belonging_[k].size();
		float nearest_dis = 1e10;
		while ( (nearest_dis < 0.2 || cluster_count > 10) && cluster_count >= 3 ){
			nearest_dis = 1e10;
			int nearest1 = -1, nearest2 = -1;
			for ( int i = 0; i < belonging_[k].size() - 1; ++i )
				if ( belonging_[k][i] == i ){
					for ( int j = i + 1; j < belonging_[k].size(); ++j )
						if ( belonging_[k][j] == j ){
							float temp_dis =  Distance(temp_centers[i], temp_centers[j]);
							if ( temp_dis < nearest_dis ){
								nearest_dis = temp_dis;
								nearest1 = i;
								nearest2 = j;
							}
						}
				}
				int count = 0;
				for ( int i = 0; i < data_->time_length; ++i ) temp_centers[nearest1]->values[i] = 0;
				float temp_alpha = 0;
				for ( int i = 0; i < belonging_[k].size(); ++i )
					if ( belonging_[k][i] == nearest1 || belonging_[k][i] == nearest2 ){
						belonging_[k][i] = nearest1;
						for ( int j = 0; j < data_->time_length; ++j ) {
							temp_centers[nearest1]->values[j] += records[k][i]->values[j];
							temp_alpha = records[k][i]->alpha;
						}
						count++;
					}
					for ( int i = 0; i <  data_->time_length; ++i ) temp_centers[nearest1]->values[i] /= count;
					temp_centers[nearest1]->alpha = temp_alpha;
					cluster_count--;
			}
		for ( int i = 0; i < temp_centers.size(); ++i )
			if ( belonging_[k][i] == i ) {
				LineChartRecord* temp_record = new LineChartRecord;
				temp_record->alpha = temp_centers[i]->alpha;
				temp_record->values.assign(temp_centers[i]->values.begin(), temp_centers[i]->values.end());
				temp_record->grid_index = temp_centers[i]->grid_index;
				cluster_centers_[k].push_back(temp_record);
				cluster_flags_[k].push_back(i);
			}
		for ( int i = 0; i < temp_centers.size(); ++i ) delete temp_centers[i];
		temp_centers.clear();
	}
}

float WrfLineChart::Distance(LineChartRecord* record1, LineChartRecord* record2){
	float temp_dis = 0;
	for ( int i = 0; i < record1->values.size(); ++i ) temp_dis += abs(record1->values[i] - record2->values[i]) / (line_max_value_ - line_min_value_);
	temp_dis /= record1->values.size();
	return temp_dis;
}

void WrfLineChart::SetClusterOn(){
	previous_mode_ = data_mode_;

	data_mode_ = CLUSTER_MODE;
	HierarchicalCluster(data_->values, cluster_centers_);

	this->updateGL();
}

void WrfLineChart::SetClusterOff(){
	data_mode_ = previous_mode_;

	this->updateGL();
}

void WrfLineChart::ApplyChanges(){

}