#include "wrf_detail_overview_widget.h"
#include <QtGui/QHBoxLayout>
#include "wrf_data_manager.h"
#include "wrf_image_viewer.h"
#include "wrf_image.h"
#include "wrf_detail_information_view_widget.h"

WrfDetailOverviewWidget::WrfDetailOverviewWidget(WrfDetailInformationViewWidget* parent)
    : parent_(parent), forecasting_result_map_(NULL){
    InitWidget();
}

WrfDetailOverviewWidget::~WrfDetailOverviewWidget(){

}

void WrfDetailOverviewWidget::InitWidget(){
    forecasting_viewer_ = new WrfImageViewer;

    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->addWidget(forecasting_viewer_);

    this->setLayout(main_layout);
}

void WrfDetailOverviewWidget::UpdateWidget(){
    /*std::map< WrfElementType, WrfGridValueMap* >::iterator iter = parent_->current_forecasting_result_->find(parent_->forecast_elements_[parent_->current_element_index_]);
    if ( iter != parent_->current_forecasting_result_->end() ){
        forecasting_viewer_->SetImage(new WrfImage(iter->second), 0);
    } else {
        forecasting_viewer_->SetImage(NULL, 0);
    }*/
}