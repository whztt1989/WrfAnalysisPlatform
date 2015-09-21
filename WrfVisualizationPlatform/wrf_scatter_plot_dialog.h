#ifndef WRF_SCATTER_PLOT_DIALOG_H_
#define WRF_SCATTER_PLOT_DIALOG_H_

#include <QtGui/QDialog>
#include "wrf_data_common.h"
#include "ui_scatter_plot_dialog.h"

class WrfScatterPlotDialog : public QDialog {
	Q_OBJECT

public:
	WrfScatterPlotDialog();
	~WrfScatterPlotDialog();

	void GetSelectedParameters(WrfModelType& x_model, WrfElementType& x_element, int& x_ens, float& x_min, float& x_max,
							WrfModelType& y_model, WrfElementType& y_element, int& y_ens, float& y_min, float& y_max);

private:
	Ui::ScatterPlotDialog scatter_dialog_;

	std::vector< WrfModelType > model_types_;
	std::vector< WrfElementType > x_element_types_, y_element_types_;

	void InitDialog();

	private slots:
		void OnXModelChanged(int);
		void OnYModelChanged(int);
};

#endif