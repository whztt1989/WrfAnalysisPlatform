#include "wrf_subregion_rms_view.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QToolTip>
#include "color_mapping_generator.h"
#include "wrf_statistic_solver.h"

WrfSubregionRmsView::WrfSubregionRmsView()
    : max_viewing_num_(-1), focus_node_index_(-1), focus_record_index_(-1){

    this->setFocusPolicy(Qt::StrongFocus);
    this->setMouseTracking(true);
    setAutoFillBackground(false);

    border_size_ = 10;
	suggestion_text_width_ = 40;
	coor_indicator_width_ = 5;
	y_value_text_width_ = 40;

    title_height_ = 30;
	node_height_ = 15;
	x_coor_text_height_ = 10;
	
	view_status_ = NORMAL_STATUS;
}

WrfSubregionRmsView::~WrfSubregionRmsView(){

}

void WrfSubregionRmsView::SetData(std::vector< std::vector< float > >& data, std::vector< std::vector< bool > >& is_data_selected, std::vector< std::vector< int > >& suggestion_values, std::vector< int >& selected_values){
    data_values_ = data;
	is_data_selected_ = is_data_selected;
	suggestion_values_ = suggestion_values;
    selected_value_ = selected_values;

    this->update();
}

void WrfSubregionRmsView::SetMaximumViewingNum(int max_num){
    max_viewing_num_ = max_num;

    this->update();
}

void WrfSubregionRmsView::SetHighlightRecordIndex(int node_index, int record_index){
    if ( record_index == -1 || node_index == -1 ){
        focus_node_index_ = -1;
        focus_record_index_ = -1;
    } else {
        focus_node_index_ = node_index;
        focus_record_index_ = record_index;
    }
    
    this->update();
}

void WrfSubregionRmsView::SetRmsUnits(std::vector< RmsUnit >& units){
	rms_units_ = units;
}

void WrfSubregionRmsView::SetViewMode(int mode){
	if ( mode == 0 ){
		if ( view_status_ & FINAL_VIEW ) view_status_ ^= FINAL_VIEW;
	} else {
		view_status_ |= FINAL_VIEW;
	}

	this->update();
}


void WrfSubregionRmsView::GetSelectionValues(std::vector< int >& n_value){
    n_value = selected_value_;
}

void WrfSubregionRmsView::initializeGL(){
    if ( glewInit() != GLEW_OK ) exit(0);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfSubregionRmsView::resizeGL(int w, int h){
    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -4);
}

