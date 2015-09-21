#include "wrf_weight_histogram.h"

#include <iostream>
#include <sstream>
#include <QGLWidget>
#include <QtGui/QMouseEvent>
#include <QToolTip>

#include "wrf_common.h"
#include "wrf_data_manager.h"

using namespace std;

WrfWeightHistogram::WrfWeightHistogram(){
	data_set_ = NULL;
	this->setMouseTracking(true);
	//this->SetData(WrfDataManager::GetInstance()->GetHistogramDataSet());

	//initialize font
	//HFONT hFont = CreateFontA(65, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
	//ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	//DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Comic Sans MS");
	//HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	//DeleteObject(hOldFont);
}

WrfWeightHistogram::~WrfWeightHistogram(){

}

void WrfWeightHistogram::SetData(WrfHistogramDataSet* data){
	if ( data == NULL || data->grid_index.size() == 0 ) return;

	data_set_ = data;
// 	data->grid_model_weight.pop_back();
//   	data->grid_model_weight.pop_back();
//   	data->grid_model_weight.pop_back();

	max_weight_data = 0;
	min_weight_data = FLT_MAX;
	segment_num = 20;
	select_control_bar = -1;
	is_dragging_control_bar =false;
	
	float temp_data;
	//initialize model_weight_histogram
	model_weight_histogram.clear();
	std::vector<float> data_cell;
	data_cell.resize(data_set_->grid_model_weight[0].size(), 0);
	std::vector< std::vector< float > > model_cell;
	model_cell.resize(data_set_->grid_model_weight[0][0].size(), data_cell);
	model_weight_histogram.resize(data_set_->grid_model_weight.size(), model_cell);

	model_weight_barchart.clear();
	std::vector<int> data_cell_new;
	data_cell_new.resize(segment_num, 0);
	std::vector< std::vector< int > > model_cell_new;
	model_cell_new.resize(data_set_->grid_model_weight[0][0].size(), data_cell_new);
	model_weight_barchart.resize(data_set_->grid_model_weight.size(), model_cell_new);

	for ( int i = 0; i < data_set_->grid_model_weight.size(); ++i ){
		for ( int k = 0; k < data_set_->grid_model_weight[i][0].size(); ++k ){
			for ( int j = 0; j < data_set_->grid_model_weight[i].size(); ++j){
				temp_data = data_set_->grid_model_weight[i][j][k];
				if(temp_data < min_weight_data)
					min_weight_data = temp_data;
				if(temp_data > max_weight_data)
					max_weight_data = temp_data;
				model_weight_histogram[i][k][j] = temp_data;
			}
		}
	}

// 	min_weight_data -= 0.1;
// 	max_weight_data += 0.1;

	//sort from small to large
	for ( int i = 0; i < model_weight_histogram.size(); ++i ){
		for ( int j = 0; j < model_weight_histogram[i].size(); ++j){
			for ( int k = 0; k < model_weight_histogram[i][j].size(); ++k ){
				for ( int l = k-1; l >= 0; --l ){
					float &current = model_weight_histogram[i][j][l+1];
					float &former = model_weight_histogram[i][j][l];
					if(current < former){
						float temp = current;
						current = former;
						former = temp;
					}else
						break;
				}
			}
		}
	}

	//intitialize bar chart data
	for ( int i = 0; i < model_weight_histogram.size(); ++i ){
		for ( int j = 0; j < model_weight_histogram[i].size(); ++j){
			for ( int k = 0; k < model_weight_histogram[i][j].size(); ++k ){
				int num = (model_weight_histogram[i][j][k] - min_weight_data) / (max_weight_data - min_weight_data) * segment_num;
				if (num == segment_num)
				{
					num--;
				}
				model_weight_barchart[i][j][num]++;
			}
		}
	}
	//accumulation
	for ( int i = 0; i < model_weight_barchart.size(); ++i ){
		for ( int j = 0; j < model_weight_barchart[i].size(); ++j){
			for ( int k = 0; k < model_weight_barchart[i][j].size(); ++k ){
				for (int l = k+1; l < model_weight_barchart[i][j].size(); l++)
				{
					model_weight_barchart[i][j][k] += model_weight_barchart[i][j][l];
				}
			}
		}
	}

	//initialize barchart pos
// 	for ( int i = 0; i < model_weight_barchart.size(); ++i ){
// 		for ( int j = 0; j < model_weight_barchart[i].size(); ++j){
// 			for ( int k = 0; k < model_weight_barchart[i][j].size(); ++k ){
// 				model_weight_barchart[i][j][k] += model_weight_barchart[i][j][l];
// 				if (model_weight_barchart[i][j][k] == 0)
// 					break;
// 			}
// 		}
// 	}

	//int bin_size = 10;

	//max_bin_count_ = 0;
	//for ( int i = 0; i < data_set_->grid_model_weight.size(); ++i ){
	//	std::vector< std::vector< int > > histogram_time_stamp;
	//	std::vector< int > temp_bin_vec;
	//	temp_bin_vec.resize(bin_size, 0);
	//	histogram_time_stamp.resize(data_set_->grid_model_weight[0][0].size(), temp_bin_vec);
	//	for ( int j = 0; j < data_set_->grid_model_weight[i].size(); ++j)
	//		for ( int k = 0; k < data_set_->grid_model_weight[i][j].size(); ++k ){
	//			int temp_bin_index = (int)(data_set_->grid_model_weight[i][j][k] * bin_size);
	//			histogram_time_stamp[k][temp_bin_index]++;
	//			if ( histogram_time_stamp[k][temp_bin_index]  > max_bin_count_ ) max_bin_count_ = histogram_time_stamp[k][temp_bin_index];
	//		}
	//	model_weight_histogram.push_back(histogram_time_stamp);
	//}

	this->updateGL();
}

