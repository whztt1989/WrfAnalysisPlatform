#include "wrf_analysis_platform.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QSplitter>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <fstream>
#include "wrf_common.h"
#include "wrf_central_viewer.h"
#include "wrf_common.h"
#include "wrf_data_stamp.h"
#include "wrf_data_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_parallel_coordinate.h"
#include "wrf_matrix_viewer.h"
#include "wrf_line_chart.h"
#include "wrf_parameter_widget.h"
#include "wrf_weight_histogram.h"
#include "wrf_stamp_viewer.h"
#include "graphwidget.h"
#include "story_line_widget.h"
#include "wrf_stamp_explorer.h"

WrfAnalysisPlatform::WrfAnalysisPlatform(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	data_manager_ = WrfDataManager::GetInstance();

	InitWidget();
	InitToolBar();
	InitConnections();

	this->showMaximized();

	/*OnActionOpenProjectTriggered();
	OnActionGenerateTestPcpTriggered();
	OnActionGenerateTestLineChartTriggered();
	OnActionGenerateTestHistogramTriggered();*/
}

WrfAnalysisPlatform::~WrfAnalysisPlatform(){

}

void WrfAnalysisPlatform::InitToolBar(){
	QAction* selecting_site_action = new QAction(QIcon("./Resources/arrow.png"), QString(tr("Selecting Site")), this);
	this->ui.mainToolBar->addAction(selecting_site_action);
	QAction* brush_add_area_action = new QAction(QIcon("./Resources/Brush.png"), QString(tr("Adding Area")), this);
	this->ui.mainToolBar->addAction(brush_add_area_action);
	QAction* erase_area_action = new QAction(QIcon("./Resources/braffleErase.png"), QString(tr("Erasing Area")), this);
	this->ui.mainToolBar->addAction(erase_area_action);
	QAction* magic_wand_action = new QAction(QIcon("./Resources/magic_wand.png"), QString(tr("Magic Wand")), this);
	this->ui.mainToolBar->addAction(magic_wand_action);
	this->ui.mainToolBar->addSeparator();

	QAction* compared_action = new QAction(QIcon("./Resources/contrast_s.png"), QString(tr("Compare")), this);
	this->ui.mainToolBar->addAction(compared_action);
	QAction* normal_action = new QAction(QIcon("./Resources/normal_s.png"), QString(tr("Normal")), this);
	this->ui.mainToolBar->addAction(normal_action);
	QAction* map_action = new QAction(QIcon("./Resources/map_guide.png"), QString("Map Guide"), this);
	this->ui.mainToolBar->addAction(map_action);
	QAction* enhanced_map_action = new QAction(QIcon("./Resources/enhanced_map.png"), QString("Enhanced Map Guide"), this);
	this->ui.mainToolBar->addAction(enhanced_map_action);
	this->ui.mainToolBar->addSeparator();

	QAction* apply_change_action = new QAction(QIcon("./Resources/data-apply-icon.png"), QString(tr("Apply changes")), this);
	this->ui.mainToolBar->addAction(apply_change_action);
	QAction* add_map_action = new QAction(QIcon("./Resources/Button-Add-icon.png"), QString(tr("Add map")), this);
	this->ui.mainToolBar->addAction(add_map_action);
	this->ui.mainToolBar->addSeparator();

	QAction* enhance_bias_parallel = new QAction(QString(tr("Cluster Dev PCP")), this);
	this->ui.mainToolBar->addAction(enhance_bias_parallel);
	QAction* enhance_abs_parallel = new QAction(QString(tr("Cluster Abs PCP")), this);
	this->ui.mainToolBar->addAction(enhance_abs_parallel);
	QAction* enhance_bias_line = new QAction(QString(tr("Cluster Dev LC")), this);
	this->ui.mainToolBar->addAction(enhance_bias_line);
	QAction* enhance_abs_line = new QAction(QString(tr("Cluster Abs LC")), this);
	this->ui.mainToolBar->addAction(enhance_abs_line);


	connect(selecting_site_action, SIGNAL(triggered()), this, SLOT(OnActionSelectingSiteTriggered()));
	connect(brush_add_area_action, SIGNAL(triggered()), this, SLOT(OnActionBrushingSiteTriggered()));
	connect(erase_area_action, SIGNAL(triggered()), this, SLOT(OnActionErasingSiteTriggered()));
	connect(magic_wand_action, SIGNAL(triggered()), this, SLOT(OnActionMagicWandTriggered()));
	connect(compared_action, SIGNAL(triggered()), this, SLOT(OnActionComparedModeTriggered()));
	connect(normal_action, SIGNAL(triggered()), this, SLOT(OnActionNormalModeTriggered()));
	connect(apply_change_action, SIGNAL(triggered()), bias_line_chart_viewer_, SLOT(ApplyChanges()));
	connect(apply_change_action, SIGNAL(triggered()), absolute_line_chart_viewer_, SLOT(ApplyChanges()));
	connect(map_action, SIGNAL(triggered()), this, SLOT(OnActionStampMapViewTriggered()));
	connect(enhanced_map_action, SIGNAL(triggered()), this, SLOT(OnActionComplexStampMapViewTriggered()));
	connect(add_map_action, SIGNAL(triggered()), this, SLOT(OnActionAddReferenceMapTriggered()));
	connect(enhance_bias_parallel, SIGNAL(triggered()), this, SLOT(OnActionClusterBiasParallelRecordsTriggered()));
	connect(enhance_abs_parallel, SIGNAL(triggered()), this, SLOT(OnActionClusterAbsoluteParallelRecordsTriggered()));
	connect(enhance_bias_line, SIGNAL(triggered()), this, SLOT(OnActionClusterBiasLineChartRecordsTriggered()));
	connect(enhance_abs_line, SIGNAL(triggered()), this, SLOT(OnActionClusterAbsoluteLineChartRecordsTriggered()));
}

