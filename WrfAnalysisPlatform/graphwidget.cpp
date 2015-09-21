/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "graphwidget.h"
#include "edge.h"
#include "node.h"
#include "wrf_stamp_generator.h"
#include "wrf_data_stamp.h"
#include "wrf_statistic_solver.h"
#include "qcolor_bar_controller.h"
#include <opencv2/core/core.hpp> 

#include <QtGui>

#include <math.h>

//! [0]
GraphWidget::GraphWidget(GraphMode mode, QWidget *parent)
    : QGraphicsView(parent), timerId(0)
{
	statistic_solver_ = new WrfStatisticSolver;
	graph_mode_ = mode;
	acc_id_ = 0;
	hier_value_maps_.resize(10);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-1000, -1000, 2000, 2000);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    //scale(qreal(0.8), qreal(0.8));
    setMinimumSize(600, 600);
    setWindowTitle(tr("Elastic Nodes"));

	if ( mode == HIER_MODE ){
		item_forces_.resize(50);
		for ( int i = 0; i < item_forces_.size(); ++i ) item_forces_[i].resize(50, 0);
	}
}
//! [1]

//! [2]
void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
	cool_t_ = 20;
}
//! [2]

//! [3]
void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        centerNode->moveBy(0, -20);
        break;
    case Qt::Key_Down:
        centerNode->moveBy(0, 20);
        break;
    case Qt::Key_Left:
        centerNode->moveBy(-20, 0);
        break;
    case Qt::Key_Right:
        centerNode->moveBy(20, 0);
        break;
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
        shuffle();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}
//! [3]

//! [4]
void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QList<Node *> nodes;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
    }

    foreach (Node *node, nodes)
        node->calculateForces(cool_t_);

    bool itemsMoved = false;
    foreach (Node *node, nodes) {
        if (node->advance())
            itemsMoved = true;
    }

	cool_t_ -= 1;
    if (!itemsMoved || cool_t_ < 0 ) {
        killTimer(timerId);
        timerId = 0;
    }
}
//! [4]

//! [5]
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, -event->delta() / 240.0));
}
//! [5]

//! [6]
void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // Shadow
    QRectF sceneRect = this->sceneRect();
    QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    if (rightShadow.intersects(rect) || rightShadow.contains(rect))
	painter->fillRect(rightShadow, Qt::darkGray);
    if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
	painter->fillRect(bottomShadow, Qt::darkGray);

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersect(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);

#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5)
    // Text
    /*QRectF textRect(sceneRect.left() + 4, sceneRect.top() + 4,
                    sceneRect.width() - 4, sceneRect.height() - 4);
    QString message(tr("Click and drag the nodes around, and zoom with the mouse "
                       "wheel or the '+' and '-' keys"));

    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(14);
    painter->setFont(font);
    painter->setPen(Qt::lightGray);
    painter->drawText(textRect.translated(2, 2), message);
    painter->setPen(Qt::black);
    painter->drawText(textRect, message);*/
#endif
}
//! [6]

void GraphWidget::mouseDoubleClickEvent(QMouseEvent *event){
	QGraphicsView::mouseDoubleClickEvent(event);
}

//! [7]
void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}
//! [7]

void GraphWidget::shuffle()
{
    /*foreach (QGraphicsItem *item, scene()->items()) {
        if (qgraphicsitem_cast<Node *>(item))
            item->setPos(-150 + qrand() % 300, -150 + qrand() % 300);
    }*/
}

void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}

