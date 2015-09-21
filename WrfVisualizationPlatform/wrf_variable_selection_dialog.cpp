#include "wrf_variable_selection_dialog.h"
#include "wrf_data_manager.h"
#include "wrf_utility.h"

WrfVariableSelectionDialog::WrfVariableSelectionDialog(){
	dialog_ui_.setupUi(this);

	InitWidget();
}

WrfVariableSelectionDialog::~WrfVariableSelectionDialog(){

}

void WrfVariableSelectionDialog::InitWidget(){
	QVBoxLayout* main_layout = new QVBoxLayout;

    WrfDataManager::GetInstance()->GetEnsembleElements(elements_);
    std::vector< WrfElementType > ens_mean_elements;
    WrfDataManager::GetInstance()->GetEnsembleMeanElements(ens_mean_elements);
    for ( int i = 0; i < ens_mean_elements.size(); ++i ) elements_.push_back(ens_mean_elements[i]);
	
	for ( int i = 0; i < elements_.size(); ++i ){
		QCheckBox* check_box = new QCheckBox(enum_element_to_string(elements_[i]));
        check_box->setChecked(true);
		main_layout->addWidget(check_box);

		check_vec_.push_back(check_box);
	}

	dialog_ui_.groupBox->setLayout(main_layout);
}

void WrfVariableSelectionDialog::GetSelectionParas(std::vector< WrfElementType >& variables){
	variables.clear();
	for ( int i = 0; i < check_vec_.size(); ++i )
		if ( check_vec_[i]->isChecked() ){
			variables.push_back(elements_[i]);
		}
}