void WrfSubregionRmsView::paintEvent(QPaintEvent* event){
    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT);

    if ( data_values_.size() == 0 ) return;

    // initial rendering parameters
    left_ = border_size_ + suggestion_text_width_ + coor_indicator_width_ + y_value_text_width_;
    x_step_ = (float)(this->width() - border_size_ - left_) / data_values_.size();
    bottom_ = border_size_ + x_coor_text_height_ + node_height_;
    y_step_ = (float)(this->height() - border_size_ - title_height_ - bottom_) / max_viewing_num_;

    node_margin_ = 1.0;
    if ( x_step_ < 6 ) node_margin_ = 0;

	if ( !(view_status_ & FINAL_VIEW) ){
		// render the pixel bar
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for ( int i = 0; i < data_values_.size(); ++i ){
			float temp_left = left_ + i * x_step_;
			float temp_bottom = bottom_;

			for ( int j = 0; j < max_viewing_num_; ++j ){
				QColor color = ColorMappingGenerator::GetInstance()->GetColor(RMS_MAPPING, data_values_[i][j], 0, 99);
				if ( is_data_selected_[i][j] || ((view_status_ & FLOATING_ON) && j >= selected_value_[i]) )
					glColor4f(color.redF(), color.greenF(), color.blueF(), 1.0);
				else
					glColor4f(color.redF(), color.greenF(), color.blueF(), 0.0);

				glRectf(temp_left, temp_bottom, temp_left + x_step_ - node_margin_, temp_bottom + y_step_);
				temp_bottom += y_step_;
			}
		}
		glDisable(GL_BLEND);

		// render the highlight record
		if ( focus_node_index_ != -1 && focus_record_index_ != -1 ){
			float temp_left = left_ + focus_node_index_ * x_step_;
			float temp_bottom = bottom_ + focus_record_index_ * y_step_;
			glColor3f(1.0, 0.6, 0.38);
			if ( y_step_ < 3 )
				glRectf(temp_left, temp_bottom, temp_left + x_step_ - node_margin_, temp_bottom + 3);
			else 
				glRectf(temp_left, temp_bottom, temp_left + x_step_ - node_margin_, temp_bottom + y_step_);
		}

		// render the selected value
		glColor3f(1.0, 0.6, 0.38);
		for ( int i = 0; i < data_values_.size(); ++i ){
			float temp_left = left_ + i * x_step_;
			float temp_bottom = bottom_ + selected_value_[i] * y_step_;
			if ( y_step_ < 3 )
				glRectf(temp_left, temp_bottom, temp_left + x_step_ - node_margin_, temp_bottom + 3);
			else 
				glRectf(temp_left, temp_bottom, temp_left + x_step_ - node_margin_, temp_bottom + y_step_);
		}

		// render the suggestion values
		if ( data_values_.size() < 10 ){
			glColor3f(0.0, 1.0, 0.0);
			glLineWidth(3.0);
			for ( int i = 0; i < suggestion_values_.size(); ++i ){
				glBegin(GL_LINES);
				for ( int j = 0; j < suggestion_values_[i].size(); ++j ){
					float temp_left = left_ + j * x_step_;
					float temp_bottom = bottom_ + suggestion_values_[i][j] * y_step_;

					glVertex3f(temp_left, temp_bottom, 0);
					glVertex3f(temp_left + x_step_ - node_margin_, temp_bottom + y_step_, 0);
				}
				glEnd();
			}
		}

		// render the selection path
		if ( selecting_path_.size() != 0 ){
			glColor3f(0.7, 0.7, 0.7);
			glLineWidth(3.0);
			glBegin(GL_LINE_STRIP);
			for ( int i = 0; i < selecting_path_.size(); ++i )
				glVertex3f(selecting_path_[i].x(), this->height() - selecting_path_[i].y(), 0);
			glEnd();
		}
	} else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for ( int i = 0; i < data_values_.size(); ++i ){
			float temp_left = left_ + i * x_step_;
			float temp_bottom = bottom_;

			for ( int j = 0; j < data_values_[i].size(); ++j ){
				if ( !is_data_selected_[i][j] ) continue;
				QColor color = ColorMappingGenerator::GetInstance()->GetColor(RMS_MAPPING, data_values_[i][j], 0, 99);
				glColor4f(color.redF(), color.greenF(), color.blueF(), 1.0);

				glRectf(temp_left, temp_bottom, temp_left + x_step_ - node_margin_, temp_bottom + y_step_);
				temp_bottom += y_step_;
			}
		}
		glDisable(GL_BLEND);
	}

    // render the coordinate
    glColor3f(0.4, 0.4, 0.4);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(left_ - coor_indicator_width_, bottom_, 0);
    glVertex3f(this->width() - border_size_, bottom_, 0);

	glVertex3f(left_ - coor_indicator_width_, bottom_, 0);
	glVertex3f(left_ - coor_indicator_width_, this->height() - border_size_ - title_height_, 0);
	
	int coor_indicator_step = 50;
	int coor_indicator_num = max_viewing_num_ / coor_indicator_step;
	while ( coor_indicator_num > 10 ){
		coor_indicator_step += 50;
		coor_indicator_num = max_viewing_num_ / coor_indicator_step;
	}
	float indicator_step_y = y_step_ * coor_indicator_step;
	for ( int i = 1; i <= coor_indicator_num; ++i ){
		glVertex3f(left_ - coor_indicator_width_, bottom_ + i * indicator_step_y, 0);
		glVertex3f(left_ - coor_indicator_width_ + 3, bottom_ + i * indicator_step_y, 0);
	}
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex3f(this->width() - border_size_ + 5, bottom_, 0);
    glVertex3f(this->width() - border_size_, bottom_ + 3, 0);
    glVertex3f(this->width() - border_size_, bottom_ - 3, 0);

	glVertex3f(left_ - coor_indicator_width_, this->height() - border_size_ - title_height_ + 5, 0);
	glVertex3f(left_ - 3 - coor_indicator_width_, this->height() - border_size_ - title_height_, 0);
	glVertex3f(left_ + 3 - coor_indicator_width_, this->height() - border_size_ - title_height_, 0);
    glEnd();

	// render the glyph
	indicator_radius_ = (int)(x_step_ * 0.25);
	if ( indicator_radius_ < 1 ) indicator_radius_ = 1;
	if ( indicator_radius_ > 5 ) indicator_radius_ = 5;
	for ( int i = 0; i < data_values_.size() && i < rms_units_.size(); ++i ){
		float center = left_ + x_step_ * (i + 0.5);
		glTranslatef(center, border_size_ + x_coor_text_height_ + node_height_ * 0.5, 0);
		if ( i == focus_node_index_ ) {
			glColor3f(1.0, 0.6, 0.38);
			float temp_left = -1 * x_step_ * 0.5;
			float temp_right = x_step_ * 0.5 - node_margin_;
			glBegin(GL_QUADS);
			glVertex3f(temp_left, -1 * indicator_radius_ - 2, 0);
			glVertex3f(temp_right, -1 * indicator_radius_ - 2, 0);
			glVertex3f(temp_right, indicator_radius_ + 2, 0);
			glVertex3f(temp_left, indicator_radius_ + 2, 0);
			glEnd();

			RenderGridInfoPie(i);
		}
		else
			RenderGridInfoPie(i);
		glTranslatef(-1 * center, -1 * (border_size_ + x_coor_text_height_ + node_height_ * 0.5), 0);
	}

	// render the text
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

    // paint the date value
    painter.setFont(normal_font);
    //painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, this->height() - bottom_ - 20, 35, 40), Qt::AlignVCenter | Qt::AlignRight, QString("0"));
    //QString max_text = QString("%0").arg(max_viewing_num_);
    //painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, border_size_ + title_height_ - 20, 35, 40), Qt::AlignVCenter | Qt::AlignRight, max_text);

	indicator_step_y = y_step_ * coor_indicator_step;
	for ( int i = 0; i <= coor_indicator_num; ++i ){
		painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, this->height() - bottom_ - i * indicator_step_y - 20, 35, 40), Qt::AlignVCenter | Qt::AlignRight, QString("%0").arg(coor_indicator_step * i));
	}

    normal_font.setBold(true);
    normal_font.setPixelSize(13);
	painter.setPen(Qt::black);
    painter.setFont(normal_font);
	painter.drawText(QRectF(left_ - 70, border_size_ + title_height_ - 30, 100, 20), Qt::AlignHCenter | Qt::AlignBottom, QString("Date Count"));

	painter.drawText(QRectF(left_, this->height() - border_size_ - x_coor_text_height_, this->width() - border_size_ - left_, x_coor_text_height_ + border_size_), Qt::AlignRight | Qt::AlignVCenter, QString("Small Regions"));

    painter.setFont(title_font);
    QString title = QString("RMS Differences of Aggregated Variables in Small Regions");
    painter.drawText(QRectF(0, 0, this->width(), title_height_), Qt::AlignCenter, title);


	// render the suggestion values
	if ( !(view_status_ & FINAL_VIEW) ){
		if ( data_values_.size() > 10 && suggestion_values_.size() > 0 ){
			float suggested_begin_y[2];
			for ( int k = 0; k < 2; ++k ){
				int control_node_num = suggestion_values_[k].size() / 5;
				if ( control_node_num > 10 ) control_node_num = 10;
				int point_per_node = suggestion_values_[k].size() / control_node_num;
				if ( suggestion_values_[k].size() % control_node_num != 0 ) control_node_num += 1;
				control_node_num += 1;
				std::vector< float > average_suggestions;
				average_suggestions.resize(control_node_num);
				for ( int i = 0; i < control_node_num - 1; ++i ){
					average_suggestions[i] = 0;
					int temp_count = 0;
					for ( int j = i * point_per_node; j < (i + 1) * point_per_node && j < data_values_.size(); ++j ){
						temp_count++;
						average_suggestions[i] += suggestion_values_[k][j];
					}
					if ( temp_count != 0 ) average_suggestions[i] /= temp_count;
				}
				average_suggestions[control_node_num - 1] = 0;
				for ( int i = 0; i < point_per_node; ++i ){
					average_suggestions[control_node_num - 1] += suggestion_values_[k][suggestion_values_[k].size() - 1 - i];
				}
				average_suggestions[control_node_num - 1] /= point_per_node;

				QPainterPath path;
				float x, y, next_x, next_y;
				path.moveTo(left_, this->height() - (bottom_ + average_suggestions[0] * y_step_));
				for ( int i = 0; i < average_suggestions.size() - 1; ++i ){
					x = left_ + i * point_per_node * x_step_;
					y = this->height() - (bottom_ + average_suggestions[i] * y_step_);

					int next_node = (i + 1) * point_per_node;
					if ( next_node >= suggestion_values_[k].size() ) next_node = suggestion_values_[k].size();
					next_x = left_ + next_node * x_step_;
					next_y = this->height() - (bottom_ + average_suggestions[i + 1] * y_step_);

					path.cubicTo((x + next_x) / 2, y, (x + next_x) / 2, next_y, next_x, next_y);
				}
				QPen pen;
				pen.setColor(QColor(160, 32, 240));
				pen.setWidth(4.0);
				pen.setStyle(Qt::DashDotLine);
				painter.setPen(pen);
				painter.drawPath(path);

				suggested_begin_y[k] = this->height() - (bottom_ + y_step_ * average_suggestions[0]);
			}

			/*float temp_bottom = suggested_begin_y[1];
			float temp_top = suggested_begin_y[0];
			float temp_center = (temp_top + temp_bottom) / 2;

			painter.drawLine(left_, temp_bottom, border_size_ + suggestion_text_width_, temp_bottom);
			painter.drawLine(border_size_ + suggestion_text_width_, temp_bottom, border_size_ + suggestion_text_width_ - 20, temp_center - 10);
			painter.drawLine(left_, temp_top, border_size_ + suggestion_text_width_, temp_top);
			painter.drawLine(border_size_ + suggestion_text_width_, temp_top, border_size_ + suggestion_text_width_ - 20, temp_center + 10);

			QPen normal_pen;
			normal_pen.setColor(Qt::black);
			normal_pen.setWidth(1.0);
			painter.setPen(normal_pen);

			normal_font.setBold(false);
			normal_font.setPixelSize(10);
			painter.setPen(Qt::black);
			painter.setFont(normal_font);

			painter.drawText(QRectF(border_size_, temp_center - 20, suggestion_text_width_, 40), Qt::AlignVCenter | Qt::AlignLeft, QString("Suggested\t\nRange"));*/
		} else if ( data_values_.size() <= 10 && suggestion_values_.size() > 0 ) {
			QPen pen;
			pen.setColor(QColor(160, 32, 240));
			pen.setWidth(4.0);
			pen.setStyle(Qt::DashDotLine);
			painter.setPen(pen);

			float temp_bottom = this->height() - (bottom_ + suggestion_values_[0][0] * y_step_);
			float temp_top = this->height() - (bottom_ + suggestion_values_[1][0] * y_step_);
			float temp_center = (temp_top + temp_bottom) / 2;

			painter.drawLine(left_, temp_bottom, border_size_ + suggestion_text_width_, temp_bottom);
			painter.drawLine(border_size_ + suggestion_text_width_, temp_bottom, border_size_ + suggestion_text_width_ - 20, temp_center + 10);
			painter.drawLine(left_, temp_top, border_size_ + suggestion_text_width_, temp_top);
			painter.drawLine(border_size_ + suggestion_text_width_, temp_top, border_size_ + suggestion_text_width_ - 20, temp_center - 10);

			QPen normal_pen;
			normal_pen.setColor(Qt::black);
			normal_pen.setWidth(1.0);
			painter.setPen(normal_pen);
			normal_font.setBold(false);
			normal_font.setPixelSize(10);
			painter.setPen(Qt::black);
			painter.setFont(normal_font);
			painter.drawText(QRectF(border_size_, temp_center - 20, suggestion_text_width_, 40), Qt::AlignVCenter | Qt::AlignLeft, QString("Suggested\t\nRange"));
		}
	} else {
		QPen pen;
		pen.setColor(QColor(160, 32, 240));
		pen.setWidth(4.0);
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);

		float temp_y = this->height() - (bottom_ + 20 * y_step_);

		painter.drawLine(left_, temp_y, this->width() - border_size_, temp_y);

		/*QPen normal_pen;
		normal_pen.setColor(Qt::black);
		normal_pen.setWidth(1.0);
		painter.setPen(normal_pen);
		normal_font.setBold(false);
		normal_font.setPixelSize(10);
		painter.setPen(Qt::black);
		painter.setFont(normal_font);
		painter.drawText(QRectF(border_size_, temp_y - 20, suggestion_text_width_, 40), Qt::AlignVCenter | Qt::AlignLeft, QString("Lower Bound"));*/
	}

	painter.setPen(Qt::black);
	normal_font.setBold(false);
	normal_font.setPixelSize(10);
	painter.setFont(normal_font);
	if ( !(view_status_ & FINAL_VIEW) )
		painter.drawText(QRectF(this->width() - border_size_ - 250, title_height_ - 2.5 * border_size_, 170, 10), Qt::AlignRight | Qt::AlignBottom, QString("Suggested Range: "));
	else
		painter.drawText(QRectF(this->width() - border_size_ - 250, title_height_ - 2.5 * border_size_, 170, 10), Qt::AlignRight | Qt::AlignBottom, QString("Lower Bound: "));

	QPen pen;
	pen.setColor(QColor(160, 32, 240));
	pen.setWidth(4.0);
	pen.setStyle(Qt::DashDotLine);
	painter.setPen(pen);
	painter.drawLine(this->width() - border_size_ - 70, title_height_ -  2 * border_size_,  this->width() - border_size_ - 30, title_height_ -  2 * border_size_);

    painter.end();

	if ( view_status_ & VIEW_DIRTY ) view_status_ ^= VIEW_DIRTY;
}

