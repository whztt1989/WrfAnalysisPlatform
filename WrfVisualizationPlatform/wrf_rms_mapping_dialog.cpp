#include "wrf_rms_mapping_dialog.h"

WrfRmsMappingDiloag::WrfRmsMappingDiloag(){
	ui_.setupUi(this);

	connect(ui_.min_value_slider, SIGNAL(valueChanged(int)), this, SLOT(OnMinValueChanged()));
	connect(ui_.max_value_slider, SIGNAL(valueChanged(int)), this, SLOT(OnMaxValueChanged()));
}

WrfRmsMappingDiloag::~WrfRmsMappingDiloag(){

}

void WrfRmsMappingDiloag::GetSelectionPara(float& min_value, float& max_value){
	min_value = ui_.min_value_slider->value();
	max_value = ui_.max_value_slider->value();
}

void WrfRmsMappingDiloag::OnMinValueChanged(){
	ui_.min_value_label->setText(QString("%0").arg(ui_.min_value_slider->value()));
}

void WrfRmsMappingDiloag::OnMaxValueChanged(){
	ui_.max_value_label->setText(QString("%0").arg(ui_.max_value_slider->value()));
}