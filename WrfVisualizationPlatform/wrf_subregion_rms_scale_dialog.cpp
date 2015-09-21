#include "wrf_subregion_rms_scale_dialog.h"

WrfSubregionRmsScaleDialog::WrfSubregionRmsScaleDialog(){
	rms_scale_ui_.setupUi(this);
}

WrfSubregionRmsScaleDialog::~WrfSubregionRmsScaleDialog(){

}

float WrfSubregionRmsScaleDialog::GetScaleValue(){
	return rms_scale_ui_.doubleSpinBox->value();
}