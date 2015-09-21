#include "wrf_event_analyzing_view.h"
#include <QtGui/QSplitter>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtCore/QLocale>
#include <iostream>
#include "wrf_mds_plot_view.h"
#include "wrf_similarity_time_line_view.h"
#include "wrf_stamp_viewer.h"
#include "wrf_data_manager.h"
#include "wrf_stamp.h"
#include "wrf_reforecast_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_image_viewer.h"
#include "wrf_rendering_element_factory.h"
#include "wrf_probabilistic_forecast_dialog.h"
#include "scatter_plot.h"
#include "wrf_scatter_plot_dialog.h"
#include "wrf_utility.h"
#include "wrf_ensemble_manager.h"
#include "wrf_ensemble_data_view.h"
#include "wrf_ensemble_selection_widget.h"

WrfEventAnalyzingView::WrfEventAnalyzingView()
    : selected_analog_number_(20), previous_selected_analog_number_(20), mds_mode_(0), ens_data_view_(NULL) {
    InitWidget();
}

WrfEventAnalyzingView::~WrfEventAnalyzingView(){

}

void WrfEventAnalyzingView::SetEventInfo(WrfElementType element, std::vector< float >& similarity){
    forecast_element_ = element;
    date_similarity_ = similarity;
}

void WrfEventAnalyzingView::SetViewingRange(MapRange& range){
    viewing_range_ = range;
}

void WrfEventAnalyzingView::SetSelectedGridIndex(std::vector< int >& selected_grid_index){
	selected_grid_index_ = selected_grid_index;
}

void WrfEventAnalyzingView::SetSelectionPath(std::vector< QPointF >& contour){
	contour_path_ = contour;
}

void WrfEventAnalyzingView::UpdateWidget(){
    forecasting_date_ = WrfReforecastManager::GetInstance()->forecasting_date();
    fhour_ =  WrfReforecastManager::GetInstance()->fcst_hour();
    year_length_ = WrfReforecastManager::GetInstance()->retrieval_year_length();
    day_length_ = WrfReforecastManager::GetInstance()->retrieval_day_length();
    WrfReforecastManager::GetInstance()->GetReferenceDateTime(retrieval_date_time_);

    std::vector< float > temp_similarity = date_similarity_;
    max_similarity_index_.resize(date_similarity_.size());
    for ( int i = 0; i < date_similarity_.size(); ++i ) max_similarity_index_[i] = i;
    WrfStatisticSolver::Sort(temp_similarity, max_similarity_index_);

    UpdateCurrentStampView();
    UpdateHistoryStampView(0);

    UpdateTimeLineView();
    UpdateMdsPlotView(mds_mode_);
}

