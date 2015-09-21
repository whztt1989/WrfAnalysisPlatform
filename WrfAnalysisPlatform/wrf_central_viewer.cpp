#include "wrf_central_viewer.h"
#include <iostream>
#include <fstream>
#include <QtGui/QMouseEvent>
#include "wrf_data_stamp.h"
#include "qcolor_bar_controller.h"
#include "wrf_data_manager.h"
#include "wrf_stamp_generator.h"

#include "loaddata.h"
#include "isotools.h"
#include "isolinecreator.h"
#include "spline3interp.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace splab;

typedef double Type;

WrfCentralViewer::WrfCentralViewer(){
	bias_map_ = NULL;
	similarity_map_ = NULL;
	grid_value_map_ = NULL;
	wind_var_map_ = NULL;

	start_longitude_ = 60;
	end_longitude_ = 150;
	start_latitude_ = 15;
	end_latitude_ = 60;

	longitude_grid_number_ = 81;
	latitude_grid_number_ = 41;

	x_bias_ = 0;
	y_bias_ = 0;
	left_ = 0;
	right_ = 1;
	bottom_ = 0;
	top_ = 1;
	scale_ = 1.0;

	selection_radius_ = 15;

	view_mode_ = SELECTING_SITE;
	selection_mode_ = NORMAL_MODE;

	is_edit_isoline = false;
	is_draw_edit_line = false;
	edit_isoline_index = -1;

	china_map_image_ = QImage("./MapData/china.bmp");
	QImage::Format formate = china_map_image_.format();
	map_left_ = 70;
	map_right_ = 138;
	map_bottom_ = 15;
	map_top_ = 60;

	this->setMinimumSize(800, 400);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->setMouseTracking(false);
	this->setFocusPolicy(Qt::StrongFocus);

	LoadMapData();
}

WrfCentralViewer::~WrfCentralViewer(){

}

void WrfCentralViewer::SetViewMode(ViewMode mode){
	view_mode_ = mode;
}

void WrfCentralViewer::SetSelectionMode(SelectionMode mode){
	if ( mode != selection_mode_ ){
		selection_mode_ = mode;
		UpdateSelectedBuffer();
		this->updateGL();
	}
}

void WrfCentralViewer::set_bias_map(WrfGridValueMap* bias_map){
	bias_map_ = bias_map;

	if ( bias_map_ != NULL ){
		start_longitude_ = bias_map_->start_longitude;
		end_longitude_ = bias_map_->end_longitude;
		start_latitude_ = bias_map_->start_latitude;
		end_latitude_ = bias_map_->end_latitude;
		longitude_grid_number_ = bias_map_->longitude_grid_number;
		latitude_grid_number_ = bias_map_->latitude_grid_number;

		is_grid_selected_.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);
		is_grid_selected_.assign(is_grid_selected_.size(), false);
		is_compared_selected_.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);
		is_compared_selected_.assign(is_compared_selected_.size(), false);
		is_candidate_selected_.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);
		is_candidate_selected_.assign(is_candidate_selected_.size(), 0);

		WrfDataManager::GetInstance()->GetSiteAlpha(site_alpha_);

		UpdateTextures();
		UpdateBiasMapBuffer();

		x_bias_ = 0;
		y_bias_ = 0;

		this->resizeGL(this->width(), this->height());
	}

	this->updateGL();
}

void WrfCentralViewer::update_bias_map(WrfGridValueMap* bias_map){
	bias_map_ = bias_map;

	if ( bias_map_ != NULL ){
		start_longitude_ = bias_map_->start_longitude;
		end_longitude_ = bias_map_->end_longitude;
		start_latitude_ = bias_map_->start_latitude;
		end_latitude_ = bias_map_->end_latitude;
		longitude_grid_number_ = bias_map_->longitude_grid_number;
		latitude_grid_number_ = bias_map_->latitude_grid_number;

		UpdateBiasMapBuffer();


		this->resizeGL(this->width(), this->height());
	}

	this->updateGL();
}

void WrfCentralViewer::set_wind_var_map(WrfGridValueMap* var_map){
	wind_var_map_ = var_map;
	this->updateGL();
}

void WrfCentralViewer::set_grid_value_map(WrfGridValueMap* value_map){
	if ( grid_value_map_ != NULL ) delete grid_value_map_;
	grid_value_map_ = value_map;

	GenerateIsoLines();

	this->updateGL();
}

void WrfCentralViewer::set_selected_area(std::vector< int >& area_grid_index){
	selected_area_.resize(area_grid_index.size());
	selected_area_.assign(area_grid_index.begin(), area_grid_index.end());

	this->updateGL();
}

void WrfCentralViewer::GetSelectedArea(std::vector< int >& area_grid_index){
	area_grid_index.resize(selected_area_.size());
	area_grid_index.assign(selected_area_.begin(), selected_area_.end());
}

void WrfCentralViewer::ClearSelection(){
	if ( selection_mode_ == NORMAL_MODE ){
		is_grid_selected_.assign(is_grid_selected_.size(), false);
	} else if ( selection_mode_ == COMPARED_MODE ){
		is_compared_selected_.assign(is_compared_selected_.size(), false);
	}
	UpdateSelectedBuffer();
	UpdateSelectedArea();

	this->updateGL();
}

void WrfCentralViewer::OnWindVarThresholdChanged(){
	this->updateGL();
}