void WrfWeightHistogram::initializeGL(){
	if ( glewInit() != GLEW_OK ){
		std::cout << "Weight Histogram Initialize Error!" << std::endl;
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfWeightHistogram::resizeGL(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, 0, 3);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -1);
}

void WrfWeightHistogram::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);
	glEnable( GL_LINE_SMOOTH );

	if ( data_set_ == NULL || data_set_->grid_model_weight.size() == 0 ) return;

	//float x_border_scale = 20.0 / this->width();
	//float y_border_scale = 10.0 / this->height();
	//float time_line_width = 30.0 / this->width();

	//float x_begin_pos = x_border_scale + time_line_width;
	//float x_step = (1.0 - x_border_scale - time_line_width) / (data_set_->grid_model_weight[0][0].size() + 1);
	//float y_begin_pos = y_border_scale;
	//float y_step = (1.0 - y_border_scale) / data_set_->grid_model_weight.size();

	//// Paint Model Weight Histogram
	//float y_pos = y_begin_pos;
	//for ( int i = 0; i < model_weight_histogram.size(); ++i ){
	//	float x_pos = x_begin_pos;
	//	float bin_size = (x_step - x_border_scale) / model_weight_histogram[0][0].size();
	//	// draw bins
	//	for ( int j = 0; j < model_weight_histogram[0].size(); ++j ){
	//		glColor3f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2]);
	//		float temp_x_pos = x_pos;
	//		for ( int k = 0; k < model_weight_histogram[0][0].size(); ++k ){
	//			float temp_y_pos = (y_step - y_border_scale) * model_weight_histogram[i][j][k] / max_bin_count_;
	//			glRectf(temp_x_pos, y_pos, temp_x_pos + bin_size, y_pos + temp_y_pos);
	//			temp_x_pos += bin_size;
	//		}
	//		x_pos += x_step;
	//	}
	//	y_pos += y_step;
	//	// Draw Blend Bins
	//}

	top_margin = 0.1f * this->height();
	left_margin = 0.12f * this->width();
	left_margin_timeline = 0.09f * this->width();
	width_range = 0.22f * this->width();
	height_range = 1.0f/(model_weight_histogram.size() + 1) * this->height();
	value_length = 15.0f;
	width_padding = 10.0f;
	left_padding = 20.0f;
	top_padding = height_range * 0.2f;
	min_value_horizon_offset = 0.01f * this->width();
	min_value_verticle_offset = 12.0f;
	max_value_horizon_offset = 0.038f * this->width();
	max_value_verticle_offset = 3.0f;
	timeline_offset = 12.0f;
	timeline_title_offset = 0.075f * this->width();
	//barchart_height_per_num = height_range*0.9f/segment_num;
	barchart_width = 0.028f * this->width();
	barchart_left_margin = 0.01f * this->width();
	barchart_left_padding = 0.017f * this->width();
	control_offset = 0.15f * barchart_width;
	control_bar_height = 0.1f * height_range;
	control_bar_width = 0.022f * this->width();
	
	glColor3f(0.0f, 0.0f, 0.0f);

	QFont f;
	f.setPixelSize(10);
	std::stringstream stream;
	//draw aid line