void WrfEventAnalyzingView::InitWidget(){
    time_line_view_ = new WrfSimilarityTimeLineView;
    time_line_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stamp_viewer_ = new WrfStampViewer;

    current_stamp_viewer_ = new WrfStampViewer;
    current_stamp_viewer_->setFixedHeight(200);
    current_event_stamps_.resize(1);
    current_event_stamps_[0] = new WrfStamp;
	connect(current_event_stamps_[0], SIGNAL(EventTimeTriggered(int)), this, SLOT(OnEventTimeTriggered(int)));

    mds_plot_ = new WrfMdsPlotView;
    mds_plot_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mds_plot_->SetTrackingPen(true);
    mds_plot_->setFixedHeight(250);
	QToolBar* mds_tool_bar = new QToolBar;
	QAction* fcst_mds_action = new QAction(QIcon("./Resources/forecast.png"), tr("fcst"), this);
	fcst_mds_action->setCheckable(true);
	fcst_mds_action->setChecked(false);
	QAction* rean_mds_action = new QAction(QIcon("./Resources/reanalysis.png"), tr("Reanalysis"), this);
	rean_mds_action->setCheckable(true);
	rean_mds_action->setChecked(true);
	QActionGroup* action_group = new QActionGroup(this);
	action_group->setExclusive(true);
	action_group->addAction(rean_mds_action);
	action_group->addAction(fcst_mds_action);

	mds_tool_bar->addAction(rean_mds_action);
	mds_tool_bar->addAction(fcst_mds_action);
	mds_tool_bar->setContentsMargins(0, 0, 0, 0);
	QVBoxLayout* mds_widget_layout = new QVBoxLayout;
	mds_widget_layout->setContentsMargins(0, 0, 0, 0);
	mds_widget_layout->setMargin(0);
	mds_widget_layout->addWidget(mds_tool_bar);
	mds_widget_layout->addWidget(mds_plot_);
	QWidget* mds_widget = new QWidget;
	mds_widget->setContentsMargins(0, 0, 0, 0);
	mds_widget->setLayout(mds_widget_layout);

	connect(rean_mds_action, SIGNAL(triggered()), this, SLOT(OnReanalysisMdsTriggered()));
	connect(fcst_mds_action, SIGNAL(triggered()), this, SLOT(OnFcstMdsTriggered()));

    average_viewer_ = new WrfImageViewer;
    average_viewer_->setMinimumSize(300, 400);
    pqpf_viewer_ = new WrfImageViewer;
    pqpf_viewer_->setMinimumSize(300, 400);
    scatter_plot_ = new ScatterPlot;

    main_tool_bar_ = new QToolBar;
    action_show_average_result_ = new QAction(QIcon("./Resources/average_t.png"), tr("Show Average"), this);
    action_show_pqpf_ = new QAction(QIcon("./Resources/calibrated_result_t.png"), tr("Show PQPF"), this);
    action_show_precipitation_scatter_plot_ = new QAction(QIcon("./Resources/scatter_plot_t1.png"), tr("Show Precipitation Scatter Plot"), this);
	action_show_scatter_plot_ = new QAction(QIcon("./Resources/scatter_plot_t.png"), tr("Show Scatter Plot"), this);
    main_tool_bar_->addAction(action_show_average_result_);
    main_tool_bar_->addAction(action_show_pqpf_);
    main_tool_bar_->addAction(action_show_precipitation_scatter_plot_);
	main_tool_bar_->addAction(action_show_scatter_plot_);

    QLabel* analog_num_lable = new QLabel(tr("Analog Number: "));
    analog_num_slider_ = new QSlider(Qt::Horizontal);
    analog_num_slider_->setRange(20, 100);
    analog_num_slider_->setValue(20);
    analog_num_slider_->setSingleStep(1);
    analog_num_slider_->setFixedWidth(200);
    QHBoxLayout* analog_num_layout = new QHBoxLayout;
    analog_num_layout->setAlignment(Qt::AlignLeft);
    analog_num_layout->addWidget(analog_num_lable);
    analog_num_layout->addWidget(analog_num_slider_);
    QWidget* analog_widget = new QWidget;
    analog_widget->setLayout(analog_num_layout);
    analog_widget->setFixedWidth(330);
    main_tool_bar_->addWidget(analog_widget);

	QSplitter* func_splitter = new QSplitter(Qt::Vertical);
    func_splitter->addWidget(current_stamp_viewer_);
    //func_splitter->addWidget(mds_widget);
    func_splitter->addWidget(time_line_view_);
	func_splitter->setFixedWidth(400);

    QHBoxLayout* center_layout = new QHBoxLayout;
    center_layout->setAlignment(Qt::AlignLeft);
    center_layout->addWidget(func_splitter);
    center_layout->addWidget(stamp_viewer_);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(main_tool_bar_);
    main_layout->addLayout(center_layout);
    main_layout->setMargin(0);

    this->setLayout(main_layout);

    connect(analog_num_slider_, SIGNAL(sliderReleased()), this, SLOT(OnAnalogNumberChanged()));
    connect(mds_plot_, SIGNAL(SelectionUpdated()), this, SLOT(OnMdsSelectedChanged()));
    connect(action_show_average_result_, SIGNAL(triggered()), this, SLOT(OnActionGenerateAverageTriggered()));
    connect(action_show_pqpf_, SIGNAL(triggered()), this, SLOT(OnActionGeneratePqpfTriggered()));
    connect(action_show_precipitation_scatter_plot_, SIGNAL(triggered()), this, SLOT(OnActionShowPrecipitationScatterPlotTriggered()));
	connect(action_show_scatter_plot_, SIGNAL(triggered()), this, SLOT(OnActionShowVariableScatterPlotTriggered()));
	connect(time_line_view_, SIGNAL(EventTimeTriggered(int)), this, SLOT(OnEventTimeTriggered(int)));
}

