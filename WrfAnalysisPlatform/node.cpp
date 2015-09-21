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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <time.h>

#include "edge.h"
#include "node.h"
#include "graphwidget.h"

//! [0]
Node::Node(GraphWidget *graphWidget, QPixmap* pixmap, int id, int l)
    : graph(graphWidget), pixmap_(pixmap), id_(id), level_(l)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

	w = pixmap_->width();
	h = pixmap_->height();

	is_fixed_pos_ = false;
	is_only_pull_ = false;
	is_fixed_y_pos_ = false;
	is_selected_ = false;
}
//! [0]

Node::Node(GraphWidget *graphWidget, QPixmap* pixmap, std::vector< float >& attrib_weight, std::vector< float >& attrib_info, int id, int l)
	: graph(graphWidget), pixmap_(pixmap), id_(id), level_(l){

		weight_.assign(attrib_weight.begin(), attrib_weight.end());
		attrib_info.assign(attrib_info.begin(), attrib_info.end());

		setFlag(ItemIsMovable);
		setFlag(ItemSendsGeometryChanges);
		setCacheMode(DeviceCoordinateCache);
		setZValue(-1);

		w = pixmap_->width();
		h = pixmap_->height();

		is_fixed_pos_ = false;
		is_only_pull_ = false;
		is_fixed_y_pos_ = false;
		is_selected_ = false;
}

Node::~Node(){
	delete pixmap_;
}

//! [1]
void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<Edge *> Node::edges() const
{
    return edgeList;
}
//! [1]