void WrfSubregionRmsView::RenderGridInfoPie(int index){

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float radius_scale = rms_units_[index].values.size() / 50.0;
	if ( radius_scale > 1.0 ) radius_scale = 1.0;
	float radius = indicator_radius_;
	const float PI = 3.14159265;
	float delta_theta = 2 * PI / rms_units_[index].values.size();
	float theta = 0;
	for ( int i = 0; i < rms_units_[index].values.size(); ++i ){
		QColor color = ColorMappingGenerator::GetInstance()->GetColor(RMS_MAPPING, rms_units_[index].values[i], 0, 99);
		RenderArc(color, radius, theta, theta + delta_theta);

		theta += delta_theta;
	}

	glDisable(GL_BLEND);
}

void WrfSubregionRmsView::RenderArc(QColor color, float radius, float begin_theta, float end_theta){
	int pie_count = (end_theta - begin_theta) / 0.2;

	while ( begin_theta + 0.2 < end_theta ){
		glColor4f(color.redF(), color.greenF(), color.blueF(), 1.0);
		glBegin(GL_TRIANGLES);
		glVertex3f(0, 0, 0);
		glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
		glVertex3f(radius * cos(begin_theta + 0.2), radius * sin(begin_theta + 0.2), 0.0);
		glEnd();

		begin_theta += 0.2;
	}
	glBegin(GL_TRIANGLES);
	glColor4f(color.redF(), color.greenF(), color.blueF(), 1.0);
	glVertex3f(0, 0, 0);
	glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
	glVertex3f(radius * cos(end_theta), radius * sin(end_theta), 0.0);
	glEnd();
}

