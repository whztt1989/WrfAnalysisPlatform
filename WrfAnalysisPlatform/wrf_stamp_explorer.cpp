#include "wrf_stamp_explorer.h"

#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

#include "mutual_matrix_viewer.h"
#include "stamp_comparing_viewer.h"
#include "graphwidget.h"
#include "wrf_data_stamp.h"
#include "node.h"
#include "wrf_data_manager.h"

WrfStampExplorer::WrfStampExplorer(GraphWidget::GraphMode mode)
	: graph_mode_(mode){
	InitWidget();
	InitActions();
}

WrfStampExplorer::~WrfStampExplorer(){

}

void WrfStampExplorer::InitWidget(){
	stamp_viewer_ = new GraphWidget(graph_mode_);
	matrix_viewer_ = new MutualMatrixViewer;
	comparing_viewer_ = new StampComparingViewer;
	tool_bar_ = new QToolBar;
	comparing_tool_bar_ = new QToolBar;

	// set information for the matrix viewer and comparing viewer
	matrix_viewer_->mutual_info = stamp_viewer_->GetMutualInfo();
	matrix_viewer_->stamp_pixmaps = stamp_viewer_->GetPixmaps();
	matrix_viewer_->stamp_items = stamp_viewer_->GetStampItems();
	comparing_viewer_->stamp_pixmaps = stamp_viewer_->GetPixmaps();
	comparing_viewer_->value_maps = stamp_viewer_->GetValueMaps();


	QVBoxLayout* vbox_layout2 = new QVBoxLayout;
	vbox_layout2->addWidget(comparing_tool_bar_);
	vbox_layout2->addWidget(comparing_viewer_);
	QWidget* comparing_widget = new QWidget;
	comparing_widget->setLayout(vbox_layout2);

	QSplitter* vsplitter1 = new QSplitter(Qt::Vertical);
	vsplitter1->addWidget(matrix_viewer_);
	vsplitter1->addWidget(comparing_widget);

	QSplitter* hsplitter1 = new QSplitter(Qt::Horizontal);
	hsplitter1->addWidget(stamp_viewer_);
	hsplitter1->addWidget(vsplitter1);

	QVBoxLayout* vbox_layout1 = new QVBoxLayout;
	vbox_layout1->addWidget(tool_bar_);
	vbox_layout1->addWidget(hsplitter1);

	this->setLayout(vbox_layout1);

	connect(stamp_viewer_, SIGNAL(BiasMapSelected(int)), this, SLOT(OnBiasMapSelected(int)));
	connect(stamp_viewer_, SIGNAL(SelectionChanged()), matrix_viewer_, SLOT(OnSelectionChanged()));
}

void WrfStampExplorer::InitActions(){
	QAction* comparing_action = new QAction(QIcon("./Resources/compare.png"), QString(tr("Compare")), NULL);
	QAction* add_roi_action = new QAction(QIcon("./Resources/Brush.png"), QString(tr("Add roi")), NULL);
	QAction* clear_roi_action = new QAction(QIcon("./Resources/braffleErase.png"), QString(tr("Clear ROI")), NULL);
	QAction* minus_action = new QAction(QIcon("./Resources/Button-Delete-icon.png"), QString(tr("Minius")), NULL);
	QAction* plus_action = new QAction(QIcon("./Resources/Button-Add-icon.png"), QString(tr("Plus")), NULL);
	QAction* add_operation_map = new QAction(QIcon("./Resources/data-add-icon.png"), QString(tr("Add Operation Map")), NULL);

	tool_bar_->addAction(comparing_action);
	comparing_tool_bar_->addAction(add_roi_action);
	comparing_tool_bar_->addAction(clear_roi_action);
	comparing_tool_bar_->addAction(minus_action);
	comparing_tool_bar_->addAction(plus_action);
	comparing_tool_bar_->addAction(add_operation_map);

	connect(comparing_action, SIGNAL(triggered()), this, SLOT(OnComparingActionTriggered()));
	connect(add_operation_map, SIGNAL(triggered()), this, SLOT(OnAddOperationMapTriggered()));
	connect(clear_roi_action, SIGNAL(triggered()), comparing_viewer_, SLOT(OnClearRoiActionTriggered()));
	connect(add_roi_action, SIGNAL(triggered()), comparing_viewer_, SLOT(OnAddRoiActionTriggered()));
	connect(minus_action, SIGNAL(triggered()), comparing_viewer_, SLOT(OnMinusActionTriggered()));
	connect(plus_action, SIGNAL(triggered()), comparing_viewer_, SLOT(OnPlusActionTriggered()));
	
}

void WrfStampExplorer::AddVarMap(WrfGridValueMap* map){
	stamp_viewer_->AddVarMap(map);
}

void WrfStampExplorer::AddOperationMap(std::vector< int >& related_ids, WrfGridValueMap* map){
	stamp_viewer_->AddOperationMap(related_ids, map);
}

void WrfStampExplorer::SetItemVarMaps(std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index){
	stamp_viewer_->SetItemVarMaps(maps, weights, changed_index);
}

void WrfStampExplorer::OnBiasMapSelected(int index){
	this->setVisible(false);
	emit BiasMapSelected(index);
}

void WrfStampExplorer::OnComparingActionTriggered(){
	std::vector< int > seleted_ids;
	std::vector< Node* >* stamp_items = stamp_viewer_->GetStampItems();
	for ( int i = 0; i < stamp_items->size(); ++i )
		if ( stamp_items->at(i)->is_selected() ){
			seleted_ids.push_back(stamp_items->at(i)->id());
		}
	if ( seleted_ids.size() == 2 ){
		comparing_viewer_->SetComparingId(seleted_ids[0], seleted_ids[1]);
	} else {
		QMessageBox::information(this, tr("Information"), tr("Please select two stamps"));
	}
}

void WrfStampExplorer::OnAddOperationMapTriggered(){
	std::vector< int > seleted_ids;
	std::vector< Node* >* stamp_items = stamp_viewer_->GetStampItems();
	for ( int i = 0; i < stamp_items->size(); ++i )
		if ( stamp_items->at(i)->is_selected() ){
			seleted_ids.push_back(stamp_items->at(i)->id());
		}
	if ( seleted_ids.size() != 2) return;

	WrfGridValueMap* temp_value_map = comparing_viewer_->GetOperationMap();

	WrfGridValueMap* value_map = new WrfGridValueMap;
	value_map->start_latitude = temp_value_map->start_latitude;
	value_map->end_latitude = temp_value_map->end_latitude;
	value_map->start_longitude = temp_value_map->start_longitude;
	value_map->end_longitude = temp_value_map->end_longitude;
	value_map->latitude_grid_number = temp_value_map->latitude_grid_number;
	value_map->longitude_grid_number = temp_value_map->longitude_grid_number;
	value_map->latitude_grid_space = temp_value_map->latitude_grid_space;
	value_map->longitude_grid_space = temp_value_map->longitude_grid_space;
	value_map->max_value = temp_value_map->max_value;
	value_map->min_value = temp_value_map->min_value;
	value_map->level = temp_value_map->level;
	value_map->weight.reserve(temp_value_map->weight.size());
	value_map->weight.assign(temp_value_map->weight.begin(), temp_value_map->weight.end());
	value_map->values.resize(temp_value_map->longitude_grid_number * temp_value_map->latitude_grid_number);
	value_map->values.assign(temp_value_map->values.begin(), temp_value_map->values.end());

	WrfDataManager::GetInstance()->AddStoryStamp(value_map, seleted_ids);
	stamp_viewer_->AddOperationMap(seleted_ids, value_map);
}