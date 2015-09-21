#include "wrf_visualization_platform.h"
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QProgressBar>
#include <QtGui/QFileDialog>
#include "wrf_data_manager.h"
#include "wrf_data_common.h"
#include "wrf_event_searching_view.h"
#include "wrf_utility.h"
#include "wrf_initialization_selection_widget.h"
#include "wrf_region_detection_view.h"
#include "wrf_grid_map_element.h"
#include "wrf_forecast_comparison_widget.h"
#include "wrf_load_ensemble_dialog.h"
#include "wrf_load_ranalysis_dialog.h"
#include "wrf_ensemble_data_view.h"
#include "wrf_probabilistic_forecast_dialog.h"
#include "wrf_searching_pre_processing_para_widget.h"
#include "wrf_process_path_selection_dialog.h"
#include "flow_chart.h"
#include "wrf_reforecast_manager.h"
#include "wrf_ensemble_manager.h"
#include "wrf_forecasting_manager.h"
#include "wrf_event_analyzing_view.h"
#include "wrf_statistic_solver.h"
#include "wrf_rms_mapping_dialog.h"
#include "wrf_subregion_rms_scale_dialog.h"
#include "wrf_variable_selection_dialog.h"
#include "wrf_event_comparison_view.h"
#include "wrf_event_comparison_variable_dialog.h"

//#define DEBUG_FLOW_CHART

WrfVisualizationPlatform::WrfVisualizationPlatform(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags){
        ui.setupUi(this);
#ifndef DEBUG_FLOW_CHART
		data_manager_ = WrfDataManager::GetInstance();
		//data_manager_->LoadDefaultData();

		InitWidgets();
#else
		flow_chart_ = new FlowChart;
        flow_chart_->setFixedWidth(240);
        flow_chart_->show();
		connect(flow_chart_, SIGNAL(ItemSelected(StepItemType)), this, SLOT(OnStepItemClicked(StepItemType)));
#endif
        is_procedure_executed_.resize(10, false);
}

WrfVisualizationPlatform::~WrfVisualizationPlatform(){

}