void WrfSubregionRmsView::mousePressEvent(QMouseEvent *event){
    if ( event->buttons() & Qt::LeftButton ){
        selecting_path_.clear();
        selecting_path_.push_back(event->pos());
    }
}

void WrfSubregionRmsView::mouseMoveEvent(QMouseEvent *event){
    if ( event->x() > left_ && event->x() < this->width() - border_size_
        && this->height() - event->y() > bottom_ && event->y() > border_size_ + title_height_ ) {

        int temp_node_index = (event->x() - left_) / x_step_;
        if ( temp_node_index < 0 ) temp_node_index = 0;
        if ( temp_node_index >= this->data_values_.size() ) temp_node_index = this->data_values_.size() - 1;

        int temp_record_index = (this->height() - event->pos().y() - bottom_) / y_step_;
        if ( temp_record_index < 0 ) temp_record_index = 0;
        if ( temp_record_index >= this->data_values_[0].size() ) temp_record_index = this->data_values_[0].size() - 1;

        if( temp_node_index != focus_node_index_ || temp_record_index != focus_record_index_ ){
			focus_node_index_ = temp_node_index;
            focus_record_index_ = temp_record_index;

			emit CurrentRecordChanged(focus_node_index_, focus_record_index_);

			view_status_ |= VIEW_DIRTY;
        }

        float rms_value = this->data_values_[focus_node_index_][focus_record_index_];
        QToolTip::showText(event->globalPos(), QString("Analog Number: %0\t\nAggregated Rms Value: %1").arg(focus_record_index_ + 1).arg(rms_value));
        
        if ( !(view_status_ & FLOATING_ON) ){
            view_status_ |= FLOATING_ON;
            view_status_ |= VIEW_DIRTY;
        }

        if ( event->buttons() & Qt::LeftButton && view_status_ & BRUSHING_ON ) {
            selecting_path_.push_back(event->pos());
            view_status_ |= VIEW_DIRTY;
        }
    } else {
        if ( focus_node_index_ != -1 ){
            focus_node_index_ = -1;
			focus_record_index_ = -1;

            emit CurrentRecordChanged(focus_node_index_, focus_record_index_);

			view_status_ |= VIEW_DIRTY;
        }

        if ( view_status_ & FLOATING_ON ){
            view_status_ ^= FLOATING_ON;
            view_status_ |= VIEW_DIRTY;
        }
    }

    if ( view_status_ & VIEW_DIRTY ) this->update();
}

