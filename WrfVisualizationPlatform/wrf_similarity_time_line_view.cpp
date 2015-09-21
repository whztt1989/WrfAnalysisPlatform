#include "wrf_similarity_time_line_view.h"
#include <QtGui/QMouseEvent>
#include <QtCore/QDateTime>
#include <QtCore/QDate>
#include <QtGui/QToolTip>
#include <QtCore/QLocale>
#include "color_mapping_generator.h"

WrfSimilarityTimeLineView::WrfSimilarityTimeLineView()
    : is_initialized_(false), current_selected_day_index_(-9999), current_selected_year_index_(-1){
	setMouseTracking(true);

	border = 10;
	title_height = 30;
	margin = 10;
	year_text_height = 20;
	color_bar_width = 50;
	date_index_bar_width = 20;

    bar_element_ = new WrfColorBarElement(SIMILARITY_MAPPING);

	setAutoFillBackground(false);
}

WrfSimilarityTimeLineView::~WrfSimilarityTimeLineView(){

}

void WrfSimilarityTimeLineView::SetData(std::vector< std::vector< float > >& similarity_values, std::vector< std::vector< int > >& time_values, std::vector< std::string >& years){
    data_values_.assign(similarity_values.begin(), similarity_values.end());
	date_time_ = time_values;
    year_strings_.assign(years.begin(), years.end());

    if ( is_initialized_ ) this->update();
}

void WrfSimilarityTimeLineView::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        std::cout << "Error initialize similarity time line view!" << std::endl;
        return;
    }
    is_initialized_ = true;
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfSimilarityTimeLineView::resizeGL(int w, int h){
    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 0, 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -1.0);
}

