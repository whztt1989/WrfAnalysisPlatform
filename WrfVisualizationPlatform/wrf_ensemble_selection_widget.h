#ifndef WRF_ENSEMBLE_SELECTION_WIDGET_H_
#define WRF_ENSEMBLE_SELECTION_WIDGET_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QSpinBox>
#include <vector>
#include "wrf_data_common.h"

class WrfEnsembleSelectionWidget : public QDialog{
	Q_OBJECT
public:
	WrfEnsembleSelectionWidget();
	~WrfEnsembleSelectionWidget();

	void GetSelectionParas(std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements);

private:
	std::vector< WrfElementType > ens_elements_;
	std::vector< WrfElementType > ens_mean_elements_;

	std::vector< QCheckBox* > ens_mean_check_boxes_;
	std::vector< QCheckBox* > ens_element_check_boxes_;

	QPushButton* load_button_;
	QPushButton* cancel_button_;

	void InitWidget();

	private slots:
		void OnCancelButtonClicked();
		void OnExecuteButtonClicked();
};

#endif