void WrfEventAnalyzingView::UpdateTimeLineView(){
    std::vector< std::string > year_str;
    for ( int i = 0; i < year_length_; ++i ){
        char year_buf[10];
        itoa(forecasting_date_.date().year() - i, year_buf, 10);
        year_str.push_back(std::string(year_buf));
    }

    std::vector< std::vector< float > > time_similarity;
    time_similarity.resize(year_length_ + 1);
	std::vector< std::vector< int > > date_time;
	date_time.resize(year_length_ + 1);

    int accu_index = 0;
    for ( int i = 0; i < day_length_; ++i ){
        time_similarity[0].push_back(date_similarity_[accu_index]);
		date_time[0].push_back(retrieval_date_time_[accu_index]);
        accu_index++;
    }

    for ( int i = 1; i <= year_length_; ++i ){
        for ( int j = -1 * day_length_; j <= day_length_; ++j ){
            time_similarity[i].push_back(date_similarity_[accu_index]);
			date_time[i].push_back(retrieval_date_time_[accu_index]);
            accu_index++;
        }
    }
    time_line_view_->SetData(time_similarity, date_time, year_str);
}

void WrfEventAnalyzingView::UpdateCurrentStampView(){
    std::vector< WrfGridValueMap* > current_fcst_maps;
    WrfDataManager::GetInstance()->GetGridValueMap(forecasting_date_, WRF_NCEP_ENSEMBLES, forecast_element_, fhour_, current_fcst_maps);
    if ( current_fcst_maps.size() == 0 ) 
        WrfDataManager::GetInstance()->GetGridValueMap(forecasting_date_, WRF_NCEP_ENSEMBLE_MEAN, forecast_element_, fhour_, current_fcst_maps);
    if ( current_fcst_maps.size() == 0 ) return;

    if ( current_fcst_maps.size() > 1 ){
        for ( int i = 1; i < current_fcst_maps.size(); ++i ){
            for ( int j = 0; j < current_fcst_maps[0]->map_range.x_grid_number * current_fcst_maps[0]->map_range.y_grid_number; ++j )
                current_fcst_maps[0]->values[j] += current_fcst_maps[i]->values[j];
        }
        for ( int i = 0; i < current_fcst_maps[0]->map_range.x_grid_number * current_fcst_maps[0]->map_range.y_grid_number; ++i )
            current_fcst_maps[0]->values[i] /= current_fcst_maps.size();

        for ( int i = 1; i < current_fcst_maps.size(); ++i ) delete current_fcst_maps[i];
        current_fcst_maps.resize(1);
    }
    
    std::vector< WrfGridValueMap* > reanalysis_map;
    WrfDataManager::GetInstance()->GetGridValueMap(forecasting_date_, WRF_REANALYSIS, forecast_element_, fhour_, reanalysis_map);
    current_event_stamps_[0]->SetValueMap(current_fcst_maps[0], NULL, viewing_range_);
    current_event_stamps_[0]->SetDateString(QLocale(QLocale::C).toString(forecasting_date_, QString("yyyy MMM dd")));
	qint64 time = forecasting_date_.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
	current_event_stamps_[0]->SetDateTime(time);

    current_stamp_viewer_->SetStamps(current_event_stamps_);

    std::vector< int > current_selected_index;
    for ( int i = 0; i < current_event_stamps_.size(); ++i ) current_selected_index.push_back(i);
    current_stamp_viewer_->SetSelectedIndex(current_selected_index);
    current_stamp_viewer_->SetTitle(QString("Current Weather"));
}