void WrfAnalysisPlatform::InitWidget(){
	central_viewer_ = new WrfCentralViewer;
	bias_parallel_viewer_ = new WrfParallelCoordinate;
	bias_parallel_viewer_->set_view_mode(WrfParallelCoordinate::BIAS_VIEW);
	absolute_parallel_viewer_ = new WrfParallelCoordinate;
	absolute_parallel_viewer_->set_view_mode(WrfParallelCoordinate::ABSOLUTE_VIEW);
	matrix_viewer_ = new WrfMatrixViewer;
	bias_line_chart_viewer_ = new WrfLineChart(WrfLineChart::BIAS_VIEW);
	absolute_line_chart_viewer_ = new WrfLineChart(WrfLineChart::ABSOLUTE_VIEW);
	para_widget_ = new WrfParameterWidget;
	histogram_viewer_ = new WrfWeightHistogram;
	random_stamp_viewer_ = new WrfStampExplorer(GraphWidget::FORCE_MODE);
	positioned_stamp_viewer_ = new WrfStampExplorer(GraphWidget::HIER_MODE);
	story_line_viewer_ = new StoryLineWidget;
	QTabWidget* tab_widget = new QTabWidget;
	tab_widget->addTab(story_line_viewer_, QString("Analysis Board"));
	tab_widget->addTab(bias_parallel_viewer_, QString("Dev Parallel Coordinate"));
	tab_widget->addTab(absolute_parallel_viewer_, QString("Absolute Parallel Coordinate"));

	/*QWidget* central_viewer_panel = new QWidget;
	QWidget* central_viewer_control_panel = new QWidget;
	central_viewer_control_panel->setFixedWidth(30);
	QVBoxLayout* layout1 = new QVBoxLayout;
	layout1->setMargin(0);
	QPushButton* selecting_site_button = new QPushButton(QIcon("./Resources/arrow.png"), QString(tr("Selecting Site")));
	layout1->addWidget(selecting_site_button);
	central_viewer_control_panel->setLayout(layout1);
	QHBoxLayout* layout2 = new QHBoxLayout;
	layout2->addWidget(central_viewer_control_panel);
	layout2->addWidget(central_viewer_);
	layout2->setMargin(0);
	central_viewer_panel->setLayout(layout2);*/

	QSplitter* central_parallel_splitter = new QSplitter(Qt::Vertical);
	central_parallel_splitter->addWidget(central_viewer_);
	central_parallel_splitter->addWidget(tab_widget);

	QSplitter* function_splitter = new QSplitter(Qt::Vertical);
	function_splitter->addWidget(para_widget_);
	function_splitter->addWidget(bias_line_chart_viewer_);
	function_splitter->addWidget(absolute_line_chart_viewer_);
	function_splitter->addWidget(histogram_viewer_);

	QSplitter* central_widget_splitter = new QSplitter(Qt::Horizontal);
	central_widget_splitter->addWidget(central_parallel_splitter);
	central_widget_splitter->addWidget(function_splitter);

	QVBoxLayout* central_layout = new QVBoxLayout;
	central_layout->addWidget(central_widget_splitter);

	this->ui.centralWidget->setLayout(central_layout);
}

