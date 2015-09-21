#include "wrf_slider_dialog.h"

WrfSliderDialog::WrfSliderDialog(){
	dialog_ui_.setupUi(this);

	connect(dialog_ui_.horizontalSlider, SIGNAL(valueChanged(int)), this, SIGNAL(ValueChanged(int)));
}

WrfSliderDialog::~WrfSliderDialog(){

}

int WrfSliderDialog::GetSelectedValue(){
	return dialog_ui_.horizontalSlider->value();
}

void WrfSliderDialog::SetValue(int value){
	dialog_ui_.horizontalSlider->setValue(value);
}

void WrfSliderDialog::SetValueRange(int min_value, int max_value){
	dialog_ui_.horizontalSlider->setRange(min_value, max_value);
	dialog_ui_.horizontalSlider->setSingleStep(1);
}