void WrfVisualizationPlatform::InitWidgets(){
    ensemble_data_view_ = new WrfEnsembleDataView;

    ensemble_region_detection_view_ = new WrfRegionDetectionView(WRF_ENSEMBLER_PQPF);

    reforecast_region_detection_view_ = new WrfRegionDetectionView(WRF_REFORECAST_PQPF);

    event_searching_view_ = new WrfEventSearchingView;

    event_analyzing_view_ = new WrfEventAnalyzingView;

    comparison_view_ = new WrfForecastComparisonWidget(ALL_COMPARISON);

    event_comparison_view_ = new WrfEventComparisonView;

    flow_chart_ = new FlowChart;
	flow_chart_->setContentsMargins(11, 11, 11, 11);
    flow_chart_->setFixedWidth(240);

	calendar_widget_ = new QCalendarWidget();
	calendar_widget_->setFixedWidth(240);
	calendar_widget_->setFixedHeight(200);
	calendar_widget_->setEnabled(false);
	calendar_widget_->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));

    main_stacked_widget_ = new QStackedWidget;
    main_stacked_widget_->addWidget(ensemble_data_view_);
    main_stacked_widget_->addWidget(ensemble_region_detection_view_);
    main_stacked_widget_->addWidget(reforecast_region_detection_view_);
    main_stacked_widget_->addWidget(event_searching_view_);
    main_stacked_widget_->addWidget(event_analyzing_view_);
    main_stacked_widget_->addWidget(comparison_view_);

	QVBoxLayout* func_layout = new QVBoxLayout;
	func_layout->addWidget(flow_chart_, 1);
	func_layout->addWidget(calendar_widget_, 0, Qt::AlignBottom);

    QHBoxLayout* central_layout = new QHBoxLayout;
    central_layout->addLayout(func_layout);
    central_layout->addWidget(main_stacked_widget_);
    this->ui.centralWidget->setLayout(central_layout);

    connect(ui.actionLoad_Ensemble_Data, SIGNAL(triggered()), this, SLOT(OnActionLoadEnsembleDataTriggered()));
    connect(ui.actionNARR, SIGNAL(triggered()), this, SLOT(OnActionLoadNarrDataTriggered()));
    connect(ui.actionChina_Reanalysis, SIGNAL(triggered()), this, SLOT(OnActionLoadChinaReanalysisDataTriggered()));
    connect(ui.actionLoad_Combined_Data, SIGNAL(triggered()), this, SLOT(OnActionLoadCombinedDataTriggered()));

    connect(ui.actionPre_Process_Data, SIGNAL(triggered()), this, SLOT(OnActionPreProcessTriggered()));
    connect(ui.actionProbabilistic_Forecast, SIGNAL(triggered()), this, SLOT(OnActionProbabilisticForecastTriggered()));

    connect(ensemble_region_detection_view_, SIGNAL(RetrievalTriggered(int)), this, SLOT(OnActionVisualAssistedFilteringTriggered(int)));
    connect(ensemble_region_detection_view_, SIGNAL(AddNewForecastTriggered()), this, SLOT(OnActionProbabilisticForecastTriggered()));
    connect(reforecast_region_detection_view_, SIGNAL(RetrievalTriggered(int)), this, SLOT(OnActionVisualAssistedFilteringTriggered(int)));
    connect(reforecast_region_detection_view_, SIGNAL(AddNewForecastTriggered()), this, SLOT(OnActionProbabilisticForecastTriggered()));

    connect(event_searching_view_, SIGNAL(FilteringApplied()), this, SLOT(OnActionApplyCalibrationTriggered()));
    connect(event_searching_view_, SIGNAL(EventAnalyzingTriggered()), this, SLOT(OnActionShowTimeEventTriggered()));

    connect(flow_chart_, SIGNAL(ItemSelected(StepItemType)), this, SLOT(OnStepItemClicked(StepItemType)));
    connect(flow_chart_, SIGNAL(ItemDoubleClicked(StepItemType)), this, SLOT(OnStepItemDoubleClicked(StepItemType)));

	connect(ui.actionRMS_Mapping, SIGNAL(triggered()), this, SLOT(OnRmsMappingChanged()));

    connect(ensemble_data_view_, SIGNAL(RetrievalTriggered()), this, SLOT(OnActionRawDataRetrievalTriggered()));
}

void WrfVisualizationPlatform::OnActionLoadEnsembleDataTriggered(){
    WrfLoadEnsembleDialog load_dialog;

    if ( load_dialog.exec() == QDialog::Accepted ){
        WrfModelType model;
        WrfElementType element;
        std::string file_name;
        load_dialog.GetSelectionParas(model, element, file_name);

		if ( model == WRF_NCEP_ENSEMBLES )
			data_manager_->LoadEnsembleData(element, file_name);
		else if ( model == WRF_NCEP_ENSEMBLE_MEAN )
			data_manager_->LoadEnsembleMeanData(element, file_name);
    }
}

void WrfVisualizationPlatform::OnActionLoadNarrDataTriggered(){
    WrfLoadReanalysisDialog load_dialog;

    if ( load_dialog.exec() == QDialog::Accepted ){
        WrfElementType element;
        std::string file_name;
        load_dialog.GetSelectionParas(element, file_name);
    }
}

void WrfVisualizationPlatform::OnActionLoadChinaReanalysisDataTriggered(){
    
}

void WrfVisualizationPlatform::OnActionLoadCombinedDataTriggered(){
    QString file_path = QFileDialog::getOpenFileName(this, tr("Open Combined Data"), ".", "*.nc");
    if ( file_path.length() != 0 ){
        data_manager_->LoadCombinedData(std::string(file_path.toLocal8Bit().data()));
    }
}

void WrfVisualizationPlatform::OnActionViewEnsembleTriggered(){
    WrfInitializationSelectionWidget initialization_dialog;
    if ( initialization_dialog.exec() == QDialog::Accepted ){
        std::vector< WrfElementType > ensemble_elements;
        std::vector< WrfElementType > ensemble_mean_elements;
        initialization_dialog.GetSelectionParas(ensemble_elements, ensemble_mean_elements, current_datetime_, fcst_hour_);

		calendar_widget_->setSelectedDate(current_datetime_.date());

        ensemble_data_view_->SetPara(current_datetime_, fcst_hour_, ensemble_elements, ensemble_mean_elements);

		WrfEnsembleManager::GetInstance()->SetCurrentDate(current_datetime_);
		WrfEnsembleManager::GetInstance()->SetForecastingParameters(fcst_hour_, ensemble_elements_, element_normalize_values_);

        is_procedure_executed_[DATA_VIEW] = true;
        flow_chart_->SetExecutedStep(DATA_VIEW);
    }
}

