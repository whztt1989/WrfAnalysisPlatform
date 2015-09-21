#include "wrf_parallel_coordinate.h"
#include <iostream>
#include <QtGui/QMouseEvent>
#include "wrf_common.h"
#include "wrf_data_manager.h"

WrfParallelCoordinate::WrfParallelCoordinate(){
	system_status_ = SYSTEM_OK;
	is_data_updated_ = false;
	view_mode_ = BIAS_VIEW;
	data_mode_ = NOMRAL;

	data_set_ = NULL;
	compared_data_set_ = NULL;
	is_high_light_on_ = false;
	is_selection_on_ = false;
	compared_painting_order_ = true;
	is_weight_on_ = true;

	this->setFixedHeight(300);
	this->setMinimumWidth(200);

	info_gain_vec_.resize(10);
	for ( int i = 0; i < 10; ++i ) info_gain_vec_[i].resize(6, 0);
	for ( int i = 0; i < 6; ++i ) info_gain_vec_[0][i] = i / 6.0;
}

WrfParallelCoordinate::~WrfParallelCoordinate(){

}

void WrfParallelCoordinate::set_view_mode(ParallelViewMode mode){
	if ( view_mode_ != mode ){
		view_mode_ = mode;
	}
}

void WrfParallelCoordinate::set_data_mode(ParallelDataMode mode){
	data_mode_ = mode;

	if ( data_mode_ == NOMRAL ){
		min_value_vec_.resize(data_set_->min_value_vec.size());
		min_value_vec_.assign(data_set_->min_value_vec.begin(), data_set_->min_value_vec.end());
		max_value_vec_.resize(data_set_->max_value_vec.size());
		max_value_vec_.assign(data_set_->max_value_vec.begin(), data_set_->max_value_vec.end());
	} else {
		if ( compared_data_set_ != NULL ){
			for ( int i = 0; i < compared_data_set_->min_value_vec.size(); ++i ){
				if ( compared_data_set_->min_value_vec[i] < min_value_vec_[i] ) min_value_vec_[i] = compared_data_set_->min_value_vec[i];
				if ( compared_data_set_->max_value_vec[i] > max_value_vec_[i] ) max_value_vec_[i] = compared_data_set_->max_value_vec[i];
			}
		}
	}

	this->updateGL();
}

void WrfParallelCoordinate::set_attrib_weight(std::vector< float >& weight){
	attrib_weight_.resize(weight.size());
	attrib_weight_.assign(weight.begin(), weight.end());
}

void WrfParallelCoordinate::UpdateView(){
	this->OnSelectedAreaChanged();
}

void WrfParallelCoordinate::initializeGL(){
	if ( glewInit() != GLEW_OK ){
		std::cout << "Error initialize PCP error!" << std::endl;
		system_status_ |= GL_INIT_ERROR;
		return;
	}

	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfParallelCoordinate::resizeGL(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -1.0);

	y_border_width_scale_ = 10.0 / this->height();
	x_border_width_scale_ = 40.0 / this->width();
	attrib_text_height_scale_ = 20.0 / this->height();
	value_text_height_scale_ = 15.0 / this->height();
	axis_width_scale_ = 5.0 / this->width();
	model_identity_scale_ = 100.0 / this->width();
	attri_weight_circle_scale_ = 40.0 / this->height();
}

void WrfParallelCoordinate::paintGL(){
	makeCurrent();

	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);

	glViewport(0, 0, this->width(), this->height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -1.0);

	if ( system_status_ != SYSTEM_OK ) return;

	if ( data_set_ != NULL ){
		PaintCoordinate();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		PaintIdentifyItems();

		if ( compared_painting_order_ ){
			if ( data_mode_ == NOMRAL ){
				DrawLineRecords(data_set_->values, is_current_model_shown_, 0.5, 0.5, 0.5, 1.0);
			} else if ( data_mode_ == CLUSTERED ){
				glLineWidth(2.0);
				if ( !is_high_light_on_ )
					DrawLineRecords(cluster_centers_, is_current_model_shown_, 0.5, 0.5, 0.5, 1.0);
				glLineWidth(1.0);
			} else if ( data_mode_ == COMPARED ){
				DrawLineRecords(data_set_->values, is_current_model_shown_, 0.5, 0.5, 0.5, 1.0);
				if ( compared_data_set_ != NULL )
					DrawLineRecords(compared_data_set_->values, is_current_model_shown_, 0.0, 1.0, 0.0, 1.0);
			}
		} else {
			if ( data_mode_ == CLUSTERED ){
				glLineWidth(2.0);
				if ( !is_high_light_on_ )
					DrawLineRecords(cluster_centers_, is_current_model_shown_, 0.5, 0.5, 0.5, 1.0);
				glLineWidth(1.0);
			} if ( data_mode_ == COMPARED && compared_data_set_ != NULL ){
				if ( compared_data_set_ != NULL )
					DrawLineRecords(compared_data_set_->values, is_current_model_shown_, 0.0, 1.0, 0.0, 1.0);
				DrawLineRecords(data_set_->values, is_current_model_shown_, 0.5, 0.5, 0.5, 1.0);
			} else if ( data_mode_ == NOMRAL )
				DrawLineRecords(data_set_->values, is_current_model_shown_, 0.5, 0.5, 0.5, 1.0);
		}

		if ( view_mode_ == BIAS_VIEW ) PaintCenterLine();

		glDisable(GL_BLEND);

		GLenum err = glGetError();
	}
}

