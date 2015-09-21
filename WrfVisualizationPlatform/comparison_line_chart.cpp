#include "comparison_line_chart.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QToolTip>
#include "wrf_statistic_solver.h"

ComparisonLineChart::ComparisonLineChart(QWidget *parent)
    : QGLWidget(parent){
    title_height_ = 40;
    coor_text_width_ = 40;
	distribution_bar_height_ = 25;

    label_width_ = 30;
    label_height_ = 20;
    label_text_width_ = 100;

    border_size_ = 10;
    margin_size_ = 3;
        
    coor_text_height_ = 30;

    base_index_ = -1;

    current_analog_num_ = -1;
    selected_analog_num_ = -1;
    current_sort_index_ = -1;

    is_data_updated_ = false;
    is_floating_ = false;
	is_suggestion_on_ = false;

    dataset_ = NULL;
    max_viewing_num_ = 0;

    context_menu_ = new QMenu;
    apply_grid_size_menu_ = new QMenu(tr("Apply Grid Size"));
    apply_grid_size_action_group_ = new QActionGroup(this);
    apply_grid_size_action_group_->setExclusive(true);
    context_menu_->addMenu(apply_grid_size_menu_);

    setAutoFillBackground(false);
    setMouseTracking(true);
}

ComparisonLineChart::~ComparisonLineChart(){

}

void ComparisonLineChart::SetMaximumViewingNum(int max_num){
    if ( dataset_ == NULL ) return;

    max_viewing_num_ = max_num;

    UpdateRenderingBase(base_index_);

    this->update();
}


void ComparisonLineChart::SetDataset(ComparisonLineChartDataset* data){
    dataset_ = data;

    base_index_ = -1;

    is_data_updated_ = true;

    max_viewing_num_ = data->values[0].size();

    sorted_index_.resize(dataset_->values[0].size());
    for ( int i = 0; i < sorted_index_.size(); ++i ) sorted_index_[i] = i;

    // update menus
    QList< QAction* > apply_actions = apply_grid_size_menu_->actions();
    for ( int i = 0; i < apply_actions.size(); ++i ){
        apply_grid_size_action_group_->removeAction(apply_actions[i]);
    }
    apply_grid_size_menu_->clear();

    for ( int i = 0; i < dataset_->label_names.size(); ++i ){
        QAction* action = new QAction(dataset_->label_names[i], this);
        action->setCheckable(true);
        action->setChecked(false);

        apply_grid_size_menu_->addAction(action);
        apply_grid_size_action_group_->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(OnApplyGridSizeTriggered()));
    }

    this->update();
}

void ComparisonLineChart::SetSortingIndex(int index){
    QList< QAction* > size_actions = apply_grid_size_menu_->actions();
    size_actions.at(index)->setChecked(true);

    current_sort_index_ = index;
    selected_analog_num_ = dataset_->stopping_values[index];

	Sort(current_sort_index_, 0, dataset_->values[0].size() - 1);
	this->update();
}

void ComparisonLineChart::SetSelectionValue(int value){
    current_mouse_pos_.setX(coor_left_ + value * value_x_step_);
    this->update();
}

void ComparisonLineChart::SetSuggestionRate(float min_rate){
	min_rate_ = min_rate;

	UpdateSuggestion();

	this->update();
}

void ComparisonLineChart::SetSuggestionOn(){
	is_suggestion_on_ = true;

	this->update();
}

void ComparisonLineChart::SetSuggestionOff(){
	is_suggestion_on_ = false;
	this->update();
}

void ComparisonLineChart::UpdateSuggestion(){
	min_distribution_index_ = -1;
	
	if ( dataset_ == NULL || dataset_->distributions.size() == 0 ) return;

	int temp_index = 0;
	while ( temp_index < dataset_->distributions.size() - 1 && dataset_->distributions[temp_index] > dataset_->distributions[0] * min_rate_ ) temp_index++;
	min_distribution_index_ = temp_index;
}

void ComparisonLineChart::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        std::cout << "Error initialize table lens!" << std::endl;
        return;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void ComparisonLineChart::resizeGL(int w, int h){
    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 0, 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -1.0);
}

