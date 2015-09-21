#include "wrf_add_map_dialog.h"

WrfAddMapDialog::WrfAddMapDialog(){
	add_map_ui_.setupUi(this);
}

WrfAddMapDialog::~WrfAddMapDialog(){

}

void WrfAddMapDialog::GetSelectionPara(int& mode, float& threshold){
	mode = add_map_ui_.map_type_combobox->currentIndex();
	threshold = add_map_ui_.threshold_spin_box->value();
}