void GraphWidget::SetItemVarMaps(std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index){
	for ( int i = 0; i < stamp_items_.size(); ++i ){
		disconnect(stamp_items_[i], SIGNAL(ItemSelected(int)), this, SLOT(OnItemSelected(int)));
	}

	stamp_items_.clear();
	stamp_pixmaps_.clear();
	scene()->clear();

	grid_value_maps_.resize(maps.size());
	grid_value_maps_.assign(maps.begin(), maps.end());

	srand((unsigned int)time(0));
	for ( int i = 0; i < grid_value_maps_.size(); ++i ){
		QString tool_tip_string = QString("Rain: %0\t\nPressure: %1  \t\nTemperature: %2  \t\nHeight: %3  \t\nRelative Humidity: %4 \t\nWind_longi: %5  \t\nWind_lati: %6  \t\n").arg(weights[i][0]).arg(weights[i][1]).arg(weights[i][2]).arg(weights[i][3]).arg(weights[i][4]).arg(weights[i][5]).arg(weights[i][6]);
		QString changed_string;
		switch (changed_index[i]){
		case -1:
			changed_string = "None";
			break;
		case 0:
			changed_string = "Rain";
			break;
		case 1:
			changed_string = "Pressure";
			break;
		case 2:
			changed_string = "Temperature";
			break;
		case 3:
			changed_string = "Height";
			break;
		case 4:
			changed_string = "Wind_longi";
			break;
		case 5:
			changed_string = "Wind_lati";
			break;
		case 6:
			changed_string = "Relative Humidity";
			break;
		default:
			break;
		}
		tool_tip_string += QString("Changed Attribute: ") + changed_string + QString("\t\n");
		WrfGridValueMap* value_map = grid_value_maps_[i];
		int w = (value_map->end_longitude - value_map->start_longitude) * 3;
		int h = (value_map->end_latitude - value_map->start_latitude) * 3;
		QPixmap* temp_map = WrfStampGenerator::GetInstance()->GenerateStamp(value_map, w, h);

		stamp_pixmaps_.push_back(temp_map);

		Node *node = new Node(this, temp_map, value_map->weight, value_map->info, i, value_map->level);

		stamp_items_.push_back(node);

		scene()->addItem(node);
		node->setPos(100.0 * rand() / RAND_MAX, 100.0 * rand() / RAND_MAX);

		//if ( i == 0 ) node->SetFixedPosOn();

		connect(node, SIGNAL(ItemSelected(int)), this, SLOT(OnItemSelected(int)));
		connect(node, SIGNAL(ItemSelectionChanged()), this, SIGNAL(SelectionChanged()));

		node->setToolTip(tool_tip_string);
	}

	UpdateItemForces();
	for ( int i = 1; i < stamp_items_.size(); ++i ) scene()->addItem(new Edge(stamp_items_[0], stamp_items_[i]));
	for ( int i = 1; i < stamp_items_.size() - 1; ++i )
		for ( int j = i + 1; j < stamp_items_.size(); ++j )
			scene()->addItem(new Edge(stamp_items_[i], stamp_items_[j]));
				
}

void GraphWidget::UpdateItemForces(){
	item_forces_.resize(grid_value_maps_.size());
	for ( int i = 0; i < item_forces_.size(); ++i ) {
		item_forces_[i].resize(grid_value_maps_.size());
		memset(item_forces_[i].data(), 0, sizeof(float) * grid_value_maps_.size());
	}

	for ( int i = 0; i < grid_value_maps_.size() - 1; ++i )
		for ( int j = i + 1; j < grid_value_maps_.size(); ++j ){
			float temp_force = statistic_solver_->GetMutalDistance(grid_value_maps_[i], grid_value_maps_[j]);
			item_forces_[i][j] = temp_force;
			item_forces_[j][i] = temp_force;
		}
}

void GraphWidget::UpdateSingleItemForces(){
	item_forces_.resize(grid_value_maps_.size());
	for ( int i = 0; i < item_forces_.size(); ++i ) item_forces_[i].resize(grid_value_maps_.size());

	for ( int i = 0; i < item_forces_.size() - 1; ++i ){
		float temp_force = statistic_solver_->GetMutalDistance(grid_value_maps_[i], grid_value_maps_[grid_value_maps_.size() - 1]);
		item_forces_[i][grid_value_maps_.size() - 1] = temp_force;
		item_forces_[grid_value_maps_.size() - 1][i] = temp_force;
	}
	item_forces_[grid_value_maps_.size() - 1][grid_value_maps_.size() - 1] = 0;
}

void GraphWidget::AddVarMap(WrfGridValueMap* map){
	QString tool_tip_string = QString("Rain: %0\t\nPressure: %1  \t\nTemperature: %2  \t\nHeight: %3  \t\nRelative Humidity: %4 \t\nWind_longi: %5  \t\nWind_lati: %6  \t\n").arg(map->weight[0]).arg(map->weight[1]).arg(map->weight[2]).arg(map->weight[3]).arg(map->weight[4]).arg(map->weight[5]).arg(map->weight[6]);
	int w = (map->end_longitude - map->start_longitude) * 3;
	int h = (map->end_latitude - map->start_latitude) * 3;
	QPixmap* temp_map = WrfStampGenerator::GetInstance()->GenerateStamp(map, w, h);

	stamp_pixmaps_.push_back(temp_map);

	Node* node = new Node(this, temp_map, map->weight, map->info, acc_id_, map->level);

	int x = -500 + 30 + temp_map->width() / 2 + (temp_map->width() + 50) * map->level;
	int y = -500 + 50.0 + hier_value_maps_[map->level].size() * (temp_map->height() + 100) + temp_map->height() / 2;
	node->setPos(x, y);

	scene()->addItem(node);

	stamp_items_.push_back(node);
	hier_value_maps_[map->level].push_back(map);
	grid_value_maps_.push_back(map);

	node->SetFixedPosOn();

	connect(node, SIGNAL(ItemSelected(int)), this, SLOT(OnItemSelected(int)));
	connect(node, SIGNAL(ItemSelectionChanged()), this, SIGNAL(SelectionChanged()));

	node->setToolTip(tool_tip_string);

	acc_id_++;

	UpdateSingleItemForces();

	if ( grid_value_maps_.size() > 2 ) UpdateLevelOnePos();
}