// 	for ( int i = 0; i <= model_weight_histogram.size(); ++i ){
// 		glBegin(GL_LINE_STRIP);
// 		glVertex3f(left_margin, top_margin + i*height_range, 0.0f);
// 		glVertex3f(left_margin + width_range*5, top_margin + i*height_range, 0.0f);
// 		glEnd();
// 	}

// 	for ( int i = 0; i <= model_weight_histogram[0].size() + 1; ++i ){
// 		glBegin(GL_LINE_STRIP);
// 		glVertex3f(left_margin + i*width_range, top_margin, 0.0f);
// 		glVertex3f(left_margin + i*width_range, top_margin + height_range*4, 0.0f);
// 		glEnd();
// 	}

	//draw timeline
	glBegin(GL_LINES);
	glVertex3f(left_margin_timeline, top_margin - 20.0f, 0.0f);
	glVertex3f(left_margin_timeline, top_margin + model_weight_histogram.size()*height_range + 20.0f, 0.0f);
	glVertex3f(left_margin_timeline, top_margin + model_weight_histogram.size()*height_range + 20.0f, 0.0f);
	glVertex3f(left_margin_timeline - 10.0f, top_margin + model_weight_histogram.size()*height_range + 10.0f, 0.0f);
	glVertex3f(left_margin_timeline, top_margin + model_weight_histogram.size()*height_range + 20.0f, 0.0f);
	glVertex3f(left_margin_timeline + 10.0f, top_margin + model_weight_histogram.size()*height_range + 10.0f, 0.0f);
	glEnd();
	
	//f.setPixelSize(15 * 0.001f * this->width());
	f.setPixelSize(12 + 3*0.001f * this->width());
	int current_time = WrfDataManager::GetInstance()->current_time();
	for ( int i = model_weight_histogram.size()-1; i >= 0; i-- ){
		glBegin(GL_LINE_STRIP);
		glVertex3f(left_margin_timeline, top_margin + i*height_range, 0.0f);
		glVertex3f(left_margin_timeline + timeline_offset, top_margin + i*height_range, 0.0f);
		glEnd();
		stream.str("");
		stream << current_time - (model_weight_histogram.size() - i) << "th";
		QGLWidget::renderText(left_margin_timeline - timeline_title_offset, 
				this->height() - top_margin - i*height_range,
				stream.str().c_str(),
				f);
	}

	//draw data diagram
	for ( int i = 0; i < model_weight_histogram.size(); ++i ){
		for ( int j = 0; j < model_weight_histogram[i].size(); ++j){
			glColor3f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2]);
			//draw polygon
			for ( int k = 0; k < model_weight_histogram[i][j].size() - 1; ++k ){
				glBegin(GL_POLYGON);
				glVertex3f(k*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding, 
					(model_weight_histogram[i][j][k] - min_weight_data)/(max_weight_data  - min_weight_data)*(height_range - top_padding*2) + top_margin + height_range*(i) + top_padding/3,
					0.0f);
				glVertex3f((k+1)*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding, 
					(model_weight_histogram[i][j][k+1] - min_weight_data)/(max_weight_data  - min_weight_data)*(height_range - top_padding*2) + top_margin + height_range*(i) + top_padding/3,
					0.0f);
				glVertex3f((k+1)*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding, 
					top_margin + height_range*(i), 
					0.0f);
				glVertex3f(k*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding, 
					top_margin + height_range*(i), 
					0.0f);
				glEnd();
			}
			//draw min and max value