void WrfEventAnalyzingView::UpdateHistoryStampView(int mode){
	int begin_number = 0;
	if ( mode == 1 ) begin_number = previous_selected_analog_number_;

    for ( int i = begin_number; i < selected_analog_number_; ++i ){
        std::vector< WrfGridValueMap* > fcst_maps;
        WrfDataManager::GetInstance()->GetGridValueMap(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]], WRF_NCEP_ENSEMBLES, forecast_element_, fhour_, fcst_maps);
        if ( fcst_maps.size() == 0 ) 
            WrfDataManager::GetInstance()->GetGridValueMap(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]], WRF_NCEP_ENSEMBLE_MEAN, forecast_element_, fhour_, fcst_maps);
        if ( fcst_maps.size() == 0 ) return;

        if ( fcst_maps.size() > 1 ){
            for ( int i = 1; i < fcst_maps.size(); ++i ){
                for ( int j = 0; j < fcst_maps[0]->map_range.x_grid_number * fcst_maps[0]->map_range.y_grid_number; ++j )
                    fcst_maps[0]->values[j] += fcst_maps[i]->values[j];
            }
            for ( int i = 0; i < fcst_maps[0]->map_range.x_grid_number * fcst_maps[0]->map_range.y_grid_number; ++i )
                fcst_maps[0]->values[i] /= fcst_maps.size();

            for ( int i = 1; i < fcst_maps.size(); ++i ) delete fcst_maps[i];
            fcst_maps.resize(1);
        }

        std::vector< WrfGridValueMap* > reanalysis_map;
        WrfDataManager::GetInstance()->GetGridValueMap(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]], WRF_REANALYSIS, forecast_element_, fhour_, reanalysis_map);

        if ( i >= event_stamps_.size() ){
            WrfStamp* stamp = new WrfStamp(fcst_maps[0], reanalysis_map[0], viewing_range_, contour_path_);
			connect(stamp, SIGNAL(EventTimeTriggered(int)), this, SLOT(OnEventTimeTriggered(int)));
            event_stamps_.push_back(stamp);
        } else {
            event_stamps_[i]->SetValueMap(fcst_maps[0], reanalysis_map[0], viewing_range_);
        }

        QDateTime base_date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0));
        qint64 temp = retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]];
        temp *= 3600000;
        QDateTime history_date = base_date.addMSecs(temp);
        event_stamps_[i]->SetDateString(QLocale(QLocale::C).toString(history_date, QString("yyyy MMM dd")));
		event_stamps_[i]->SetDateTime(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]]);
    }

    stamp_viewer_->SetStamps(event_stamps_);
    std::vector< int > selected_index;
    for ( int i = 0; i < event_stamps_.size() && i < selected_analog_number_; ++i ) selected_index.push_back(i);
    stamp_viewer_->SetSelectedIndex(selected_index);
    stamp_viewer_->SetTitle(QString("Historical Analogs"));
}

void WrfEventAnalyzingView::UpdateMdsPlotView(int mode){
    // update mds viewer
    std::vector< std::vector< float > > mds_data;
    mds_data.resize(event_stamps_.size() < selected_analog_number_? event_stamps_.size():selected_analog_number_);
    for ( int i = 0; i < event_stamps_.size() && i < selected_analog_number_; ++i ){
        QImage* image;
		if ( mds_mode_ == 0 )
			image = event_stamps_[i]->GetReanalysisImage();
		else 
			image = event_stamps_[i]->GetFcstImage();
        mds_data[i].resize(image->width() * image->height());
        for ( int h = 0; h < image->height(); ++h )
            for ( int w = 0; w < image->width(); ++w ){
                QColor color = QColor(image->pixel(w, h));
                mds_data[i][h * image->width() + w] = color.hslHueF();
            }
    }
    std::cout << "MDS data size: " << mds_data.size() << std::endl;
    mds_plot_->SetData(mds_data);
}

void WrfEventAnalyzingView::OnMdsSelectedChanged(){
    std::vector< int > selected_index;
    std::vector< bool > is_selected;

    mds_plot_->GetSelectionIndex(is_selected);

    for ( int i = 0; i < is_selected.size(); ++i )
        if ( is_selected[i] ) selected_index.push_back(i);

    if ( selected_index.size() == 0 ){
        for ( int i = 0; i < event_stamps_.size() && i < selected_analog_number_; ++i ) selected_index.push_back(i);
        is_selected.assign(is_selected.size(), true);
        mds_plot_->SetSelectionIndex(is_selected);
    }
    
    stamp_viewer_->SetSelectedIndex(selected_index);
}

void WrfEventAnalyzingView::OnAnalogNumberChanged(){
	previous_selected_analog_number_ = selected_analog_number_;
    selected_analog_number_ = analog_num_slider_->value();
    UpdateHistoryStampView(1);
    UpdateMdsPlotView(mds_mode_);
}

void WrfEventAnalyzingView::OnActionGenerateAverageTriggered(){
    std::vector< int > selected_time_vec;
    std::vector< bool > is_selected;
    mds_plot_->GetSelectionIndex(is_selected);

    for ( int i = 0; i < is_selected.size(); ++i )
        if ( is_selected[i] ) selected_time_vec.push_back(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]]);

    WrfGridValueMap* value_map = WrfStatisticSolver::GenerateAverageResult(selected_time_vec, forecast_element_, fhour_);

    average_viewer_->ClearElement();
    average_viewer_->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(value_map));
    average_viewer_->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(value_map->element_type));
    average_viewer_->SetMapRange(viewing_range_);
    average_viewer_->SetTitle("Average Reanalysis");

    average_viewer_->show();
}

