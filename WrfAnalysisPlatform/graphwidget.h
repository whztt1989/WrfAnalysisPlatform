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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QtGui/QGraphicsView>
#include <QtGui/QPixmap>

class WrfGridValueMap;
class WrfStampItem;
class Node;
class WrfStatisticSolver;

//! [0]
class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
	enum GraphMode{
		FORCE_MODE = 0x0,
		HIER_MODE
	};

    GraphWidget(GraphMode mode, QWidget *parent = 0);

    void itemMoved();

	void SetItemVarMaps(std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index);
	void AddVarMap(WrfGridValueMap* map);
	void AddOperationMap(std::vector< int >& related_ids, WrfGridValueMap* map);

	std::vector< std::vector< float > > item_forces_;
	std::vector< std::vector< float > >* GetMutualInfo() { return &item_forces_; }
	std::vector< QPixmap* >* GetPixmaps() { return &stamp_pixmaps_; }
	std::vector< Node* >* GetStampItems() { return &stamp_items_; }
	std::vector< WrfGridValueMap* >* GetValueMaps() { return &grid_value_maps_; }

public slots:
    void shuffle();
    void zoomIn();
    void zoomOut();
	void OnItemSelected(int);

signals:
	void BiasMapSelected(int);
	void SelectionChanged();

protected:
    void keyPressEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);

	void mouseDoubleClickEvent(QMouseEvent *event);

private:
    int timerId;
    Node *centerNode;
	GraphMode graph_mode_;
	int acc_id_;
	int cool_t_;

	std::vector< std::vector< WrfGridValueMap* > > hier_value_maps_;
	std::vector< WrfGridValueMap* > grid_value_maps_;
	std::vector< Node* > stamp_items_;
	std::vector< QPixmap* > stamp_pixmaps_;
	WrfStatisticSolver* statistic_solver_;

	void UpdateItemForces();
	void UpdateSingleItemForces();
	void UpdateLevelOnePos();
};
//! [0]

#endif
