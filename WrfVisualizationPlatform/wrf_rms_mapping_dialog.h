#ifndef RMS_MAPPING_DIALOG_H_
#define RMS_MAPPING_DIALOG_H_

#include <QtGui/QDialog>
#include "ui_rms_mapping_dialog.h"

class WrfRmsMappingDiloag : public QDialog{
	Q_OBJECT

public:
	WrfRmsMappingDiloag();
	~WrfRmsMappingDiloag();

	void GetSelectionPara(float& min_value, float& max_value);

private:
	Ui::RmsDialog ui_;

	private slots:
		void OnMinValueChanged();
		void OnMaxValueChanged();
};

#endif