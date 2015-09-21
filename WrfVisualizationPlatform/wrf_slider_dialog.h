#ifndef WRF_SLIDER_DIALOG_H_
#define WRF_SLIDER_DIALOG_H_

#include <QtGui/QDialog>
#include <vector>

#include "wrf_data_common.h"
#include "ui_slider_dialog.h"

class WrfSliderDialog : public QDialog{
	Q_OBJECT

public:
	WrfSliderDialog();
	~WrfSliderDialog();

	int GetSelectedValue();
	void SetValueRange(int min_value, int max_value);
	void SetValue(int value);

signals:
	void ValueChanged(int);

private:
	Ui::SliderDialog dialog_ui_;
};

#endif