void WrfAnalysisPlatform::InitConnections(){
	connect(ui.actionLoad_Historical_Data, SIGNAL(triggered()), this, SLOT(OnActionLoadHistoricalDataTriggered()));
	connect(ui.actionLoad_Numerical_Data, SIGNAL(triggered()), this, SLOT(OnActionLoadNumericalDataTriggered()));
	connect(ui.actionView_Variation_Map, SIGNAL(triggered()), this, SLOT(OnActionViewVarationMapTriggered()));
	connect(ui.actionGenerate_Pcp, SIGNAL(triggered()), this, SLOT(OnActionGenerateTestPcpTriggered()));
	connect(ui.actionOpen_Project, SIGNAL(triggered()), this, SLOT(OnActionOpenProjectTriggered()));
	connect(ui.actionSave_Project, SIGNAL(triggered()), this, SLOT(OnActionSaveProjectTriggered()));
	connect(ui.actionGenerate_Bias_Map, SIGNAL(triggered()), this, SLOT(OnActionGenerateBiasMapTriggered()));
	connect(ui.actionGenerate_Line_Chart, SIGNAL(triggered()), this, SLOT(OnActionGenerateTestLineChartTriggered()));
	connect(ui.actionGenerate_Matrix_Chart, SIGNAL(triggered()), this, SLOT(OnActionGenerateTestMatrixChartTriggered()));
	connect(ui.actionGenerate_Histogram, SIGNAL(triggered()), this, SLOT(OnActionGenerateTestHistogramTriggered()));
	connect(ui.actionStart_Editing, SIGNAL(triggered()), this, SLOT(OnActionStartEditingTriggered()));
	connect(ui.actionEnd_Editing, SIGNAL(triggered()), this, SLOT(OnActionEndEditingTriggered()));
	connect(ui.actionParallel_Absolute_View, SIGNAL(triggered()), this, SLOT(OnActionParallelAbsoluteViewTriggered()));
	connect(ui.actionParallel_Bias_View, SIGNAL(triggered()), this, SLOT(OnActionParallelBiasViewTriggered()));
	connect(ui.actionAbsolute_Line_View, SIGNAL(triggered()), this, SLOT(OnActionAbsoluteLineViewTriggered()));
	connect(ui.actionBias_Line_View, SIGNAL(triggered()), this, SLOT(OnActionBiasLineViewTriggered()));
	connect(ui.actionStamp_Map_View, SIGNAL(triggered()), this, SLOT(OnActionStampMapViewTriggered()));
	connect(ui.actionAdd_Reference_Map, SIGNAL(triggered()), this, SLOT(OnActionAddReferenceMapTriggered()));
	connect(ui.actionSum, SIGNAL(triggered()), this, SLOT(OnActionSumTriggered()));
	connect(ui.actionMinus, SIGNAL(triggered()), this, SLOT(OnActionMiniusTriggered()));
	connect(ui.actionVariation, SIGNAL(triggered()), this, SLOT(OnActionVariationTriggered()));
	connect(ui.actionComplex_Stamp_Map_View, SIGNAL(triggered()), this, SLOT(OnActionComplexStampMapViewTriggered()));
	connect(ui.actionAccept_Region_Growing, SIGNAL(triggered()), this, SLOT(OnActionAcceptRegionGrowingTriggered()));
	connect(ui.actionCluster_Absolute_Line_Chart_Records, SIGNAL(triggered()), this, SLOT(OnActionClusterAbsoluteLineChartRecordsTriggered()));
	connect(ui.actionCluster_Bias_Line_Chart_Records, SIGNAL(triggered()), this, SLOT(OnActionClusterBiasLineChartRecordsTriggered()));
	connect(ui.actionCluster_Absolute_Parallel_Records, SIGNAL(triggered()), this, SLOT(OnActionClusterAbsoluteParallelRecordsTriggered()));
	connect(ui.actionCluster_Bis_Parallel_Records, SIGNAL(triggered()), this, SLOT(OnActionClusterBiasParallelRecordsTriggered()));

	connect(central_viewer_, SIGNAL(SelectedAreaChanged()), bias_parallel_viewer_, SLOT(OnSelectedAreaChanged()));
	connect(central_viewer_, SIGNAL(SelectedAreaChanged()), absolute_parallel_viewer_, SLOT(OnSelectedAreaChanged()));
	connect(central_viewer_, SIGNAL(SelectedAreaChanged()), bias_line_chart_viewer_, SLOT(OnSelectedAreaChanged()));
	connect(central_viewer_, SIGNAL(SelectedAreaChanged()), absolute_line_chart_viewer_, SLOT(OnSelectedAreaChanged()));
	connect(central_viewer_, SIGNAL(SelectedAreaChanged()), this, SLOT(OnSelectedAreaChanged()));
	connect(para_widget_, SIGNAL(WindVarThresholdChanged()), central_viewer_, SLOT(OnWindVarThresholdChanged()));
	connect(para_widget_, SIGNAL(BiasWeightChanged()), this, SLOT(OnBiasWeightChanged()));
	connect(para_widget_, SIGNAL(HistoryLengthChanged(int)), this, SLOT(OnHistoryLengthChanged(int)));
	connect(bias_parallel_viewer_, SIGNAL(AttributeSelectedChanged(WrfGeneralDataStampType)), bias_line_chart_viewer_, SLOT(OnSelectedAttributeChanged(WrfGeneralDataStampType)));
	connect(bias_parallel_viewer_, SIGNAL(AttributeSelectedChanged(WrfGeneralDataStampType)), absolute_line_chart_viewer_, SLOT(OnSelectedAttributeChanged(WrfGeneralDataStampType)));
	connect(bias_parallel_viewer_, SIGNAL(InfoGainSelected(int)), this, SLOT(OnInfoGainSelected(int)));
	connect(absolute_parallel_viewer_, SIGNAL(AttributeSelectedChanged(WrfGeneralDataStampType)), bias_line_chart_viewer_, SLOT(OnSelectedAttributeChanged(WrfGeneralDataStampType)));
	connect(absolute_parallel_viewer_, SIGNAL(AttributeSelectedChanged(WrfGeneralDataStampType)), absolute_line_chart_viewer_, SLOT(OnSelectedAttributeChanged(WrfGeneralDataStampType)));
	connect(absolute_parallel_viewer_, SIGNAL(InfoGainSelected(int)), this, SLOT(OnInfoGainSelected(int)));
	connect(random_stamp_viewer_, SIGNAL(BiasMapSelected(int)), this, SLOT(OnBiasMapSelected(int)));
	connect(story_line_viewer_, SIGNAL(BiasMapSelected(int)), this, SLOT(OnStoryStampSelected(int)));
	connect(positioned_stamp_viewer_, SIGNAL(BiasMapSelected(int)), this, SLOT(OnStoryStampSelected(int)));
	connect(bias_parallel_viewer_, SIGNAL(AttribWeightChanged()), this, SLOT(OnAttribWeightChanged()));
	connect(absolute_parallel_viewer_, SIGNAL(AttribWeightChanged()), this, SLOT(OnAttribWeightChanged()));

	connect(bias_parallel_viewer_, SIGNAL(HighLightChanged()), central_viewer_, SLOT(OnHighLightChanged()));
	connect(bias_parallel_viewer_, SIGNAL(HighLightChanged()), absolute_parallel_viewer_, SLOT(OnHighLightChanged()));
	connect(bias_parallel_viewer_, SIGNAL(HighLightChanged()), bias_line_chart_viewer_, SLOT(OnHighLightChanged()));
	connect(bias_parallel_viewer_, SIGNAL(HighLightChanged()), absolute_line_chart_viewer_, SLOT(OnHighLightChanged()));

	connect(bias_parallel_viewer_, SIGNAL(HighLightOff()), central_viewer_, SLOT(OnHighLightOff()));
	connect(bias_parallel_viewer_, SIGNAL(HighLightOff()), absolute_parallel_viewer_, SLOT(OnHighLightOff()));
	connect(bias_parallel_viewer_, SIGNAL(HighLightOff()), bias_line_chart_viewer_, SLOT(OnHighLightOff()));
	connect(bias_parallel_viewer_, SIGNAL(HighLightOff()), absolute_line_chart_viewer_, SLOT(OnHighLightOff()));

	connect(absolute_parallel_viewer_, SIGNAL(HighLightChanged()), central_viewer_, SLOT(OnHighLightChanged()));
	connect(absolute_parallel_viewer_, SIGNAL(HighLightChanged()), bias_parallel_viewer_, SLOT(OnHighLightChanged()));
	connect(absolute_parallel_viewer_, SIGNAL(HighLightChanged()), bias_line_chart_viewer_, SLOT(OnHighLightChanged()));
	connect(absolute_parallel_viewer_, SIGNAL(HighLightChanged()), absolute_line_chart_viewer_, SLOT(OnHighLightChanged()));

	connect(absolute_parallel_viewer_, SIGNAL(HighLightOff()), central_viewer_, SLOT(OnHighLightOff()));
	connect(absolute_parallel_viewer_, SIGNAL(HighLightOff()), bias_parallel_viewer_, SLOT(OnHighLightOff()));
	connect(absolute_parallel_viewer_, SIGNAL(HighLightOff()), bias_line_chart_viewer_, SLOT(OnHighLightOff()));
	connect(absolute_parallel_viewer_, SIGNAL(HighLightOff()), absolute_line_chart_viewer_, SLOT(OnHighLightOff()));

	connect(bias_line_chart_viewer_, SIGNAL(HighLightChanged()), central_viewer_, SLOT(OnHighLightChanged()));
	connect(bias_line_chart_viewer_, SIGNAL(HighLightChanged()), bias_parallel_viewer_, SLOT(OnHighLightChanged()));
	connect(bias_line_chart_viewer_, SIGNAL(HighLightChanged()), absolute_parallel_viewer_, SLOT(OnHighLightChanged()));
	connect(bias_line_chart_viewer_, SIGNAL(HighLightChanged()), absolute_line_chart_viewer_, SLOT(OnHighLightChanged()));

	connect(bias_line_chart_viewer_, SIGNAL(HighLightOff()), central_viewer_, SLOT(OnHighLightOff()));
	connect(bias_line_chart_viewer_, SIGNAL(HighLightOff()), bias_parallel_viewer_, SLOT(OnHighLightOff()));
	connect(bias_line_chart_viewer_, SIGNAL(HighLightOff()), absolute_parallel_viewer_, SLOT(OnHighLightOff()));
	connect(bias_line_chart_viewer_, SIGNAL(HighLightOff()), absolute_line_chart_viewer_, SLOT(OnHighLightOff()));

	connect(absolute_line_chart_viewer_, SIGNAL(HighLightChanged()), central_viewer_, SLOT(OnHighLightChanged()));
	connect(absolute_line_chart_viewer_, SIGNAL(HighLightChanged()), bias_parallel_viewer_, SLOT(OnHighLightChanged()));
	connect(absolute_line_chart_viewer_, SIGNAL(HighLightChanged()), absolute_parallel_viewer_, SLOT(OnHighLightChanged()));
	connect(absolute_line_chart_viewer_, SIGNAL(HighLightChanged()), bias_line_chart_viewer_, SLOT(OnHighLightChanged()));

	connect(absolute_line_chart_viewer_, SIGNAL(HighLightOff()), central_viewer_, SLOT(OnHighLightOff()));
	connect(absolute_line_chart_viewer_, SIGNAL(HighLightOff()), bias_parallel_viewer_, SLOT(OnHighLightOff()));
	connect(absolute_line_chart_viewer_, SIGNAL(HighLightOff()), absolute_parallel_viewer_, SLOT(OnHighLightOff()));
	connect(absolute_line_chart_viewer_, SIGNAL(HighLightOff()), bias_line_chart_viewer_, SLOT(OnHighLightOff()));

}

