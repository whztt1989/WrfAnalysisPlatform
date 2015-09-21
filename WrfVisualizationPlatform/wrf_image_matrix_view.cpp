#include "wrf_image_matrix_view.h"
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QScrollBar>
#include "wrf_data_common.h"
#include "wrf_image_viewer.h"
#include "wrf_data_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_rendering_element_factory.h"
#include "wrf_grid_rms_error_element.h"
#include "wrf_reforecast_manager.h"
#include "wrf_grid_map_element.h"
#include "wrf_image_element.h"

WrfImageMatrixView::WrfImageMatrixView()
    : viewer_size_(4), current_image_index_(0), current_beginning_index_(0){

    this->setMinimumSize(300, 300);
    InitWidget();
}

WrfImageMatrixView::~WrfImageMatrixView(){

}

void WrfImageMatrixView::InitWidget(){
    QGridLayout* grid_layout = new QGridLayout;
    image_viewers_.resize(16);
    for ( int i = 0; i < 16; ++i ) {
        image_viewers_[i] = new WrfImageViewer(i);
        grid_layout->addWidget(image_viewers_[i], i / 4, i % 4, 1, 1);
        connect(image_viewers_[i], SIGNAL(SelectionUpdated(int)), this, SLOT(OnBrushingPathUpdated(int)));
        connect(image_viewers_[i], SIGNAL(ImageSelected(int)), this, SLOT(OnImageSelected(int)));
        connect(image_viewers_[i], SIGNAL(SelectionFinished(int)), this, SLOT(OnBrushingPathFinished(int)));
		connect(image_viewers_[i], SIGNAL(ViewChanged(int)), this, SLOT(OnViewChanged(int)));
    }

	scroll_widget_ = new QScrollArea;
	scroll_widget_->setWidgetResizable(false);
    scroll_widget_->setLayout(grid_layout);

    QHBoxLayout* main_layout = new QHBoxLayout;
	main_layout->setMargin(0);
    main_layout->addWidget(scroll_widget_);

    this->setLayout(main_layout);
}

void WrfImageMatrixView::AddValueMap(WrfValueMap* value_map, MapRange range, QString title){
    value_maps_.push_back(value_map);
    titles_.push_back(title);

    int viewer_index = value_maps_.size() - 1;
    if ( viewer_index >= image_viewers_.size() ){
        QGridLayout* grid_layout = dynamic_cast< QGridLayout* >(scroll_widget_->layout());
        image_viewers_.push_back(new WrfImageViewer(viewer_index));
        grid_layout->addWidget(image_viewers_[viewer_index], viewer_index / 4, viewer_index % 4, 1, 1);
        connect(image_viewers_[viewer_index], SIGNAL(SelectionUpdated(int)), this, SLOT(OnBrushingPathUpdated(int)));
        connect(image_viewers_[viewer_index], SIGNAL(ImageSelected(int)), this, SLOT(OnImageSelected(int)));
        connect(image_viewers_[viewer_index], SIGNAL(SelectionFinished(int)), this, SLOT(OnBrushingPathFinished(int)));
    }

    image_viewers_[viewer_index]->ClearElement();
    image_viewers_[viewer_index]->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(value_map));
    image_viewers_[viewer_index]->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(value_map->element_type));
    image_viewers_[viewer_index]->SetMapRange(range);
    image_viewers_[viewer_index]->SetTitle(title);

	scroll_widget_->updateGeometry();
	scroll_widget_->update();
}

void WrfImageMatrixView::AddRmsMapView(MapRange& range){
    WrfGridRmsErrorElement* rms_element_ = new WrfGridRmsErrorElement;

    std::vector< std::vector< float > > sorted_rms_values;
    WrfReforecastManager::GetInstance()->GetSortedSelectedAggregatedRmsValues(sorted_rms_values);

    std::vector< RmsUnit > rms_units;

    for ( int i = 0; i < range.y_grid_number - 1; ++i )
        for ( int j = 0; j < range.x_grid_number - 1; ++j ){
            RmsUnit unit;
            unit.lon = (j + 0.5) * range.x_grid_space + range.start_x;
            unit.lat = (i + 0.5) * range.y_grid_space + range.start_y;
            unit.values.resize(sorted_rms_values[i * range.x_grid_number + j].size());

            for ( int k = 0; k < unit.values.size(); ++k )
                unit.values[k] = sorted_rms_values[i * range.x_grid_number + j][k];

            rms_units.push_back(unit);
        }

    rms_element_->SetRmsUnits(rms_units);
    rms_element_->SetMaxSize(range.x_grid_space < range.y_grid_space? range.x_grid_space:range.y_grid_space);

    value_maps_.push_back(NULL);
    titles_.push_back("Rms View");

    int viewer_index = value_maps_.size() - 1;
    if ( viewer_index >= image_viewers_.size() ){
        QGridLayout* grid_layout = dynamic_cast< QGridLayout* >(scroll_widget_->layout());
        image_viewers_.push_back(new WrfImageViewer(viewer_index));
        grid_layout->addWidget(image_viewers_[viewer_index], viewer_index / 4, viewer_index % 4, 1, 1);
        connect(image_viewers_[viewer_index], SIGNAL(SelectionUpdated(int)), this, SLOT(OnBrushingPathUpdated(int)));
        connect(image_viewers_[viewer_index], SIGNAL(ImageSelected(int)), this, SLOT(OnImageSelected(int)));
        connect(image_viewers_[viewer_index], SIGNAL(SelectionFinished(int)), this, SLOT(OnBrushingPathFinished(int)));
    }
    image_viewers_[viewer_index]->ClearElement();
	WrfGridMapELement* map_element = new WrfGridMapELement();
	map_element->SetName(std::string("Geographical Map"));
	map_element->SetViewingRange(range);
    image_viewers_[viewer_index]->AddRenderingElement(map_element);
	image_viewers_[viewer_index]->AddRenderingElement(rms_element_);
    image_viewers_[viewer_index]->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(WRF_RMS));
    image_viewers_[viewer_index]->SetMapRange(range);
    image_viewers_[viewer_index]->SetTitle("RMS Difference Glyph Map");
}