//! [2]
void Node::calculateForces(int t)
{
    if ( !scene() || scene()->mouseGrabberItem() == this || graph == NULL ) {
        newPos = pos();
        return;
    }

//! [2]

//! [3]
    // Sum up all forces pushing this item away
	float k_value = 100.0;

    qreal xvel = 0;
    qreal yvel = 0;
	double total_weight = 0;
    foreach (QGraphicsItem *item, scene()->items()) {
        Node *node = qgraphicsitem_cast<Node *>(item);
        if (!node || node->id() == id_ )
            continue;
		if ( is_fixed_pos_ && (!this->collidesWithItem(node) || (this->collidesWithItem(node) && node->level() > 0)) ) continue;
		
		if ( this->level() > 0 && !this->collidesWithItem(node) && node->level() == 0 ) continue;

        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();

		float scale = 2.0;
		if ( this->level() > 0 ) scale = 1.5;

		double l = dx * dx + dy * dy;
		if (l > 0) {
			if ( is_only_pull_ ){
				if ( l < 400 ){
					xvel += (dx * k_value * k_value) / l;
					yvel += (dy * k_value * k_value) / l;
				}
			} else {
				xvel += (dx * k_value * k_value) / l * scale;
				yvel += (dy * k_value * k_value) / l * scale;
			}
		}
    }
//! [3]

//! [4]
    // Now subtract all forces pulling items together
    double weight = (edgeList.size() + 1) * 10;
    foreach (Edge *edge, edgeList) {
		if ( is_fixed_pos_ ) continue;
		QPointF vec;
		if (edge->sourceNode() == this)
			vec = mapToItem(edge->destNode(), 0, 0);
		else
			vec = mapToItem(edge->sourceNode(), 0, 0);
		qreal dx = vec.x();
		qreal dy = vec.y();
		/*double l = sqrt(dx * dx + dy * dy);
		if ( l != 0 ){
		xvel += dx * l / k_value;
		yvel += dy * l / k_value;
		}*/

		double l = sqrt(dx * dx + dy * dy);
		xvel -= vec.x() / weight / (0.1 + graph->item_forces_[edge->sourceNode()->id()][edge->destNode()->id()]);
		yvel -= vec.y() / weight / (0.1 + graph->item_forces_[edge->sourceNode()->id()][edge->destNode()->id()]);
    }
	
//! [4]

//! [5]
    if (qAbs(xvel) < 0.1 && qAbs(yvel) < 0.1)
        xvel = yvel = 0;
//! [5]

//! [6]
	if ( abs(xvel) + abs(yvel) > 40 ) 
		cool_scale_ = 1.0;
	else
		cool_scale_ *= 0.5;
    QRectF sceneRect = scene()->sceneRect();
	if ( this->level() == 0 )
		newPos = pos() + QPointF(xvel, yvel);
	else
		newPos = pos() + QPointF(xvel, yvel) * cool_scale_;
    newPos.setX(qMin(qMax(newPos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    newPos.setY(qMin(qMax(newPos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}
//! [6]

//! [7]
bool Node::advance()
{
    if (newPos == pos())
        return false;

    setPos(newPos);
    return true;
}
//! [7]

//! [8]
QRectF Node::boundingRect() const
{
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5)
    // Add some extra space around the circle for easier touching with finger
    qreal adjust = 30;
    return QRectF( -10 - adjust, -10 - adjust,
                  20 + adjust * 2, 20 + adjust * 2);
#else
    return QRectF( -10 - w / 2, -10 - h / 2,
                  w + 20, h + 70 );
#endif
}
//! [8]

//! [9]
QPainterPath Node::shape() const
{
    QPainterPath path;
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5)
    // Add some extra space around the circle for easier touching with finger
    path.addEllipse( -40, -40, 80, 80);
#else
    path.addEllipse(QPoint(0, 0), w + 20, h + 20);
#endif
    return path;
}
//! [9]

//! [10]
void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    /*painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);

    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor(Qt::yellow).light(120));
        gradient.setColorAt(0, QColor(Qt::darkYellow).light(120));
    } else {
        gradient.setColorAt(0, Qt::yellow);
        gradient.setColorAt(1, Qt::darkYellow);
    }
    painter->setBrush(gradient);*/

    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(QPoint(0, 0), 20 + w, 20 + h);
	painter->drawPixmap(QPoint(-w/ 2, -h / 2), *pixmap_);

	float histogram_height = 50;

	if ( is_selected_ ){
		painter->setPen(QPen(Qt::black, 5, Qt::DashDotLine));
		painter->drawLine(-w / 2, -h / 2, -w / 2, h / 2 + histogram_height);
		painter->drawLine(-w / 2, -h / 2, w / 2, -h / 2);
		painter->drawLine(w / 2, h / 2 + histogram_height, w / 2, -h / 2);
		painter->drawLine(w / 2, h / 2 + histogram_height, -w / 2, h / 2 + histogram_height);
	}

	if ( weight_.size() != 0 ){
		int step = w / weight_.size();
		if ( step > 20 ) step = 20;
		int bin_size = step - 5;
		int x_pos = -w / 2;
		int y_pos = h / 2;

		QString item_names[7];
		item_names[0] = QString("R");
		item_names[1] = QString("P");
		item_names[2] = QString("T");
		item_names[3] = QString("H");
		item_names[4] = QString("RH");
		item_names[5] = QString("WX");
		item_names[6] = QString("WY");
		painter->setPen(QPen(Qt::black));
		for ( int i = 0; i < weight_.size(); ++i ){
			painter->drawText(x_pos, y_pos, item_names[i]);
			if ( level_ == 0 )
				painter->fillRect(QRectF(x_pos, y_pos, bin_size, (histogram_height - 1) * weight_[i] + 1), Qt::green);
			else 
				painter->fillRect(QRectF(x_pos, y_pos, bin_size, (histogram_height - 1) * weight_[i] + 1), Qt::blue);
			x_pos += step;
		}
	}

	if ( info_.size() != 0 ){
		int step = h / info_.size();
		int bin_size = step - 5;
		int x_pos = w / 2;
		int y_pos = -h / 2;

		painter->setPen(QPen(Qt::black));
		for ( int i = 0; i < info_.size(); ++i ){
			painter->drawRect(x_pos, y_pos, histogram_height * info_[i], bin_size);
			y_pos += step;
		}
	}
}
//! [10]

//! [11]
QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if ( graph == NULL ){
		return QGraphicsItem::itemChange(change, value);
	}

    switch (change) {
    case ItemPositionHasChanged:
        foreach (Edge *edge, edgeList)
            edge->adjust();
        graph->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}
//! [11]

//! [12]
void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	is_selected_ = !is_selected_;
	emit ItemSelectionChanged();

    update();
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
	emit ItemSelected(id_);
}
//! [12]
