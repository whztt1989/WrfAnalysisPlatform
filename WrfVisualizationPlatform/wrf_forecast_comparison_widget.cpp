#include "wrf_forecast_comparison_widget.h"
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <iostream>
#include "wrf_image_matrix_view.h"
#include "wrf_forecasting_manager.h"
#include "wrf_data_manager.h"
#include "wrf_reforecast_manager.h"
#include "wrf_ensemble_manager.h"
#include "wrf_add_map_dialog.h"

WrfForecastComparisonWidget::WrfForecastComparisonWidget(WrfComparisonType type){
    InitWidget();
}

WrfForecastComparisonWidget::~WrfForecastComparisonWidget(){

}

void WrfForecastComparisonWidget::InitWidget(){
    image_matrix_view_ = new WrfImageMatrixView;

    QVBoxLayout* main_layout = new QVBoxLayout;
	QToolBar* main_tool_bar = new QToolBar;
	action_manual_region_selection_ = new QAction(QIcon("./Resources/painter.jpg"), tr("Manual Selection"), this);
	action_manual_region_selection_->setCheckable(true);
	action_manual_region_selection_->setChecked(false);

	action_add_map_ = new QAction(QIcon("./Resources/plus.png"), tr("Add Map"), this);

	main_tool_bar->addAction(action_manual_region_selection_);
	main_tool_bar->addAction(action_add_map_);
	main_layout->addWidget(main_tool_bar);
    main_layout->addWidget(image_matrix_view_);
    main_layout->setMargin(0);

    this->setLayout(main_layout);

	connect(action_manual_region_selection_, SIGNAL(triggered()), this, SLOT(OnActionManualRegionSelectionTriggered()));
	connect(action_add_map_, SIGNAL(triggered()), this, SLOT(OnActionAddMapTriggered()));
}

void WrfForecastComparisonWidget::UpdateWidget(){
	if ( image_matrix_view_->GetMapSize() == 0 ){
		WrfGridValueMap* ens_average_map = WrfEnsembleManager::GetInstance()->GenerateAverageMap(WRF_ACCUMULATED_PRECIPITATION);
		this->image_matrix_view_->AddValueMap(ens_average_map, ens_average_map->map_range, QString("Ensemble Mean Forecast"));

		std::vector< WrfGridValueMap* > refcst_maps;
		std::vector< ProbabilisticPara > refcst_paras;
		WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_REFORECAST_PQPF, refcst_paras, refcst_maps);

		MapRange ensemble_map_range;
		WrfDataManager::GetInstance()->GetEnsembleMapRange(ensemble_map_range);

		for ( int i = 0; i < refcst_maps.size(); ++i ){
			QString title = QString("Initial Probability of APCP > %0mm N=%1").arg(refcst_paras[i].thresh).arg(refcst_paras[i].analog_number);
			image_matrix_view_->AddValueMap(refcst_maps[i], ensemble_map_range, title);
		}
	}
    /*image_matrix_view_->Clear();

    MapRange ensemble_map_range;
    WrfDataManager::GetInstance()->GetEnsembleMapRange(ensemble_map_range);

    int fhour = WrfReforecastManager::GetInstance()->fcst_hour();

    std::vector< WrfGridValueMap* > maps, ens_fcst_maps, refcst_maps, adapted_maps;
    std::vector< ProbabilisticPara > paras, ens_fcst_paras, refcst_paras, adapted_paras;

    switch ( comparison_type_ ){
    case REFORECAST_COMPARISON:{
        WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_REFORECAST_PQPF, paras, maps);
        WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_ADAPTED_REFORECAST_PQPF, adapted_paras, adapted_maps);
		
        
        for ( int i = 0; i < maps.size(); ++i ){
            QString title = QString("Probibility of Precip. > %0mm ").arg(paras[i].thresh);
            image_matrix_view_->AddValueMap(maps[i], ensemble_map_range, title);
            image_matrix_view_->AddValueMap(adapted_maps[i], ensemble_map_range, QString("Adapted ") + title);

            std::vector< WrfGridValueMap* > reanalysis_map;
            WrfDataManager::GetInstance()->GetGridValueMap(paras[i].time, WRF_REANALYSIS, paras[i].element, fhour, reanalysis_map);
            image_matrix_view_->AddValueMap(reanalysis_map[0], ensemble_map_range, QString("Reanalysis Result"));
        }
        break;
                               }
    case ENSEMBLE_COMPARISON:{
        break;
                             }
    case ALL_COMPARISON:{
        WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_ENSEMBLE_AVERAGE, paras, maps);
        WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_ADAPTED_REFORECAST_PQPF, adapted_paras, adapted_maps);
        WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_REFORECAST_PQPF, refcst_paras, refcst_maps);
		WrfForecastingManager::GetInstance()->GetForecastingMaps(WRF_ENSEMBLER_PQPF, ens_fcst_paras, ens_fcst_maps);

        for ( int i = 0; i < maps.size(); ++i ){
            QString title = QString("Ensemble Average of APCP");
            image_matrix_view_->AddValueMap(maps[i], ensemble_map_range, title);
			title = QString("Ensemble Probability of APCP > %0mm").arg(ens_fcst_paras[i].thresh);
			image_matrix_view_->AddValueMap(ens_fcst_maps[i], ensemble_map_range, title);
            title = QString("Reforecast Probability of APCP > %0mm ").arg(refcst_paras[i].thresh);
            image_matrix_view_->AddValueMap(refcst_maps[i], ensemble_map_range, title);
            title = QString("Adapted Reforecast Probability of APCP > %0mm ").arg(adapted_paras[i].thresh);
            image_matrix_view_->AddValueMap(adapted_maps[i], ensemble_map_range, title);
        }

		std::vector< WrfGridValueMap* > reanalysis_map;
		WrfDataManager::GetInstance()->GetGridValueMap(paras[0].time, WRF_REANALYSIS, paras[0].element, fhour, reanalysis_map);

		QDateTime base_date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0));

		qint64 temp = paras[0].time;
		temp *= 3600000;
		QDateTime history_date = base_date.addMSecs(temp);
		image_matrix_view_->AddValueMap(reanalysis_map[0], ensemble_map_range, QString("Reanalysis Result"));

		image_matrix_view_->AddGeographicaMap(ensemble_map_range);
        break;
                        }
    default:
        break;
    }*/
}

