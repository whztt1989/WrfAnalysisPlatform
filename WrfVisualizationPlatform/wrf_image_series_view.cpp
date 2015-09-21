#include "wrf_image_series_view.h"
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QRadioButton>
#include "wrf_image_viewer.h"
#include "wrf_rendering_element_factory.h"

WrfImageSeriesView::WrfImageSeriesView(int index)
    : view_index_(index) {
    InitWidgets();

    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

WrfImageSeriesView::~WrfImageSeriesView(){

}

void WrfImageSeriesView::SetTitle(QString title){
    title_string_ = title;
}

void WrfImageSeriesView::SetMaps(std::vector< WrfGridValueMap* >& maps, std::vector< QString >& titles){
    maps_ = maps;
    titles_ = titles;

    this->UpdateWidgets();
}

void WrfImageSeriesView::InitWidgets(){
    image_viewer_ = new WrfImageViewer();
    image_button_group_ = new QButtonGroup();
    title_label_ = new QLabel;

    connect(image_button_group_, SIGNAL(buttonClicked(int)), this, SLOT(OnButtonClicked(int)));
}

void WrfImageSeriesView::UpdateWidgets(){
    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->setAlignment(Qt::AlignLeft);

    QHBoxLayout* viewer_layout = new QHBoxLayout;
    viewer_layout->setAlignment(Qt::AlignLeading);

    image_viewer_->setFixedSize(150, (float)maps_[0]->map_range.y_grid_number / maps_[0]->map_range.x_grid_number * 150);
    image_viewer_->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(maps_[0]));
    image_viewer_->SetTitle("");
    //image_viewer_->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(maps_[0]->element_type));
    image_viewer_->SetMapRange(maps_[0]->map_range);

    /*QVBoxLayout* button_layout = new QVBoxLayout;
    QWidget* button_widget = new QWidget;
    button_widget->setFixedWidth(150);
    for ( int i = 0; i < maps_.size(); ++i ){
        QRadioButton* radio_button = new QRadioButton(titles_[i]);
        button_layout->addWidget(radio_button);
        image_button_group_->addButton(radio_button, i);
    }
    button_widget->setLayout(button_layout);*/

    viewer_layout->addWidget(image_viewer_);
    //viewer_layout->addWidget(button_widget);

    title_label_->setText(title_string_);

    main_layout->addWidget(title_label_);
    main_layout->addLayout(viewer_layout);

    this->setLayout(main_layout);
}

void WrfImageSeriesView::OnButtonClicked(int index){
    if ( index >= maps_.size() ) return;
    //image_viewer_->SetImage(viewing_images_[index], 0);
}

void WrfImageSeriesView::mouseDoubleClickEvent(QMouseEvent* event){
    emit SeriesSelected(view_index_);
}