void ComparisonLineChart::paintEvent(QPaintEvent* event){
    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT);

    if ( dataset_ == NULL ) return;

    if ( is_data_updated_ ){
        UpdateRenderingBase(base_index_);
        is_data_updated_ = false;
    }

	coor_bottom_ = border_size_ + coor_text_height_;
	if ( dataset_->label_names.size() <= 1 ){
		coor_top_ = this->height() - border_size_ - title_height_ - distribution_bar_height_;
	} else {
		coor_top_ = this->height() - border_size_ - title_height_ - label_height_ - distribution_bar_height_;
	}
	value_y_step_ = (coor_top_ - coor_bottom_) / (max_value_ - min_value_);

	coor_left_ = coor_text_width_ + border_size_;
	coor_right_ = this->width() - border_size_;
	value_x_step_ = (coor_right_ - coor_left_) / max_viewing_num_;
        
    glColor3f(0.4, 0.4, 0.4);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(coor_left_, coor_bottom_, 0);
    glVertex3f(coor_right_, coor_bottom_, 0);

	int x_coor_indicator_step = 50;
	int x_coor_indicator_num = max_viewing_num_ / x_coor_indicator_step;
	while ( x_coor_indicator_num > 10 ){
		x_coor_indicator_step += 50;
		x_coor_indicator_num = max_viewing_num_ / x_coor_indicator_step;
	}
	float indicator_step_x = value_x_step_ * x_coor_indicator_step;
	for ( int i = 1; i <= x_coor_indicator_num; ++i ){
		glVertex3f(coor_left_ + i * indicator_step_x, coor_bottom_, 0);
		glVertex3f(coor_left_ + i * indicator_step_x, coor_bottom_ + 3, 0);
	}

    glVertex3f(coor_left_, coor_bottom_, 0);
    glVertex3f(coor_left_, coor_top_, 0);

	float avail_steps[] = {0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2, 5, 10, 20, 50, 100};
	int coor_indicator_index = 0;
	int y_coor_indicator_num = max_value_ / avail_steps[coor_indicator_index];
	while ( y_coor_indicator_num > 5 && coor_indicator_index < 13 ){
		coor_indicator_index++;
		y_coor_indicator_num = max_value_ / avail_steps[coor_indicator_index];
	}
	float y_coor_indicator_step = avail_steps[coor_indicator_index];
	float indicator_step_y = value_y_step_ * y_coor_indicator_step;

	for ( int i = 1; i <= y_coor_indicator_num; ++i ){
		glVertex3f(coor_left_, coor_bottom_ + i * indicator_step_y, 0);
		glVertex3f(coor_left_ + 3, coor_bottom_ + i * indicator_step_y, 0);
	}
    glEnd();
    
    glBegin(GL_TRIANGLES);
    glVertex3f(coor_right_ + 5, coor_bottom_, 0);
    glVertex3f(coor_right_, coor_bottom_ + 3, 0);
    glVertex3f(coor_right_, coor_bottom_ - 3, 0);

    glVertex3f(coor_left_, coor_top_ + 5, 0);
    glVertex3f(coor_left_ - 3, coor_top_, 0);
    glVertex3f(coor_left_ + 3, coor_top_, 0);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // render lines
    glLineWidth(1.0);
    for ( int i = 0; i < dataset_->values.size(); ++i ){
        if ( i == current_sort_index_ ) continue;
        glBegin(GL_LINE_STRIP);
        float x, y;
        if ( base_index_ == -1 ){
            for ( int j = 0; j < max_viewing_num_; ++j ){
                if ( !is_floating_ && selected_analog_num_ != -1 && j > selected_analog_num_ )
                    glColor4f(dataset_->label_colors[i].redF(), dataset_->label_colors[i].greenF(), dataset_->label_colors[i].blueF(), 0.2);
                else
                    glColor4f(dataset_->label_colors[i].redF(), dataset_->label_colors[i].greenF(), dataset_->label_colors[i].blueF(), 1.0);
                y = dataset_->values[i][sorted_index_[j]] * value_y_step_ + coor_bottom_;
                x = coor_left_ + j * value_x_step_;
                glVertex3f(x, y, 0);
            }
        } else {
            for ( int j = 0; j < max_viewing_num_; ++j ){
                if ( !is_floating_ && selected_analog_num_ != -1 && j > selected_analog_num_ )
                    glColor4f(dataset_->label_colors[i].redF(), dataset_->label_colors[i].greenF(), dataset_->label_colors[i].blueF(), 0.2);
                else
                    glColor4f(dataset_->label_colors[i].redF(), dataset_->label_colors[i].greenF(), dataset_->label_colors[i].blueF(), 1.0);
                y = (dataset_->values[i][sorted_index_[j]] - dataset_->values[base_index_][sorted_index_[j]]) * value_y_step_ + coor_bottom_;
                x = coor_left_ + j * value_x_step_;
                glVertex3f(x, y, 0);
            }
        }
        glEnd();
    }

    if ( current_sort_index_ != -1 ){
        glLineWidth(2.0);
        float x, y;
        glBegin(GL_LINE_STRIP);
        for ( int i = 0; i < max_viewing_num_; ++i ){
            if ( !is_floating_ && selected_analog_num_ != -1 && i > selected_analog_num_ )
                glColor4f(dataset_->label_colors[current_sort_index_].redF(), dataset_->label_colors[current_sort_index_].greenF(), dataset_->label_colors[current_sort_index_].blueF(), 0.2);
            else
                glColor4f(dataset_->label_colors[current_sort_index_].redF(), dataset_->label_colors[current_sort_index_].greenF(), dataset_->label_colors[current_sort_index_].blueF(), 1.0);
            y = dataset_->values[current_sort_index_][sorted_index_[i]] * value_y_step_ + coor_bottom_;
            x = coor_left_ + i * value_x_step_;
            glVertex3f(x, y, 0);
        }
        glEnd();
    }
    
    glDisable(GL_BLEND);

	// render distribution bar
	if ( dataset_->distributions.size() != 0 ){
		int date_num_per_bin = dataset_->values[0].size() / dataset_->distributions.size();
		int max_distribution_bin = max_viewing_num_ / date_num_per_bin;
		for ( int i = 0; i < max_distribution_bin; ++i ){
			float temp_left = coor_left_ + i * date_num_per_bin * value_x_step_;
			float temp_right = coor_left_ + (i + 1) * date_num_per_bin * value_x_step_;
			if ( temp_right > coor_right_ ) temp_right = coor_right_;

			float temp_color = 1.0 - dataset_->distributions[i];
			glColor3f(temp_color, temp_color, temp_color);
			glRectf(temp_left, coor_top_ + 10, temp_right, coor_top_ + distribution_bar_height_);
		}
	}


    // render bar
	if ( is_floating_ ){
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3F07);
		glColor3f(0.5, 0.5, 0.5);
		glLineWidth(2.0);
		glBegin(GL_LINES);
		glVertex3f(current_mouse_pos_.x(), coor_bottom_, 0);
		glVertex3f(current_mouse_pos_.x(), coor_top_, 0);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

    if ( current_sort_index_ != -1 ){
        float x_pos = coor_left_ + value_x_step_ * dataset_->stopping_values[current_sort_index_];
        glColor3f(1.0, 0.6, 0.38);
        glLineWidth(3.0);
        glBegin(GL_LINES);
        glVertex3f(x_pos, coor_bottom_, 0);
        glVertex3f(x_pos, coor_top_, 0);
        glEnd();
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont normal_font;
    normal_font.setFamily("arial");
    normal_font.setBold(false);
    normal_font.setPixelSize(10);

	if ( dataset_->label_names.size() > 1 ){
		float label_left = this->width() / 2 - dataset_->label_names.size() * (label_width_ + label_text_width_) / 2;
		// render label marker
		for ( int i = 0; i < dataset_->label_names.size(); ++i ){
			painter.fillRect(QRectF(label_left, title_height_ + margin_size_, label_width_, label_height_), dataset_->label_colors[i]);

			/*if ( i == current_sort_index_ ){
				QColor color(255, 153, 97, 255);
				painter.setPen(color);
				normal_font.setBold(true);

				painter.fillRect(QRectF(label_left, title_height_ + margin_size_ + label_height_ + 2, label_width_ + label_text_width_ * 0.7, 5), color);
			} else {
				painter.setPen(Qt::black);
				normal_font.setBold(false);
			}*/

			painter.setFont(normal_font);

			QString name = dataset_->label_names[i] + QString(" (%0)").arg(dataset_->stopping_values[i]);

			painter.drawText(QRectF(label_left + label_width_ + 2, title_height_ + margin_size_, label_text_width_, label_height_), Qt::AlignLeft | Qt::AlignVCenter, name);

			label_left += label_width_ + label_text_width_;
		}
		normal_font.setBold(false);
	}

	painter.setPen(Qt::black);
	painter.setFont(normal_font);

	// render distribution bar
	painter.drawText(QRectF(coor_right_ - 250, title_height_ - border_size_, 170, 10), Qt::AlignVCenter | Qt::AlignRight, QString("Date Similarity: Min"));
	painter.drawText(QRectF(coor_right_ - 30, title_height_ - border_size_, 30, 10), Qt::AlignVCenter | Qt::AlignLeft, QString("Max"));
	QLinearGradient gradient(coor_right_ - 75,  title_height_ - border_size_, coor_right_ - 35, title_height_ - border_size_);
	gradient.setColorAt(0, QColor(255.0, 255.0, 255.0));
	gradient.setColorAt(1, QColor(0.0, 0.0, 0.0));
	painter.setBrush(QBrush(gradient));
	painter.drawRect(QRectF(coor_right_ - 75, title_height_ - border_size_, 40, 10));
	painter.setPen(Qt::lightGray);
	painter.drawLine(coor_right_ - 75, title_height_ - border_size_, coor_right_ - 35, title_height_ - border_size_);
	painter.drawLine(coor_right_ - 75, title_height_ + 10 - border_size_, coor_right_ - 35, title_height_ + 10 - border_size_);
	painter.drawLine(coor_right_ - 35, title_height_ - border_size_, coor_right_ - 35, title_height_ + 10 - border_size_);
	painter.drawLine(coor_right_ - 75, title_height_ - border_size_, coor_right_ - 75, title_height_ + 10 - border_size_);

    // render coordinate values
    painter.setPen(Qt::black);
	painter.drawText(QRectF(coor_left_ - 40, this->height() - coor_bottom_ - 20, 35, 40), Qt::AlignVCenter | Qt::AlignRight, QString("0"));
    //painter.drawText(QRectF(coor_left_ - 40, this->height() - coor_top_ - 20, 35, 40), Qt::AlignVCenter | Qt::AlignRight, QString::number((double)max_value_, 'g', 4));
    painter.drawText(QRectF(coor_left_ - 20, this->height() - coor_bottom_ + 5, 40, 20), Qt::AlignHCenter | Qt::AlignTop, QString("0"));
    //painter.drawText(QRectF(coor_right_ - 20, this->height() - coor_bottom_ + 5, 40, 20), Qt::AlignHCenter | Qt::AlignTop, QString("%0").arg(max_viewing_num_));

	for ( int i = 1; i <= x_coor_indicator_num; ++i ){
		painter.drawText(QRectF(coor_left_ + indicator_step_x * i - 20, this->height() - coor_bottom_ + 5, 40, 20), Qt::AlignHCenter | Qt::AlignTop, QString("%0").arg(x_coor_indicator_step * i));
	}

	for ( int i = 1; i <= y_coor_indicator_num; ++i ){
		painter.drawText(QRectF(coor_left_ - 40, this->height() - coor_bottom_ - i * indicator_step_y - 20, 35, 40), Qt::AlignVCenter | Qt::AlignRight, QString("%0").arg(y_coor_indicator_step * i));
	}

    // render coordinate names
    normal_font.setBold(true);
    normal_font.setPixelSize(13);
    painter.setFont(normal_font);
    painter.drawText(QRectF(coor_left_, this->height() - coor_bottom_, coor_right_ - coor_left_, coor_text_height_), Qt::AlignHCenter | Qt::AlignBottom, QString("Date Count"));

	painter.translate(border_size_, this->height() - coor_bottom_);
	painter.rotate(-90);
	painter.drawText(QRectF(0, 0, coor_top_ - coor_bottom_, coor_text_width_), Qt::AlignTop | Qt::AlignHCenter, QString("Region Average RMS Difference"));
	painter.rotate(90);
	painter.translate(-1 * border_size_, -1 * (this->height() - coor_bottom_));

    // render title
    QFont title_font;
    title_font.setFamily("arial");
    title_font.setBold(true);
    title_font.setPixelSize(16);
    painter.setFont(title_font);
    painter.drawText(QRectF(0, 0, this->width(), title_height_), Qt::AlignCenter, dataset_->data_name);

	// render suggestion bar
	if ( is_suggestion_on_ && min_distribution_index_ != -1){
		QPen pen;
		pen.setColor(QColor(160, 32, 240));
		pen.setWidth(4.0);
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);

		int date_num_per_bin = dataset_->values[0].size() / dataset_->distributions.size();
		float min_rate_x = coor_left_ + min_distribution_index_ * date_num_per_bin * value_x_step_;
		if ( min_rate_x > coor_right_ ) min_rate_x = coor_right_;

		painter.drawLine(min_rate_x, this->height() - coor_top_, min_rate_x, this->height() - coor_bottom_);
	}

	QPen pen;
	pen.setColor(QColor(160, 32, 240));
	pen.setWidth(4.0);
	pen.setStyle(Qt::DashDotLine);
	painter.setPen(pen);
	painter.drawLine(coor_right_ - 70, title_height_ -  2 * border_size_,  coor_right_ - 30, title_height_ -  2 * border_size_);

	pen.setColor(Qt::black);
	pen.setWidth(1.0);
	painter.setPen(pen);
	normal_font.setBold(false);
	normal_font.setPixelSize(10);
	painter.setFont(normal_font);
	painter.drawText(QRectF(coor_right_ - 250, title_height_ - 2.5 * border_size_, 170, 10), Qt::AlignRight | Qt::AlignBottom, QString("Suggested Range: "));

    painter.end();
}

