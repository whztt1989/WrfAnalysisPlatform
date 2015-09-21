#include "scatter_plot.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QToolTip>

ScatterPlot::ScatterPlot(){
    axis_names_[0] = "x";
    axis_names_[1] = "y";
    axis_ranges_[0][0] = 0;
    axis_ranges_[0][1] = 1;
    axis_ranges_[1][0] = 0;
    axis_ranges_[1][1] = 1;

    view_border_ = 10;
    text_size_ = 30;
	is_floating_ = false;

	this->setFixedSize(500, 450);

	setMouseTracking(true);
	setAutoFillBackground(false);
}

ScatterPlot::~ScatterPlot(){

}

void ScatterPlot::SetData(std::vector< float >& values){
    data_values_.assign(values.begin(), values.end());

    UpdateViewData();
}

void ScatterPlot::SetAxisValueRange(float x_min, float x_max, float y_min, float y_max){
    axis_ranges_[0][0] = x_min;
    axis_ranges_[0][1] = x_max;
    axis_ranges_[1][0] = y_min;
    axis_ranges_[1][1] = y_max;

    UpdateViewData();

    this->updateGL();
}

void ScatterPlot::SetAxisNames(std::string& x_name, std::string& y_name){
    axis_names_[0] = x_name;
    axis_names_[1] = y_name;

    this->updateGL();
}

void ScatterPlot::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        std::cout << "Error initialize threshold adjusting chart!" << std::endl;
        return;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void ScatterPlot::resizeGL(int w, int h){
    view_size_ = this->height() * 0.9;
    if ( view_size_ > this->width() * 0.9 ) view_size_ = this->width() * 0.9;
    view_left_ = this->width() / 2 - view_size_ / 2;
    view_top_ = this->height() / 2 - view_size_ / 2;

    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2);

    glClear(GL_COLOR_BUFFER_BIT);
}

void ScatterPlot::paintEvent(QPaintEvent* event){
	makeCurrent();

    glViewport(view_left_, view_top_, view_size_, view_size_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, view_size_, 0, view_size_, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2);
    glClear(GL_COLOR_BUFFER_BIT);

    // paint the axis
    float axis_min = view_border_ + text_size_;
    float axis_max = view_size_ - view_border_;
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(axis_min, axis_min, 0);
    glVertex3f(axis_max, axis_min, 0);

    glVertex3f(axis_min, axis_min, 0);
    glVertex3f(axis_min, axis_max, 0);
    glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3f(axis_max, axis_min, 0);
	glVertex3f(axis_max - 5, axis_min - 3, 0);
	glVertex3f(axis_max - 5, axis_min + 3, 0);

	glVertex3f(axis_min, axis_max, 0);
	glVertex3f(axis_min - 3, axis_max - 5, 0);
	glVertex3f(axis_min + 3, axis_max - 5, 0);
	glEnd();

    glLineStipple(1, 0x00FF);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    glVertex3f(axis_min, axis_min, 0);
    glVertex3f(axis_max, axis_max, 0);

	if ( is_floating_ ){
		glColor3f(1.0, 0.6, 0.38);
		glVertex3f(axis_min, this->height() - mouse_pos_.y() - view_top_, 0);
		glVertex3f(axis_max, this->height() - mouse_pos_.y() - view_top_, 0);

		glVertex3f(mouse_pos_.x() - view_left_, axis_min, 0);
		glVertex3f(mouse_pos_.x() - view_left_, axis_max, 0);
	}
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // paint points
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(2.0);
    float region_width = axis_max - axis_min;
    glBegin(GL_POINTS);
    for ( int i = 0; i < point_pos_.size() / 2; ++i )
        glVertex3f(point_pos_[2 * i] * region_width + axis_min, point_pos_[2 * i + 1] * region_width + axis_min, 0);
    glEnd();

	// paint text
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::black);
	
	// axis value
	QFont normal_font;
	normal_font.setFamily("arial");
	normal_font.setBold(false);
	normal_font.setPixelSize(10);
	painter.setFont(normal_font);
	painter.drawText(QRectF(view_left_ + axis_min - 20, this->height() - text_size_ - view_border_ - view_top_, 40, text_size_), Qt::AlignCenter, QString::number(axis_ranges_[0][0]));
	painter.drawText(QRectF(view_left_ + axis_max - 20, this->height() - text_size_ - view_border_ - view_top_, 40, text_size_), Qt::AlignCenter, QString::number(axis_ranges_[0][1]));
	painter.drawText(QRectF(view_left_ + axis_min - text_size_, this->height() - text_size_ - view_border_ - 20 - view_top_, text_size_, 40), Qt::AlignCenter, QString::number(axis_ranges_[1][0]));
	painter.drawText(QRectF(view_left_ + axis_min - text_size_, view_top_ + view_border_ - 20, text_size_, 40), Qt::AlignCenter, QString::number(axis_ranges_[1][1]));

	// axis name
	normal_font.setBold(true);
	normal_font.setPixelSize(13);
	painter.setFont(normal_font);
	painter.drawText(QRectF(view_left_ + axis_min, this->height() - text_size_ - view_border_ - view_top_, axis_max - axis_min, text_size_), Qt::AlignCenter, QString::fromLocal8Bit(axis_names_[0].c_str()));
	painter.translate(view_left_ + axis_min, this->height() - text_size_ - view_border_ - view_top_);
	painter.rotate(-90);
	painter.drawText(QRectF(0, -1 * text_size_, axis_max - axis_min, text_size_), Qt::AlignCenter, QString::fromLocal8Bit(axis_names_[1].c_str()));
	painter.rotate(90);
	painter.translate(-1 * (view_left_ + axis_min), -1 * (this->height() - text_size_ - view_border_ - view_top_));

	painter.end();
}

void ScatterPlot::mouseMoveEvent(QMouseEvent *event){
	if ( event->pos().x() > view_left_ + view_border_ + text_size_ && event->pos().x() < view_left_ + view_size_ + text_size_ 
		&& event->pos().y() > view_top_ + view_border_ && event->pos().y() < (this->height() - view_top_  - text_size_ - view_border_) ){
			is_floating_ = true;
			mouse_pos_ = event->pos();
			this->update();

			float region_size = view_size_ - 2 * view_border_ - text_size_;
			float x = (event->pos().x() - view_left_ - view_border_ - text_size_) / region_size * (axis_ranges_[0][1] - axis_ranges_[0][0]) + axis_ranges_[0][0];
			float y = (this->height() - event->pos().y() - view_top_ - view_border_ - text_size_) / region_size * (axis_ranges_[1][1] - axis_ranges_[1][0]) + axis_ranges_[1][0];
			QToolTip::showText(event->globalPos(), QString("X Value: %0   Y Value: %1").arg(x).arg(y));
	} else {
		if ( is_floating_ ){
			is_floating_ = false;

			this->update();
		}
	}
}

void ScatterPlot::UpdateViewData(){
    point_pos_.resize(data_values_.size());
        
    for ( int i = 0; i < data_values_.size() / 2; ++i ){
        point_pos_[2 * i] = (data_values_[2 * i] - axis_ranges_[0][0]) / (axis_ranges_[0][1] - axis_ranges_[0][0]);
        point_pos_[2 * i + 1] = (data_values_[2 * i + 1] - axis_ranges_[1][0]) / (axis_ranges_[1][1] - axis_ranges_[1][0]);
    }
}