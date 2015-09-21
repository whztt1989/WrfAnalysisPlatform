#include "wrf_searching_pre_processing_para_widget.h"
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QButtonGroup>
#include "wrf_utility.h"
#include "wrf_data_manager.h"

WrfSearchingPreProcessingParaWidget::WrfSearchingPreProcessingParaWidget(){
    WrfDataManager::GetInstance()->GetEnsembleElements(ens_elements_);
    WrfDataManager::GetInstance()->GetEnsembleMeanElements(ens_mean_elements_);

    InitWidget();
}

WrfSearchingPreProcessingParaWidget::~WrfSearchingPreProcessingParaWidget(){

}

void WrfSearchingPreProcessingParaWidget::InitWidget(){
    QVBoxLayout* main_layout = new QVBoxLayout;

    if ( ens_elements_.size() != 0 ){
        QGroupBox* element_group = new QGroupBox(tr("Ensemble Variables"));
        QVBoxLayout* element_layer_layout = new QVBoxLayout;
        QHBoxLayout* element_layout;
        for ( int i = 0; i < ens_elements_.size(); ++i ){
            element_layout = new QHBoxLayout;
            element_layout->setAlignment(Qt::AlignLeft);
            element_layer_layout->addLayout(element_layout);

            QCheckBox* check_box = new QCheckBox(tr(enum_element_to_string(ens_elements_[i])));
            check_box->setFixedWidth(100);
            check_box->setChecked(true);

            QLabel* weight_label = new QLabel(tr("Weight: "));
            QDoubleSpinBox* weight_spin_box = new QDoubleSpinBox;
            weight_spin_box->setMaximum(1.0);
            weight_spin_box->setMinimum(0.0);
            weight_spin_box->setSingleStep(0.1);
            if ( i == 0 )
                weight_spin_box->setValue(0.7);
            else 
                weight_spin_box->setValue(0.0);
            weight_spin_box->setFixedWidth(100);

            QLabel* normalize_label = new QLabel(tr("Normalize Value: "));
            normalize_label->setFixedWidth(100);
            QDoubleSpinBox* normalize_spin_box = new QDoubleSpinBox;
            normalize_spin_box->setFixedWidth(100);
            normalize_spin_box->setMaximum(10000.0);
            normalize_spin_box->setMinimum(1.0);
            normalize_spin_box->setSingleStep(1);
            switch ( ens_elements_[i] ){
            case WRF_ACCUMULATED_PRECIPITATION:
            case WRF_PRECIPITABLE_WATER:
                normalize_spin_box->setValue(1.0);
                break;
            case WRF_TEMPERATURE_500HPA:
                normalize_spin_box->setValue(1.0);
                break;
            default:
                normalize_spin_box->setValue(1.0);
                break;
            }

            element_layout->addWidget(check_box);
            element_layout->addWidget(weight_label);
            element_layout->addWidget(weight_spin_box);
            element_layout->addWidget(normalize_label);
            element_layout->addWidget(normalize_spin_box);

            ens_check_boxes_.push_back(check_box);
            element_weight_spin_boxes_.push_back(weight_spin_box);
            normalize_value_spin_boxes_.push_back(normalize_spin_box);
        }
        element_group->setLayout(element_layer_layout);
        main_layout->addWidget(element_group);
    }
    
    if ( ens_mean_elements_.size() != 0 ){
        QGroupBox* mean_element_group = new QGroupBox(tr("Ensemble Mean Variables"));
        QVBoxLayout* mean_element_layer_layout = new QVBoxLayout;
        QHBoxLayout* mean_element_layout;
        for ( int i = 0; i < ens_mean_elements_.size(); ++i ){
            mean_element_layout = new QHBoxLayout;
            mean_element_layout->setAlignment(Qt::AlignLeft);
            mean_element_layer_layout->addLayout(mean_element_layout);

            QCheckBox* check_box = new QCheckBox(tr(enum_element_to_string(ens_mean_elements_[i])));
            check_box->setFixedWidth(100);
            check_box->setChecked(true);

            QLabel* weight_label = new QLabel(tr("Weight: "));
            QDoubleSpinBox* weight_spin_box = new QDoubleSpinBox;
            weight_spin_box->setMaximum(1.0);
            weight_spin_box->setMinimum(0.0);
            weight_spin_box->setSingleStep(0.1);
            if ( i == 0 )
                weight_spin_box->setValue(0.3);
            else 
                weight_spin_box->setValue(0.0);
            weight_spin_box->setFixedWidth(100);

            QLabel* normalize_label = new QLabel(tr("Normalize Value: "));
            normalize_label->setFixedWidth(100);
            QDoubleSpinBox* normalize_spin_box = new QDoubleSpinBox;
            normalize_spin_box->setFixedWidth(100);
            normalize_spin_box->setMaximum(10000.0);
            normalize_spin_box->setMinimum(1.0);
            normalize_spin_box->setSingleStep(1);
            switch ( ens_mean_elements_[i] ){
            case WRF_ACCUMULATED_PRECIPITATION:
            case WRF_PRECIPITABLE_WATER:
                normalize_spin_box->setValue(1.0);
                break;
            case WRF_TEMPERATURE_500HPA:
                normalize_spin_box->setValue(1.0);
                break;
            default:
                normalize_spin_box->setValue(1.0);
                break;
            }

            mean_element_layout->addWidget(check_box);
            mean_element_layout->addWidget(weight_label);
            mean_element_layout->addWidget(weight_spin_box);
            mean_element_layout->addWidget(normalize_label);
            mean_element_layout->addWidget(normalize_spin_box);

            ens_mean_check_boxes_.push_back(check_box);
            element_weight_spin_boxes_.push_back(weight_spin_box);
            normalize_value_spin_boxes_.push_back(normalize_spin_box);
        }
        mean_element_group->setLayout(mean_element_layer_layout);
        main_layout->addWidget(mean_element_group);
    }
    
    QGroupBox* para_group = new QGroupBox(tr("Searching Parameters"));
    QGridLayout* parameter_layout = new QGridLayout;

    QLabel* year_label = new QLabel(tr("Year: "));
    year_number_spin_box_ = new QSpinBox;
    year_number_spin_box_->setMaximum(40);
    year_number_spin_box_->setMinimum(1);
    year_number_spin_box_->setSingleStep(1);
    year_number_spin_box_->setValue(12);
    QLabel* day_label = new QLabel(tr("Day: "));
    day_number_spin_box_ = new QSpinBox;
    day_number_spin_box_->setMaximum(60);
    day_number_spin_box_->setMinimum(10);
    day_number_spin_box_->setSingleStep(1);
    day_number_spin_box_->setValue(35);

    QLabel* grid_size_label = new QLabel(tr("Retrieval Grid Size: "));
    grid_size_spin_box_ = new QSpinBox;
    grid_size_spin_box_->setMaximum(6);
    grid_size_spin_box_->setMinimum(2);
    grid_size_spin_box_->setSingleStep(1);
    grid_size_spin_box_->setValue(4);
    QLabel* analog_num_label = new QLabel(tr("Analog Number: "));
    analog_number_spin_box_ = new QSpinBox;
    analog_number_spin_box_->setMinimum(10);
    analog_number_spin_box_->setMaximum(100);
    analog_number_spin_box_->setSingleStep(1);
    analog_number_spin_box_->setValue(50);

    parameter_layout->addWidget(year_label, 0, 0, 1, 1);
    parameter_layout->addWidget(year_number_spin_box_, 0, 1, 1, 1);
    parameter_layout->addWidget(day_label, 0, 2, 1, 1);
    parameter_layout->addWidget(day_number_spin_box_, 0, 3, 1, 1);
    parameter_layout->addWidget(grid_size_label, 1, 0, 1, 1);
    parameter_layout->addWidget(grid_size_spin_box_, 1, 1, 1, 1);
    parameter_layout->addWidget(analog_num_label, 1, 2, 1, 1);
    parameter_layout->addWidget(analog_number_spin_box_, 1, 3, 1, 1);

    para_group->setLayout(parameter_layout);
    
    preprocess_button_ = new QPushButton(tr("Process"));
    cancel_button_ = new QPushButton(tr("Cancel"));
    QHBoxLayout* button_layout = new QHBoxLayout;
    button_layout->setAlignment(Qt::AlignRight);
    button_layout->addWidget(cancel_button_);
    button_layout->addWidget(preprocess_button_);

    main_layout->addWidget(para_group);
    main_layout->addLayout(button_layout);

    this->setLayout(main_layout);

    connect(preprocess_button_, SIGNAL(clicked()), this, SLOT(OnSearchButtonClicked()));
    connect(cancel_button_, SIGNAL(clicked()), this, SLOT(OnCancelButtonClicked()));
}

