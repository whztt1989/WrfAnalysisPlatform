#include "qcolor_bar_controller.h"
#include <assert.h>

QColorBarController* QColorBarController::controller_ = 0;

QColorBarController* QColorBarController::GetInstance(ColorBarType bar_type){
	if ( controller_ == 0 ){
		controller_ = new QColorBarController;
	}
	controller_->SetBarType(bar_type);
	return controller_;
}

bool QColorBarController::DeleteInstance(){
	if (controller_ != 0){
		delete controller_;
		return true;
	} else {
		return false;
	}
}

QColorBarController::QColorBarController(){
	color_bar_type_ = HEAT_MAP;

	ConstructColorList();
}

QColorBarController::~QColorBarController(){

}

void QColorBarController::SetBarType(ColorBarType bar_type){
	color_bar_type_ = bar_type;

	ConstructColorList();
}

QColor QColorBarController::GetColor(float min_value, float max_value, float current_value){
	if ( current_value > max_value ) current_value = max_value;
	if ( current_value < min_value ) current_value = min_value;

	float value_index = (current_value - min_value) / (max_value - min_value) * 100;

    assert(value_index <= 100 && value_index >= 0);

	std::list< ColorIndex >::iterator iter = color_index_list_.begin();
	iter++;
	std::list< ColorIndex >::iterator pre_iter = color_index_list_.begin();
	while ( iter != color_index_list_.end() && iter->value_index < value_index ) {
		pre_iter = iter;
		iter++;
	}
	QColor current_color;
	if ( iter == color_index_list_.end() ){
		current_color = pre_iter->color;
	} else {
		float rate = (value_index - pre_iter->value_index) / (iter->value_index - pre_iter->value_index);
		if ( rate < 0 ) rate = 0;
		if ( rate > 1 ) rate = 1;
		current_color.setRed(pre_iter->color.red() * (1.0 - rate) + iter->color.red() * rate);
		current_color.setGreen(pre_iter->color.green() * (1.0 - rate) + iter->color.green() * rate);
		current_color.setBlue(pre_iter->color.blue() * (1.0 - rate) + iter->color.blue() * rate);
	}

	return current_color;
}

void QColorBarController::ConstructColorList(){
	color_index_list_.clear();

	if ( color_bar_type_ == HEAT_MAP ){
		ColorIndex index1;
		index1.value_index = 0;
		index1.color.setRedF(0.085);
		index1.color.setGreenF(0.532);
		index1.color.setBlueF(0.201);
		color_index_list_.push_back(index1);

		ColorIndex index3;
		index3.value_index = 50.0;
		index3.color.setRedF(0.865);
		index3.color.setGreenF(0.865);
		index3.color.setBlueF(0.865);
		color_index_list_.push_back(index3);

		ColorIndex index2;
		index2.value_index = 100.0;
		index2.color.setRedF(0.758);
		index2.color.setGreenF(0.214);
		index2.color.setBlueF(0.233);
		color_index_list_.push_back(index2);
	} else if ( color_bar_type_ == METEO_COLOR_MAP ){
		ColorIndex index1;
		index1.value_index = 0;
		index1.color.setRed(128);
		index1.color.setGreen(0);
		index1.color.setBlue(255);
		color_index_list_.push_back(index1);

		ColorIndex index2;
		index2.value_index = 20;
		index2.color.setRed(0);
		index2.color.setGreen(0);
		index2.color.setBlue(255);
		color_index_list_.push_back(index2);

		ColorIndex index3;
		index3.value_index = 40;
		index3.color.setRed(0);
		index3.color.setGreen(255);
		index3.color.setBlue(0);
		color_index_list_.push_back(index3);

		ColorIndex index4;
		index4.value_index = 60;
		index4.color.setRed(255);
		index4.color.setGreen(255);
		index4.color.setBlue(0);
		color_index_list_.push_back(index4);

		ColorIndex index5;
		index5.value_index = 80;
		index5.color.setRed(255);
		index5.color.setGreen(0);
		index5.color.setBlue(0);
		color_index_list_.push_back(index5);

		ColorIndex index6;
		index6.value_index = 100;
		index6.color.setRed(255);
		index6.color.setGreen(0);
		index6.color.setBlue(0);
		color_index_list_.push_back(index5);
	} else if ( color_bar_type_ == GRAY_MAP ){
        ColorIndex index1;
        index1.value_index = 0;
        index1.color.setRedF(0);
        index1.color.setGreenF(0);
        index1.color.setBlueF(0);
        color_index_list_.push_back(index1);

        ColorIndex index2;
        index2.value_index = 100.0;
        index2.color.setRedF(1);
        index2.color.setGreenF(1);
        index2.color.setBlueF(1);
        color_index_list_.push_back(index2);
    }
}