void WrfCentralViewer::UpdateBackgroundTexture(){
	
}

void WrfCentralViewer::UpdateTextures(){
	// update bias texture
	bias_pixel_value_.resize(longitude_grid_number_ * latitude_grid_number_ * 4, 1.0);
	memset(bias_pixel_value_.data(), 0, bias_pixel_value_.size() * sizeof(float));
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bias_tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, bias_pixel_value_.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	// update selected area texture
	selected_area_pixel_value_.resize(longitude_grid_number_ * latitude_grid_number_ * 4, 1.0);
	memset(selected_area_pixel_value_.data(), 0, selected_area_pixel_value_.size() * sizeof(float));
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, selected_tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, selected_area_pixel_value_.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	compared_area_pixel_values_.resize(longitude_grid_number_ * latitude_grid_number_ * 4, 1.0);
	memset(compared_area_pixel_values_.data(), 0, compared_area_pixel_values_.size() * sizeof(float));

	// update candidate selected area texture
	candidate_area_pixel_value_.resize(longitude_grid_number_ * latitude_grid_number_ * 4, 1.0);
	memset(candidate_area_pixel_value_.data(), 0, candidate_area_pixel_value_.size() * sizeof(float));
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, candidate_tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, candidate_area_pixel_value_.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void WrfCentralViewer::initializeGL(){
	if (glewInit() != GLEW_OK){
		std::cout << "Central viewer glewInit error!" << std::endl;
		exit(0);
	}

	// Background color (159, 188, 227)
	//glClearColor(0.623f, 0.737f, 0.890f, 1.0f);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glGenTextures(1, &selected_tex_);
	if ( selected_tex_ == -1 ){
		std::cout << "Central viewer: Generate selected area texture error!" << std::endl;
		exit(0);
	}

	glGenTextures(1, &candidate_tex_);
	if ( candidate_tex_ == -1 ){
		std::cout << "Central viewer: Generate candidate selected area texture error!" << std::endl;
		exit(0);
	}

	glGenTextures(1, &bias_tex_);
	if ( bias_tex_ == -1 ){
		std::cout << "Central viewer: Generate bias texture error!" << std::endl;
		exit(0);
	}

	UpdateTextures();
}

void WrfCentralViewer::resizeGL(int w, int h){
	if ( abs(end_longitude_ - start_longitude_) / w > abs(end_latitude_ - start_latitude_) / h ){
		float degree_per_pixel = abs(end_longitude_ - start_longitude_) / w;
		left_ = start_longitude_;
		right_ = end_longitude_;
		bottom_ = (end_latitude_ + start_latitude_) / 2 - degree_per_pixel * this->height() / 2;
		top_ = (end_latitude_ + start_latitude_) / 2 + degree_per_pixel * this->height() / 2;
	} else {
		float degree_per_pixel = abs(end_latitude_ - start_latitude_) / h;
		left_ = (end_longitude_ + start_longitude_) / 2 - degree_per_pixel * this->width() / 2;
		right_ = (end_longitude_ + start_longitude_) / 2 + degree_per_pixel * this->width() / 2;
		bottom_ = start_latitude_;
		top_ = end_latitude_;
	}

	x_bias_ = 0;
	y_bias_ = 0;
	scale_ = 1.0;
}

void WrfCentralViewer::paintGL(){
	glViewport(0, 0, this->width(), this->height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left_ + x_bias_, right_ + x_bias_, bottom_ + y_bias_, top_ + y_bias_, 1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);

	glPushMatrix();
	glScalef(scale_, scale_, scale_);

	PaintBackgroundMap();

	PaintBiasMap();

	//PaintWindVariance();
	
	PaintSelectedArea();

	PaintCandidateSelectedArea();

	//PaintIsoLines();

	//PaintSelectIsoLine();

	//PaintEditIsoLine();

	glPopMatrix();

	PaintColorBar();

	//PaintTipIcons();

	glDisable(GL_BLEND);
}

void WrfCentralViewer::UpdateBiasMapBuffer(){
	// Update bias values
	for ( int i = 0; i < bias_map_->values.size(); ++i ){
		QColor color = QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(bias_map_->min_value, bias_map_->max_value, bias_map_->values[i]);
		int temp_index = i * 4;
		bias_pixel_value_[temp_index] = color.redF();
		bias_pixel_value_[temp_index + 1] = color.greenF();
		bias_pixel_value_[temp_index + 2] = color.blueF();
		bias_pixel_value_[temp_index + 3] = site_alpha_[i];
	}

	makeCurrent();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bias_tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, bias_pixel_value_.data());
	glDisable(GL_TEXTURE_2D);
	GLenum err = glGetError();
}

void WrfCentralViewer::PaintSites(){

}