void WrfVisualizationPlatform::OnActionPreProcessTriggered(){
    WrfSearchingPreProcessingParaWidget* para_widget = new WrfSearchingPreProcessingParaWidget;
    if ( para_widget->exec() != QDialog::Accepted ) return;

    para_widget->GetParameters(searching_year_length_, searching_day_length_, searching_grid_size_, searching_analog_num_, ensemble_elements_, ensemble_mean_elements_, element_weight_, element_normalize_values_);

    WrfReforecastManager::GetInstance()->SetForecastingDate(current_datetime_);
    WrfReforecastManager::GetInstance()->SetRetrievalParameters(searching_year_length_, searching_day_length_, searching_grid_size_, fcst_hour_, searching_analog_num_, ensemble_elements_, ensemble_mean_elements_, element_weight_, element_normalize_values_);
    WrfReforecastManager::GetInstance()->LoadData();
    WrfReforecastManager::GetInstance()->PreProcessData();

    WrfEnsembleManager::GetInstance()->SetCurrentDate(current_datetime_);
    WrfEnsembleManager::GetInstance()->SetForecastingParameters(fcst_hour_, ensemble_elements_, element_normalize_values_);

    flow_chart_->SetExecutedStep(PRE_PROCESSING);
    is_procedure_executed_[PRE_PROCESSING] = true;

	reforecast_region_detection_view_->UpdateWidget();

	is_procedure_executed_[RMS] = true;
	flow_chart_->SetExecutedStep(RMS);

	ensemble_region_detection_view_->UpdateWidget();

	is_procedure_executed_[UNCERTAINTY] = true;
	flow_chart_->SetExecutedStep(UNCERTAINTY);
}

void WrfVisualizationPlatform::OnActionVisualAssistedFilteringTriggered(int mode){
    if ( mode == 0 )
        ensemble_region_detection_view_->GetSelectedRegionContour(region_contour);
    else
        reforecast_region_detection_view_->GetSelectedRegionContour(region_contour);

    if ( region_contour.size() == 0 ) return;

    UpdateRetrievalMapRange(region_contour);

    MapRange ensemble_map_range;
    WrfDataManager::GetInstance()->GetEnsembleMapRange(ensemble_map_range);
    WrfStatisticSolver::GetSelectedGridIndex(region_contour, ensemble_map_range.start_x + ensemble_map_range.x_grid_space * 0.5,
        ensemble_map_range.start_y + ensemble_map_range.y_grid_space * 0.5, ensemble_map_range.x_grid_space, ensemble_map_range.y_grid_space,
        ensemble_map_range.x_grid_number, ensemble_map_range.y_grid_number, selected_grid_index);

	WrfVariableSelectionDialog variable_dialog;
	std::vector< WrfElementType > retrieval_elements;
	if ( variable_dialog.exec() == QDialog::Accepted ) {
		variable_dialog.GetSelectionParas(retrieval_elements);
	} else 
		return;

	event_searching_view_->SetRetrievalElements(retrieval_elements);
    event_searching_view_->SetSelectedGridPointIndex(selected_grid_index);

    main_stacked_widget_->setCurrentIndex(3);
    is_procedure_executed_[ADJUSTMENT] = true;
    flow_chart_->SetExecutedStep(ADJUSTMENT);
}