void WrfEventAnalyzingView::OnActionGeneratePqpfTriggered(){
    std::vector< int > selected_time_vec;
    std::vector< bool > is_selected;
    mds_plot_->GetSelectionIndex(is_selected);

    for ( int i = 0; i < is_selected.size(); ++i )
        if ( is_selected[i] ) selected_time_vec.push_back(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]]);

    WrfProbabilisticForecastDialog prob_dialog;

    if ( prob_dialog.exec() ){
        ProbabilisticPara para;
        prob_dialog.GetSelectionParas(para);
        
        WrfGridValueMap* value_map = WrfStatisticSolver::GeneratePqpfResult(selected_time_vec, para, fhour_);
        pqpf_viewer_->ClearElement();
        pqpf_viewer_->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(value_map));
        pqpf_viewer_->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(value_map->element_type));
        pqpf_viewer_->SetMapRange(viewing_range_);
        pqpf_viewer_->SetTitle("Average Reanalysis");

        pqpf_viewer_->show();
    }
}

void WrfEventAnalyzingView::OnActionShowPrecipitationScatterPlotTriggered(){
    scatter_plot_->SetAxisNames(std::string("Forecast Average"), std::string("Reanalysis"));

    std::vector< float > plot_values;

    MapRange reanalysis_map_range;
    WrfDataManager::GetInstance()->GetModelMapRange(WRF_REANALYSIS, reanalysis_map_range);

	std::vector< int > selected_index;
	for ( int i = 0; i < reanalysis_map_range.x_grid_number; ++i )
		for ( int j = 0; j < reanalysis_map_range.y_grid_number; ++j ){
			float lon = reanalysis_map_range.start_x + reanalysis_map_range.x_grid_space * i;
			float lat = reanalysis_map_range.start_y + reanalysis_map_range.y_grid_space * j;

			if ( lon > viewing_range_.start_x && lon < viewing_range_.end_x 
				&& lat > viewing_range_.start_y && lat < viewing_range_.end_y ){
					selected_index.push_back(j * reanalysis_map_range.x_grid_number + i);
			}
		}

    std::vector< int > selected_time_vec;
    std::vector< bool > is_selected;
    mds_plot_->GetSelectionIndex(is_selected);

    for ( int i = 0; i < is_selected.size(); ++i )
        if ( is_selected[i] ) selected_time_vec.push_back(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]]);

    for ( int t = 0; t < selected_time_vec.size(); ++t ){
        std::vector< WrfGridValueMap* > fcst_maps;
        WrfDataManager::GetInstance()->GetGridValueMap(selected_time_vec[t], WRF_NCEP_ENSEMBLES, forecast_element_, fhour_, fcst_maps);
        if ( fcst_maps.size() == 0 ) 
            WrfDataManager::GetInstance()->GetGridValueMap(selected_time_vec[t], WRF_NCEP_ENSEMBLE_MEAN, forecast_element_, fhour_, fcst_maps);
        if ( fcst_maps.size() == 0 ) return;

        if ( fcst_maps.size() > 1 ){
            for ( int i = 1; i < fcst_maps.size(); ++i ){
                for ( int j = 0; j < fcst_maps[0]->map_range.x_grid_number * fcst_maps[0]->map_range.y_grid_number; ++j )
                    fcst_maps[0]->values[j] += fcst_maps[i]->values[j];
            }
            for ( int i = 0; i < fcst_maps[0]->map_range.x_grid_number * fcst_maps[0]->map_range.y_grid_number; ++i )
                fcst_maps[0]->values[i] /= fcst_maps.size();

            for ( int i = 1; i < fcst_maps.size(); ++i ) delete fcst_maps[i];
            fcst_maps.resize(1);
        }

        std::vector< WrfGridValueMap* > reanalysis_map;
        WrfDataManager::GetInstance()->GetGridValueMap(selected_time_vec[t], WRF_REANALYSIS, forecast_element_, fhour_, reanalysis_map);

        WrfGridValueMap* converted_map = WrfStatisticSolver::Convert2Map(fcst_maps[0], reanalysis_map[0]->map_range);

        for ( int i = 0; i < selected_index.size(); ++i ){
            float num_value = converted_map->values[i];
            float reana_value = reanalysis_map[0]->values[i];
            if ( num_value > 3000 || reana_value > 3000 || num_value < 0 || reana_value < 0) {
                num_value = 0;
                reana_value = 0;
            }
            plot_values.push_back(num_value);
            plot_values.push_back(reana_value);
        }
    }

    scatter_plot_->SetData(plot_values);
    scatter_plot_->SetAxisValueRange(0, 50, 0, 50);
    scatter_plot_->show();
}

