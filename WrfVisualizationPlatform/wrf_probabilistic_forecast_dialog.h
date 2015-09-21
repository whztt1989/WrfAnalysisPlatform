#ifndef WRF_PROBABILISTIC_FORECAST_DIALOG_H_
#define WRF_PROBABILISTIC_FORECAST_DIALOG_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QDoubleSpinBox>
#include <vector>
#include "wrf_data_common.h"
#include "ui_probabilistic_forecast_dialog.h"

class WrfProbabilisticForecastDialog : public QDialog{
    Q_OBJECT
public:
    WrfProbabilisticForecastDialog();
    ~WrfProbabilisticForecastDialog(); 

    void GetSelectionParas(ProbabilisticPara& para);

private:
	Ui::ProbabilisticForecastDialog dialog_ui_;

    std::vector< WrfElementType > candidate_elements_;

    std::vector< QCheckBox* > element_check_boxes_;

    QString file_name_;

    void InitWidget();
};

#endif