void WrfVisualizationPlatform::OnActionApplyCalibrationTriggered(){
    // reforecast calibration
    /*std::vector< WrfGridValueMap* > maps;
    std::vector< ProbabilisticPara > paras;

    WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_REFORECAST_PQPF, paras, maps);

    for ( int i = 0; i < paras.size(); ++i ){
        WrfGridValueMap* adapted_map = WrfForecastingManager::GetInstance()->GetForecastingMap(WRF_ADAPTED_REFORECAST_PQPF, paras[i]);
        if ( adapted_map == NULL ) return;

        WrfGridValueMap* new_map = WrfReforecastManager::GetInstance()->GenerateForecastingMap(paras[i]);
        memcpy(adapted_map->values, new_map->values, adapted_map->map_range.x_grid_number * adapted_map->map_range.y_grid_number * sizeof(float));
        delete new_map;
    }*/

    comparison_view_->UpdateWidget();

    is_procedure_executed_[RMS_RESULT] = true;
    flow_chart_->SetExecutedStep(RMS_RESULT);

    main_stacked_widget_->setCurrentIndex(5);
}

void WrfVisualizationPlatform::OnActionProbabilisticForecastTriggered(){
    WrfProbabilisticForecastDialog prob_dialog;

    if ( prob_dialog.exec() ){
        ProbabilisticPara para;
        prob_dialog.GetSelectionParas(para);
        qint64 temp_time = current_datetime_.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
        para.time = (int)temp_time;

        WrfForecastingManager::GetInstance()->AddProbabilisticForecast(para);

        reforecast_region_detection_view_->UpdateWidget();

        is_procedure_executed_[RMS] = true;
        flow_chart_->SetExecutedStep(RMS);

        ensemble_region_detection_view_->UpdateWidget();

        is_procedure_executed_[UNCERTAINTY] = true;
        flow_chart_->SetExecutedStep(UNCERTAINTY);
    }
}

void WrfVisualizationPlatform::OnActionShowTimeEventTriggered(){
    std::vector< float > date_similarity;
    event_searching_view_->GetDateSimilarity(date_similarity);

    event_analyzing_view_->SetEventInfo(WRF_ACCUMULATED_PRECIPITATION, date_similarity);
    event_analyzing_view_->SetViewingRange(retrieval_map_range_);
	event_analyzing_view_->SetSelectionPath(region_contour);
	event_analyzing_view_->SetSelectedGridIndex(selected_grid_index);

    event_analyzing_view_->UpdateWidget();

    is_procedure_executed_[EVENT_RESULT] = true;
    flow_chart_->SetExecutedStep(EVENT_RESULT);
    main_stacked_widget_->setCurrentIndex(4);
}

void WrfVisualizationPlatform::OnStepItemClicked(StepItemType type){
    switch ( type ) {
    case DATA_VIEW:
#ifndef DEBUG_FLOW_CHART
		if ( !is_procedure_executed_[DATA_VIEW] ) OnActionViewEnsembleTriggered();
		if ( is_procedure_executed_[DATA_VIEW] ) main_stacked_widget_->setCurrentIndex(0);
#else
		is_procedure_executed_[DATA_VIEW] = true;
		flow_chart_->SetExecutedStep(DATA_VIEW);
#endif
        break;
    case PRE_PROCESSING:
#ifndef DEBUG_FLOW_CHART
		if ( !is_procedure_executed_[DATA_VIEW] ) return;
		OnActionPreProcessTriggered();
#else
		flow_chart_->SetExecutedStep(PRE_PROCESSING);
		is_procedure_executed_[PRE_PROCESSING] = true;
#endif
        break;
    case UNCERTAINTY:
#ifndef DEBUG_FLOW_CHART
		if ( !is_procedure_executed_[PRE_PROCESSING] ) return;
		if ( !is_procedure_executed_[UNCERTAINTY] ) OnActionProbabilisticForecastTriggered();
		main_stacked_widget_->setCurrentIndex(1);
#else
		is_procedure_executed_[UNCERTAINTY] = true;
		flow_chart_->SetExecutedStep(UNCERTAINTY);
#endif
        break;
    case RMS:
#ifndef DEBUG_FLOW_CHART
		if ( !is_procedure_executed_[PRE_PROCESSING] ) return;
		if ( !is_procedure_executed_[RMS] ) OnActionProbabilisticForecastTriggered();
		main_stacked_widget_->setCurrentIndex(2);
#else
		is_procedure_executed_[RMS] = true;
		flow_chart_->SetExecutedStep(RMS);
#endif		
        break;
    case ADJUSTMENT:
#ifndef DEBUG_FLOW_CHART
		if ( !(is_procedure_executed_[UNCERTAINTY] || is_procedure_executed_[RMS]) ) return;
		main_stacked_widget_->setCurrentIndex(3);
        is_procedure_executed_[ADJUSTMENT] = true;
        flow_chart_->SetExecutedStep(ADJUSTMENT);
#else
		is_procedure_executed_[ADJUSTMENT] = true;
		flow_chart_->SetExecutedStep(ADJUSTMENT);
#endif
        break;
    case EVENT_RESULT:
#ifndef DEBUG_FLOW_CHART
		if ( !(is_procedure_executed_[ADJUSTMENT] ) ) return;
		main_stacked_widget_->setCurrentIndex(4);
#else
		is_procedure_executed_[EVENT_RESULT] = true;
		flow_chart_->SetExecutedStep(EVENT_RESULT);
#endif
        break;
    case RMS_RESULT:
#ifndef DEBUG_FLOW_CHART
		if ( !(is_procedure_executed_[ADJUSTMENT] ) ) return;
		main_stacked_widget_->setCurrentIndex(5);
#else
		is_procedure_executed_[RMS_RESULT] = true;
		flow_chart_->SetExecutedStep(RMS_RESULT);
#endif
        break;
    case COMPARISON:
        is_procedure_executed_[COMPARISON] = true;
        flow_chart_->SetExecutedStep(COMPARISON);
        main_stacked_widget_->setCurrentIndex(6);
        break;
    default:
        break;
    }
}

