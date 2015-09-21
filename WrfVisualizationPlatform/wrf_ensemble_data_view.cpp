#include "wrf_ensemble_data_view.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include "wrf_image_matrix_view.h"
#include "wrf_data_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_utility.h"
#include "wrf_image_series_view.h"

WrfEnsembleDataView::WrfEnsembleDataView(){
    data_manager_ = WrfDataManager::GetInstance();

    InitWidget();
}

WrfEnsembleDataView::~WrfEnsembleDataView(){
  
}

void WrfEnsembleDataView::InitWidget(){
    main_tool_bar_ = new QToolBar;
    action_manual_region_selection_ = new QAction(QIcon("./Resources/painter.jpg"), tr("Manual Selection"), this);
    action_manual_region_selection_->setCheckable(true);
    action_manual_region_selection_->setChecked(false);
    action_apply_region_retrieval_ = new QAction(QIcon("./Resources/find.png"), tr("Retrieval"), this);

    main_tool_bar_->addAction(action_manual_region_selection_);
    main_tool_bar_->addAction(action_apply_region_retrieval_);

    image_matrix_view_ = new WrfImageMatrixView;
    image_series_scroll_area_ = new QScrollArea;
    image_series_scroll_area_->setFixedWidth(200);
    image_series_scroll_widget_ = new QWidget;
    image_series_scroll_widget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    QHBoxLayout* main_splitter = new QHBoxLayout;
    main_splitter->addWidget(image_matrix_view_);
    main_splitter->addWidget(image_series_scroll_area_, 0, Qt::AlignRight);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(main_tool_bar_);
    main_layout->addLayout(main_splitter);
	main_layout->setMargin(0);

    this->setLayout(main_layout);

    connect(action_manual_region_selection_, SIGNAL(triggered()), this, SLOT(OnActionManualRegionSelectionTriggered()));
    connect(action_apply_region_retrieval_, SIGNAL(triggered()), this, SIGNAL(RetrievalTriggered()));
}

void WrfEnsembleDataView::SetPara(QDateTime date, int forecast_hour, std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements){
    viewing_date_ = date;
    fhour_ = forecast_hour;
    ens_elements_ = ensemble_elements;
    ens_mean_elements_ = ensemble_mean_elements;

    // clear data
    image_matrix_view_->Clear();
    for ( int i = 0; i < viewing_maps_.size(); ++i )
        for ( int j = 0; j < viewing_maps_[i].size(); ++j )
            if ( viewing_maps_[i][j] != NULL ) delete viewing_maps_[i][j];
    viewing_maps_.clear();

    LoadViewingData();

    UpdateWidget();
}

void WrfEnsembleDataView::LoadViewingData(){
    viewing_maps_.resize(ens_elements_.size() + ens_mean_elements_.size());
    for ( int i = 0; i < ens_elements_.size(); ++i ){
        data_manager_->GetGridValueMap(viewing_date_, WRF_NCEP_ENSEMBLES, ens_elements_[i], fhour_, viewing_maps_[i]);
    }
    for ( int i = 0; i < ens_mean_elements_.size(); ++i ){
        data_manager_->GetGridValueMap(viewing_date_, WRF_NCEP_ENSEMBLE_MEAN, ens_mean_elements_[i], fhour_, viewing_maps_[i + ens_elements_.size()]);
    }
}

void WrfEnsembleDataView::UpdateWidget(){
    series_titles_.clear();

    for ( int i = 0; i < ens_elements_.size(); ++i ){
        std::vector< QString > image_titles;
        for ( int j = 0; j < viewing_maps_[i].size(); ++j ){
            QString temp_tile = enum_model_to_string(WRF_NCEP_ENSEMBLES) + QString(" %0").arg(j);
            image_titles.push_back(temp_tile);
        }

        value_map_titles_.push_back(image_titles);
        series_titles_.push_back(enum_element_to_string(ens_elements_[i]));
    }
    
    for ( int i = 0; i < ens_mean_elements_.size(); ++i ){
        std::vector< QString > image_titles;
        QString temp_tile = enum_model_to_string(WRF_NCEP_ENSEMBLE_MEAN);
        image_titles.push_back(temp_tile);

        value_map_titles_.push_back(image_titles);
        series_titles_.push_back(enum_element_to_string(ens_mean_elements_[i]));
    }

    for ( int i = 0; i < viewing_maps_[0].size(); ++i ){
        QString temp_title = series_titles_[0] + "    " + value_map_titles_[0][i];
        image_matrix_view_->AddValueMap(viewing_maps_[0][i], viewing_maps_[0][i]->map_range, temp_title);
    }

    for ( int i = 0; i < image_series_.size(); ++i ){
        delete image_series_[i];
    }
    image_series_.clear();
    QLayout* series_layout = image_series_scroll_widget_->layout();
    if ( series_layout != NULL ) delete series_layout;

    QVBoxLayout* temp_layout = new QVBoxLayout;
    temp_layout->setAlignment(Qt::AlignLeft);

    for ( int i = 0; i < series_titles_.size(); ++i ){
        WrfImageSeriesView* view = new WrfImageSeriesView(i);
        view->SetTitle(series_titles_[i]);
        view->SetMaps(viewing_maps_[i], value_map_titles_[i]);
        temp_layout->addWidget(view);
        image_series_.push_back(view);

        connect(view, SIGNAL(SeriesSelected(int)), this, SLOT(OnImageSeriesSelected(int)));
    }
    image_series_scroll_widget_->setLayout(temp_layout);
    image_series_scroll_widget_->updateGeometry();
    image_series_scroll_area_->setWidget(image_series_scroll_widget_);
    image_series_scroll_area_->updateGeometry();
}

void WrfEnsembleDataView::OnImageSeriesSelected(int index){
    if ( index == -1 ) return;
    image_matrix_view_->Clear();

    for ( int i = 0; i < viewing_maps_[index].size(); ++i ){
        QString temp_title = series_titles_[index] + "    " + value_map_titles_[index][i];
        image_matrix_view_->AddValueMap(viewing_maps_[index][i], viewing_maps_[index][i]->map_range, temp_title);
    }
}

void WrfEnsembleDataView::OnActionManualRegionSelectionTriggered(){
    if ( action_manual_region_selection_->isChecked() )
        image_matrix_view_->SetTrackingPen(true);
    else
        image_matrix_view_->SetTrackingPen(false);
}

void WrfEnsembleDataView::GetSelectedRegionContour(std::vector< QPointF >& contour){
    image_matrix_view_->GetSelectedRegionContour(contour);
}