#ifndef WRF_SUBREGION_RMS_SCALE_DIALOG_H_
#define WRF_SUBREGION_RMS_SCALE_DIALOG_H_

#include <QtGui/QDialog>
#include "ui_subregion_rms_scale_dialog.h"

class WrfSubregionRmsScaleDialog : public QDialog{
	Q_OBJECT

public:
	WrfSubregionRmsScaleDialog();
	~WrfSubregionRmsScaleDialog();

	float GetScaleValue();

private:
	Ui::SubregionRmsScaleDialog rms_scale_ui_;
};

#endif