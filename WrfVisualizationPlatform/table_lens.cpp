#include "table_lens.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QToolTip>
#include <time.h>
#include "color_mapping_generator.h"

TableLens::TableLens(int index, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), dataset_(NULL), lens_index_(index), view_status_(NORMAL_STATUS),
	focus_node_index_(-1), focus_record_index_(-1), max_scale_(-1), current_covering_rate_(1){

	border_size_ = 10;
	energy_coor_width_ = 40;
	y_value_text_width_ = 40;
	coor_indicator_width_ = 5;

    title_height_ = 30;
	node_height_ = 15;
	x_coor_text_height_ = 10;

	view_status_ = SCALE_MODE;

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAutoFillBackground(false);
}

TableLens::~TableLens(){

}

void TableLens::SetDataset(TableLensDataset* data){
    dataset_ = data;

    focus_node_index_ = -1;
	focus_record_index_ = -1;

	if ( max_scale_ < 0 ) {
		max_scale_ = 2;
		scale_threshold_ = max_scale_;
	}

	if ( dataset_->scale_value_ranges[1] > 2 ) 
		max_scale_ = 2;
	else
		max_scale_ = dataset_->scale_value_ranges[1];

	if ( scale_threshold_ > max_scale_ ) scale_threshold_ = max_scale_;

	int accu_count = 0;
	for ( int i = 0; i < dataset_->record_values.size(); ++i ) accu_count += dataset_->record_values[i].size();
	sorted_values_.resize(accu_count);
	accu_count = 0;
	for ( int i = 0; i < dataset_->scale_values.size(); ++i )
		for ( int j = 0; j < dataset_->scale_values[i].size(); ++j ) {
			dataset_->is_record_selected[i][j] = dataset_->scale_values[i][j] < scale_threshold_;
			sorted_values_[accu_count] = dataset_->scale_values[i][j];
			accu_count++;
		}
	sort(sorted_values_.begin(), sorted_values_.end());

	mutual_suggestion_index_ = -1;

	int temp_min_index = -1;
	float temp_min_energy = 1e10;
	for ( int i = 0; i < dataset_->energy_value.size(); ++i )
		if ( dataset_->energy_value[i] < temp_min_energy ){
			temp_min_energy = dataset_->energy_value[i];
			temp_min_index = i;
		}
	if ( temp_min_index != -1 ){
		mutual_suggestion_index_ = temp_min_index;
	}

    this->update();
}

void TableLens::SetRmsUnits(std::vector< RmsUnit >& units){
	rms_units_ = units;
}

void TableLens::SetViewMode(int mode){
	if ( mode == 0 ) {
		view_status_ |= RMS_MODE;
		if ( view_status_ & SCALE_MODE ) view_status_ ^= SCALE_MODE;
	} else if ( mode == 1 ){
		view_status_ |= SCALE_MODE;
		if ( view_status_ & RMS_MODE ) view_status_ ^= RMS_MODE;
	}

	this->update();
}

void TableLens::SetSuggestionRate(float rate){

	this->update();
}

void TableLens::SetSuggestionOn(){
	view_status_ |= SUGGESTION_ON;

	this->update();
}

void TableLens::SetSuggestionOff(){
	if ( view_status_ & view_status_ ) view_status_ ^= SUGGESTION_ON;

	this->update();
}

void TableLens::UpdateSuggestion(){
}

void TableLens::SetHighlightRecordIndex(int node_index, int record_index){
    focus_node_index_ = node_index;
	focus_record_index_ = record_index;

    this->update();
}

void TableLens::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        std::cout << "Error initialize table lens!" << std::endl;
        return;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void TableLens::resizeGL(int w, int h){
    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 0, 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -1.0);
}