void ComparisonLineChart::UpdateRenderingBase(int base){
    min_value_ = 1e10;
    max_value_ = -1e10;

    if ( base == -1 ){
        for ( int i = 0; i < dataset_->values.size(); ++i )
            for ( int j = 0; j < max_viewing_num_; ++j ){
                if ( dataset_->values[i][sorted_index_[j]] > max_value_ ) max_value_ = dataset_->values[i][sorted_index_[j]];
                if ( dataset_->values[i][sorted_index_[j]] < min_value_ ) min_value_ = dataset_->values[i][sorted_index_[j]];
                if ( max_value_ > 40 ) max_value_ = 40;
            }
    } else {
        for ( int i = 0; i < dataset_->values.size(); ++i )
            if ( i != base ) {
                for ( int j = 0; j < max_viewing_num_; ++j ){
                    float temp_value = dataset_->values[i][sorted_index_[j]] - dataset_->values[base][sorted_index_[j]];
                    if ( temp_value > max_value_ ) max_value_ = temp_value;
                    if ( temp_value < min_value_ ) min_value_ = temp_value;
                }
            }
    }

	if ( min_value_ > 0 ) min_value_ = 0;
}

void ComparisonLineChart::mousePressEvent(QMouseEvent *event){

}

void ComparisonLineChart::mouseMoveEvent(QMouseEvent *event){
    if ( event->pos().x() >= coor_left_ && event->pos().x() <= coor_right_ + 5
        && event->pos().y() >= this->height() - coor_top_ && event->pos().y() <= this->height() - coor_bottom_ ){
            current_mouse_pos_ = event->pos();
			
            if ( dataset_ != NULL ){
				if ( current_mouse_pos_.x() > coor_right_ ){
					current_mouse_pos_.setX(coor_right_);
					current_analog_num_ = max_viewing_num_;
				} else {
					current_analog_num_ = (event->pos().x() - coor_left_) / value_x_step_;
				}
                QToolTip::showText(event->globalPos(), QString("Analog Num: %0 \t\nRegion Average Aggregated RMS Value:%1").arg(current_analog_num_).arg(dataset_->values[current_sort_index_][sorted_index_[current_analog_num_]]));
            }
            is_floating_ = true;

            this->update();
    } else {
        if ( is_floating_ ){
            is_floating_ = false;
            this->update();
        }
    }
}

