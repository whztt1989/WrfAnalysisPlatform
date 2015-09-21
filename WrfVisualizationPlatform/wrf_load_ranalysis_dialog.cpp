#include "wrf_load_ranalysis_dialog.h"

#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QFileDialog>
#include "wrf_utility.h"

WrfLoadReanalysisDialog::WrfLoadReanalysisDialog(){
    InitWidget();
}

WrfLoadReanalysisDialog::~WrfLoadReanalysisDialog(){

}

void WrfLoadReanalysisDialog::GetSelectionParas(WrfElementType& element, std::string& file_name){
    for ( int i = 0; i < element_check_boxes_.size(); ++i )
        if ( element_check_boxes_[i]->isChecked() ){
            element = candidate_elements_[i];
            break;
        }
    file_name = std::string(file_name_.toLocal8Bit().data());
}

void WrfLoadReanalysisDialog::InitWidget(){
    this->setWindowTitle("Load Reanalysis Data");

    QGroupBox* model_group = new QGroupBox("Models");
    QHBoxLayout* model_layout = new QHBoxLayout;
    model_layout->setAlignment(Qt::AlignLeft);

    // add ensemble mean, ensemble member,
    QButtonGroup* model_buttons = new QButtonGroup;
    QCheckBox* observation_check_box = new QCheckBox("Reanalysis Data");
    observation_check_box->setChecked(true);
    observation_check_box->setEnabled(false);
    model_layout->addWidget(observation_check_box);
    model_group->setLayout(model_layout);
    
    QGroupBox* element_group = new QGroupBox("Variables");
    QHBoxLayout* element_layout = new QHBoxLayout;
    element_layout->setAlignment(Qt::AlignLeft);
    QButtonGroup* element_buttons = new QButtonGroup;
    element_buttons->setExclusive(true);

    for ( int i = 1; i < 6; ++i ){
        QCheckBox* check_box = new QCheckBox(enum_element_to_string(WrfElementType(i)));
        element_buttons->addButton(check_box);
        element_layout->addWidget(check_box);
        candidate_elements_.push_back(WrfElementType(i));
        element_check_boxes_.push_back(check_box);
    }
    element_group->setLayout(element_layout);

    QHBoxLayout* button_layout = new QHBoxLayout;
    button_layout->setAlignment(Qt::AlignRight);
    load_button_ = new QPushButton(tr("Import"));
    cancel_button_ = new QPushButton(tr("Cancel"));
    button_layout->addWidget(cancel_button_);
    button_layout->addWidget(load_button_);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(model_group);
    main_layout->addWidget(element_group);
    main_layout->addLayout(button_layout);
    this->setLayout(main_layout);

    connect(load_button_, SIGNAL(clicked()), this, SLOT(OnLoadButtonClicked()));
    connect(cancel_button_, SIGNAL(clicked()), this, SLOT(reject()));
}

void WrfLoadReanalysisDialog::OnLoadButtonClicked(){
    file_name_ = QFileDialog::getOpenFileName(this, tr("Select Ensemble Data File"), ".", "*.nc");
    if ( file_name_.length() != 0 ){
        this->accept();
    }
}