void WrfParallelCoordinate::PaintCenterLine(){
	float begin_pos = x_border_width_scale_ + axis_width_scale_ / 2;
	float end_pos = 1.0 - 2 * x_border_width_scale_ - axis_width_scale_ / 2 - model_identity_scale_;

	float x1, y1, x2, y2;
	y1 = 2 * y_border_width_scale_ + value_text_height_scale_ + attri_weight_circle_scale_;
	y2 = 1.0 - y_border_width_scale_ - attrib_text_height_scale_ - value_text_height_scale_;

	glColor4f(1.0, 0.0, 0.0, 1.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	glVertex2f(begin_pos, (y1 + y2) / 2);
	glVertex2f(end_pos, (y1 + y2) / 2);
	glEnd();
}

void WrfParallelCoordinate::PaintCoordinate(){
	int attrib_num = data_set_->attrib_name_vec.size();
	float begin_pos = x_border_width_scale_ + axis_width_scale_ / 2;
	float end_pos = 1.0 - 2 * x_border_width_scale_ - axis_width_scale_ / 2 - model_identity_scale_;
	float step_size = (end_pos - begin_pos) / (attrib_num - 1);
	float axis_size = 3.0 / this->width();

	glColor3f(0.0, 0.0, 0.0);

	float x1, y1, x2, y2;
	int y_axis_name, y_max, y_min;
	x1 = begin_pos - axis_size / 2;
	y1 = 2 * y_border_width_scale_ + value_text_height_scale_ + attri_weight_circle_scale_;
	axis_bottom_y_value_ = y1;
	x2 = begin_pos + axis_size / 2;
	y2 = 1.0 - y_border_width_scale_ - attrib_text_height_scale_ - value_text_height_scale_;
	axis_top_y_value_ = y2;

	y_axis_name = (int)((y_border_width_scale_ + attrib_text_height_scale_) * this->height()) - 3;
	y_max = (int)((y_border_width_scale_ + attrib_text_height_scale_ + value_text_height_scale_) * this->height()) - 3;
	y_min = (int)((1.0 - y_border_width_scale_ * 2 - attri_weight_circle_scale_) * this->height());
	
	adjust_plane_pos_.resize(attrib_num * 2);
	axis_pos_.resize(attrib_num);
	for ( int i = 0; i < attrib_num; ++i ){
		glColor3f(0.0, 0.0, 0.0);

		glRectf(x1, y1, x2, y2);

		/// TODO: Draw the axis attribute name and value range
		int axis_x = (int)((x1 + x2) / 2 * this->width());
		axis_pos_[i] = axis_x;
		QString axis_name = QString::fromLocal8Bit(data_set_->attrib_name_vec.at(i).c_str());
		QString min_value = QString("%0").arg(min_value_vec_.at(i));
		QString max_value = QString("%0").arg(max_value_vec_.at(i));
		this->renderText(axis_x - axis_name.length() * 3, y_axis_name, axis_name);
		this->renderText(axis_x - max_value.length() * 3, y_max, max_value);
		this->renderText(axis_x - min_value.length() * 3, y_min, min_value);

		glTranslatef(x1 + axis_size / 2, y_border_width_scale_ + attri_weight_circle_scale_ / 2, 0);
		if ( info_gain_vec_.size() != 0 )
			DrawTheropyPlane(attri_weight_circle_scale_ / 2  * 0.8, attri_weight_circle_scale_ / 2, i);
		DrawCirclePlane(attri_weight_circle_scale_ / 2 * 0.7, 1.0, 0.8, 0.8, 0.8, 1.0);
		if ( is_weight_on_ )
			DrawCirclePlane(attri_weight_circle_scale_ / 2 * 0.7, attrib_weight_[i], 0.0, 1.0, 0.0, 0.5);
		glTranslatef(-1 * (x1 + axis_size / 2), -1 * (y_border_width_scale_ + attri_weight_circle_scale_ / 2), 0);

		adjust_plane_pos_[i * 2] = (x1 + axis_size / 2) * this->width();
		adjust_plane_pos_[i * 2 + 1] = (1.0 - y_border_width_scale_ + attri_weight_circle_scale_ / 2) * this->height();

		x1 += step_size;
		x2 += step_size;
	}
}

void WrfParallelCoordinate::PaintLines(){
	int attrib_num = data_set_->attrib_name_vec.size();
	float begin_pos = x_border_width_scale_ + axis_width_scale_ / 2;
	float end_pos = 1.0 - model_identity_scale_ - axis_width_scale_ / 2 - 2 * x_border_width_scale_;
	float step_size = (end_pos - begin_pos) / (attrib_num - 1);

	std::vector< float > axis_x;

	float x, y1, y2, base, axis_length;
	x = begin_pos;
	base = y_border_width_scale_ * 2 + attri_weight_circle_scale_ + value_text_height_scale_;
	axis_length = 1.0 - 3 * y_border_width_scale_ - attrib_text_height_scale_ - 2 * value_text_height_scale_ - attri_weight_circle_scale_;

	for ( int i = 0; i < attrib_num; ++i ){
		axis_x.push_back(x);
		x += step_size;
	}

	for ( int i = 0; i < data_set_->values.size(); ++i ){
		for ( int j = 0; j < data_set_->values[i].size(); ++j ){
			WrfParallelRecord record = data_set_->values[i][j];
			//glColor4f(record.r, record.g, record.b, record.alpha);
			y1 = (record.data[0] - min_value_vec_[0]) / (max_value_vec_[0] - min_value_vec_[0]);
			y1 = base + y1 * axis_length;
			for ( int k = 1; k < data_set_->attrib_name_vec.size(); ++k ){
				y2 = (record.data[k] - min_value_vec_[k]) / (max_value_vec_[k] - min_value_vec_[k]);
				y2 = base + y2 * axis_length;
				glBegin(GL_LINES);
				glVertex3f(axis_x[k - 1], y1, 0);
				glVertex3f(axis_x[k], y2, 0);
				glEnd();
				y1 = y2;
			}	
		}
	}
}

void WrfParallelCoordinate::PaintIdentifyItems(){
	float begin_pos = 1.0 - x_border_width_scale_ - model_identity_scale_;
	float begin_y = y_border_width_scale_ + attrib_text_height_scale_ + value_text_height_scale_;
	float step_y = (1.0 - 2 * y_border_width_scale_ - attrib_text_height_scale_ - 2 * value_text_height_scale_) / data_set_->model_vec.size();
	float size_y = step_y * 0.66 / 2;
	float block_margin = 3.0 / this->height();
	int x = (int)(begin_pos * this->width());
	int text_bias_y = 15.0;
	int text_bias_x = 0.5 * model_identity_scale_ * this->width() + 10;

	for ( int i = 0; i < data_set_->model_vec.size(); ++i ){
		WrfParallelModel model = data_set_->model_vec.at(i);
		glColor4f(model.r, model.g, model.b, 1.0);
		glRectf(begin_pos, 1.0 - begin_y, begin_pos + 0.5 * model_identity_scale_, 1.0 - begin_y - size_y);
		current_identity_pos_[i][0] = begin_pos * this->width();
		current_identity_pos_[i][1] = begin_y * this->height();
		current_identity_pos_[i][2] = (begin_pos + 0.5 * model_identity_scale_) * this->width();
		current_identity_pos_[i][3] = (begin_y + size_y) * this->height();

		glColor3f(0.0, 0.0, 0.0);
		int y = (int)((begin_y) * this->height());
		this->renderText(x + text_bias_x, y + text_bias_y, QString::fromLocal8Bit(model.model_name.c_str()));

		begin_y += step_y;
	}
}

void WrfParallelCoordinate::set_data_set(WrfParallelDataSet* data_set_t){
	data_set_ = data_set_t;

	current_identity_pos_.resize(data_set_->model_vec.size());
	for ( int i = 0; i < current_identity_pos_.size(); ++i ) current_identity_pos_[i].resize(4);

	historical_identity_pos_.resize(data_set_->model_vec.size());
	for ( int i = 0; i < historical_identity_pos_.size(); ++i ) historical_identity_pos_[i].resize(4);

	is_current_model_shown_.resize(data_set_->model_vec.size());
	is_current_model_shown_.assign(is_current_model_shown_.size(), true);

	model_painting_order_.resize(data_set_->model_vec.size());
	for ( int i = 0; i < model_painting_order_.size(); ++i ) model_painting_order_[i] = i;

	is_record_highlight_.resize(data_set_->model_vec.size());
	for ( int i = 0; i < data_set_->model_vec.size(); ++i ){
		is_record_highlight_[i].resize(data_set_->values[i].size());
		is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);
	}

	min_value_vec_.resize(data_set_->min_value_vec.size());
	min_value_vec_.assign(data_set_->min_value_vec.begin(), data_set_->min_value_vec.end());
	max_value_vec_.resize(data_set_->max_value_vec.size());
	max_value_vec_.assign(data_set_->max_value_vec.begin(), data_set_->max_value_vec.end());

	if ( attrib_weight_.size() == 0 ) WrfDataManager::GetInstance()->GetAttribWeight(attrib_weight_);
	highlight_index_.resize(WrfDataManager::GetInstance()->longitude_grid_number() * WrfDataManager::GetInstance()->latitude_grid_number());
	highlight_index_.assign(highlight_index_.size(), false);
	//WrfDataManager::GetInstance()->GetAttribWeightTheropy(6, info_gain_vec_);

	is_data_updated_ = true;

	this->updateGL();
}