// Load value of a specific range of time
void WrfAnalysisPlatform::OnActionLoadHistoricalDataTriggered(){
	/*QString root_dir = QFileDialog::getExistingDirectory(this, tr("Select Root Path"), ".");
	if ( root_dir.length() != 0 ){
		data_manager_->SetRootDir(root_dir.toLocal8Bit().data());
		data_manager_->LoadDefaultData();
	}*/

	std::ifstream input("info.dat");
	if ( input.good() ){
		std::string str;
		int current_day;
		int day_length;

		getline(input, str);
		input >> current_day >> day_length;
		data_manager_->SetRootDir(str);
		data_manager_->LoadDefaultData(current_day, day_length);

		input.close();
	}
	//data_manager_->SetRootDir("G:/VisData/meteorology/data");
	//data_manager_->SetRootDir("./meteorology/data");
}

// Load value of a specific time
void WrfAnalysisPlatform::OnActionLoadNumericalDataTriggered(){
	
}

void WrfAnalysisPlatform::OnActionViewVarationMapTriggered(){
	WrfGridValueMap* bias_map = data_manager_->GetBiasMap();
	central_viewer_->set_bias_map(bias_map);
	central_viewer_->set_grid_value_map(bias_map);
}

void WrfAnalysisPlatform::OnActionGenerateTestPcpTriggered(){
	WrfParallelDataSet* pcp_data = WrfDataManager::GetInstance()->GetTestPcpData();

	/*WrfParallelModel model;
	model.model_name = "T639";
	model.r = 1.0;
	model.g = 0.0;
	model.b = 0.0;
	for ( int i = 0; i < 4; ++i ) pcp_data->model_vec.push_back(model);

	for ( int i = 0; i < 7; ++i ) pcp_data->attrib_name_vec.push_back("Rain");

	for ( int i = 0; i < 7; ++i ){
		pcp_data->max_value_vec.push_back(10.0);
		pcp_data->min_value_vec.push_back(0.0);
	}

	WrfParallelRecord record;
	record.alpha = 1.0;
	record.r = 1.0;
	record.g = 0.0;
	record.b = 0.0;
	for ( int i = 0; i <7; ++i ) record.data[i] = 5.0;
	
	pcp_data->values.push_back(record);*/

	/*std::ofstream ofs("series.txt");
	boost::archive::text_oarchive oa(ofs);
	oa << pcp_data;*/
	//std::ifstream ifs("series.txt");
	//boost::archive::text_iarchive ia(ifs);
	// read class state from archive
	//ia >> pcp_data;

	bias_parallel_viewer_->set_data_set(pcp_data);
}