// 			glColor3f(0.0f, 0.0f, 0.0f);
// 			glBegin(GL_LINES);
// 			glVertex3f((model_weight_histogram[i][j].size() - 1)*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding, 
// 					(model_weight_histogram[i][j][model_weight_histogram[i][j].size()-1] - min_weight_data)*(height_range - top_padding)/(max_weight_data-min_weight_data) + top_margin + height_range*(i), 
// 					0.0f);
// 			glVertex3f((model_weight_histogram[i][j].size() - 1)*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding + value_length, 
// 					(model_weight_histogram[i][j][model_weight_histogram[i][j].size()-1] - min_weight_data)*(height_range - top_padding)/(max_weight_data-min_weight_data) + top_margin + height_range*(i), 
// 					0.0f);
// 			glVertex3f(left_margin + width_range*j + left_padding, 
// 					(model_weight_histogram[i][j][0] - min_weight_data)*(height_range - top_padding)/(max_weight_data-min_weight_data) + top_margin + height_range*(i), 
// 					0.0f);
// 			glVertex3f(left_margin + width_range*j + left_padding - value_length, 
// 					(model_weight_histogram[i][j][0] - min_weight_data)*(height_range - top_padding)/(max_weight_data-min_weight_data) + top_margin + height_range*(i), 
// 					0.0f);
// 			glEnd();
// 			

			//min value
			glColor3f(0.0f, 0.0f, 0.0f);
			stream.str("");
			stream.precision(3);
			stream<<model_weight_histogram[i][j][0];
			f.setPixelSize(12);
			QGLWidget::renderText(left_margin + width_range*j + left_padding - min_value_horizon_offset,
				this->height() - top_margin - height_range*(i) + min_value_verticle_offset,
				stream.str().c_str(),
				f);
			//max value
			stream.str("");
			stream<<model_weight_histogram[i][j][model_weight_histogram[i][j].size() - 1];
			QGLWidget::renderText(left_margin + width_range*(j+1) - max_value_horizon_offset, 
				this->height() - top_margin - height_range*(i) + min_value_verticle_offset, 
				stream.str().c_str(),
				f);
		}
	}
	
	//draw compare line
	for ( int i = 0; i < model_weight_histogram.size(); ++i ){
		for ( int j = 0; j < model_weight_histogram[i].size(); ++j){
			for ( int k = 0; k < model_weight_histogram[i][j].size(); ++k ){
				if (i < model_weight_histogram.size() - 1)
				{
					glColor3f(0.5f, 0.5f, 0.5f);
					glBegin(GL_LINE_STRIP);
					for ( int k = 0; k < model_weight_histogram[i][j].size(); ++k ){
						glVertex3f(k*(width_range-width_padding)/model_weight_histogram[i][j].size() + left_margin + width_range*j + left_padding, 
							(model_weight_histogram[i][j][k] - min_weight_data)/(max_weight_data  - min_weight_data)*(height_range - top_padding*2) + top_margin + height_range*(i + 1) + top_padding/3,
							0.0f);
					}
					glEnd();
				}
			}
		}
	}

	//draw barchart
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	barchart_height.clear();
	for ( int i = 0; i < model_weight_barchart.size(); ++i ){
		for ( int j = 0; j < model_weight_barchart[i].size(); ++j){
			float current_height = 0;

 			barchart_height_per_num = (model_weight_histogram[i][j][model_weight_histogram[i][j].size() - 1] - min_weight_data)/(max_weight_data  - min_weight_data)*(height_range - top_padding*2) + top_padding/3;
			barchart_height_per_num /= segment_num;
			for ( int k = 0; k < model_weight_barchart[i][j].size(); ++k ){
				if (model_weight_barchart[i][j][k] == 0)
					break;
				//if(k > 1)
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f - (k)*1.0f/segment_num);
				//else
					//glColor4f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2], 1.0f - (k)*1.0f/segment_num);
				
				glBegin(GL_POLYGON);
				glVertex3f(left_margin + width_range*3 + left_padding + j*(barchart_width+barchart_left_padding) + barchart_left_margin, 
					height_range*(i) + top_margin + current_height,  
					0.0f);

				//if(k > 1)
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f - (k+1)*1.0f/segment_num);
				//else
					//glColor4f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2], 1.0f - (k+1)*1.0f/segment_num);

				glVertex3f(left_margin + width_range*3 + left_padding + j*(barchart_width+barchart_left_padding) + barchart_left_margin, 
					height_range*(i) + top_margin + current_height + barchart_height_per_num, 
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + barchart_width +  j*(barchart_width+barchart_left_padding) + barchart_left_margin, 
					height_range*(i) + top_margin + current_height + barchart_height_per_num, 
					0.0f);

				//if(k > 1)
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f - (k)*1.0f/segment_num);
				//else
					//glColor4f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2], 1.0f - (k)*1.0f/segment_num);

				glVertex3f(left_margin + width_range*3 + left_padding + barchart_width +  j*(barchart_width+barchart_left_padding) + barchart_left_margin, 
					height_range*(i) + top_margin + current_height, 
					0.0f);
				glEnd();
				current_height += barchart_height_per_num;
			}
			barchart_height.push_back(current_height);
			//draw control bar
			if (select_control_bar == i*model_weight_barchart[0].size() + j)
			{
				glColor4f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2], 1.0f);
			}else
				glColor4f(data_set_->model_rgb_color[3 * j], data_set_->model_rgb_color[3 * j + 1], data_set_->model_rgb_color[3 * j + 2], 0.5f);

			glBegin(GL_POLYGON);
			if (control_bar_pos_percent.size() < model_weight_barchart.size() * model_weight_barchart[0].size())
			{
				glVertex3f(left_margin + width_range*3 + left_padding + j*(barchart_width+barchart_left_padding) + barchart_left_margin - control_offset, 
					height_range*(i) + top_margin + current_height/2 - control_bar_height/2,  
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + j*(barchart_width+barchart_left_padding) + barchart_left_margin - control_offset, 
					height_range*(i) + top_margin + current_height/2 + control_bar_height/2,  
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + barchart_width +  j*(barchart_width+barchart_left_padding) + barchart_left_margin + control_offset, 
					height_range*(i) + top_margin + current_height/2 + control_bar_height/2,  
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + barchart_width +  j*(barchart_width+barchart_left_padding) + barchart_left_margin + control_offset, 
					height_range*(i) + top_margin + current_height/2 - control_bar_height/2,  
					0.0f);
				
				//record control bar pos
				control_bar_pos_percent.push_back(0.5f);
				current_control_bar_pos.push_back(0.5f * current_height/(barchart_height_per_num*segment_num));
			}else{
				glVertex3f(left_margin + width_range*3 + left_padding + j*(barchart_width+barchart_left_padding) + barchart_left_margin - control_offset, 
					height_range*(i) + top_margin + control_bar_pos_percent[i*model_weight_barchart[0].size() + j]*barchart_height[i*model_weight_barchart[0].size() + j] - control_bar_height/2,  
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + j*(barchart_width+barchart_left_padding) + barchart_left_margin - control_offset, 
					height_range*(i) + top_margin + control_bar_pos_percent[i*model_weight_barchart[0].size() + j]*barchart_height[i*model_weight_barchart[0].size() + j] + control_bar_height/2,  
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + barchart_width +  j*(barchart_width+barchart_left_padding) + barchart_left_margin + control_offset, 
					height_range*(i) + top_margin + control_bar_pos_percent[i*model_weight_barchart[0].size() + j]*barchart_height[i*model_weight_barchart[0].size() + j] + control_bar_height/2,  
					0.0f);
				glVertex3f(left_margin + width_range*3 + left_padding + barchart_width +  j*(barchart_width+barchart_left_padding) + barchart_left_margin + control_offset, 
					height_range*(i) + top_margin + control_bar_pos_percent[i*model_weight_barchart[0].size() + j]*barchart_height[i*model_weight_barchart[0].size() + j] - control_bar_height/2,  
					0.0f);
			}
			glEnd();
		}
	}
	glDisable(GL_BLEND);
}

