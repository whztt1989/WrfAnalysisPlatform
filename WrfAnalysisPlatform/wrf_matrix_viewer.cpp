#include "wrf_matrix_viewer.h"
#include "qcolor_bar_controller.h"

WrfMatrixViewer::WrfMatrixViewer(){
	data_ = NULL;

	this->setMinimumSize(300, 300);
}

WrfMatrixViewer::~WrfMatrixViewer(){

}

void WrfMatrixViewer::set_data(WrfMatrixChartDataSet* data){
	data_ = data;

	matrix_mapping_.resize(data->values.size());
	for ( int i = 0; i < matrix_mapping_.size(); ++i ) matrix_mapping_[i] = i;

	this->updateGL();
}

void WrfMatrixViewer::GetResidualIndex(std::vector< int >& residual_index){
	residual_index.clear();

	for ( int i = 0; i < matrix_mapping_.size(); ++i )
		if ( matrix_mapping_[i] != -1 ) residual_index.push_back(matrix_mapping_[i]);
}

void WrfMatrixViewer::initializeGL(){
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfMatrixViewer::resizeGL(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -1.0);
}

void WrfMatrixViewer::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT);

	if ( data_ == NULL ) return;

	float boarder_scale = 0.05;
	float text_scale = 0.1;
	float interval_scale = 0.05;
	float matrix_size = 1.0 - 2 * boarder_scale - 2 * text_scale;
	float node_interval = 0.01;
	float text_pixel_per_char = 6;

	float x_scale, y_scale;
	int min_size;
	if ( this->width() < this->height() ){
		x_scale = 1.0;
		y_scale = (float)this->width() / this->height();
		min_size = this->width();
	} else {
		x_scale = (float)this->height() / this->width();
		y_scale = 1.0;
		min_size = this->height();
	}
	// paint coordinate name
	glColor3f(0.0, 0.0, 0.0);
	QString header_text = QString("Matrix Chart");
	int header_x_pos = 0.5 * this->width() - text_pixel_per_char * header_text.length() / 2;
	int header_y_pos = 0.5 * this->height() - 0.4 * min_size;
	this->renderText(header_x_pos, header_y_pos, header_text);

	QString y_coor_text = QString::fromLocal8Bit(data_->y_coor_name.c_str());
	int y_name_x_pos = 0.5 * this->width() - 0.35 * min_size - text_pixel_per_char * y_coor_text.length();
	int y_name_y_pos = 0.5 * this->height();
	this->renderText(y_name_x_pos, y_name_y_pos, y_coor_text);

	QString x_coor_text = QString::fromLocal8Bit(data_->x_coor_name.c_str());
	int x_name_x_pos = 0.5 * this->width() - text_pixel_per_char * x_coor_text.length();
	int x_name_y_pos = 0.5 * this->height() + 0.4 * min_size;
	this->renderText(x_name_x_pos, x_name_y_pos, x_coor_text);

	// paint matrix node
	int temp_index = 0;
	float x_step = matrix_size / data_->x_size;
	float y_step = matrix_size / data_->y_size;
	float step = x_step;
	if ( step > y_step ) step = y_step;
	for ( int i = 0; i < data_->y_size; ++i )
		for ( int j = 0; j < data_->x_size; ++j ){
			if ( matrix_mapping_[temp_index] == -1 ) continue;
			float x = 0.5 - (data_->x_size / 2.0 - j) * step * x_scale;
			float y = 0.5 - (data_->y_size / 2.0 - i) * step * y_scale ;

			QColor color = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0.0, 1.0, data_->values[matrix_mapping_[temp_index]].value);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			
			float x1 = x + (step - node_interval) * x_scale;
			float y1 = y + (step - node_interval) * y_scale;
			glRectf(x, y, x1, y1);
		}

	// paint color bar
	float bar_x_pos = 0.5 + 0.4 * x_scale;
	float bar_x1_pos = 0.5 + 0.4 * x_scale + 10.0 / this->width();
	float bar_y_pos = 0.5 - 0.35 * y_scale;
	float value = 0.0;
	float value_step = 1.0 / 25;
	float pos_step = 0.7 * y_scale / 25;
	QColor color = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0.0, 1.0, value);
	for ( int i = 0; i < 25; ++i ){
		float next_value = value + value_step;
		QColor next_color = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0.0, 1.0, next_value);

		glBegin(GL_TRIANGLES);
			glColor3f(next_color.redF(), next_color.greenF(), next_color.blueF());
			glVertex3f(bar_x_pos, bar_y_pos + pos_step, 0.0);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			glVertex3f(bar_x_pos, bar_y_pos, 0.0);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			glVertex3f(bar_x1_pos, bar_y_pos, 0.0);

			glColor3f(next_color.redF(), next_color.greenF(), next_color.blueF());
			glVertex3f(bar_x_pos, bar_y_pos + pos_step, 0.0);
			glColor3f(color.redF(), color.greenF(), color.blueF());
			glVertex3f(bar_x1_pos, bar_y_pos, 0.0);
			glColor3f(next_color.redF(), next_color.greenF(), next_color.blueF());
			glVertex3f(bar_x1_pos, bar_y_pos + pos_step, 0.0);
		glEnd();

		color = next_color;
		value = next_value;
		bar_y_pos += pos_step;
	}
}