void WrfCentralViewer::PaintWindVariance(){
	if ( wind_var_map_ == NULL ) return;
	
	float thresh = WrfDataManager::GetInstance()->wind_var_threshold();

	std::vector< float > vertex_pos;
	vertex_pos.resize(bias_map_->latitude_grid_number * bias_map_->longitude_grid_number * 2);
	std::vector< float > vertex_color;
	vertex_color.resize(bias_map_->latitude_grid_number * bias_map_->longitude_grid_number * 4);
	std::vector< int > line_index;
	line_index.resize((bias_map_->longitude_grid_number - 1) * (bias_map_->latitude_grid_number - 1) * 4);

	float temp_latitude, temp_longitude;
	int vertex_index;
	temp_latitude = bias_map_->start_latitude;
	vertex_index = 0;
	for ( int i = 0; i < bias_map_->latitude_grid_number; ++i ){
		temp_longitude = bias_map_->start_longitude;
		for ( int j = 0; j < bias_map_->longitude_grid_number; ++j ){
			vertex_pos[vertex_index * 2] = temp_longitude;
			vertex_pos[vertex_index * 2 + 1] = temp_latitude;

			vertex_color[vertex_index * 4] = 0.0;
			vertex_color[vertex_index * 4 + 1] = 0.0;
			vertex_color[vertex_index * 4 + 2] = 0.0;
			if ( wind_var_map_->values[vertex_index] > thresh )
				vertex_color[vertex_index * 4 + 3] = site_alpha_[vertex_index] * wind_var_map_->values[vertex_index];
			else
				vertex_color[vertex_index * 4 + 3] = 0.0;

			vertex_index++;
			temp_longitude += bias_map_->longitude_grid_space;

			if ( i == 0 || j == 0 ) continue;
			int left_bottom = (i - 1) * bias_map_->longitude_grid_number + j - 1;
			int left_top = i * bias_map_->longitude_grid_number + j - 1;
			int right_bottom = (i - 1) * bias_map_->longitude_grid_number + j;
			int right_top = i * bias_map_->longitude_grid_number + j;
			int temp_index = ((i - 1) * (bias_map_->longitude_grid_number - 1) + j - 1) * 4;
			line_index[temp_index] = left_top;
			line_index[temp_index + 1] = left_bottom;
			line_index[temp_index + 2] = left_bottom;
			line_index[temp_index + 3] = right_bottom;
		}
		temp_latitude += bias_map_->latitude_grid_space;
	}


	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, (float*)vertex_pos.data());
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, (float*)vertex_color.data());
	glDrawElements(GL_LINES, line_index.size(), GL_UNSIGNED_INT, (GLvoid*)line_index.data());

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

}



