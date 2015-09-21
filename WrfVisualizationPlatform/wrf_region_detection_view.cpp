#include "wrf_region_detection_view.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include "wrf_image_matrix_view.h"
#include "wrf_data_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_utility.h"
#include "wrf_image_series_view.h"
#include "wrf_reforecast_manager.h"
#include "wrf_ensemble_manager.h"
#include "wrf_forecasting_manager.h"

WrfRegionDetectionView::WrfRegionDetectionView(WrfForecastingType type)
    : forecasting_type_(type){

    InitWidget();
}

WrfRegionDetectionView::~WrfRegionDetectionView(){
  
}

void WrfRegionDetectionView::InitWidget(){
    image_matrix_view_ = new WrfImageMatrixView;

    region_detection_tool_bar_ = new QToolBar;
    action_add_new_forecast_ = new QAction(QIcon("./Resources/plus.png"), tr("Add new forecast"), this);
    action_manual_region_selection_ = new QAction(QIcon("./Resources/painter.jpg"), tr("Manual Selection"), this);
    action_manual_region_selection_->setCheckable(true);
    action_manual_region_selection_->setChecked(false);
    action_apply_region_retrieval_ = new QAction(QIcon("./Resources/find.png"), tr("Retrieval"), this);

    region_detection_tool_bar_->addAction(action_add_new_forecast_);
    region_detection_tool_bar_->addAction(action_manual_region_selection_);
    region_detection_tool_bar_->addAction(action_apply_region_retrieval_);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(region_detection_tool_bar_);
    main_layout->addWidget(image_matrix_view_);
	main_layout->setMargin(0);
    this->setLayout(main_layout);

    connect(action_manual_region_selection_, SIGNAL(triggered()), this, SLOT(OnActionManualRegionSelectionTriggered()));
    connect(action_apply_region_retrieval_, SIGNAL(triggered()), this, SLOT(OnRetrievalTriggered()));
    connect(action_add_new_forecast_, SIGNAL(triggered()), this, SIGNAL(AddNewForecastTriggered()));
}

void WrfRegionDetectionView::UpdateView(){
	image_matrix_view_->UpdateWidget();
}

void WrfRegionDetectionView::UpdateWidget(){
    image_matrix_view_->Clear();

	WrfGridValueMap* ens_average_map = WrfEnsembleManager::GetInstance()->GenerateAverageMap(WRF_ACCUMULATED_PRECIPITATION);
	image_matrix_view_->AddValueMap(ens_average_map, ens_average_map->map_range, "Ensemble Average of APCP");

    if ( forecasting_type_ == WRF_ENSEMBLER_PQPF ){
        /*std::vector< WrfElementType > ens_elements;
        WrfDataManager::GetInstance()->GetEnsembleElements(ens_elements);

        for ( int i = 0; i < ens_elements.size(); ++i ){
            QString temp_title = QString::fromLocal8Bit(enum_element_to_string(ens_elements[i]));
            temp_title += QString(" ") + QString::fromLocal8Bit(enum_model_to_string(WRF_ELEMENT_UNCERTAINTY));
            WrfGridValueMap* value_map = WrfEnsembleManager::GetInstance()->GetUncertaintyMap(ens_elements[i]);
            image_matrix_view_->AddValueMap(value_map, value_map->map_range, temp_title);
        }*/
    } else if ( forecasting_type_ == WRF_REFORECAST_PQPF ){
        MapRange ens_map_range;
        WrfDataManager::GetInstance()->GetEnsembleMapRange(ens_map_range);
        image_matrix_view_->AddRmsMapView(ens_map_range);   
    }

    std::vector< WrfGridValueMap* > prob_maps;
    std::vector< ProbabilisticPara > prob_paras;
    WrfForecastingManager::GetInstance()->GetForecastingMaps(forecasting_type_, prob_paras, prob_maps);
    for ( int i = 0; i < prob_maps.size(); ++i ){
        if ( prob_maps[i] == NULL ) continue;
        QDateTime date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0)).addDays(prob_paras[i].time / 24);
		QString title;
		if ( forecasting_type_ == WRF_ENSEMBLER_PQPF ){
			title = QString("Probibility of  ") + QString::fromLocal8Bit(enum_element_to_string(prob_paras[i].element)) + QString(" >  %0mm").arg(prob_paras[i].thresh);
		} else {
			title = QString("Initial Probibility of  ") + QString::fromLocal8Bit(enum_element_to_string(prob_paras[i].element)) + QString(" >  %0mm N=%1").arg(prob_paras[i].thresh).arg(prob_paras[i].analog_number);
		}
        image_matrix_view_->AddValueMap(prob_maps[i], prob_maps[i]->map_range, title);
    }
}

void WrfRegionDetectionView::OnActionManualRegionSelectionTriggered(){
    if ( action_manual_region_selection_->isChecked() )
        image_matrix_view_->SetTrackingPen(true);
    else
        image_matrix_view_->SetTrackingPen(false);
}

void WrfRegionDetectionView::GetSelectedRegionContour(std::vector< QPointF >& contour){
    image_matrix_view_->GetSelectedRegionContour(contour);
}

void WrfRegionDetectionView::OnRetrievalTriggered(){
    emit RetrievalTriggered(forecasting_type_);
}