void WrfVisualizationPlatform::OnStepItemDoubleClicked(StepItemType type){
    switch ( type ) {
    case DATA_VIEW:
        OnActionViewEnsembleTriggered();
        break;
    case PRE_PROCESSING:
#ifndef DEBUG_FLOW_CHART
        if ( !is_procedure_executed_[DATA_VIEW] ) return;
        OnActionPreProcessTriggered();
#else
        flow_chart_->SetExecutedStep(PRE_PROCESSING);
        is_procedure_executed_[PRE_PROCESSING] = true;
#endif
        break;
    case UNCERTAINTY:
#ifndef DEBUG_FLOW_CHART
        if ( !is_procedure_executed_[PRE_PROCESSING] ) return;
        if ( !is_procedure_executed_[UNCERTAINTY] ) OnActionProbabilisticForecastTriggered();
        main_stacked_widget_->setCurrentIndex(1);
#else
        is_procedure_executed_[UNCERTAINTY] = true;
        flow_chart_->SetExecutedStep(UNCERTAINTY);
#endif
        break;
    case RMS:
#ifndef DEBUG_FLOW_CHART
        if ( !is_procedure_executed_[PRE_PROCESSING] ) return;
        if ( !is_procedure_executed_[RMS] ) OnActionProbabilisticForecastTriggered();
        main_stacked_widget_->setCurrentIndex(2);
#else
        is_procedure_executed_[RMS] = true;
        flow_chart_->SetExecutedStep(RMS);
#endif		
        break;
    case ADJUSTMENT:
#ifndef DEBUG_FLOW_CHART
        if ( !(is_procedure_executed_[UNCERTAINTY] || is_procedure_executed_[RMS]) ) return;
        main_stacked_widget_->setCurrentIndex(3);
		event_searching_view_->UpdateView();
        is_procedure_executed_[ADJUSTMENT] = true;
        flow_chart_->SetExecutedStep(ADJUSTMENT);
#else
        is_procedure_executed_[ADJUSTMENT] = true;
        flow_chart_->SetExecutedStep(ADJUSTMENT);
#endif
        break;
    case EVENT_RESULT:
#ifndef DEBUG_FLOW_CHART
        if ( !(is_procedure_executed_[ADJUSTMENT] ) ) return;
        main_stacked_widget_->setCurrentIndex(4);
#else
        is_procedure_executed_[EVENT_RESULT] = true;
        flow_chart_->SetExecutedStep(EVENT_RESULT);
#endif
        break;
    case RMS_RESULT:
#ifndef DEBUG_FLOW_CHART
        if ( !(is_procedure_executed_[ADJUSTMENT] ) ) return;
        main_stacked_widget_->setCurrentIndex(5);
#else
        is_procedure_executed_[RMS_RESULT] = true;
        flow_chart_->SetExecutedStep(RMS_RESULT);
#endif
        break;
    case COMPARISON:
        is_procedure_executed_[COMPARISON] = true;
        flow_chart_->SetExecutedStep(COMPARISON);
        main_stacked_widget_->setCurrentIndex(6);
        break;
    default:
        break;
    }
}

