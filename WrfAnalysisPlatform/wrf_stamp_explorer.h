#ifndef WRF_STAMP_EXPLORER_H_
#define WRF_STAMP_EXPLORER_H_

#include <QtGui/QWidget>
#include <QtGui/QToolBar>
#include <vector>
#include "graphwidget.h"

class MutualMatrixViewer;
class StampComparingViewer;
class WrfGridValueMap;

class WrfStampExplorer : public QWidget
{
	Q_OBJECT

public:
	WrfStampExplorer(GraphWidget::GraphMode mode);
	~WrfStampExplorer();

	void AddVarMap(WrfGridValueMap* map);
	void AddOperationMap(std::vector< int >& related_ids, WrfGridValueMap* map);
	void SetItemVarMaps(std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index);

signals:
	void BiasMapSelected(int);

private:
	GraphWidget* stamp_viewer_;
	MutualMatrixViewer* matrix_viewer_;
	StampComparingViewer* comparing_viewer_;
	QToolBar* tool_bar_;
	QToolBar* comparing_tool_bar_;
	GraphWidget::GraphMode graph_mode_;

	void InitWidget();
	void InitActions();

	private slots:
		void OnComparingActionTriggered();
		void OnBiasMapSelected(int);
		void OnAddOperationMapTriggered();
};

#endif