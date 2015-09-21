#include "mutual_matrix_viewer.h"
#include <iostream>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include "qcolor_bar_controller.h"
#include "node.h"


MutualMatrixViewer::MutualMatrixViewer(){
	mutual_info = NULL;
	stamp_pixmaps = NULL;
	stamp_items = NULL;

	this->setMinimumWidth(600);
	this->setFixedHeight(400);
}

MutualMatrixViewer::~MutualMatrixViewer(){

}

void MutualMatrixViewer::UpdateViewer(){

}

void MutualMatrixViewer::paintEvent(QPaintEvent *event){
	if ( stamp_pixmaps == NULL || mutual_info == NULL ) return;

	QPainter* painter = new QPainter(this);

	// Draw Mutual information Matrix
	int begin_x = 10;
	int begin_y = 10;
	int end_x = this->width()- 10;
	int end_y = this->height() - 10;

	if ( this->width() > this->height() ){
		begin_x += (this->width() - this->height()) / 2;
		end_x -= (this->width() - this->height()) / 2;
	} else {
		begin_y += (this->height() - this->width()) / 2;
		end_y -= (this->height() - this->width()) / 2;
	}

	int step = (end_x - begin_x) / (stamp_pixmaps->size() + 1);

	int item_size = step - 10;
	for ( int i = 1; i <= stamp_pixmaps->size(); ++i ){
		int temp_y = begin_y + i * step;
		if ( stamp_items->at(i - 1)->is_selected() ){
			painter->fillRect(begin_x - 5, temp_y - 5, end_x - begin_x + 5, step, Qt::yellow);
		}
	}

	for ( int i = 1; i <= stamp_pixmaps->size(); ++i ){
		int temp_x = begin_x + i * step;
		int temp_y = begin_y + i * step;
		float scale =  (float)stamp_pixmaps->at(i - 1)->height() / stamp_pixmaps->at(i - 1)->width();
		if ( stamp_items->at(i - 1)->is_selected() ){
			painter->fillRect(begin_x - 5, temp_y - 5, end_x - begin_x + 5, step, Qt::yellow);
		}

		painter->drawPixmap(temp_x, begin_y + (1.0 - scale) / 2 * item_size, item_size, item_size * scale, *stamp_pixmaps->at(i - 1));
		painter->drawPixmap(begin_x , temp_y + (1.0 - scale) / 2 * item_size, item_size, item_size * scale, *stamp_pixmaps->at(i - 1));
	}

	float min_value = 1e10;
	float max_value = -1e10;
	for ( int i = 0; i < stamp_pixmaps->size(); ++i )
		for ( int j = 0; j < stamp_pixmaps->size(); ++j )
			if ( i != j ){
				int temp_x = begin_x + (i + 1) * step;
				int temp_y = begin_y + (j + 1) * step;
				float value = 1.0 - mutual_info->at(i)[j];
				if ( value < 0 ) value = 0;
				if ( mutual_info->at(i)[j] > max_value ) max_value = mutual_info->at(i)[j];
				if ( mutual_info->at(i)[j] < min_value ) min_value = mutual_info->at(i)[j];
				QColor color = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0, 1, value);
				painter->fillRect(QRectF(temp_x, temp_y, item_size, item_size), color);
			}

	// Draw Color Bar
	QLinearGradient gradient(0, begin_y + 40, 0, end_y - 40);
	for ( int i = 0; i <= 25; ++i )
		gradient.setColorAt(i / 25.0,  QColorBarController::GetInstance(HEAT_MAP)->GetColor(0, 1, i / 25.0));
	painter->setBrush(gradient);
	painter->drawRect(end_x + 40, begin_y + 40, 20, end_y - begin_y - 80);

	delete painter;
}

void MutualMatrixViewer::mouseDoubleClickEvent(QMouseEvent *event){
	QPointF scene_pos = event->posF();

	int begin_x = 10;
	int begin_y = 10;
	int end_x = this->width()- 10;
	int end_y = this->height() - 10;

	if ( this->width() > this->height() ){
		begin_x += (this->width() - this->height()) / 2;
		end_x -= (this->width() - this->height()) / 2;
	} else {
		begin_y += (this->height() - this->width()) / 2;
		end_y -= (this->height() - this->width()) / 2;
	}
	int step = (end_x - begin_x) / (stamp_pixmaps->size() + 1);

	int x_index = (int)((scene_pos.rx() - begin_x) / step);
	int y_index = (int)((scene_pos.ry() - begin_y) / step);
	if ( x_index > 0 && y_index == 0 && x_index <= stamp_items->size() ){
		bool is_selected = stamp_items->at(x_index - 1)->is_selected();
		stamp_items->at(x_index - 1)->set_is_selected(!is_selected);
		stamp_items->at(x_index - 1)->update();
	} else if ( y_index > 0 && x_index == 0 && y_index<= stamp_items->size() ){
		bool is_selected = stamp_items->at(y_index - 1)->is_selected();
		stamp_items->at(y_index - 1)->set_is_selected(!is_selected);
		stamp_items->at(y_index - 1)->update();
	}

	this->update();
}

void MutualMatrixViewer::OnSelectionChanged(){
	this->update();
}