void WrfForecastComparisonWidget::OnActionManualRegionSelectionTriggered(){
	if ( action_manual_region_selection_->isChecked() )
		image_matrix_view_->SetTrackingPen(true);
	else
		image_matrix_view_->SetTrackingPen(false);
}

void WrfForecastComparisonWidget::OnActionAddMapTriggered(){
	WrfAddMapDialog add_map_dialog;

	if ( add_map_dialog.exec() == QDialog::Accepted ){
		int mode;
		float threshold;
		add_map_dialog.GetSelectionPara(mode, threshold);

		switch ( mode ){
		case 0:
			// reforecast probability
			{
				QDateTime current_datetime = WrfReforecastManager::GetInstance()->forecasting_date();
				ProbabilisticPara para;
				qint64 temp_time = current_datetime.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
				para.time = (int)temp_time;
				para.element = WRF_ACCUMULATED_PRECIPITATION;
				para.thresh = threshold;
				para.analog_number = -1;
				QString para_name = QString("Calibrated Probability of APCP > %0mm").arg(threshold);
				WrfGridValueMap* prob_map = WrfReforecastManager::GetInstance()->GenerateForecastingMap(para);
				this->image_matrix_view_->AddValueMap(prob_map, prob_map->map_range, para_name);
			}
			break;
		case 1:
			// ensemble probability
			{
				QDateTime current_datetime = WrfReforecastManager::GetInstance()->forecasting_date();
				ProbabilisticPara para;
				qint64 temp_time = current_datetime.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
				para.time = (int)temp_time;
				para.element = WRF_ACCUMULATED_PRECIPITATION;
				para.thresh = threshold;
				QString para_name = QString("Ensemble Probability of APCP > %0mm").arg(threshold);
				WrfGridValueMap* prob_map = WrfEnsembleManager::GetInstance()->GenerateEnsembleForecastingMap(para);
				this->image_matrix_view_->AddValueMap(prob_map, prob_map->map_range, para_name);
			}
			break;
		case 2:
			// reanalysis
			{
				QDateTime current_datetime = WrfReforecastManager::GetInstance()->forecasting_date();
				qint64 temp_time = current_datetime.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
				int fhour = WrfReforecastManager::GetInstance()->fcst_hour();
				MapRange ensemble_map_range;
				WrfDataManager::GetInstance()->GetEnsembleMapRange(ensemble_map_range);

				std::vector< WrfGridValueMap* > reanalysis_map;
				WrfDataManager::GetInstance()->GetGridValueMap(temp_time, WRF_REANALYSIS, WRF_ACCUMULATED_PRECIPITATION, fhour, reanalysis_map);

				QDateTime base_date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0));

				image_matrix_view_->AddValueMap(reanalysis_map[0], ensemble_map_range, QString("Observed Map"));
			}
			break;
		case 3:
			// geographical map
			{
				MapRange ensemble_map_range;
				WrfDataManager::GetInstance()->GetEnsembleMapRange(ensemble_map_range);
				image_matrix_view_->AddGeographicaMap(ensemble_map_range);
			}
			break;
		default:
			break;
		}
	}
}