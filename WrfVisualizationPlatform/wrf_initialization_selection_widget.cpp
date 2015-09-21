#include "wrf_initialization_selection_widget.h"
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDateTimeEdit>
#include "wrf_utility.h"
#include "wrf_data_manager.h"

WrfInitializationSelectionWidget::WrfInitializationSelectionWidget(){
    WrfDataManager::GetInstance()->GetEnsembleElements(ens_elements_);
    WrfDataManager::GetInstance()->GetEnsembleMeanElements(ens_mean_elements_);

    InitWidget();
}

WrfInitializationSelectionWidget::~WrfInitializationSelectionWidget(){

}

void WrfInitializationSelectionWidget::GetSelectionParas(std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements, QDateTime& date_time, int& fcst_hour){
    ensemble_elements.clear();
    for ( int i = 0; i < ens_element_check_boxes_.size(); ++i )
        if ( ens_element_check_boxes_[i]->isChecked() ) ensemble_elements.push_back(ens_elements_[i]);
    ensemble_mean_elements.clear();
    for ( int i = 0; i < ens_mean_check_boxes_.size(); ++i )
        if ( ens_mean_check_boxes_[i]->isChecked() ) ensemble_mean_elements.push_back(ens_mean_elements_[i]);
    
    QDate date = calendar_widget_->selectedDate();
	date_time = QDateTime(date);
    fcst_hour = fcst_hour_spin_box_->value();
}

void WrfInitializationSelectionWidget::InitWidget(){
    QGroupBox* date_group = new QGroupBox(tr("Date"));
    QHBoxLayout* date_layout = new QHBoxLayout;
    calendar_widget_ = new QCalendarWidget;
    calendar_widget_->setSelectedDate(QDate(2013, 12, 30));
	calendar_widget_->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    date_layout->addWidget(calendar_widget_);
    date_group->setLayout(date_layout);

    QGroupBox* fcst_hour_group = new QGroupBox(tr("Forecast End Hour"));
    QHBoxLayout* fcst_hour_layout = new QHBoxLayout;
    QLabel* fcst_hour_label = new QLabel(tr("Forecast Hour"));
    fcst_hour_spin_box_ = new QSpinBox;
    fcst_hour_spin_box_->setSingleStep(1);
    fcst_hour_spin_box_->setValue(24);
    fcst_hour_layout->addWidget(fcst_hour_label);
    fcst_hour_layout->addWidget(fcst_hour_spin_box_);
    fcst_hour_group->setLayout(fcst_hour_layout);

    QGroupBox* ens_element_group = new QGroupBox(tr("Ensemble Members"));
    QVBoxLayout* ens_element_layer_layout = new QVBoxLayout;
    QHBoxLayout* ens_element_layout;
    for ( int i = 0; i < ens_elements_.size(); ++i ){
        if ( i % 4 == 0 ) {
            ens_element_layout = new QHBoxLayout;
            ens_element_layout->setAlignment(Qt::AlignLeft);
            ens_element_layer_layout->addLayout(ens_element_layout);
        }
        QCheckBox* check_box = new QCheckBox(tr(enum_element_to_string(ens_elements_[i])));
        check_box->setChecked(true);
        ens_element_layout->addWidget(check_box);
        ens_element_check_boxes_.push_back(check_box);
    }
    ens_element_group->setLayout(ens_element_layer_layout);

    QGroupBox* ens_mean_group = new QGroupBox(tr("Ensemble Mean"));
    QVBoxLayout* ens_mean_layer_layout = new QVBoxLayout;
    QHBoxLayout* ens_mean_layout;
    for ( int i = 0; i < ens_mean_elements_.size(); ++i ){
        if ( i % 3 == 0 ){
            ens_mean_layout = new QHBoxLayout;
            ens_mean_layout->setAlignment(Qt::AlignLeft);
            ens_mean_layer_layout->addLayout(ens_mean_layout);
        }
        QCheckBox* check_box = new QCheckBox(tr(enum_element_to_string(ens_mean_elements_[i])));
        check_box->setChecked(true);
        ens_mean_layout->addWidget(check_box);
        ens_mean_check_boxes_.push_back(check_box);
    }
    ens_mean_group->setLayout(ens_mean_layer_layout);

    load_button_ = new QPushButton(tr("Import"));
    cancel_button_ = new QPushButton(tr("Cancel"));
    QHBoxLayout* button_layout = new QHBoxLayout;
    button_layout->setAlignment(Qt::AlignRight);
    button_layout->addWidget(cancel_button_);
    button_layout->addWidget(load_button_);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(date_group);
    main_layout->addWidget(fcst_hour_group);
    main_layout->addWidget(ens_element_group);
    main_layout->addWidget(ens_mean_group);
    main_layout->addLayout(button_layout);

    this->setLayout(main_layout);

    connect(load_button_, SIGNAL(clicked()), this, SLOT(OnExecuteButtonClicked()));
    connect(cancel_button_, SIGNAL(clicked()), this, SLOT(OnCancelButtonClicked()));
}

void WrfInitializationSelectionWidget::OnCancelButtonClicked(){
    this->done(QDialog::Rejected);
}

void WrfInitializationSelectionWidget::OnExecuteButtonClicked(){
    this->done(QDialog::Accepted);
}