void WrfEventAnalyzingView::OnActionShowVariableScatterPlotTriggered(){
	WrfScatterPlotDialog scatter_dialog;
	if ( scatter_dialog.exec() == QDialog::Accepted ){
		WrfModelType x_model, y_model;
		WrfElementType x_element, y_element;
		int x_ens, y_ens;
		float x_min = 1e10, x_max = -1e10, y_min = 1e10, y_max = -1e10;

		scatter_dialog.GetSelectedParameters(x_model, x_element, x_ens, x_min, x_max, y_model, y_element, y_ens, y_min, y_max);

		int fhour = WrfEnsembleManager::GetInstance()->FcstHour();

		std::vector< int > selected_time_vec;
		std::vector< bool > is_selected;
		mds_plot_->GetSelectionIndex(is_selected);

		for ( int i = 0; i < is_selected.size(); ++i )
			if ( is_selected[i] ) selected_time_vec.push_back(retrieval_date_time_[max_similarity_index_[max_similarity_index_.size() - 1 - i]]);

		ScatterPlot* plot = new ScatterPlot;
		plot->SetAxisNames(std::string(enum_element_to_string(x_element)), std::string(enum_element_to_string(y_element)));
		std::vector< float > plot_values;
		for ( int t = 0; t < selected_time_vec.size(); ++t ){
			std::vector< float* > x_data, y_data;
			WrfDataManager::GetInstance()->GetGridData(selected_time_vec[t], x_model, x_element, fhour, x_data);
			WrfDataManager::GetInstance()->GetGridData(selected_time_vec[t], y_model, y_element, fhour, y_data);

			for ( int i = 0; i < selected_grid_index_.size(); ++i ){
				int grid_index = selected_grid_index_[i];
				float temp_x = 0;
				if ( x_ens == -1 ){
					for ( int j = 0; j < x_data.size(); ++j )
						temp_x += *(x_data[j] + grid_index);
					temp_x /= x_data.size();
				} else {
					temp_x = *(x_data[x_ens] + grid_index);
				}
				plot_values.push_back(temp_x);
				float temp_y = 0;
				if ( y_ens == -1 ){
					for ( int j = 0; j < y_data.size(); ++j )
						temp_y += *(y_data[j] + grid_index);
					temp_y /= y_data.size();
				} else {
					temp_y = *(y_data[y_ens] + grid_index);
				}
				plot_values.push_back(temp_y);
			}
		}

		plot->SetData(plot_values);

		plot->SetAxisValueRange(x_min, x_max, y_min, y_max);
		plot->show();
	}
}

void WrfEventAnalyzingView::OnFcstMdsTriggered(){
	mds_mode_ = 1;
	UpdateMdsPlotView(mds_mode_);
}

void WrfEventAnalyzingView::OnReanalysisMdsTriggered(){
	mds_mode_ = 0;
	UpdateMdsPlotView(mds_mode_);
}

void WrfEventAnalyzingView::OnEventTimeTriggered(int time){
	if ( time == -1 ) return;

	WrfEnsembleSelectionWidget initialization_dialog;
	if ( initialization_dialog.exec() == QDialog::Accepted ){
		if ( ens_data_view_ == NULL ) ens_data_view_ = new WrfEnsembleDataView;

		std::vector< WrfElementType > ensemble_elements;
		std::vector< WrfElementType > ensemble_mean_elements;
		initialization_dialog.GetSelectionParas(ensemble_elements, ensemble_mean_elements);

		QDateTime base_date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0));
		qint64 temp = time;
		temp *= 3600000;
		QDateTime history_date = base_date.addMSecs(temp);
		ens_data_view_->SetPara(history_date, WrfEnsembleManager::GetInstance()->FcstHour(), ensemble_elements, ensemble_mean_elements);

		ens_data_view_->setWindowTitle(QString("Ensemble data of ") + QLocale(QLocale::C).toString(history_date, QString("yyyy MMM dd")));
		ens_data_view_->show();
	}
}