void WrfParallelCoordinate::set_compared_data_set(WrfParallelDataSet* data_set_t){
	compared_data_set_ = data_set_t;

	min_value_vec_.resize(data_set_->min_value_vec.size());
	min_value_vec_.assign(data_set_->min_value_vec.begin(), data_set_->min_value_vec.end());
	max_value_vec_.resize(data_set_->max_value_vec.size());
	max_value_vec_.assign(data_set_->max_value_vec.begin(), data_set_->max_value_vec.end());

	for ( int i = 0; i < compared_data_set_->min_value_vec.size(); ++i ){
		if ( compared_data_set_->min_value_vec[i] < min_value_vec_[i] ) min_value_vec_[i] = compared_data_set_->min_value_vec[i];
		if ( compared_data_set_->max_value_vec[i] > max_value_vec_[i] ) max_value_vec_[i] = compared_data_set_->max_value_vec[i];
	}

	this->updateGL();
}

void WrfParallelCoordinate::set_highlight_index(const std::vector< bool >& highlight_index_t){
	highlight_index_.resize(highlight_index_t.size());
	highlight_index_.assign(highlight_index_t.begin(), highlight_index_t.end());
}

void WrfParallelCoordinate::DrawCirclePlane(float radius, float angle, float r, float g, float b, float a){
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

void WrfParallelCoordinate::DrawTheropyPlane(float inner_radius, float outer_radius, int index){
	outer_radius *= this->height();
	inner_radius *= this->height();

	float pi = 3.14159;
	float theta = pi / 2.0;
	float theta_step = -1 * pi / 18.0;
	float x = 0, y = outer_radius / this->height();
	float next_x, next_y;
	glBegin(GL_TRIANGLES);
	for ( int i = 0; i < 36; ++i ){
		int temp_index = (int)((float)i / 36 * info_gain_vec_[0].size());
		float alpha = info_gain_vec_[index][temp_index];
		glColor4f(alpha, alpha, alpha, 1.0);

		theta += theta_step;
		next_x = outer_radius * cos(theta) / this->width();
		next_y = outer_radius * sin(theta) / this->height();

		glVertex2f(0, 0);
		glVertex2f(x, y);
		glVertex2f(next_x, next_y);

		x = next_x;
		y = next_y;
	}
	glEnd();


	theta = pi / 2.0;
	x = 0;
	y = inner_radius / this->height();
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLES);
	for ( int i = 0; i < 36; ++i ){
		theta += theta_step;
		next_x = inner_radius * cos(theta) / this->width();
		next_y = inner_radius * sin(theta) / this->height();

		glVertex2f(0, 0);
		glVertex2f(x, y);
		glVertex2f(next_x, next_y);

		x = next_x;
		y = next_y;
	}
	glEnd();
}