void WrfSearchingPreProcessingParaWidget::GetParameters(int& year_length, int& day_length, int& grid_size, int& analog_num,
    std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements, 
    std::vector< float >& element_weights, std::vector< float >& normalize_values){

    year_length = year_number_spin_box_->value();
    day_length = day_number_spin_box_->value();
    grid_size = grid_size_spin_box_->value();
    analog_num = analog_number_spin_box_->value();

    normalize_values.clear();
    element_weights.clear();

    ensemble_elements.clear();
    for ( int i = 0; i < ens_check_boxes_.size(); ++i )
        if ( ens_check_boxes_[i]->isChecked() ){
            ensemble_elements.push_back(ens_elements_[i]);
            element_weights.push_back(element_weight_spin_boxes_[i]->value());
            normalize_values.push_back(normalize_value_spin_boxes_[i]->value());
        }

    for ( int i = 0; i < ens_mean_check_boxes_.size(); ++i )
        if ( ens_mean_check_boxes_[i]->isChecked() ) {
            ensemble_mean_elements.push_back(ens_mean_elements_[i]);
            element_weights.push_back(element_weight_spin_boxes_[i + ens_elements_.size()]->value());
            normalize_values.push_back(normalize_value_spin_boxes_[i + ens_elements_.size()]->value());
        }
}

void WrfSearchingPreProcessingParaWidget::OnCancelButtonClicked(){
    this->done(QDialog::Rejected);
}

void WrfSearchingPreProcessingParaWidget::OnSearchButtonClicked(){
    this->done(QDialog::Accepted);
}