void WrfImageMatrixView::AddGeographicaMap(MapRange& range){
	value_maps_.push_back(NULL);

	int viewer_index = value_maps_.size() - 1;
	if ( viewer_index >= image_viewers_.size() ){
		QGridLayout* grid_layout = dynamic_cast< QGridLayout* >(scroll_widget_->layout());
		image_viewers_.push_back(new WrfImageViewer(viewer_index));
		grid_layout->addWidget(image_viewers_[viewer_index], viewer_index / 4, viewer_index % 4, 1, 1);
		connect(image_viewers_[viewer_index], SIGNAL(SelectionUpdated(int)), this, SLOT(OnBrushingPathUpdated(int)));
		connect(image_viewers_[viewer_index], SIGNAL(ImageSelected(int)), this, SLOT(OnImageSelected(int)));
		connect(image_viewers_[viewer_index], SIGNAL(SelectionFinished(int)), this, SLOT(OnBrushingPathFinished(int)));
	}
	image_viewers_[viewer_index]->ClearElement();
	WrfImageElement* background_element_ = new WrfImageElement("./Resources/earth.png");
	background_element_->SetName(std::string("Geographical Background"));
	background_element_->SetViewingRange(range);
	image_viewers_[viewer_index]->AddRenderingElement(background_element_);
	image_viewers_[viewer_index]->SetMapRange(range);
	image_viewers_[viewer_index]->SetTitle("Geographical Map");
}

void WrfImageMatrixView::UpdateRmsElement(){
    
}

void WrfImageMatrixView::UpdateWidgets(int size){
    if ( size != viewer_size_ ){
		QLayout* old_layout = this->layout();
		if ( old_layout != NULL ){
			delete old_layout;
		}

		QGridLayout* grid_layout = new QGridLayout;
		if ( size == 1 && current_image_index_ < image_viewers_.size() ){
			grid_layout->addWidget(image_viewers_[current_image_index_], 0, 0, 1, 1);
			for ( int i = 0; i < image_viewers_.size(); ++i )
				if ( i != current_image_index_ ) image_viewers_[i]->setVisible(false);
				else image_viewers_[i]->setVisible(true);
		} else {
			for ( int i = 0; i < image_viewers_.size(); ++i ) {
				grid_layout->addWidget(image_viewers_[i], i / 4, i % 4, 1, 1);
				image_viewers_[i]->setVisible(true);
			}
		}

		this->setLayout(grid_layout);
        for ( int i = 0; i < image_viewers_.size(); ++i ) image_viewers_[i]->update();
    }
}

void WrfImageMatrixView::SetViewedImageNumber(int num){
    int temp_size = (int)sqrt((double)num);

    this->UpdateWidgets(temp_size);

	viewer_size_ = temp_size;
}

void WrfImageMatrixView::SetTrackingPen(bool enabled){
    for ( int i = 0; i < image_viewers_.size() && i < value_maps_.size(); ++i ){
        image_viewers_[i]->SetTrackingPen(enabled);
    }
}

void WrfImageMatrixView::SetTrackingScaling(bool enabled){

}

void WrfImageMatrixView::OnBrushingPathUpdated(int viewer_index){
    WrfImageViewer* image_viewer = image_viewers_[viewer_index];
    std::vector< QPointF > contour;
    image_viewer->GetSelectionPath(contour);
    for ( int i = 0; i < image_viewers_.size() && i < value_maps_.size(); ++i )
        if ( i != viewer_index ){
            image_viewers_[i]->SetSelectionPath(contour);
        }
}

void WrfImageMatrixView::OnBrushingPathFinished(int viewer_index){

}

void WrfImageMatrixView::GetSelectedRegionContour(std::vector< QPointF >& contour){
    if ( image_viewers_.size() != 0 ){
        image_viewers_[0]->GetSelectionPath(contour);
    }
}

void WrfImageMatrixView::OnImageSelected(int index){
    if ( index != -1 && viewer_size_ != 1 ){
        current_image_index_ = index;
        SetViewedImageNumber(1);
    } else {
        SetViewedImageNumber(16);
    }
}

void WrfImageMatrixView::OnViewChanged(int index){
	if ( index != -1 && viewer_size_ != 1 ){
		WrfImageViewer* image_viewer = image_viewers_[index];
		float rendering_scale, left, right, bottom, top;
		image_viewer->GetViewPara(rendering_scale, left, right, bottom, top);
		for ( int i = 0; i < image_viewers_.size() && i < value_maps_.size(); ++i )
			if ( i != index ){
				image_viewers_[i]->SetViewPara(rendering_scale, left, right, bottom, top);
			}
	}
}

void WrfImageMatrixView::Clear(){
    current_image_index_ = 0;
    titles_.clear();
    value_maps_.clear();

    for ( int i = 0; i < image_viewers_.size(); ++i ) image_viewers_[i]->ClearElement();
}

void WrfImageMatrixView::UpdateWidget(){
    for ( int i = 0; i < image_viewers_.size(); ++i ) image_viewers_[i]->update();
}