void WrfAnalysisPlatform::OnActionGenerateTestLineChartTriggered(){
	/*WrfLineChartDataSet* data_set = new WrfLineChartDataSet;
	data_set->max_value = 1.0;
	data_set->min_value = -1.0;
	data_set->time_length = 7;
	data_set->values.resize(4);

	data_set->colors.resize(3 * 4);
	data_set->colors[0] = 1.0;
	data_set->colors[1] = 0.0;
	data_set->colors[2] = 0.0;

	data_set->colors[3] = 0.0;
	data_set->colors[4] = 1.0;
	data_set->colors[5] = 0.0;

	data_set->colors[6] = 0.0;
	data_set->colors[7] = 0.0;
	data_set->colors[8] = 1.0;

	data_set->colors[9] = 1.0;
	data_set->colors[10] = 1.0;
	data_set->colors[11] = 0.0;

	data_set->line_names.push_back("T639");
	data_set->line_names.push_back("EC_FINE");
	data_set->line_names.push_back("NCEP");
	data_set->line_names.push_back("JAPAN_GSM");

	srand((int)time(0));
	for (int i = 0; i < 4; ++i ){
		LineChartRecord* record = new LineChartRecord;
		record->alpha = 0;
		for ( int j = 0; j < 7; ++j ){
			LineCharPoint point;
			point.value = (float)rand() / RAND_MAX;
			record->values.push_back(point);
		}
		data_set->values[i].push_back(record);
	}*/
}