void WrfCentralViewer::PaintBackgroundMap(){
	glColor3f(0.0, 0.0, 0.0);
	for ( int i = 0; i < map_data_.size(); ++i ){
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, (float*)map_data_[i].data());
		glDrawArrays(GL_LINE_STRIP, 0, map_data_[i].size() / 2);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

void WrfCentralViewer::PaintBiasMap(){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bias_tex_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); 
	glBegin(GL_TRIANGLES);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(start_longitude_, start_latitude_, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(end_longitude_, start_latitude_, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(start_longitude_, end_latitude_, 0);

	glTexCoord2f(1.0f, 0.0f); glVertex3f(end_longitude_, start_latitude_, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(end_longitude_, end_latitude_, 0.0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(start_longitude_, end_latitude_, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void WrfCentralViewer::DrawCirclePlane(float radius, float r, float g, float b, float a){
	glColor4f(r, g, b, a);
	
	float pi = 3.14159;
	float theta = 0;
	float theta_step = pi / 18.0;
	float x = radius, y = 0;
	float next_x, next_y;
	glBegin(GL_TRIANGLES);
	for ( int i = 0; i < 36; ++i ){
		theta += theta_step;
		next_x = radius * cos(theta);
		next_y = radius * sin(theta);
		
		glVertex2f(0, 0);
		glVertex2f(x, y);
		glVertex2f(next_x, next_y);

		x = next_x;
		y = next_y;
	}
	glEnd();
}

void WrfCentralViewer::PaintColorBar(){
	float right_bias = 50.0;
	float bar_width = 30.0;
	float bar_height_scale = 0.7;
	float min_value, max_value;

	if ( bias_map_ != NULL ){
		min_value = bias_map_->min_value;
		max_value = bias_map_->max_value;
	} else {
		min_value = 0;
		max_value = 1.0;
	}

	float bar_pos_x = right_ + x_bias_ - (right_bias + bar_width) / this->width() * (right_ - left_);
	float bar_pos_x1 = right_ + x_bias_ - right_bias / this->width() * (right_ - left_);
	float bar_pos_y = bottom_ + y_bias_ + (0.5f - bar_height_scale / 2) * (top_ - bottom_);

	float bar_step = 1.0 / 25 * 0.7 * (top_ - bottom_);
	float value_step = 1.0 / 25 * (max_value - min_value);
	float value = min_value;
	float next_pos_y, next_value;
	QColor color = QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(min_value, max_value, value);
	for ( int i = 0; i < 25; ++i ){
		next_pos_y = bar_pos_y + bar_step;
		next_value = value + value_step;
		QColor next_color = QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(min_value, max_value, value);
		glBegin(GL_TRIANGLES);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			glVertex2f(bar_pos_x, bar_pos_y);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			glVertex2f(bar_pos_x1, bar_pos_y);
			glColor3f(next_color.redF(), next_color.greenF(), next_color.blueF());
			glVertex2f(bar_pos_x, next_pos_y);

			glColor3f(next_color.redF(), next_color.greenF(), next_color.blueF());
			glVertex2f(bar_pos_x, next_pos_y);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			glVertex2f(bar_pos_x1, bar_pos_y);
			glColor3f(next_color.redF(), next_color.greenF(), next_color.blueF());
			glVertex2f(bar_pos_x1, next_pos_y);
		glEnd();

		bar_pos_y = next_pos_y;
		color = next_color;
		value = next_value;
	}
}

void WrfCentralViewer::PaintGridPoints(){
	if ( bias_map_ != NULL ){
		glPointSize(2.0);
		glColor3f(0.0, 0.0, 0.0);

		glBegin(GL_POINTS);
		float temp_latitude, temp_longitude;
		int vertex_index = 0;
		temp_latitude = bias_map_->start_latitude;
		for ( int i = 0; i < bias_map_->latitude_grid_number; ++i ){
			temp_longitude = bias_map_->start_longitude;
			for ( int j = 0; j < bias_map_->longitude_grid_number; ++j ){
				glVertex2f(temp_longitude, temp_latitude);
				temp_longitude += bias_map_->longitude_grid_space;
			}
			temp_latitude += bias_map_->latitude_grid_space;
		}
		glEnd();
	}
}

void WrfCentralViewer::PaintSelectedArea(){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, selected_tex_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
	glBegin(GL_TRIANGLES);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(start_longitude_, start_latitude_, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(end_longitude_, start_latitude_, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(start_longitude_, end_latitude_, 0);

	glTexCoord2f(1.0f, 0.0f); glVertex3f(end_longitude_, start_latitude_, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(end_longitude_, end_latitude_, 0.0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(start_longitude_, end_latitude_, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void WrfCentralViewer::PaintCandidateSelectedArea(){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, candidate_tex_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
	glBegin(GL_TRIANGLES);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(start_longitude_, start_latitude_, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(end_longitude_, start_latitude_, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(start_longitude_, end_latitude_, 0);

	glTexCoord2f(1.0f, 0.0f); glVertex3f(end_longitude_, start_latitude_, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(end_longitude_, end_latitude_, 0.0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(start_longitude_, end_latitude_, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void WrfCentralViewer::PaintTipIcons(){

}

void WrfCentralViewer::mousePressEvent(QMouseEvent* event){
	previous_press_pos_ = event->pos();

	if ( bias_map_ == NULL ) return;
	float pos_x = (event->posF().x() / this->width() * (right_ - left_) + left_ + x_bias_) / scale_;
	float pos_y = ((1.0 - event->posF().y() / this->height()) * (top_ - bottom_) + bottom_ + y_bias_) / scale_;
	int longitude_index = (int)((pos_x - bias_map_->start_longitude) / bias_map_->longitude_grid_space + 0.5);
	if ( longitude_index < 0 || longitude_index >= bias_map_->longitude_grid_number ) return;
	int latitude_index = (int)((pos_y - bias_map_->start_latitude) / bias_map_->latitude_grid_space + 0.5);
	if ( latitude_index < 0 || latitude_index >= bias_map_->latitude_grid_number ) return;
	int temp_index = latitude_index * bias_map_->longitude_grid_number + longitude_index;

	if ( event->button() == Qt::LeftButton ){
		if(is_edit_isoline){//start to draw edit line
			is_draw_edit_line = true;

			isotools::Point2D temp;
			temp.x = pos_x;
			temp.y = pos_y;
			editList.push_back(temp);
		}
		
		if ( view_mode_ == SELECTING_SITE ){
			emit SiteSelected(temp_index);
		} else if ( view_mode_ == MAGIC_WAND ){
			selection_lines_.clear();
			selection_lines_.push_back(event->pos());

			this->updateGL();
		} else if ( view_mode_ == ADDING_AREA ){
			AddSelection(event->pos());
			UpdateSelectedBuffer();
			this->updateGL();
		}
		
	} else if (event->button() == Qt::RightButton){
		if(is_edit_isoline){//end to draw edit line

			is_draw_edit_line = false;

			if(edit_isoline_index < 0 || edit_isoline_detail < 0 || isolineList.empty()){
				editList.clear();
				this->updateGL();
				return;
			}
				
			list<isotools::Point2D>::iterator iter;
			float min_distance = FLT_MAX;
			int num = 0;
			for(iter = isolineList[edit_isoline_index][edit_isoline_detail].points.begin(); iter != isolineList[edit_isoline_index][edit_isoline_detail].points.end(); iter++, num++){
				float temp = this->GetEulerDistance(editList[editList.size()-1].x, editList[editList.size()-1].y, (*iter).x, (*iter).y);
				if(temp < min_distance){
					edit_end_pos = num;
					min_distance = temp;
				}
			}
			//search for aid point
			num = 0;
			min_distance = FLT_MAX;
			for(iter = isolineList[edit_isoline_index][edit_isoline_detail].points.begin(); iter != isolineList[edit_isoline_index][edit_isoline_detail].points.end(); iter++, num++){
				float temp = this->GetEulerDistance(editList[editList.size()/2].x, editList[editList.size()/2].y, (*iter).x, (*iter).y);
				if(temp < min_distance){
					edit_aid_pos = num;
					min_distance = temp;
				}
			}
			
			//update selected isoline data
			//cout<<"start="<<edit_start_pos<<" aid="<<edit_aid_pos<<" end="<<edit_end_pos<<endl;
			list<isotools::Point2D> &tempList = isolineList[edit_isoline_index][edit_isoline_detail].points;
			if(edit_start_pos <= edit_aid_pos && edit_aid_pos <= edit_end_pos){
				list<isotools::Point2D>::iterator itList;
				num = 0;
				//delete origin data
				for( itList = tempList.begin(); itList != tempList.end(); num++)
				{
					if(num > edit_start_pos && num < edit_end_pos)
					{
						tempList.erase( itList++);
					}
					else
						itList++;
				 }
				num = 0;
				//add new data
				for( itList = tempList.begin(); itList != tempList.end(); itList++, num++)
				{
					if(num > edit_start_pos)
					{
						for(int i = 0; i < editList.size(); i++)
						{
							tempList.insert(itList, editList[i]);
						}
						break;
					}
				 }

			}else if(edit_start_pos >= edit_aid_pos && edit_aid_pos >= edit_end_pos){
				list<isotools::Point2D>::iterator itList;
				num = 0;
				//delete origin data
				for( itList = tempList.begin(); itList != tempList.end(); num++)
				{
					if(num < edit_start_pos && num > edit_end_pos)
					{
						tempList.erase( itList++);
					}
					else
						itList++;
				 }
				num = 0;
				//add new data
				for( itList = tempList.begin(); itList != tempList.end(); itList++, num++)
				{
					if(num > edit_end_pos)
					{
						for(int i = editList.size()-1; i >= 0; i--)
						{
							tempList.insert(itList, editList[i]);
						}
						break;
					}
				 }
			}else{
				list<isotools::Point2D>::iterator itList;
				num = 0;
				//delete origin data
				for( itList = tempList.begin(); itList != tempList.end(); num++)
				{
					if(num > edit_start_pos || num < edit_end_pos)
					{
						tempList.erase( itList++);
					}
					else
						itList++;
				 }
				//add new data
				for(int i = 0; i < editList.size(); i++)
				{
					tempList.push_back(editList[i]);
				}
			}

			editList.clear();
			this->updateGL();
		}
	}
}

void WrfCentralViewer::mouseMoveEvent(QMouseEvent* event){
	if(is_edit_isoline){
		float tolerence = 0.1f;
		if(is_draw_edit_line)
			return;

		edit_isoline_index = -1;
		float pos_x = (event->posF().x() / this->width() * (right_ - left_) + left_ + x_bias_) / scale_;
		float pos_y = ((1.0 - event->posF().y() / this->height()) * (top_ - bottom_) + bottom_ + y_bias_) / scale_;
		for(int i = 0; i < isolineList.size(); i++){
			for(int j = 0; j < isolineList[i].size(); j++){
				list<isotools::Point2D>::iterator iter;
				int num = 0;
				for(iter = isolineList[i][j].points.begin(); iter != isolineList[i][j].points.end(); iter++, num++){
					if(this->GetEulerDistance(pos_x, pos_y, (*iter).x, (*iter).y) < tolerence){
						edit_isoline_index = i;
						edit_isoline_detail = j;
						edit_start_pos = num;
						this->updateGL();
						return;
					}
				}
			}
		}
		this->updateGL();
		return;
	}
	if ( view_mode_ == MAGIC_WAND && (event->buttons() & Qt::LeftButton)){
		selection_lines_.push_back(event->pos());

		this->updateGL();
	} else if (view_mode_ == ADDING_AREA && (event->buttons() & Qt::LeftButton)){
		AddSelection(event->pos());
		UpdateSelectedBuffer();
		this->updateGL();
	} else if ( event->buttons() & Qt::RightButton ){
		QPoint temp_pos = event->pos();
		x_bias_ += -1 * (float)(temp_pos.rx() - previous_press_pos_.rx()) / this->width() * (right_ - left_);
		y_bias_ += (float)(temp_pos.ry() - previous_press_pos_.ry()) / this->height() * (top_ - bottom_);

		previous_press_pos_ = temp_pos;
		this->updateGL();
	}
}

void WrfCentralViewer::mouseReleaseEvent(QMouseEvent* event){
	if ( view_mode_ == MAGIC_WAND && event->button() == Qt::LeftButton ){
		selection_lines_.push_back(event->pos());
		UpdateCandidateSelectedArea();

		this->updateGL();
	} else if ( view_mode_ == ADDING_AREA && event->button() == Qt::LeftButton ){
		UpdateSelectedArea();
		UpdateSelectedBuffer();

		this->updateGL();
	}
}

void WrfCentralViewer::wheelEvent(QWheelEvent* event){
	float temp_scale = scale_ + event->delta() / 1000.0;
	if ( temp_scale < 0.1 ) temp_scale = 0.1;
	if ( temp_scale > 10 ) temp_scale = 10;

	QPoint temp_pos = event->pos();
	float x = (float)temp_pos.x() / this->width() * (right_ - left_) + left_;
	float y = (1.0 - (float)temp_pos.y() / this->height()) * (top_ - bottom_) + bottom_;

	x_bias_ = temp_scale / scale_ * (x + x_bias_) - x;
	y_bias_ = temp_scale / scale_ * (y + y_bias_) - y;

	scale_ = temp_scale;

	this->updateGL();
}

void WrfCentralViewer::AddSelection(QPoint point){
	int grid_x, grid_y;
	GetGridIndexByWindowPos(point.x(), point.y(), grid_x, grid_y);

	int selection_grid_size = (float)selection_radius_ / this->width() * (right_ - left_) / scale_ / bias_map_->latitude_grid_space;
	if ( selection_grid_size == 0 ) selection_grid_size = 1;
	for ( int i = grid_y - selection_grid_size; i <= grid_y + selection_grid_size; ++i ){
		if ( i < 0 || i >= bias_map_->latitude_grid_number ) continue;
		for ( int j = grid_x - selection_grid_size; j <= grid_x + selection_grid_size; ++j )
			if ( j >= 0 && j < bias_map_->longitude_grid_number ){
				if ( sqrt(pow(grid_x - j, 2.0) + pow(grid_y - i, 2.0)) < selection_grid_size ){
					int temp_index = i * bias_map_->longitude_grid_number + j;
					if ( selection_mode_ == NORMAL_MODE )
						is_grid_selected_[temp_index] = true;
					else if ( selection_mode_ == COMPARED_MODE )
						is_compared_selected_[temp_index] = true;
				}
			}
	}
}

void WrfCentralViewer::UpdateSelectedArea(){
	selected_area_.clear();
	if ( selection_mode_ == NORMAL_MODE ){
		for ( int i = 0; i < is_grid_selected_.size(); ++i )
			if ( is_grid_selected_[i] ) selected_area_.push_back(i);
		WrfDataManager::GetInstance()->set_seleted_grid_index(selected_area_);
	} else if ( selection_mode_ == COMPARED_MODE ){
		for ( int i = 0; i < is_grid_selected_.size(); ++i )
			if ( is_compared_selected_[i] ) selected_area_.push_back(i);
		WrfDataManager::GetInstance()->set_compared_grid_index(selected_area_);
	}

	emit SelectedAreaChanged();
}

void WrfCentralViewer::UpdateCandidateSelectedArea(){
	is_candidate_selected_.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);
	is_candidate_selected_.assign(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number, 0);

	CandidateRegionGrowing(0.1, 0.5, 0.5, is_candidate_selected_);

	UpdateCandidateSelectedBuffer();
}

void WrfCentralViewer::AcceptCandidateArea(){
	for ( int i = 0; i < is_candidate_selected_.size(); ++i ){
		if ( is_candidate_selected_[i] == 2 )
			is_grid_selected_[i] = true;
	}
	is_candidate_selected_.assign(is_candidate_selected_.size(), false);
	UpdateCandidateSelectedBuffer();
	UpdateSelectedBuffer();

	UpdateSelectedArea();

	this->updateGL();
}

void WrfCentralViewer::CandidateRegionGrowing(float threshold, float bias_alpha, float wind_alpha, std::vector< char >& is_selected){
	std::vector< QPoint > reserved_vec;
	reserved_vec.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);

	int current_index = -1;
	int origin_grid_index = -1;
	for ( int i = 0; i < selection_lines_.size(); ++i ){
		QPointF temp_point = selection_lines_[i];
		int grid_x, grid_y;
		GetGridIndexByWindowPos(temp_point.x(), temp_point.y(), grid_x, grid_y);
		if ( grid_x == -1 || grid_y == -1 ) continue;
		current_index++;
		reserved_vec[current_index] = QPoint(grid_x, grid_y);
		origin_grid_index = grid_y * bias_map_->longitude_grid_number + grid_x;
		is_candidate_selected_[origin_grid_index] = 1;
	}

	int current_grid_index, right_grid_index, bottom_grid_index, left_grid_index, top_grid_index;
	while ( current_index >= 0 ){
		QPoint temp_point = reserved_vec[current_index];
		current_index--;

		current_grid_index = temp_point.y() * bias_map_->longitude_grid_number + temp_point.x();
		is_selected[current_grid_index] = true;
		is_candidate_selected_[current_grid_index] = 2;

		right_grid_index = current_grid_index + 1;
		if ( temp_point.x() + 1 < bias_map_->longitude_grid_number
			&& (abs(bias_map_->values[right_grid_index] - bias_map_->values[origin_grid_index]) * bias_alpha + abs(wind_var_map_->values[right_grid_index] - wind_var_map_->values[origin_grid_index])* wind_alpha < threshold)
			&& !is_selected[right_grid_index] && is_candidate_selected_[right_grid_index] == 0 ){
			
			current_index++;
			reserved_vec[current_index] = QPoint(temp_point.x() + 1, temp_point.y());
			is_candidate_selected_[right_grid_index] = 1;
		}

		left_grid_index = current_grid_index - 1;
		if ( temp_point.x() - 1 >= 0 
			&& (abs(bias_map_->values[left_grid_index] - bias_map_->values[origin_grid_index]) * bias_alpha + abs(wind_var_map_->values[left_grid_index] - wind_var_map_->values[origin_grid_index])* wind_alpha < threshold)
			&& !is_selected[left_grid_index] && is_candidate_selected_[left_grid_index] == 0 ){

			current_index++;
			reserved_vec[current_index] = QPoint(temp_point.x() - 1, temp_point.y());
			is_candidate_selected_[left_grid_index] = 1;
		}

		top_grid_index = current_grid_index + bias_map_->longitude_grid_number;
		if ( temp_point.y() + 1 < bias_map_->latitude_grid_number
			&& (abs(bias_map_->values[top_grid_index] - bias_map_->values[origin_grid_index]) * bias_alpha + abs(wind_var_map_->values[top_grid_index] - wind_var_map_->values[origin_grid_index])* wind_alpha < threshold)
			&& !is_selected[top_grid_index] && is_candidate_selected_[top_grid_index] == 0 ){

			current_index++;
			reserved_vec[current_index] = QPoint(temp_point.x(), temp_point.y() + 1);
			is_candidate_selected_[top_grid_index] = 1;
		}

		bottom_grid_index = current_grid_index - bias_map_->longitude_grid_number;
		if ( temp_point.y() - 1 >= 0
			&& (abs(bias_map_->values[bottom_grid_index] - bias_map_->values[origin_grid_index]) * bias_alpha + abs(wind_var_map_->values[bottom_grid_index] - wind_var_map_->values[origin_grid_index])* wind_alpha < threshold)
			&& !is_selected[bottom_grid_index] && is_candidate_selected_[bottom_grid_index] == 0 ){

			current_index++;
			reserved_vec[current_index] = QPoint(temp_point.x(), temp_point.y() - 1);
			is_candidate_selected_[bottom_grid_index] = 1;
		}
	}
}

void WrfCentralViewer::UpdateSelectedBuffer(){
	float selected_color[4] = {0.3, 0.3, 0.3, 0.5};
	float compared_color[4] = {0.0, 1.0, 0.0, 0.5};
	if ( view_mode_ == HIGHLIGHT_MODE ){
		memset(selected_area_pixel_value_.data(), 0, selected_area_pixel_value_.size() * sizeof(float));
		for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ){
			if ( is_grid_high_light_[i] ){
				int temp_index = i * 4;
				selected_area_pixel_value_[temp_index] = selected_color[0];
				selected_area_pixel_value_[temp_index + 1] = selected_color[1];
				selected_area_pixel_value_[temp_index + 2] = selected_color[2];
				selected_area_pixel_value_[temp_index + 3] = selected_color[3];
			}
		}

		makeCurrent();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, selected_tex_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, selected_area_pixel_value_.data());
		glDisable(GL_TEXTURE_2D);
	} else if ( selection_mode_ == NORMAL_MODE ){
		memset(selected_area_pixel_value_.data(), 0, selected_area_pixel_value_.size() * sizeof(float));
		for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ){
			if ( is_grid_selected_[i] ){
				int temp_index = i * 4;
				selected_area_pixel_value_[temp_index] = selected_color[0];
				selected_area_pixel_value_[temp_index + 1] = selected_color[1];
				selected_area_pixel_value_[temp_index + 2] = selected_color[2];
				selected_area_pixel_value_[temp_index + 3] = selected_color[3];
			}
		}

		makeCurrent();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, selected_tex_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, selected_area_pixel_value_.data());
		glDisable(GL_TEXTURE_2D);
	} else if ( selection_mode_ == COMPARED_MODE ){
		memset(compared_area_pixel_values_.data(), 0, compared_area_pixel_values_.size() * sizeof(float));
		for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ){
			if ( is_grid_selected_[i] ){
				int temp_index = i * 4;
				compared_area_pixel_values_[temp_index] = selected_color[0];
				compared_area_pixel_values_[temp_index + 1] = selected_color[1];
				compared_area_pixel_values_[temp_index + 2] = selected_color[2];
				compared_area_pixel_values_[temp_index + 3] = selected_color[3];
			}
			if ( is_compared_selected_[i] ){
				int temp_index = i * 4;
				compared_area_pixel_values_[temp_index] = compared_color[0];
				compared_area_pixel_values_[temp_index + 1] = compared_color[1];
				compared_area_pixel_values_[temp_index + 2] = compared_color[2];
				compared_area_pixel_values_[temp_index + 3] = compared_color[3];
			}
		}

		makeCurrent();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, selected_tex_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, compared_area_pixel_values_.data());
		glDisable(GL_TEXTURE_2D);
	} 
}

void WrfCentralViewer::UpdateCandidateSelectedBuffer(){
	float candidate_color[4] = {0.5, 0.5, 0.5, 0.5};
	memset(candidate_area_pixel_value_.data(), 0, candidate_area_pixel_value_.size() * sizeof(float));
	for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ){
		if ( is_candidate_selected_[i] == 2 ){
			int temp_index = i * 4;
			candidate_area_pixel_value_[temp_index] = candidate_color[0];
			candidate_area_pixel_value_[temp_index + 1] = candidate_color[1];
			candidate_area_pixel_value_[temp_index + 2] = candidate_color[2];
			candidate_area_pixel_value_[temp_index + 3] = candidate_color[3];
		}
	}

	makeCurrent();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, candidate_tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, longitude_grid_number_, latitude_grid_number_, 0, GL_RGBA, GL_FLOAT, candidate_area_pixel_value_.data());
	glDisable(GL_TEXTURE_2D);
}

void WrfCentralViewer::LoadMapData(){
	std::ifstream map_input("./MapData/new_map.dat");
	
	if ( map_input.good() ){
		std::string header_info;
		getline(map_input, header_info);
		std::string test;
		getline(map_input, test);

		int point_number;
		int para[8];
		while ( !map_input.eof() ){
			map_input >> point_number >> para[0] >> para[1] >> para[2] >> para[3] >> para[4] >> para[5] >> para[6];
			std::vector< float > point_vec;
			point_vec.resize(point_number * 2);
			for ( int i = 0; i < point_number; ++i ){
				map_input >> point_vec[2 * i] >> point_vec[2 * i + 1];
			}
			map_data_.push_back(point_vec);
		}
		map_input.close();
	} else {
		std::cout << "Load map data error!" << std::endl;
	}
}

//paint isolines @gcdofree
void WrfCentralViewer::PaintIsoLines(){

	glColor3f(0.0, 0.0, 0.0);

	for(int i = 0; i < isolineList.size(); i++){
		for(int j = 0; j < isolineList[i].size(); j++){
			glBegin(GL_LINE_LOOP);
			list<isotools::Point2D>::iterator iter;
			for(iter = isolineList[i][j].points.begin(); iter != isolineList[i][j].points.end(); iter++){
				glVertex3f((*iter).x, (*iter).y, 0.0f);
			}
			glEnd();
		}
	}
}

void WrfCentralViewer::PaintSelectIsoLine(){

	if(!is_edit_isoline || edit_isoline_index == -1)
		return;

	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_LINE_LOOP);
	list<isotools::Point2D>::iterator iter;
	for(iter = isolineList[edit_isoline_index][edit_isoline_detail].points.begin(); iter != isolineList[edit_isoline_index][edit_isoline_detail].points.end(); iter++){
		glVertex3f((*iter).x, (*iter).y, 0.0f);
	}
	glEnd();
}

void WrfCentralViewer::PaintEditIsoLine(){
	if(!is_edit_isoline || !is_draw_edit_line)
		return;

	//draw line
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < editList.size(); i++){
		glVertex3f(editList[i].x, editList[i].y, 0.0f);
	}
	glEnd();

	//draw mark
	float offset = 0.1f;
	for(int i = 0; i < editList.size(); i++){
		glBegin(GL_LINES);
		glVertex3f(editList[i].x - offset, editList[i].y - offset, 0.0f);
		glVertex3f(editList[i].x + offset, editList[i].y + offset, 0.0f);
		glVertex3f(editList[i].x - offset, editList[i].y + offset, 0.0f);
		glVertex3f(editList[i].x + offset, editList[i].y - offset, 0.0f);
		glEnd();
	}

}

//generate isolines @gcdofree
void WrfCentralViewer::GenerateIsoLines(){

	isolineCreator.readGridData(grid_value_map_->start_longitude, grid_value_map_->longitude_grid_space, grid_value_map_->latitude_grid_number,
		grid_value_map_->start_latitude, grid_value_map_->latitude_grid_space, grid_value_map_->longitude_grid_number, 0, 0.2, grid_value_map_->values.data());

	isolineValueVector.clear();
	isolineList.clear();

	isolineCreator.generateContourLine(isolineValueVector, isolineList);
}

float WrfCentralViewer::GetEulerDistance(float x1, float y1, float x2, float y2){
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

void WrfCentralViewer::getInterpolationResult(vector<float> originData, vector<float> &resultData, vector<float> originIndex, vector<float> resultIndex){
	Type xi[100], yi[100];
	for (int i = 0; i < originData.size(); i++)
	{
		xi[i] = originIndex[i];
		yi[i] = originData[i];
	}

	Type Ml = 0, Mr = 0;
	Vector<Type> x( originData.size(), xi ), y( originData.size(), yi );

	Spline3Interp<Type> poly( x, y, Ml, Mr );
	poly.calcCoefs();

	for (int i = 0; i < resultIndex.size(); i++)
	{
		resultData.push_back(poly.evaluate(resultIndex[i]));
	}
}

void WrfCentralViewer::StartEditIsolines(){
	is_edit_isoline = true;
	setMouseTracking(true);
}

void WrfCentralViewer::EndEditIsolines(){
	is_edit_isoline = false;
	setMouseTracking(false);
}

void WrfCentralViewer::GetGridIndex(float longitude, float latitude, int& grid_x, int& grid_y){
	grid_x = (int)((longitude - bias_map_->start_longitude) / bias_map_->longitude_grid_space + 0.5);
	if ( grid_x < 0 || grid_x >= bias_map_->longitude_grid_number ) grid_x = -1;
	grid_y = (int)((latitude - bias_map_->start_latitude) / bias_map_->latitude_grid_space + 0.5);
	if ( grid_y < 0 || grid_y >= bias_map_->latitude_grid_number ) grid_y = -1;
}

void WrfCentralViewer::GetGridIndexByWindowPos(int pos_x, int pos_y, int& grid_x, int& grid_y){
	float longitude = ((float)pos_x / this->width() * (right_ - left_) + left_ + x_bias_) / scale_;
	float latitude = ((1.0 - (float)pos_y / this->height()) * (top_ - bottom_) + bottom_ + y_bias_) / scale_;
	grid_x = (int)((longitude - bias_map_->start_longitude) / bias_map_->longitude_grid_space + 0.5);
	if ( grid_x < 0 || grid_x >= bias_map_->longitude_grid_number ) grid_x = -1;
	grid_y = (int)((latitude - bias_map_->start_latitude) / bias_map_->latitude_grid_space + 0.5);
	if ( grid_y < 0 || grid_y >= bias_map_->latitude_grid_number ) grid_y = -1;
}

void WrfCentralViewer::OnHighLightChanged(){
	if ( view_mode_ != HIGHLIGHT_MODE ) previous_mode_ = view_mode_;

	view_mode_ = HIGHLIGHT_MODE;

	WrfDataManager::GetInstance()->GetHighLightIndex(is_grid_high_light_);

	UpdateSelectedBuffer();

	this->updateGL();
}

void WrfCentralViewer::OnHighLightOff(){
	view_mode_ = previous_mode_;

	UpdateSelectedBuffer();

	this->updateGL();
}