#ifndef WRF_ISO_LINE_PLOT_DIALOG_H_
#define WRF_ISO_LINE_PLOT_DIALOG_H_

#include <QtGui/QDialog>
#include <vector>

#include "wrf_data_common.h"
#include "ui_iso_line_dialog.h"

class WrfIsoLinePlotDialog : public QDialog{
	Q_OBJECT

public:
	WrfIsoLinePlotDialog();
	~WrfIsoLinePlotDialog();

	void GetSelectedParameters(int& mode, int& datetime, WrfModelType& model_type, WrfElementType& element_type, int& fhour, float& iso_value, int& ens);

private:
	Ui::IsoDialog dialog_ui_;

	std::vector< WrfModelType > model_types;
	std::vector< WrfElementType > element_types;

	void InitDialog();

	private slots:
		void OnModelSelectionChanged(int);
};

#endif