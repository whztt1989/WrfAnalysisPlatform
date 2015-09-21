#ifndef WRF_EVENT_COMPARISON_VARIABLE_DIALOG_H_
#define WRF_EVENT_COMPARISON_VARIABLE_DIALOG_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QPushButton>
#include <vector>
#include "wrf_data_common.h"

class WrfEventComparisonVariableDialog : public QDialog
{
    Q_OBJECT

public:
    WrfEventComparisonVariableDialog();
    ~WrfEventComparisonVariableDialog();

    void GetParameters(int& year_length, int& day_length, std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements);

private:
    std::vector< WrfElementType > ens_mean_elements_;
    std::vector< WrfElementType > ens_elements_;

    std::vector< QCheckBox* > ens_check_boxes_;
    std::vector< QCheckBox* > ens_mean_check_boxes_;
    QSpinBox* year_number_spin_box_;
    QSpinBox* day_number_spin_box_;

    QPushButton* preprocess_button_;
    QPushButton* cancel_button_;

    void InitWidget();

    private slots:
        void OnCancelButtonClicked();
        void OnSearchButtonClicked();
};

#endif