void WrfParallelCoordinate::OnSelectedAreaChanged(){
	if ( data_mode_ == NOMRAL ){
		if ( view_mode_ == BIAS_VIEW )
			this->set_data_set(WrfDataManager::GetInstance()->GetParallelDataSet(false));
		else
			this->set_data_set(WrfDataManager::GetInstance()->GetParallelDataSet(true));
	} else if ( data_mode_ == COMPARED ){
		if ( view_mode_ == BIAS_VIEW )
			this->set_compared_data_set(WrfDataManager::GetInstance()->GetComparedParallelDataSet(false));
		else
			this->set_compared_data_set(WrfDataManager::GetInstance()->GetComparedParallelDataSet(true));
	} else if ( data_mode_ == CLUSTERED ){
		data_mode_ = NOMRAL;
		if ( view_mode_ == BIAS_VIEW )
			this->set_data_set(WrfDataManager::GetInstance()->GetParallelDataSet(false));
		else
			this->set_data_set(WrfDataManager::GetInstance()->GetParallelDataSet(true));
	}
	if ( is_high_light_on_ ){
		is_high_light_on_ = false;
		emit HighLightOff();
	}
}

void WrfParallelCoordinate::mousePressEvent(QMouseEvent* event){
	int x = event->pos().x();
	int y = event->pos().y();
	move_begin_pos_ = event->pos();

	if ( event->button() == Qt::LeftButton ){
		for ( int i = 0; i < adjust_plane_pos_.size() / 2; ++i ){
			float radius = sqrt(pow(adjust_plane_pos_[i * 2] - x, 2.0) + pow(adjust_plane_pos_[2 * i + 1] - y, 2.0));
			if ( radius < 80 ) {
				float scale = 1.0;
				if ( x - adjust_plane_pos_[i * 2] > 0 ) scale *= -1;
				float current_weight = attrib_weight_[i] + 0.2 * scale;
				if ( current_weight < 0 ) current_weight = 0;
				if ( current_weight > 1 ) current_weight = 1;

				float sum_all = 0;
				for ( int k = 0; k < attrib_weight_.size(); ++k ) if ( k != i ) sum_all += attrib_weight_[k];
				if ( sum_all == 0 ) break;

				float bias = attrib_weight_[i] - current_weight;
				attrib_weight_[i] = current_weight;
				for ( int k = 0; k < attrib_weight_.size(); ++k ) if ( k != i ) attrib_weight_[k] += bias * attrib_weight_[k] / sum_all;

				WrfDataManager::GetInstance()->SetAttribWeight(attrib_weight_);

				emit AttribWeightChanged();

				break;
			}
		}

		for ( int i = 0; i < current_identity_pos_.size(); ++i ){
			if ( (x - current_identity_pos_[i][0]) * (x - current_identity_pos_[i][2]) < 0 && (y - current_identity_pos_[i][1]) * (y - current_identity_pos_[i][3]) < 0 )
				is_current_model_shown_[i] = !is_current_model_shown_[i];
		}
	} else if ( event->button() == Qt::RightButton ){
		for ( int i = 0; i < current_identity_pos_.size(); ++i ){
			if ( (x - current_identity_pos_[i][0]) * (x - current_identity_pos_[i][2]) < 0 && (y - current_identity_pos_[i][1]) * (y - current_identity_pos_[i][3]) < 0 ){
				for ( int j = 0; j < model_painting_order_.size(); ++j )
					if ( model_painting_order_[j] == i ){
						for ( int k = j; k < model_painting_order_.size() - 1; ++k ) model_painting_order_[k] = model_painting_order_[k + 1];
						model_painting_order_[model_painting_order_.size() - 1] = i;
						this->updateGL();
						return;
					}
			}
		}

		bool is_control_panel_hitted = false;
		for ( int i = 0; i < adjust_plane_pos_.size() / 2; ++i ){
			float radius = sqrt(pow(adjust_plane_pos_[i * 2] - x, 2.0) + pow(adjust_plane_pos_[2 * i + 1] - y, 2.0));
			if ( radius < 80 ) {
				is_control_panel_hitted = true;
				emit InfoGainSelected(i);
				break;
			}
		}
		if ( !is_control_panel_hitted ) emit InfoGainSelected(-1);
	}
	

	this->updateGL();
}