void WrfAnalysisPlatform::OnActionGenerateTestHistogramTriggered(){
	WrfHistogramDataSet* histogram = WrfDataManager::GetInstance()->GetTestHistogram();

	histogram_viewer_->SetData(histogram);
}

void WrfAnalysisPlatform::OnActionGenerateTestMatrixChartTriggered(){
	WrfMatrixChartDataSet* data_set = new WrfMatrixChartDataSet;
	data_set->x_coor_name = "Time";
	data_set->y_coor_name = "Attribute";
	data_set->x_size = 7;
	data_set->y_size = 7;
	for ( int i = 0; i < data_set->y_size; ++i )
		for ( int j = 0; j < data_set->x_size; ++j ){
			WrfMatrixChartRecord record;
			record.x_index = j;
			record.y_index = i;
			record.value = 0.5;
			data_set->values.push_back(record);
		}
	matrix_viewer_->set_data(data_set);
}

void WrfAnalysisPlatform::OnActionOpenProjectTriggered(){
	//QString project_path = QFileDialog::getOpenFileName(this, tr("Open Project"), ".", QString("*.wrf"));
	QString project_path = QString("./test.wrf");
	if ( project_path.length() != 0 ){
		if ( data_manager_ != NULL ) delete data_manager_;
		std::ifstream ifs(project_path.toLocal8Bit().data());
		boost::archive::text_iarchive ia(ifs);
		ia >> data_manager_;

		WrfDataManager::instance_ = data_manager_;
		
		for ( int i = 0; i < 2; ++i ) {
			WrfGridValueMap* value_map = WrfDataManager::GetInstance()->GetStoryStamp(i);
			std::vector< int > related_ids;
			positioned_stamp_viewer_->AddVarMap(value_map);
		}
		positioned_stamp_viewer_->show();
		this->UpdateSystem();
	}
}

void WrfAnalysisPlatform::OnActionSaveProjectTriggered(){
	QString project_path = QFileDialog::getSaveFileName(this, tr("Save Project"), ".", QString("*.wrf"));
	if ( project_path.length() != 0 ){
		std::ofstream ofs(project_path.toLocal8Bit().data());
		boost::archive::text_oarchive oa(ofs);
		oa << data_manager_;
	}
}

void WrfAnalysisPlatform::OnActionGenerateBiasMapTriggered(){
	if ( data_manager_ != NULL ){
		WrfGridValueMap* bias_map = data_manager_->GetBiasMap();
		central_viewer_->set_bias_map(bias_map);

		WrfGridValueMap* wind_var_map = data_manager_->GetWindVarMap();
		central_viewer_->set_wind_var_map(wind_var_map);
	}
}

void WrfAnalysisPlatform::OnActionSelectingSiteTriggered(){
	central_viewer_->SetViewMode(WrfCentralViewer::SELECTING_SITE);	
}

void WrfAnalysisPlatform::OnActionBrushingSiteTriggered(){
	central_viewer_->SetViewMode(WrfCentralViewer::ADDING_AREA);
}

