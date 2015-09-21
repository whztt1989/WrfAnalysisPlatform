#include "wrf_iso_line_plot_dialog.h"
#include "wrf_utility.h"
#include "wrf_data_manager.h"
#include "wrf_ensemble_manager.h"

WrfIsoLinePlotDialog::WrfIsoLinePlotDialog(){
	dialog_ui_.setupUi(this);

	InitDialog();
}

WrfIsoLinePlotDialog::~WrfIsoLinePlotDialog(){

}

void WrfIsoLinePlotDialog::GetSelectedParameters(int& mode, int& datetime, WrfModelType& model_type, WrfElementType& element_type, int& fhour, float& iso_value, int& ens){
	mode = dialog_ui_.line_type_combo_box->currentIndex();
	QDateTime temp_date = dialog_ui_.date_edit->dateTime();
	datetime = temp_date.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
	model_type = model_types.at(dialog_ui_.model_combo_box->currentIndex());
	element_type = element_types.at(dialog_ui_.element_combo_box->currentIndex());
	fhour = dialog_ui_.fhour_spin_box->value();
	iso_value = dialog_ui_.iso_spin_box->value();
	ens = dialog_ui_.ens_number_spin_box->value();
}

void WrfIsoLinePlotDialog::InitDialog(){
	model_types.push_back(WRF_NCEP_ENSEMBLES);
	model_types.push_back(WRF_NCEP_ENSEMBLE_MEAN);
	WrfDataManager::GetInstance()->GetEnsembleElements(element_types);

	for ( int i = 0; i < model_types.size(); ++i )
		dialog_ui_.model_combo_box->addItem(QString::fromLocal8Bit(enum_model_to_string(model_types[i])));
	for ( int i = 0; i < element_types.size(); ++i )
		dialog_ui_.element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(element_types[i])));

	dialog_ui_.date_edit->setDate(WrfEnsembleManager::GetInstance()->CurrrentDate().date());
	dialog_ui_.fhour_spin_box->setValue(WrfEnsembleManager::GetInstance()->FcstHour());

	connect(dialog_ui_.model_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnModelSelectionChanged(int)));
}

void WrfIsoLinePlotDialog::OnModelSelectionChanged(int index){
	if ( index == 0 ){
		WrfDataManager::GetInstance()->GetEnsembleElements(element_types);

		dialog_ui_.element_combo_box->clear();
		for ( int i = 0; i < element_types.size(); ++i )
			dialog_ui_.element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(element_types[i])));
	} else {
		WrfDataManager::GetInstance()->GetEnsembleMeanElements(element_types);

		dialog_ui_.element_combo_box->clear();
		for ( int i = 0; i < element_types.size(); ++i )
			dialog_ui_.element_combo_box->addItem(QString::fromLocal8Bit(enum_element_to_string(element_types[i])));
	}
}