void WrfParallelCoordinate::mouseDoubleClickEvent(QMouseEvent* event){
	int x = event->pos().x();
	int y = event->pos().y();
	for ( int i = 0; i < axis_pos_.size(); ++i )
		if ( abs(x - axis_pos_[i]) < 20 ){
			if ( is_high_light_on_ || !(data_mode_ == NOMRAL || data_mode_ == COMPARED) ) break;
			emit AttributeSelectedChanged(WrfGeneralDataStampType(i));
			return;
		}

	if ( data_mode_ == COMPARED ){
		compared_painting_order_ = !compared_painting_order_;
		this->updateGL();
	} else {
		// clear the highlight
		is_high_light_on_ = false;
		highlight_index_.assign(highlight_index_.size(), false);
		
		for ( int i = 0; i < is_record_highlight_.size(); ++i ) is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);
		WrfDataManager::GetInstance()->SetHighLightIndex(highlight_index_);

		emit HighLightOff();
		this->updateGL();
	}
}

void WrfParallelCoordinate::mouseMoveEvent(QMouseEvent *event){
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

void WrfParallelCoordinate::mouseReleaseEvent(QMouseEvent *event){
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
		float low_value = min_value_vec_[selection_axis_index_] + ((float)(this->height() - low_y) / this->height() - axis_bottom_y_value_) / (axis_top_y_value_ - axis_bottom_y_value_) * (max_value_vec_[selection_axis_index_] - min_value_vec_[selection_axis_index_]);
		float high_value = min_value_vec_[selection_axis_index_] + ((float)(this->height() - high_y) / this->height() - axis_bottom_y_value_) / (axis_top_y_value_ - axis_bottom_y_value_) * (max_value_vec_[selection_axis_index_] - min_value_vec_[selection_axis_index_]);
		if ( data_mode_ == NOMRAL || data_mode_ == CLUSTERED ){
			for ( int i = 0; i < data_set_->values.size(); ++i )
				if ( model_painting_order_[model_painting_order_.size() - 1] == i ){
					for ( int j = 0; j < data_set_->values[i].size(); ++j )
						if ( data_set_->values[i][j].data[selection_axis_index_] >= low_value && data_set_->values[i][j].data[selection_axis_index_] <= high_value) {
							is_record_highlight_[i][j] = true;
							highlight_index_[data_set_->values[i][j].grid_index] = true;
						}
						else
							is_record_highlight_[i][j] = false;
				} else 
					for ( int j = 0; j < data_set_->values[i].size(); ++j )
						is_record_highlight_[i][j] = false;
		} else if ( data_mode_ == CLUSTERED ){
			for ( int i = 0; i < cluster_centers_.size(); ++i ){
				if ( model_painting_order_[model_painting_order_.size() - 1] == i ){
					for ( int j = 0; j < cluster_centers_[i].size(); ++j )
						if ( cluster_centers_[i][j].data[selection_axis_index_] >= low_value && cluster_centers_[i][j].data[selection_axis_index_] <= high_value) {
							for ( int k = 0; k < data_set_->values[i].size(); ++k )
								if ( belonging_[i][k] == cluster_flags_[i][j] ){
									is_record_highlight_[i][j] = true;
									highlight_index_[data_set_->values[i][k].grid_index] = true;
								} else {
									is_record_highlight_[i][k] = false;
								}
						}
				} else 
					for (int j = 0; j < data_set_->values[i].size(); ++j )
						is_record_highlight_[i][j] = false;
			}
				
		}

		WrfDataManager::GetInstance()->SetHighLightIndex(highlight_index_);
		emit HighLightChanged();
		this->updateGL();
	}
}

