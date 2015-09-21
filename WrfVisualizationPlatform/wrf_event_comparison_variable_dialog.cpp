#include "wrf_event_comparison_variable_dialog.h"
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QButtonGroup>
#include "wrf_utility.h"
#include "wrf_data_manager.h"

WrfEventComparisonVariableDialog::WrfEventComparisonVariableDialog(){
    WrfDataManager::GetInstance()->GetEnsembleElements(ens_elements_);
    WrfDataManager::GetInstance()->GetEnsembleMeanElements(ens_mean_elements_);

    InitWidget();
}

WrfEventComparisonVariableDialog::~WrfEventComparisonVariableDialog(){

}

void WrfEventComparisonVariableDialog::InitWidget(){
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

            element_layout->addWidget(check_box);

            ens_check_boxes_.push_back(check_box);
        }
        element_group->setLayout(element_layer_layout);
        main_layout->addWidget(element_group);
    }

    if ( ens_mean_elements_.size() != 0 ){
        QGroupBox* mean_element_group = new QGroupBox(tr("Ensemble Mean Variables"));
        QHBoxLayout* mean_element_layer_layout = new QHBoxLayout;
        for ( int i = 0; i < ens_mean_elements_.size(); ++i ){
            

            QCheckBox* check_box = new QCheckBox(tr(enum_element_to_string(ens_mean_elements_[i])));
            check_box->setFixedWidth(100);
            check_box->setChecked(true);

            mean_element_layer_layout->addWidget(check_box);

            ens_mean_check_boxes_.push_back(check_box);
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
    year_number_spin_box_->setValue(10);
    QLabel* day_label = new QLabel(tr("Day: "));
    day_number_spin_box_ = new QSpinBox;
    day_number_spin_box_->setMaximum(60);
    day_number_spin_box_->setMinimum(10);
    day_number_spin_box_->setSingleStep(1);
    day_number_spin_box_->setValue(35);

    parameter_layout->addWidget(year_label, 0, 0, 1, 1);
    parameter_layout->addWidget(year_number_spin_box_, 0, 1, 1, 1);
    parameter_layout->addWidget(day_label, 0, 2, 1, 1);
    parameter_layout->addWidget(day_number_spin_box_, 0, 3, 1, 1);

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

void WrfEventComparisonVariableDialog::GetParameters(int& year_length, int& day_length, std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements){

        year_length = year_number_spin_box_->value();
        day_length = day_number_spin_box_->value();

        ensemble_elements.clear();
        for ( int i = 0; i < ens_check_boxes_.size(); ++i )
            if ( ens_check_boxes_[i]->isChecked() ){
                ensemble_elements.push_back(ens_elements_[i]);
            }

            for ( int i = 0; i < ens_mean_check_boxes_.size(); ++i )
                if ( ens_mean_check_boxes_[i]->isChecked() ) {
                    ensemble_mean_elements.push_back(ens_mean_elements_[i]);
                }
}

void WrfEventComparisonVariableDialog::OnCancelButtonClicked(){
    this->done(QDialog::Rejected);
}

void WrfEventComparisonVariableDialog::OnSearchButtonClicked(){
    this->done(QDialog::Accepted);
}