void WrfSimilarityTimeLineView::paintEvent(QPaintEvent* event){
	makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT);

    if ( data_values_.size() == 0 || year_strings_.size() == 0 ) return;

	// paint time line view
	int day_count = data_values_[1].size();
	y_step = (this->height() - 2 * border - title_height - year_text_height - margin) / day_count;
	int bottom_y = this->height() - (border + title_height + year_text_height + margin + y_step * day_count);
	x_step = (this->width() - 2 * border - color_bar_width - date_index_bar_width - 2 * margin) / year_strings_.size();
	if ( y_step > 30 ) y_step = 30;

	float time_line_x, time_line_y;
	for ( int i = 0; i < data_values_.size(); ++i ){
		glLineWidth(1.0);
		time_line_x = border + (year_strings_.size() - 1 - i) * x_step;
		for ( int j = 0; j < data_values_[i].size(); ++j ){
			time_line_y = this->height() - (border + title_height + margin + year_text_height + y_step * j);
			if ( data_values_[i][j] >= 0 ){
				QColor color = ColorMappingGenerator::GetInstance()->GetColor(SIMILARITY_MAPPING, 0, 99, data_values_[i][j]);
				glColor3f(color.redF(), color.greenF(), color.blueF());
				glRectf(time_line_x, time_line_y, time_line_x + x_step - 1, time_line_y - y_step);
			} else {
				glColor3f(0.7, 0.7, 0.7);
				glBegin(GL_LINES);
				glVertex3f(time_line_x, time_line_y, 0);
				glVertex3f(time_line_x + x_step, time_line_y - y_step, 0);
				glVertex3f(time_line_x + x_step, time_line_y, 0);
				glVertex3f(time_line_x, time_line_y - y_step, 0);
				glEnd();
			}

			glColor3f(0.7, 0.7, 0.7);
			glBegin(GL_LINE_LOOP);
			glVertex3f(time_line_x, time_line_y, 0);
			glVertex3f(time_line_x + x_step, time_line_y, 0);
			glVertex3f(time_line_x + x_step, time_line_y - y_step, 0);
			glVertex3f(time_line_x, time_line_y - y_step, 0);
			glEnd();
		}

		if ( i == 0 ){
			glColor3f(0.823, 0.333, 0.823);
			glLineWidth(2.0);
			time_line_y -= y_step;
			time_line_x = border + (year_strings_.size() - 1 - i) * x_step;
			glBegin(GL_QUADS);
			glVertex3f(time_line_x, time_line_y, 0);
			glVertex3f(time_line_x + x_step, time_line_y, 0);
			glVertex3f(time_line_x + x_step, time_line_y - y_step, 0);
			glVertex3f(time_line_x, time_line_y - y_step, 0);
			glEnd();
		}
	}

	// date index bar
	glColor3f(0.4, 0.4, 0.4);
	glLineWidth(3.0);
	glBegin(GL_LINES);
	glVertex3f(this->width() - border - color_bar_width - date_index_bar_width - margin, bottom_y, 0);
	glVertex3f(this->width() - border - color_bar_width - date_index_bar_width - margin, this->height() - border - title_height - year_text_height - margin, 0);
	glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3f(this->width() - border - color_bar_width - date_index_bar_width - margin, bottom_y - 8, 0);
    glVertex3f(this->width() - border - color_bar_width - date_index_bar_width - margin - 5, bottom_y, 0);
    glVertex3f(this->width() - border - color_bar_width - date_index_bar_width - margin + 5, bottom_y, 0);
    glEnd();

	// current selected index

	if ( current_selected_day_index_ != -9999 ){
		glLineWidth(2.0);
		float temp_y = this->height() - (border + title_height + margin + year_text_height + y_step * current_selected_day_index_);
		glColor3f(0.823, 0.333, 0.823);
		glLineWidth(2.0);
		glBegin(GL_LINE_LOOP);
		glVertex3f(border, temp_y, 0);
		glVertex3f(this->width() - border - margin - color_bar_width - date_index_bar_width, temp_y, 0);
		glVertex3f(this->width() - border - 2 * margin - color_bar_width - date_index_bar_width, temp_y - y_step, 0);
		glVertex3f(border, temp_y - y_step, 0);
		glEnd();
	}

    // color bar
	glLineWidth(1.0);
	
    bar_element_->Render(this->width() - color_bar_width - border, this->width() - border, bottom_y + 20, this->height() - border - title_height - margin - year_text_height - 20);

    /*glBegin(GL_QUADS);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f(this->width() - color_bar_width - border, bottom_y + 20, 0);
    glVertex3f(this->width() - color_bar_width - border + 30, bottom_y + 20, 0);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(this->width() - color_bar_width - border + 30, this->height() - border - title_height - margin - year_text_height - 20, 0);
    glVertex3f(this->width() - color_bar_width - border, this->height() - border - title_height - margin - year_text_height - 20, 0);
    glEnd();
    glColor3f(0.7, 0.7, 0.7);
    glBegin(GL_LINE_LOOP);
    glVertex3f(this->width() - color_bar_width - border, bottom_y + 20, 0);
    glVertex3f(this->width() - color_bar_width - border + 30, bottom_y + 20, 0);
    glVertex3f(this->width() - color_bar_width - border + 30, this->height() - border - title_height - margin - year_text_height - 20, 0);
    glVertex3f(this->width() - color_bar_width - border, this->height() - border - title_height - margin - year_text_height - 20, 0);
    glEnd();*/
	

	QFont normal_font;
	normal_font.setFamily("arial");
	normal_font.setBold(false);
	normal_font.setPixelSize(10);

	QFont title_font;
	title_font.setFamily("arial");
	title_font.setBold(true);
	title_font.setPixelSize(16);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::black);

	// paint title
	painter.setFont(title_font);
	painter.drawText(QRect(0, border, this->width(), title_height), Qt::AlignCenter, QString("Time Similarity"));

    // paint year text
    painter.setFont(normal_font);
    for (int i = 0; i < year_strings_.size(); ++i ){
        float text_x = border + (year_strings_.size() - 1 - i) * x_step;
		painter.drawText(QRectF(text_x, border + title_height + margin, x_step, year_text_height), Qt::AlignCenter, QString::fromLocal8Bit(year_strings_[i].c_str()).right(2));
    }

    // paint color bar
	//painter.drawText(QRect(this->width() - border - color_bar_width + 35, this->height() - (bottom_y + 50), 30, 30), Qt::AlignHCenter | Qt::AlignBottom, QString("0.0"));
	//painter.drawText(QRect(this->width() - border - color_bar_width + 35, title_height + border + year_text_height + margin + 20, 30, 30), Qt::AlignHCenter | Qt::AlignTop, QString("1.0"));
    bar_element_->Render(painter, this->width() - border - color_bar_width, this->width() - border, this->height() - (bottom_y + 20), border + title_height + margin + year_text_height + 20);

	// paint current index
	if ( current_selected_day_index_ != -9999 ){
		float temp_y = border + title_height + margin + year_text_height + y_step * current_selected_day_index_;
		painter.drawText(QRectF(this->width() - border - margin - color_bar_width - date_index_bar_width + 5, temp_y - 20, 40, 40), Qt::AlignLeft | Qt::AlignVCenter, QString("%0").arg(current_selected_day_index_ - day_count / 2));
	}

	painter.end();
}