void GraphWidget::AddOperationMap(std::vector< int >& related_ids, WrfGridValueMap* map){
	// add var map
	QString tool_tip_string;
	if ( map->level == 0 ){
		tool_tip_string = QString("Rain: %0\t\nPressure: %1  \t\nTemperature: %2  \t\nHeight: %3  \t\nRelative Humidity: %4 \t\nWind_longi: %5  \t\nWind_lati: %6  \t\n").arg(map->weight[0]).arg(map->weight[1]).arg(map->weight[2]).arg(map->weight[3]).arg(map->weight[4]).arg(map->weight[5]).arg(map->weight[6]);
	} else {
		tool_tip_string = QString("Operation Map");
	}

	int w = (map->end_longitude - map->start_longitude) * 3;
	int h = (map->end_latitude - map->start_latitude) * 3;
	QPixmap* temp_map = WrfStampGenerator::GetInstance()->GenerateStamp(map, w, h);

	stamp_pixmaps_.push_back(temp_map);

	Node* node = new Node(this, temp_map, map->weight, map->info, acc_id_, map->level);
	//node->SetOnlyPullOn();

	scene()->addItem(node);
	node->setPos(-500 + 30 + temp_map->width() / 2 + (temp_map->width() + 50) * map->level, -500 + 50.0 + hier_value_maps_[map->level].size() * (temp_map->height() + 100) + temp_map->height() / 2);

	stamp_items_.push_back(node);
	hier_value_maps_[map->level].push_back(map);
	grid_value_maps_.push_back(map);

	connect(node, SIGNAL(ItemSelected(int)), this, SLOT(OnItemSelected(int)));
	connect(node, SIGNAL(ItemSelectionChanged()), this, SIGNAL(SelectionChanged()));

	node->setToolTip(tool_tip_string);

	acc_id_++;

	UpdateSingleItemForces();

	for ( int i = 0; i < related_ids.size(); ++i )
		for ( int j = 0; j < stamp_items_.size(); ++j )
			if ( stamp_items_[j]->id() == related_ids[i] ){
				Edge* new_edget = new Edge(node, stamp_items_[j]);
				scene()->addItem(new_edget);
				break;
			}
}

void GraphWidget::OnItemSelected(int id){
	if ( id < grid_value_maps_.size() ){
		emit BiasMapSelected(id);
	}
}

void GraphWidget::UpdateLevelOnePos(){
	std::vector< std::vector< float > > weight;
	std::vector< int > indexes;
	for ( int i = 0; i < grid_value_maps_.size(); ++i )
		if ( grid_value_maps_[i]->level == 0 ) {
			weight.push_back(grid_value_maps_[i]->weight);
			indexes.push_back(i);
		}
	if ( weight.size() == 0 ) return;
	cv::Mat data_map(weight.size(), weight[0].size(), CV_32F);
	for ( int i = 0; i < weight.size(); ++i )
		for ( int j = 0; j < weight[i].size(); ++j )
			data_map.at<float>(i, j) = weight[i][j];
	cv::PCA pca(data_map, cv::Mat(), 0);
	std::vector< std::vector< float > > scaled_vec;
	scaled_vec.resize(2);
	for ( int i = 0; i < scaled_vec.size(); ++i ) scaled_vec[i].resize(weight[0].size());
	for ( int i = 0; i < 2; ++i )
		for ( int j = 0; j < weight[0].size(); ++j )
			scaled_vec[i][j] = pca.eigenvectors.at<float>(i, j);
	std::vector< float > pos;
	for ( int i = 0; i < weight.size(); ++i ){
		float x = 0, y = 0;
		for ( int j = 0; j < weight[0].size(); ++j ){
			x += scaled_vec[0][j] * weight[i][j];
			y += scaled_vec[1][j] * weight[i][j];
		}
		pos.push_back(x);
		pos.push_back(y);
	}
	float min_x = 1e10, min_y = 1e10;
	float max_x = -1e10, max_y = -1e10;
	for ( int i = 0; i < pos.size() / 2; ++i ){
		if ( pos[i * 2] > max_x ) max_x = pos[i * 2];
		if ( pos[i * 2] < min_x ) min_x = pos[i * 2];
		if ( pos[i * 2 + 1] > max_y ) max_y = pos[i * 2 + 1];
		if ( pos[i * 2 + 1] < min_y ) min_y = pos[i * 2 + 1];
	}

	QRectF sceneRect = this->sceneRect();
	for ( int i = 0; i < pos.size() / 2; ++i ){
		int x = sceneRect.left() + 400 + (pos[i * 2] - min_x) / (max_x - min_x) * (sceneRect.width() - 800);
		int y = sceneRect.top() + 400 + (pos[i * 2 + 1] - min_y) / (max_y - min_y) * (sceneRect.height() - 800);
		stamp_items_[indexes[i]]->setPos(x, y);
	}
}