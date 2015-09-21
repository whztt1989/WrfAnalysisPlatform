#include "wrf_probabilistic_forecast_dialog.h"
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QFileDialog>
#include "wrf_utility.h"
#include "wrf_data_manager.h"

WrfProbabilisticForecastDialog::WrfProbabilisticForecastDialog(){
    WrfDataManager::GetInstance()->GetObservationElements(candidate_elements_);

	dialog_ui_.setupUi(this);

    InitWidget();
}

WrfProbabilisticForecastDialog::~WrfProbabilisticForecastDialog(){

}

void WrfProbabilisticForecastDialog::GetSelectionParas(ProbabilisticPara& para){
    for ( int i = 0; i < element_check_boxes_.size(); ++i )
        if ( element_check_boxes_[i]->isChecked() ){
            para.element = candidate_elements_[i];
            break;
        }
	para.analog_number = dialog_ui_.analog_number_spinbox->value();
    para.thresh = dialog_ui_.thresh_spinbox->value();
    para.para_name = std::string(enum_element_to_string(para.element)) + std::string(" > ") + std::string(dialog_ui_.thresh_spinbox->text().toLocal8Bit().data());
}

void WrfProbabilisticForecastDialog::InitWidget(){
    this->setWindowTitle("Probabilistic Forecasting");

    QHBoxLayout* element_layout = new QHBoxLayout;
    element_layout->setAlignment(Qt::AlignLeft);
    QButtonGroup* element_buttons = new QButtonGroup;
    element_buttons->setExclusive(true);

    for ( int i = 0; i < candidate_elements_.size(); ++i ){
        QCheckBox* check_box = new QCheckBox(enum_element_to_string(candidate_elements_[i]));
		check_box->setChecked(true);
        element_buttons->addButton(check_box);
        element_layout->addWidget(check_box);
        element_check_boxes_.push_back(check_box);
    }
    dialog_ui_.variable_group_box->setLayout(element_layout);
}