void WrfParallelCoordinate::DrawBsplineCurve(std::vector< float >& point_pos, std::vector< float >& t_values, std::vector< float >& point_t_values, std::vector< int >& t_range_vec, int curve_level){
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

void WrfParallelCoordinate::DrawLineRecords(std::vector< std::vector< WrfParallelRecord > >& records, std::vector< bool >& model_visibility, float r, float g, float b, float alpha_scale){
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

	int attrib_num = data_set_->attrib_name_vec.size();
	float begin_pos = x_border_width_scale_ + axis_width_scale_ / 2;
	float end_pos = 1.0 - model_identity_scale_ - axis_width_scale_ / 2 - 2 * x_border_width_scale_;
	float step_size = (end_pos - begin_pos) / (attrib_num - 1);

	std::vector< float > axis_x;

	float x, y1, y2, y3, base, axis_length;
	x = begin_pos;
	base = y_border_width_scale_ * 2 + attri_weight_circle_scale_ + value_text_height_scale_;
	axis_length = 1.0 - 3 * y_border_width_scale_ - attrib_text_height_scale_ - 2 * value_text_height_scale_ - attri_weight_circle_scale_;

	for ( int i = 0; i < attrib_num; ++i ){
		axis_x.push_back(x);
		x += step_size;
	}

	// Get the three median value of each segment of the parallel coordinate
	std::vector< std::vector< float > > median_value_set;
	for ( int i = 0; i < records.size(); ++i ){
		std::vector< float > median_values;
		if ( data_set_->attrib_name_vec.size() <= 0 ) continue;
		median_values.resize((data_set_->attrib_name_vec.size() - 1) * 6, 0);
		for ( int j = 0; j < records[i].size(); ++j ){
			WrfParallelRecord record = records[i][j];
			//glColor4f(record.r, record.g, record.b, record.alpha);
			y1 = (record.data[0] - min_value_vec_[0]) / (max_value_vec_[0] - min_value_vec_[0]);
			y1 = base + y1 * axis_length;
			for ( int k = 1; k < data_set_->attrib_name_vec.size(); ++k ){
				y2 = (record.data[k] - min_value_vec_[k]) / (max_value_vec_[k] - min_value_vec_[k]);
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
			median_values[k] /= records[i].size();
		median_value_set.push_back(median_values);
	}

	float alpha1 = 0.5, alpha2 = 0.2;
	float median_y1, middle_y, median_y2;
	std::vector< float > control_point_pos;
	control_point_pos.resize(18, 0);
	for ( int i = 0; i < records.size(); ++i ){
		int current_index = model_painting_order_[i];
		if ( !model_visibility[current_index] ) continue;
		for ( int j = 0; j < records[current_index].size(); ++j ){
			WrfParallelRecord record = records[current_index][j];
			if ( (data_mode_ == NOMRAL || data_mode_ == CLUSTERED) && is_high_light_on_ ){
				float alpha = 0.05 * record.alpha;
				if ( is_record_highlight_[current_index][j] ) alpha = record.alpha;
				if ( data_mode_ == NOMRAL || data_mode_ == CLUSTERED )
					glColor4f(data_set_->model_vec[current_index].r, data_set_->model_vec[current_index].g, data_set_->model_vec[current_index].b, alpha);
				else
					glColor4f(r, g, b, alpha);
			} else {
				if ( data_mode_ == NOMRAL || data_mode_ == CLUSTERED )
					glColor4f(data_set_->model_vec[current_index].r, data_set_->model_vec[current_index].g, data_set_->model_vec[current_index].b, record.alpha * alpha_scale);
				else 
					glColor4f(r, g, b, record.alpha * alpha_scale);
			}

			y1 = (record.data[0] - min_value_vec_[0]) / (max_value_vec_[0] - min_value_vec_[0]);
			y1 = base + y1 * axis_length;
			y2 = (record.data[1] - min_value_vec_[1]) / (max_value_vec_[1] - min_value_vec_[1]);
			y2 = base + y2 * axis_length;
			y3 = (record.data[2] - min_value_vec_[2]) / (max_value_vec_[2] - min_value_vec_[2]);
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

			for ( int k = 1; k < data_set_->attrib_name_vec.size() - 2; ++k ){
				y1 = y2;

				y2 = (record.data[k + 1] - min_value_vec_[k + 1]) / (max_value_vec_[k + 1] - min_value_vec_[k + 1]);
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
			y2 = (record.data[data_set_->attrib_name_vec.size() - 1] - min_value_vec_[data_set_->attrib_name_vec.size() - 1]) / (max_value_vec_[data_set_->attrib_name_vec.size() - 1] - min_value_vec_[data_set_->attrib_name_vec.size() - 1]);
			y2 = base + y2 * axis_length;

			int temp_index = (data_set_->attrib_name_vec.size() - 2) * 6;
			control_point_pos[0] = axis_x[data_set_->attrib_name_vec.size() - 2];
			control_point_pos[1] = y1;
			control_point_pos[2] = axis_x[data_set_->attrib_name_vec.size() - 2];
			control_point_pos[3] = y1;
			control_point_pos[4] = (median_value_set[current_index][temp_index + 0] - median_value_set[current_index][temp_index - 2]) / 0.66 * 0.05 + axis_x[data_set_->attrib_name_vec.size() - 2];
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
			control_point_pos[12] = (median_value_set[current_index][temp_index + 4] - axis_x[data_set_->attrib_name_vec.size() - 1]) / 0.33 * 0.05 + axis_x[data_set_->attrib_name_vec.size() - 1];
			control_point_pos[13] = (median_value_set[current_index][temp_index + 5] - y2) / 0.33 * 0.05 + y2;
			control_point_pos[14] = axis_x[data_set_->attrib_name_vec.size() - 1];
			control_point_pos[15] = y2;
			control_point_pos[16] = axis_x[data_set_->attrib_name_vec.size() - 1];
			control_point_pos[17] = y2;

			DrawBsplineCurve(control_point_pos, t_values, point_t_values, t_range_vec, curve_level);
		}
	}
}

void WrfParallelCoordinate::HierarchicalCluster(std::vector< std::vector< WrfParallelRecord > >& records, std::vector< float >& weight, std::vector< std::vector< WrfParallelRecord > >& centers){
	centers.clear();
	centers.resize(records.size());
	belonging_.clear();
	belonging_.resize(records.size());
	cluster_flags_.clear();
	cluster_flags_.resize(records.size());
	for ( int k = 0; k < records.size(); ++k ){
		std::vector< WrfParallelRecord > temp_centers;
		belonging_[k].resize(records[k].size());
		for ( int i = 0; i < belonging_[k].size(); ++i ) belonging_[k][i] = i;
		temp_centers.resize(records[k].size());
		temp_centers.assign(records[k].begin(), records[k].end());
		int cluster_count = belonging_[k].size();
		float nearest_dis = 1e10;
		while ( (nearest_dis < 0.2 || cluster_count > 10) && cluster_count >= 3 ){
			nearest_dis = 1e10;
			int nearest1 = -1, nearest2 = -1;
			for ( int i = 0; i < belonging_[k].size() - 1; ++i )
				if ( belonging_[k][i] == i ){
					for ( int j = i + 1; j < belonging_[k].size(); ++j )
						if ( belonging_[k][j] == j ){
							float temp_dis =  Distance(temp_centers[i], temp_centers[j], weight);
							if ( temp_dis < nearest_dis ){
								nearest_dis = temp_dis;
								nearest1 = i;
								nearest2 = j;
							}
						}
				}
			int count = 0;
			for ( int i = 0; i < weight.size(); ++i ) temp_centers[nearest1].data[i] = 0;
			float temp_alpha = 0;
			for ( int i = 0; i < belonging_[k].size(); ++i )
				if ( belonging_[k][i] == nearest1 || belonging_[k][i] == nearest2 ){
					belonging_[k][i] = nearest1;
					for ( int j = 0; j < weight.size(); ++j ) {
						temp_centers[nearest1].data[j] += records[k][i].data[j];
						temp_alpha = records[k][i].alpha;
					}
					count++;
				}
			for ( int i = 0; i <  weight.size(); ++i ) temp_centers[nearest1].data[i] /= count;
			temp_centers[nearest1].alpha = temp_alpha;
			cluster_count--;
		}
		for ( int i = 0; i < temp_centers.size(); ++i )
			if ( belonging_[k][i] == i ) {
				cluster_centers_[k].push_back(temp_centers[i]);
				cluster_flags_[k].push_back(i);
			}
	}
}

float WrfParallelCoordinate::Distance(WrfParallelRecord& record1, WrfParallelRecord& record2, std::vector< float >& weight){
	float temp_dis = 0;
	for ( int i = 0; i < weight.size(); ++i )
		temp_dis += abs(record1.data[i] - record2.data[i]) / (data_set_->max_value_vec[i] - data_set_->min_value_vec[i]);
	return temp_dis;
}

void WrfParallelCoordinate::SetClusterOn(){
	HierarchicalCluster(data_set_->values, attrib_weight_, cluster_centers_);
	data_mode_ = CLUSTERED;

	this->updateGL();
}

void WrfParallelCoordinate::SetClusterOff(){
	data_mode_ = NOMRAL;

	this->updateGL();
}

void WrfParallelCoordinate::OnHighLightChanged(){
	is_high_light_on_ = true;

	WrfDataManager::GetInstance()->GetHighLightIndex(highlight_index_);
	for ( int i = 0; i < is_record_highlight_.size(); ++i ) is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);
	for ( int i = 0; i < data_set_->values.size(); ++i )
		for ( int j = 0; j < data_set_->values[i].size(); ++j )
			if ( highlight_index_[data_set_->values[i][j].grid_index] ) is_record_highlight_[i][j] = true;
	this->updateGL();
}

void WrfParallelCoordinate::OnHighLightOff(){
	is_high_light_on_ = false;
	for ( int i = 0; i < is_record_highlight_.size(); ++i ) is_record_highlight_[i].assign(is_record_highlight_[i].size(), false);

	this->updateGL();
}