void TableLens::paintEvent(QPaintEvent* event){
    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT);

    if ( dataset_ == NULL ) return;

	left_ = border_size_ + energy_coor_width_ + coor_indicator_width_ + y_value_text_width_;
	x_step_ = (float)(this->width() - border_size_ - left_) / dataset_->record_values.size();
	bottom_ = border_size_ + x_coor_text_height_ + node_height_;

	if ( view_status_ & ABS_VALUE_ON ){
		coor_height_ = (this->height() - title_height_ - border_size_ - bottom_) / 2;
		coor_center_ = bottom_ + coor_height_;
	} else {
		coor_height_ = (this->height() - title_height_ - border_size_ - bottom_);
		coor_center_ = bottom_;
	}

	node_margin_ = 1.0;
	if ( x_step_ < 6 ) node_margin_ = 0;

    // render the value lines
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if ( view_status_ & RMS_MODE ){
		for ( int i = 0; i < dataset_->record_values.size(); ++i )
			for ( int j = 0; j < dataset_->record_values[i].size(); ++j ){
				float temp_value = dataset_->record_values[i][j];
				temp_value = (temp_value - dataset_->value_ranges[0]) / (dataset_->value_ranges[1] - dataset_->value_ranges[0]);

				if ( dataset_->is_record_selected[i][j])
					glColor4f(0.4, 0.4, 0.4, 1.0);
				else
					glColor4f(0.6, 0.6, 0.6, 0.0);

				glRectf(left_ + i * x_step_ + (x_step_ - node_margin_) * j / dataset_->record_values[i].size(), coor_center_, left_ + i * x_step_ + (x_step_ - node_margin_) * (j + 1) / dataset_->record_values[i].size(), coor_center_ + coor_height_ * temp_value);
			}
	} else {
		for ( int i = 0; i < dataset_->scale_values.size(); ++i ){
			for ( int j = 0; j < dataset_->scale_values[i].size(); ++j ){
				float temp_value = dataset_->scale_values[i][j] / max_scale_;
				if ( temp_value > 1 ) temp_value = 1;

				if ( dataset_->is_record_selected[i][j])
					glColor4f(0.4, 0.4, 0.4, 1.0);
				else
					glColor4f(0.6, 0.6, 0.6, 0.0);

				glRectf(left_ + i * x_step_ + (x_step_ - node_margin_) * j / dataset_->record_values[i].size(), coor_center_, left_ + i * x_step_ + (x_step_ - node_margin_) * (j + 1) / dataset_->record_values[i].size(), coor_center_ + coor_height_ * temp_value);
			}
		}
	}

	if ( view_status_ & ABS_VALUE_ON ){
		for ( int i = 0; i < dataset_->record_values.size(); ++i ){
			float temp_value = dataset_->record_absolute_values[i];
			temp_value = (temp_value - dataset_->absolute_value_ranges[0]) / (dataset_->absolute_value_ranges[1] - dataset_->absolute_value_ranges[0]);

			glRectf(left_ + i * x_step_, coor_center_, left_ + (i + 1) * x_step_ - node_margin_, coor_center_ - coor_height_ * temp_value);
		}
	}
	
    glDisable(GL_BLEND);

	// render the highlight record
	glColor4f(1.0, 0.6, 0.38, 1.0);
    if ( focus_record_index_ != -1 && focus_node_index_ != -1 ){
		float temp_value;

		if ( view_status_ & RMS_MODE ){
			temp_value = dataset_->record_values[focus_node_index_][focus_record_index_];
			temp_value = (temp_value - dataset_->value_ranges[0]) / (dataset_->value_ranges[1] - dataset_->value_ranges[0]);
		} else {
			temp_value = dataset_->scale_values[focus_node_index_][focus_record_index_] / max_scale_;
			if ( temp_value > 1 ) temp_value = 1;
		}

		int temp_left = (int)(left_ + focus_node_index_ * x_step_ + (float)focus_record_index_ / dataset_->record_values[focus_node_index_].size() * (x_step_ - node_margin_));
		int temp_right = (int)(temp_left + 1.0 / dataset_->record_values[focus_node_index_].size() * (x_step_ - node_margin_));
		if ( temp_right - temp_left < 3 ) temp_right = temp_left + 3;

		glRectf(temp_left, coor_center_, temp_right, coor_center_ + coor_height_ * temp_value);

		if ( view_status_ & ABS_VALUE_ON ){
			float temp_value = dataset_->record_absolute_values[focus_node_index_];
			temp_value = (temp_value - dataset_->absolute_value_ranges[0]) / (dataset_->absolute_value_ranges[1] - dataset_->absolute_value_ranges[0]);

			glRectf(left_ + temp_left, coor_center_, left_ + temp_right, coor_center_ - coor_height_ * temp_value);
		}
    }

    // render the coordinate
    glLineWidth(2.0);
    glColor4f(0.4, 0.4, 0.4, 1.0);
	glBegin(GL_LINES);
	glVertex3f(left_ - coor_indicator_width_, coor_center_, 0);
	glVertex3f(this->width() - border_size_, coor_center_, 0);

	glVertex3f(left_ - coor_indicator_width_, coor_center_, 0);
	glVertex3f(left_ - coor_indicator_width_, coor_center_ + coor_height_, 0);
	
	float coor_indicator_step;
	int coor_indicator_num;
	float indicator_step_y;
	if ( view_status_ & SCALE_MODE ){
		float avail_steps[] = {0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0};
		int coor_indicator_index = 0;
		coor_indicator_num = max_scale_ / avail_steps[coor_indicator_index];
		while ( coor_indicator_num > 5 && coor_indicator_index < 7 ){
			coor_indicator_index++;
			coor_indicator_num = max_scale_ / avail_steps[coor_indicator_index];
		}
		coor_indicator_step = avail_steps[coor_indicator_index];
		indicator_step_y = coor_height_ * coor_indicator_step / max_scale_;
	} else {
		float avail_steps[] = {1, 2, 5, 10, 20, 50, 100, 500, 1000};
		int coor_indicator_index = 0;
		coor_indicator_num = dataset_->value_ranges[1] / avail_steps[coor_indicator_index];
		while ( coor_indicator_num > 5 && coor_indicator_index < 9 ){
			coor_indicator_index++;
			coor_indicator_num = dataset_->value_ranges[1] / avail_steps[coor_indicator_index];
		}
		coor_indicator_step = avail_steps[coor_indicator_index];
		indicator_step_y = coor_height_ * coor_indicator_step / dataset_->value_ranges[1];
	}

	for ( int i = 1; i <= coor_indicator_num; ++i ){
		glVertex3f(left_ - coor_indicator_width_, bottom_ + i * indicator_step_y, 0);
		glVertex3f(left_ - coor_indicator_width_ + 3, bottom_ + i * indicator_step_y, 0);

		glVertex3f(border_size_ + energy_coor_width_, bottom_ + i * indicator_step_y, 0);
		glVertex3f(border_size_ + energy_coor_width_ - 3, bottom_ + i * indicator_step_y, 0);
	}

	// coordinate for mutual info
	glVertex3f(border_size_ + energy_coor_width_, coor_center_, 0);
	glVertex3f(border_size_, coor_center_, 0);

	glVertex3f(border_size_ + energy_coor_width_, coor_center_, 0);
	glVertex3f(border_size_ + energy_coor_width_, coor_center_ + coor_height_, 0);

	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(left_ - coor_indicator_width_, coor_center_ + coor_height_ + 5, 0);
	glVertex3f(left_ - 3 - coor_indicator_width_, coor_center_ + coor_height_, 0);
	glVertex3f(left_ + 3 - coor_indicator_width_, coor_center_ + coor_height_, 0);

	glVertex3f(this->width() - border_size_ + 5, coor_center_, 0);
	glVertex3f(this->width() - border_size_, coor_center_ - 3, 0);
	glVertex3f(this->width() - border_size_, coor_center_ + 3, 0);

	glVertex3f(border_size_ + energy_coor_width_, coor_center_ + coor_height_ + 5, 0);
	glVertex3f(border_size_ + energy_coor_width_ - 3, coor_center_ + coor_height_, 0);
	glVertex3f(border_size_ + energy_coor_width_ + 3, coor_center_ + coor_height_, 0);

	glVertex3f(border_size_ - 5, coor_center_, 0);
	glVertex3f(border_size_, coor_center_ - 3, 0);
	glVertex3f(border_size_, coor_center_ + 3, 0);
	glEnd();

	if ( view_status_ & ABS_VALUE_ON ){
		glBegin(GL_LINES);
		glVertex3f(left_ - coor_indicator_width_, coor_center_, 0);
		glVertex3f(left_ - coor_indicator_width_, coor_center_ - coor_height_, 0);
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex3f(left_ - coor_indicator_width_, coor_center_ - coor_height_ - 5, 0);
		glVertex3f(left_ - 3 - coor_indicator_width_, coor_center_ - coor_height_, 0);
		glVertex3f(left_ + 3 - coor_indicator_width_, coor_center_ - coor_height_, 0);
		glEnd();
	}


    // render scale threshold lines
	glLineWidth(3.0);
	glColor4f(1.0, 0.6, 0.38, 1.0);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x3F07);
	glBegin(GL_LINES);
	glVertex3f(left_ - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glVertex3f(this->width() - border_size_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);

	glVertex3f(border_size_ + energy_coor_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glVertex3f(border_size_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(left_ - 8 - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glVertex3f(left_ + 8 - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glVertex3f(left_ - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_ + 8, 0);

	glVertex3f(left_ - 8 - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glVertex3f(left_ + 8 - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_, 0);
	glVertex3f(left_ - coor_indicator_width_, coor_center_ + coor_height_ * scale_threshold_ / max_scale_ - 8, 0);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	// render the glyph
	indicator_radius_ = (int)(x_step_ * 0.25);
	if ( indicator_radius_ < 1 ) indicator_radius_ = 1;
	if ( indicator_radius_ > 5 ) indicator_radius_ = 5;
	for ( int i = 0; i < dataset_->record_values.size() && i < rms_units_.size(); ++i ){
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

    // render the indicator
	QFont normal_font;
	normal_font.setFamily("arial");
	normal_font.setBold(false);
	normal_font.setPixelSize(10);

	QFont bold_font;
	bold_font.setFamily("arial");
	bold_font.setBold(true);
	bold_font.setPixelSize(13);

	QFont title_font;
	title_font.setFamily("arial");
	title_font.setBold(true);
	title_font.setPixelSize(16);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::black);
        
	if ( view_status_ & SCALE_MODE ){
		painter.setFont(normal_font);
		//painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, border_size_ + title_height_ - 20, 35, 40), Qt::AlignRight | Qt::AlignVCenter, QString::number(max_scale_, 'g', 3));
		painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, border_size_ + title_height_ + coor_height_ - 40, 35, 40), Qt::AlignBottom | Qt::AlignRight, QString::number(0, 'g', 3));

		painter.setFont(bold_font);
		painter.drawText(QRectF(left_ - 80, border_size_ + title_height_ - 30, 100, 20), Qt::AlignHCenter | Qt::AlignBottom, QString("Ratio"));
	} else {
		painter.setFont(normal_font);
		//painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, border_size_ + title_height_ - 20, 35, 40), Qt::AlignRight | Qt::AlignVCenter, QString::number(dataset_->value_ranges[1], 'g', 3));
		painter.drawText(QRectF(left_ - coor_indicator_width_ - 40, border_size_ + title_height_ + coor_height_ - 40, 35, 40), Qt::AlignBottom | Qt::AlignRight, QString::number(dataset_->value_ranges[0], 'g', 3));

		painter.setFont(bold_font);
		painter.drawText(QRectF(left_ - 70, border_size_ + title_height_ - 30, 100, 20), Qt::AlignHCenter | Qt::AlignBottom, QString("RMS"));
	}

	painter.setFont(normal_font);
	for ( int i = 1; i <= coor_indicator_num; ++i ){
		painter.drawText(QRectF(border_size_ + energy_coor_width_, this->height() - coor_center_ - i * indicator_step_y - 20, left_ - coor_indicator_width_ - border_size_ - energy_coor_width_, 40), Qt::AlignCenter, QString("%0").arg(coor_indicator_step * i));
	}

	if ( view_status_ & ABS_VALUE_ON ){
		/*painter.setFont(normal_font);
		painter.drawText(QRectF(0, -1 * coor_height_, x_coor_text_width_ - 5, 40), Qt::AlignTop | Qt::AlignRight, QString::number(dataset_->absolute_value_ranges[2 * i], 'g', 4));
		painter.drawText(QRectF(0, -20, x_coor_text_width_- 5, 40), Qt::AlignRight | Qt::AlignVCenter, QString::number(dataset_->absolute_value_ranges[2 * i + 1], 'g', 4));*/

		painter.setFont(bold_font);
		painter.drawText(QRectF(0, 0, 0, coor_height_), Qt::AlignLeft | Qt::AlignVCenter, QString("ABS"));
	}

	painter.setFont(bold_font);
	painter.drawText(QRectF(left_, this->height() - border_size_ - x_coor_text_height_, this->width() - border_size_ - left_, x_coor_text_height_ + border_size_), Qt::AlignRight | Qt::AlignVCenter, QString("Small Regions"));

	// render energy curve
	if ( dataset_->energy_value.size() != 0 ){
		QPen pen;
		pen.setColor(Qt::darkGray);
		pen.setWidth(2.0);
		painter.setPen(pen);

		QPainterPath path;
		path.moveTo(0.0, 0.0);
		for ( int i = 0; i < dataset_->energy_value.size(); ++i ){
			path.lineTo(-1 * dataset_->energy_value[i] * energy_coor_width_, (float)-1 * coor_height_ * i / (dataset_->energy_value.size() - 1));
		}

		painter.translate(border_size_ + energy_coor_width_, this->height()- coor_center_);
		painter.drawPath(path);
		painter.translate(-1 * (border_size_ + energy_coor_width_), -1 * (this->height()- coor_center_));

		painter.setPen(Qt::black);
		painter.setFont(bold_font);
		painter.drawText(QRectF(border_size_, this->height() - border_size_ - x_coor_text_height_, energy_coor_width_, x_coor_text_height_ + border_size_), Qt::AlignLeft | Qt::AlignVCenter, QString("Bias"));
	}

    // render the title
	painter.setPen(Qt::black);
	painter.setFont(title_font);
	painter.drawText(QRectF(0, 0, this->width(), title_height_), Qt::AlignCenter, dataset_->data_name);

	// render the suggestion lines
	if ( view_status_ & SUGGESTION_ON ){
		QPen pen;
		pen.setColor(QColor(160, 32, 240));
		pen.setWidth(4.0);
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);

		if ( mutual_suggestion_index_ != -1 ){
			float mutual_suggestion_scale = mutual_suggestion_index_ * max_scale_ / 20;
			painter.drawLine(left_, this->height() - (coor_center_ + coor_height_ * mutual_suggestion_scale / max_scale_), this->width() - border_size_, this->height() - (coor_center_ + coor_height_ * mutual_suggestion_scale / max_scale_));
			painter.drawLine(border_size_ + energy_coor_width_ - dataset_->energy_value[mutual_suggestion_index_] * energy_coor_width_, this->height() - (coor_center_ + coor_height_ * mutual_suggestion_scale / max_scale_), border_size_ + energy_coor_width_, this->height() - (coor_center_ + coor_height_ * mutual_suggestion_scale / max_scale_));
		}
	}

	painter.setFont(normal_font);
	painter.setPen(QColor(160, 32, 240));
	float temp_height = this->height() - (coor_center_ + coor_height_ * scale_threshold_ / max_scale_);
	int temp_rate = current_covering_rate_ * 100;
	painter.drawText(QRectF(left_ + 15, temp_height - 35, 100, 30), Qt::AlignLeft | Qt::AlignBottom, QString("%0/%").arg(temp_rate));

	painter.setPen(Qt::black);
	normal_font.setBold(false);
	normal_font.setPixelSize(10);
	painter.setFont(normal_font);
	painter.drawText(QRectF(this->width() - border_size_ - 250, title_height_ - 2.5 * border_size_, 170, 10), Qt::AlignRight | Qt::AlignBottom, QString("Suggested Range: "));

	QPen pen;
	pen.setColor(QColor(160, 32, 240));
	pen.setWidth(4.0);
	pen.setStyle(Qt::DashDotLine);
	painter.setPen(pen);
	painter.drawLine(this->width() - border_size_ - 70, title_height_ -  2 * border_size_,  this->width() - border_size_ - 30, title_height_ -  2 * border_size_);

    painter.end();

	if ( view_status_ & VIEW_DIRTY ) view_status_ ^= VIEW_DIRTY;
}

void TableLens::RenderGridInfoPie(int index){

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

void TableLens::RenderArc(QColor color, float radius, float begin_theta, float end_theta){
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

void TableLens::mousePressEvent(QMouseEvent *event){
	if ( view_status_ & SCALE_MODE && abs(event->pos().x() - left_) < 10 && event->buttons() & Qt::LeftButton)
		view_status_ |= ADJUSTING_ON;
}

void TableLens::mouseMoveEvent(QMouseEvent *event){
    if ( view_status_ & ADJUSTING_ON ){
        float temp_value = (this->height() - event->pos().y() - coor_center_) / coor_height_;
        if ( temp_value >= 0 && temp_value <= 1 ){
            scale_threshold_ = temp_value * max_scale_;

            UpdateSelection();

            view_status_ |= VIEW_DIRTY;
        }
    } else { 
		if ( event->x() > left_ && event->x() < this->width() - border_size_
			&& this->height() - event->y() > bottom_ && event->y() > border_size_ + title_height_ ) {

			int temp_node_index = (event->x() - left_) / x_step_;
			if ( temp_node_index < 0 ) temp_node_index = 0;
			if ( temp_node_index >= dataset_->record_values.size() ) temp_node_index = dataset_->record_values.size() - 1;

			int temp_record_index = (event->x() - left_ - temp_node_index * x_step_) / (x_step_ - node_margin_) * dataset_->record_values[temp_node_index].size();
			if ( temp_record_index < 0 ) temp_record_index = 0;
			if ( temp_record_index >= dataset_->record_values[temp_node_index].size() ) temp_record_index = dataset_->record_values[temp_node_index].size() - 1;

			if( temp_node_index != focus_node_index_ || temp_record_index != focus_record_index_ ){
				focus_node_index_ = temp_node_index;
				focus_record_index_ = temp_record_index;

				emit CurrentRecordChanged(focus_node_index_, focus_record_index_);

				view_status_ |= VIEW_DIRTY;
			}

			if ( temp_node_index != -1 && temp_record_index != -1 ){
				QToolTip::showText(event->globalPos(), QString("Rms Value: %0\t\nAbsolute Value: %1\t\nScale Value: %2").arg(dataset_->record_values[temp_node_index][temp_record_index]).arg(dataset_->record_absolute_values[temp_node_index]).arg(dataset_->scale_values[temp_node_index][temp_record_index]));
			}
		} 
    }

	if ( view_status_ & VIEW_DIRTY ) this->update();
}

void TableLens::mouseReleaseEvent(QMouseEvent *event){
	if ( view_status_ & ADJUSTING_ON ) view_status_ ^= ADJUSTING_ON;
}

void TableLens::UpdateSelection(){
	int accu_count = 0, selected_count = 0;
    for ( int i = 0; i < dataset_->scale_values.size(); ++i )
		for ( int j = 0; j < dataset_->scale_values[i].size(); ++j ) {
        dataset_->is_record_selected[i][j] = dataset_->scale_values[i][j] < scale_threshold_;

		accu_count++;
		selected_count += dataset_->is_record_selected[i][j];
    }

	if ( accu_count != 0 ) current_covering_rate_ = (float)selected_count / accu_count;

    emit SelectionChanged(lens_index_);
}