void WrfVisualizationPlatform::UpdateRetrievalMapRange(std::vector< QPointF >& region_contour){
    retrieval_map_range_.start_x = 1e10;
    retrieval_map_range_.start_y = 1e10;
    retrieval_map_range_.end_x = -1e10;
    retrieval_map_range_.end_y = -1e10;
    for ( int i = 0; i < region_contour.size(); ++i ){
        if ( region_contour[i].rx() < retrieval_map_range_.start_x ) retrieval_map_range_.start_x = region_contour[i].rx();
        if ( region_contour[i].rx() > retrieval_map_range_.end_x ) retrieval_map_range_.end_x = region_contour[i].rx();
        if ( region_contour[i].ry() < retrieval_map_range_.start_y ) retrieval_map_range_.start_y = region_contour[i].ry();
        if ( region_contour[i].ry() > retrieval_map_range_.end_y ) retrieval_map_range_.end_y = region_contour[i].ry();
    }
}

void WrfVisualizationPlatform::OnRmsMappingChanged(){
	WrfRmsMappingDiloag dialog;

	if ( dialog.exec() == QDialog::Accepted ){
		float min_thresh, max_thresh;
		dialog.GetSelectionPara(min_thresh, max_thresh);

		ColorMappingGenerator::GetInstance()->SetRmsMapping(min_thresh, max_thresh);
		// update each of the view

		reforecast_region_detection_view_->UpdateView();
		event_searching_view_->UpdateDataAndView();
	}
}

void WrfVisualizationPlatform::OnActionRawDataRetrievalTriggered(){
    ensemble_data_view_->GetSelectedRegionContour(region_contour);
    if ( region_contour.size() == 0 ) return;
    UpdateRetrievalMapRange(region_contour);

    WrfEventComparisonVariableDialog* para_widget = new WrfEventComparisonVariableDialog;
    if ( para_widget->exec() != QDialog::Accepted ) return;
    para_widget->GetParameters(searching_year_length_, searching_day_length_, ensemble_elements_, ensemble_mean_elements_);
    WrfReforecastManager::GetInstance()->SetForecastingDate(current_datetime_);
    WrfReforecastManager::GetInstance()->SetRetrievalParameters(searching_year_length_, searching_day_length_, searching_grid_size_, fcst_hour_, searching_analog_num_, ensemble_elements_, ensemble_mean_elements_, element_weight_, element_normalize_values_);
    WrfReforecastManager::GetInstance()->LoadData();

    MapRange ensemble_map_range;
    WrfDataManager::GetInstance()->GetEnsembleMapRange(ensemble_map_range);
    WrfStatisticSolver::GetSelectedGridIndex(region_contour, ensemble_map_range.start_x + ensemble_map_range.x_grid_space * 0.5,
        ensemble_map_range.start_y + ensemble_map_range.y_grid_space * 0.5, ensemble_map_range.x_grid_space, ensemble_map_range.y_grid_space,
        ensemble_map_range.x_grid_number, ensemble_map_range.y_grid_number, selected_grid_index);
    event_comparison_view_->SetViewingRange(retrieval_map_range_);

    std::vector< WrfElementType > retrieval_elements;
    retrieval_elements = ensemble_elements_;
    for ( int i = 0; i < ensemble_mean_elements_.size(); ++i ) retrieval_elements.push_back(ensemble_mean_elements_[i]);

    if ( retrieval_elements.size() != 0 ){
        event_comparison_view_->SetSelectedGridIndex(selected_grid_index);
        event_comparison_view_->SetBasicVariables(retrieval_elements);
        event_comparison_view_->showMaximized();
    }

    /*event_comparison_view_->LoadData();
    event_comparison_view_->show();*/
}