void ComparisonLineChart::mouseReleaseEvent(QMouseEvent *event){

}

void ComparisonLineChart::mouseDoubleClickEvent(QMouseEvent *event){
	selected_analog_num_ = current_analog_num_;
	dataset_->stopping_values[current_sort_index_] = selected_analog_num_;

	emit AnalogNumberChanged(current_analog_num_);

	this->update();
}

void ComparisonLineChart::contextMenuEvent(QContextMenuEvent *event){
    QCursor cur = this->cursor();

    context_menu_->exec(cur.pos());
}

void ComparisonLineChart::Sort(int attrib_index, int begin, int end){
    if ( begin >= end ) return;

    int first = begin;
    int last = end;
    int random_index = begin + (end - begin) * (float)rand() / RAND_MAX;
    int key_index = sorted_index_[random_index];

    sorted_index_[random_index] = sorted_index_[first];
    sorted_index_[first] = key_index;

    float key = dataset_->values[attrib_index][sorted_index_[first]];    

    while ( first < last ){
        while ( first < last && dataset_->values[attrib_index][sorted_index_[last]] >= key ) last--;
        sorted_index_[first] = sorted_index_[last];
        while ( first < last && dataset_->values[attrib_index][sorted_index_[first]] <= key ) first++;
        sorted_index_[last] = sorted_index_[first];
    }
    sorted_index_[first] = key_index;

    Sort(attrib_index, begin, first - 1);
    Sort(attrib_index, first + 1, end);
}

void ComparisonLineChart::OnApplyGridSizeTriggered(){
    QList< QAction* > grid_size_actions = apply_grid_size_menu_->actions();
    for ( int i = 0; i < grid_size_actions.size(); ++i )
        if ( grid_size_actions[i]->isChecked() ){
            emit GridSizeChanged(i);
            current_sort_index_ = i;
            selected_analog_num_ = dataset_->stopping_values[current_sort_index_];
            SetSortingIndex(current_sort_index_);
            this->update();
            break;
        }
}

void ComparisonLineChart::OnApplyAnalogNumberTriggered(){
    
}