void WrfSimilarityTimeLineView::mouseMoveEvent(QMouseEvent *event){
    int day_count = data_values_[1].size();
    int bottom_y = this->height() - (border + title_height + year_text_height + margin + y_step * day_count);

	if ( (event->pos().x() - border) * (event->pos().x() - (this->width() - 2 * margin - border - color_bar_width - date_index_bar_width)) < 0 
		&& (event->pos().y() - (this->height() - bottom_y)) * (event->pos().y() - border - title_height - year_text_height - margin) < 0 ){
			int temp_index = (event->pos().y() - border - title_height - year_text_height - margin) / y_step;
			int temp_year_index = year_strings_.size() - 1 - (event->pos().x() - border) / x_step;
			if ( temp_index != current_selected_day_index_ || temp_year_index != current_selected_year_index_){
				current_selected_day_index_ = temp_index;
				current_selected_year_index_ = temp_year_index;

				if ( current_selected_year_index_ >= 0 && current_selected_year_index_ < date_time_.size() 
					&& current_selected_day_index_ >= 0 && current_selected_day_index_ < date_time_[current_selected_year_index_].size() ){
					QDateTime base_date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0));
					qint64 temp = date_time_[current_selected_year_index_][current_selected_day_index_];
					temp *= 3600000;
					QDateTime history_date = base_date.addMSecs(temp);

					QToolTip::showText(event->globalPos(), QString("Event Time: %0").arg(QLocale(QLocale::C).toString(history_date, QString("yyyy MMM dd"))));
					this->update();
				}
			}
	} else {
		int temp_index = -9999;
		int temp_year_index = -9999;
		if ( temp_index != current_selected_day_index_ ){
			current_selected_day_index_ = temp_index;
			current_selected_year_index_ = temp_year_index;
			this->update();
		}
	}
}

void WrfSimilarityTimeLineView::mouseDoubleClickEvent(QMouseEvent *event){
	int day_count = data_values_[1].size();
	int bottom_y = this->height() - (border + title_height + year_text_height + margin + y_step * day_count);
	if ( (event->pos().x() - border) * (event->pos().x() - (this->width() - 2 * margin - border - color_bar_width - date_index_bar_width)) < 0 
		&& (event->pos().y() - (this->height() - bottom_y)) * (event->pos().y() - border - title_height - year_text_height - margin) < 0 ){
			int temp_index = (event->pos().y() - border - title_height - year_text_height - margin) / y_step;
			int temp_year_index = year_strings_.size() - 1 - (event->pos().x() - border) / x_step;
			current_selected_day_index_ = temp_index;
			current_selected_year_index_ = temp_year_index;

			if ( current_selected_year_index_ >= 0 && current_selected_year_index_ < date_time_.size() 
				&& current_selected_day_index_ >= 0 && current_selected_day_index_ < date_time_[current_selected_year_index_].size() ){
					emit EventTimeTriggered(date_time_[current_selected_year_index_][current_selected_day_index_]);
			}
	} 
}