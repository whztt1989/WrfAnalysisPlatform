#include "wrf_scatter_plot_dialog.h"
#include "wrf_data_manager.h"
#include "wrf_utility.h"

WrfScatterPlotDialog::WrfScatterPlotDialog(){
	scatter_dialog_.setupUi(this);

	this->InitDialog();
}

WrfScatterPlotDialog::~WrfScatterPlotDialog(){

}

void WrfScatterPlotDialog::GetSelectedParameters(WrfModelType& x_model, WrfElementType& x_element, int& x_ens, float& x_min, float& x_max,
	WrfModelType& y_model, WrfElementType& y_element, int& y_ens, float& y_min, float& y_max){

	x_model = model_types_.at(scatter_dialog_.x_model_combo_box->currentIndex());
	x_element = x_element_types_.at(scatter_dialog_.x_element_combo_box->currentIndex());
	x_ens = scatter_dialog_.x_ens_spin_box->value();
	x_min = scatter_dialog_.x_min_spin_box->value();
	x_max = scatter_dialog_.x_max_spin_box->value();

	y_model = model_types_.at(scatter_dialog_.y_model_combo_box->currentIndex());
	y_element = y_element_types_.at(scatter_dialog_.y_element_combo_box->currentIndex());
	y_ens = scatter_dialog_.y_ens_spin_box->value();
	y_min = scatter_dialog_.y_min_spin_box->value();
	y_max = scatter_dialog_.y_max_spin_box->value();
}

void WrfScatterPlotDialog::InitDialog(){
	model_types_.push_back(WRF_NCEP_ENSEMBLES);
	model_types_.push_back(WRF_NCEP_ENSEMBLE_MEAN);

	WrfDataManager::GetInstance()->GetEnsembleElements(x_element_types_);
	WrfDataManager::GetInstance()->GetEnsembleElements(y_element_types_);

	for ( int i = 0; i < model_types_.size(); ++i ){
		scatter_dialog_.x_model_combo_box->addItem(QString::fromLocal8Bit(enum_model_to_string(model_types_[i])));
		scatter_dialog_.y_model_combo_box->addItem(QString::fromLocal8Bit(enum_model_to_string(model_types_[i])));
	}
	for ( int i = 0; i < x_element_types_.size(); ++i )
		scatter_dialog_.x_element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(x_element_types_[i])));
	for ( int i = 0; i < y_element_types_.size(); ++i )
		scatter_dialog_.y_element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(y_element_types_[i])));

	connect(scatter_dialog_.x_model_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnXModelChanged(int)));
	connect(scatter_dialog_.y_model_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnYModelChanged(int)));
}



void WrfScatterPlotDialog::OnXModelChanged(int index){
	if ( index == 0 ){
		WrfDataManager::GetInstance()->GetEnsembleElements(x_element_types_);
	} else {
		WrfDataManager::GetInstance()->GetEnsembleMeanElements(x_element_types_);
	}

	scatter_dialog_.x_element_combo_box->clear();
	for ( int i = 0; i < x_element_types_.size(); ++i )
		scatter_dialog_.x_element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(x_element_types_[i])));
}

void WrfScatterPlotDialog::OnYModelChanged(int index){
	if ( index == 0 ){
		WrfDataManager::GetInstance()->GetEnsembleElements(y_element_types_);
	} else {
		WrfDataManager::GetInstance()->GetEnsembleMeanElements(y_element_types_);
	}

	scatter_dialog_.y_element_combo_box->clear();
	for ( int i = 0; i < y_element_types_.size(); ++i )
		scatter_dialog_.y_element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(y_element_types_[i])));
}