void WrfAnalysisPlatform::OnActionMagicWandTriggered(){
	central_viewer_->SetViewMode(WrfCentralViewer::MAGIC_WAND);
}

void WrfAnalysisPlatform::OnActionErasingSiteTriggered(){
	central_viewer_->ClearSelection();
}

void WrfAnalysisPlatform::OnActionComparedModeTriggered(){
	central_viewer_->SetSelectionMode(WrfCentralViewer::COMPARED_MODE);
	bias_parallel_viewer_->set_data_mode(WrfParallelCoordinate::COMPARED);
	absolute_parallel_viewer_->set_data_mode(WrfParallelCoordinate::COMPARED);
	bias_line_chart_viewer_->set_data_mode(WrfLineChart::COMPARED_MODE);
	absolute_line_chart_viewer_->set_data_mode(WrfLineChart::COMPARED_MODE);
}

void WrfAnalysisPlatform::OnActionNormalModeTriggered(){
	central_viewer_->SetSelectionMode(WrfCentralViewer::NORMAL_MODE);
	bias_parallel_viewer_->set_data_mode(WrfParallelCoordinate::NOMRAL);
	absolute_parallel_viewer_->set_data_mode(WrfParallelCoordinate::NOMRAL);
	bias_line_chart_viewer_->set_data_mode(WrfLineChart::NORMAL_MODE);
	absolute_line_chart_viewer_->set_data_mode(WrfLineChart::NORMAL_MODE);
}

void WrfAnalysisPlatform::OnSelectedSiteChanged(int index){
	
}

void WrfAnalysisPlatform::OnSelectedAreaChanged(){
	WrfHistogramDataSet* histogram_set = WrfDataManager::GetInstance()->GetHistogramDataSet();
	histogram_viewer_->SetData(histogram_set);
}

void WrfAnalysisPlatform::OnBiasWeightChanged(){
	if ( data_manager_ != NULL ){
		WrfGridValueMap* bias_map = data_manager_->GetBiasMap();
		central_viewer_->update_bias_map(bias_map);
	}
}

void WrfAnalysisPlatform::UpdateSystem(){

}

void WrfAnalysisPlatform::OnActionStartEditingTriggered(){
	central_viewer_->StartEditIsolines();
}

void  WrfAnalysisPlatform::OnActionEndEditingTriggered(){
	central_viewer_->EndEditIsolines();
}

void WrfAnalysisPlatform::OnActionParallelBiasViewTriggered(){
	bias_parallel_viewer_->set_view_mode(WrfParallelCoordinate::BIAS_VIEW);
}

void WrfAnalysisPlatform::OnActionParallelAbsoluteViewTriggered(){
	bias_parallel_viewer_->set_view_mode(WrfParallelCoordinate::ABSOLUTE_VIEW);
}

void WrfAnalysisPlatform::OnActionBiasLineViewTriggered(){
	bias_line_chart_viewer_->set_view_mode(WrfLineChart::BIAS_VIEW);
}

void WrfAnalysisPlatform::OnActionAbsoluteLineViewTriggered(){
	bias_line_chart_viewer_->set_view_mode(WrfLineChart::ABSOLUTE_VIEW);
}

void WrfAnalysisPlatform::OnActionStampMapViewTriggered(){
	random_stamp_viewer_->showMaximized();
}

void WrfAnalysisPlatform::OnActionComplexStampMapViewTriggered(){
	positioned_stamp_viewer_->showMaximized();
}

void WrfAnalysisPlatform::OnInfoGainSelected(int index){
	WrfGridValueMap* temp_value_map = central_viewer_->bias_map();
	if ( temp_value_map == NULL || temp_value_map->level > 0 ) return;
	if ( index == -1 ){
		std::vector< std::vector< float > > weights;
		std::vector< int > changed_index;
		std::vector< WrfGridValueMap* > value_maps;
		WrfDataManager::GetInstance()->GetAverageInfoGainMaps(value_maps, weights, changed_index);
		random_stamp_viewer_->SetItemVarMaps(value_maps, weights, changed_index);

		random_stamp_viewer_->show();
	} else {
		std::vector< std::vector< float > > weights;
		std::vector< int > changed_index;
		std::vector< WrfGridValueMap* > value_maps;
		WrfDataManager::GetInstance()->GetSingleAttributeInfoGainMaps(index, value_maps, weights, changed_index);
		random_stamp_viewer_->SetItemVarMaps(value_maps, weights, changed_index);

		random_stamp_viewer_->show();
	}
}

void WrfAnalysisPlatform::OnActionAddReferenceMapTriggered(){
	WrfGridValueMap* temp_value_map = central_viewer_->bias_map();

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

	std::vector< int > related_ids;
	int index = WrfDataManager::GetInstance()->AddStoryStamp(value_map, related_ids);
	story_line_viewer_->AddStoryStamp(index);
	positioned_stamp_viewer_->AddVarMap(value_map);
}