void WrfSubregionRmsView::mouseDoubleClickEvent(QMouseEvent *event){

	if ( event->x() > left_ && event->x() < this->width() - border_size_
		&& this->height() - event->y() > bottom_ && event->y() > border_size_ + title_height_ ) {

		for ( int i = 0; i < selected_value_.size(); ++i ) selected_value_[i] = focus_record_index_;
	}

	emit SelectionValueChanged();

	this->update();
}

void WrfSubregionRmsView::ApplySelection(){
    if ( !(view_status_ & BRUSHING_ON) || selecting_path_.size() < 3 ) return;

    for ( int i = 0; i < selecting_path_.size() - 1; ++i ){
        QPoint p1 = selecting_path_[i];
        QPoint p2 = selecting_path_[i + 1];

        int x1 = (p1.x() - left_) / x_step_;
        int x2 = (p2.x() - left_) / x_step_;

        int step = x1 <= x2? 1:-1;
        while ( true ){
            if ( x1 >= 0 && x1 < data_values_.size() ){
				float y = this->height() - p1.y() + (x1 * x_step_ + left_ - p1.x()) / (p2.x() - p1.x()) * (p1.y() - p2.y());
                int temp_n = (y - bottom_) / y_step_;
                if ( temp_n >= 0 && temp_n < data_values_[0].size() ){
                    selected_value_[x1] = temp_n;
                }
            }
            if ( x1 == x2 ) break;
            x1 += step;
        }
    }

    selecting_path_.clear();

    emit SelectionValueChanged();

    this->update();
}

void WrfSubregionRmsView::OnBrushActionTriggered(bool b){
    if ( b )
		view_status_ |= BRUSHING_ON;
	else
		if ( view_status_ & BRUSHING_ON ) view_status_ ^= BRUSHING_ON;

	this->update();
}