void WrfWeightHistogram::mouseMoveEvent(QMouseEvent* event){
	if ( model_weight_histogram.size() == 0 ) return;

	float pos_x = event->posF().x();
	float pos_y = event->posF().y();
	//tooltip
	int chart_x = (pos_x - left_margin - left_padding) / width_range;
	int chart_y = model_weight_histogram.size() - (this->height() - pos_y - top_margin) / height_range;
	
	if (chart_x >= 0 && chart_x < model_weight_histogram[0].size() && chart_y >= 0 && chart_y < model_weight_histogram.size() &&  
		pos_x > (left_margin + left_padding) && this->height() - pos_y < top_margin + height_range*model_weight_histogram.size())
	{
		float min_value = model_weight_histogram[model_weight_histogram.size() - chart_y - 1][chart_x][0];
		float max_value = model_weight_histogram[model_weight_histogram.size() - chart_y - 1][chart_x][model_weight_histogram[chart_y][chart_x].size() - 1];
		std::stringstream text;
		text<<"min: "<<min_value<<" max: "<<max_value;
		QPoint point;
		point.setX(event->globalPos().x() - event->x() + chart_x*width_range + left_margin + left_padding);
		point.setY(event->globalPos().y() - event->y() + this->height() - (model_weight_histogram.size() - chart_y - 0.5f )*height_range - top_margin);
		QToolTip::showText(point, text.str().c_str());
	}
	//control bar selection
	int control_bar_x = (pos_x - left_margin - width_range*3 - left_padding - barchart_left_margin + control_offset) / (barchart_width+barchart_left_padding);
	int control_bar_y = (this->height() - pos_y - top_margin) / height_range;
	//cout<<control_bar_x<<" "<<control_bar_y<<endl;
	int current_selection_item_num = control_bar_y * model_weight_histogram[0].size() + control_bar_x;
	if (control_bar_x >=0 && control_bar_x < model_weight_histogram[0].size() && control_bar_y >=0 && control_bar_y < model_weight_histogram.size())
	{
		if (height() - pos_y > control_bar_pos_percent[current_selection_item_num]*barchart_height[current_selection_item_num] + top_margin + height_range*control_bar_y &&
			height() - pos_y < control_bar_pos_percent[current_selection_item_num]*barchart_height[current_selection_item_num] + top_margin + height_range*control_bar_y + control_bar_height &&
			pos_x > left_margin + width_range*3 + left_padding + barchart_left_margin + (barchart_width+barchart_left_padding)*control_bar_x &&
			pos_x < left_margin + width_range*3 + left_padding + barchart_left_margin + (barchart_width+barchart_left_padding)*control_bar_x + barchart_width)
		{
			//std::cout<<this->height() - pos_y<<" "<<control_bar_pos[current_selection_item_num].y()<<std::endl;
			select_control_bar = current_selection_item_num;
			if (is_dragging_control_bar && 
				(height() - pos_y - control_bar_height/2) > top_margin + height_range*control_bar_y &&
				(height() - pos_y - control_bar_height/2) < top_margin + height_range*control_bar_y + barchart_height[current_selection_item_num])
			{
				
				control_bar_pos_percent[current_selection_item_num] = (height() - pos_y - control_bar_height/2 - top_margin - height_range*control_bar_y) / barchart_height[current_selection_item_num];
				current_control_bar_pos[current_selection_item_num] = control_bar_pos_percent[current_selection_item_num] * barchart_height[current_selection_item_num]/(barchart_height_per_num*segment_num);
			}
		}else
			select_control_bar = -1;
		this->updateGL();
	}
}

void WrfWeightHistogram::mousePressEvent(QMouseEvent* event){
	if (select_control_bar >= 0)
	{
		is_dragging_control_bar = true;
	}else
		is_dragging_control_bar = false;
	this->updateGL();
}

void WrfWeightHistogram::mouseReleaseEvent(QMouseEvent* event){
	is_dragging_control_bar = false;
	this->updateGL();
}
// 
// void WrfWeightHistogram::resizeEvent(QResizeEvent* event){
// 	control_bar_pos.clear();
// 	this->updateGL();
// }