void WrfAnalysisPlatform::OnBiasMapSelected(int index){
	std::vector< float > attrib_weight;
	WrfDataManager::GetInstance()->GetInfoGainWeight(index, attrib_weight);
	WrfDataManager::GetInstance()->SetAttribWeight(attrib_weight);

	bias_parallel_viewer_->set_attrib_weight(attrib_weight);
	absolute_parallel_viewer_->set_attrib_weight(attrib_weight);

	WrfGridValueMap* value_map = WrfDataManager::GetInstance()->GetBiasMap();
	central_viewer_->update_bias_map(value_map);

	bias_parallel_viewer_->updateGL();
}

void WrfAnalysisPlatform::OnStoryStampSelected(int index){
	WrfGridValueMap* value_map = WrfDataManager::GetInstance()->GetStoryStamp(index);
	central_viewer_->update_bias_map(value_map);
	bias_parallel_viewer_->set_attrib_weight(value_map->weight);
	absolute_parallel_viewer_->set_attrib_weight(value_map->weight);
	if ( value_map->level == 0 ){
		WrfDataManager::GetInstance()->SetAttribWeight(value_map->weight);
	}

	bias_parallel_viewer_->updateGL();
	absolute_parallel_viewer_->updateGL();
}

void WrfAnalysisPlatform::OnActionSumTriggered(){
	WrfGridValueMap* value_map = story_line_viewer_->GetOperationResult(StoryLineWidget::SUM_OP);

	if ( value_map != NULL ){
		central_viewer_->set_bias_map(value_map);
	}
}

void WrfAnalysisPlatform::OnActionMiniusTriggered(){
	WrfGridValueMap* value_map = story_line_viewer_->GetOperationResult(StoryLineWidget::MINUS_OP);

	if ( value_map != NULL ){
		central_viewer_->set_bias_map(value_map);
	}
}

void WrfAnalysisPlatform::OnActionVariationTriggered(){
	WrfGridValueMap* value_map = story_line_viewer_->GetOperationResult(StoryLineWidget::VAR_OP);

	if ( value_map != NULL ){
		central_viewer_->set_bias_map(value_map);
	}
}

void WrfAnalysisPlatform::OnActionAcceptRegionGrowingTriggered(){
	central_viewer_->AcceptCandidateArea();
}

void WrfAnalysisPlatform::OnHistoryLengthChanged(int length){
	if ( data_manager_ != NULL ){
		data_manager_->SetHistoryLength(length);

		WrfGridValueMap* bias_map = data_manager_->GetBiasMap();
		central_viewer_->update_bias_map(bias_map);

		WrfHistogramDataSet* histogram = WrfDataManager::GetInstance()->GetHistogramDataSet();
		histogram_viewer_->SetData(histogram);

		bias_parallel_viewer_->OnSelectedAreaChanged();
		absolute_parallel_viewer_->OnSelectedAreaChanged();
		bias_line_chart_viewer_->OnSelectedAreaChanged();
		absolute_line_chart_viewer_->OnSelectedAreaChanged();
	}
}

void WrfAnalysisPlatform::OnAttribWeightChanged(){
	if ( data_manager_ != NULL ){
		WrfGridValueMap* bias_map = data_manager_->GetBiasMap();
		central_viewer_->update_bias_map(bias_map);
	}
}

void WrfAnalysisPlatform::OnActionClusterBiasParallelRecordsTriggered(){
	if ( !bias_parallel_viewer_->is_cluster_on() )
		bias_parallel_viewer_->SetClusterOn();
	else
		bias_parallel_viewer_->SetClusterOff();
}

void WrfAnalysisPlatform::OnActionClusterBiasLineChartRecordsTriggered(){
	if ( !bias_line_chart_viewer_->is_cluster_on() )
		bias_line_chart_viewer_->SetClusterOn();
	else
		bias_line_chart_viewer_->SetClusterOff();
}

void WrfAnalysisPlatform::OnActionClusterAbsoluteParallelRecordsTriggered(){
	if ( !absolute_parallel_viewer_->is_cluster_on() )
		absolute_parallel_viewer_->SetClusterOn();
	else
		absolute_parallel_viewer_->SetClusterOff();
}

void WrfAnalysisPlatform::OnActionClusterAbsoluteLineChartRecordsTriggered(){
	if ( !absolute_line_chart_viewer_->is_cluster_on() )
		absolute_line_chart_viewer_->SetClusterOn();
	else
		absolute_line_chart_viewer_->SetClusterOff();
}