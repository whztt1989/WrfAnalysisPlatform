#ifndef WRF_RANK_PARA_DIALOG_H_
#define WRF_RANK_PARA_DIALOG_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QPushButton>
#include <vector>
#include "wrf_data_common.h"

class WrfRankParaDialog : public QDialog
{
    Q_OBJECT

public:
    WrfRankParaDialog();
    ~WrfRankParaDialog();

    void GetParameters(int& analog_num,
        std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements, 
        std::vector< float >& element_weights, std::vector< float >& normalize_values);

private:
    std::vector< WrfElementType > ens_mean_elements_;
    std::vector< WrfElementType > ens_elements_;

    std::vector< QCheckBox* > ens_check_boxes_;
    std::vector< QCheckBox* > ens_mean_check_boxes_;
    std::vector< QDoubleSpinBox* > element_weight_spin_boxes_;
    std::vector< QDoubleSpinBox* > normalize_value_spin_boxes_;
    QSpinBox* analog_number_spin_box_;

    QPushButton* preprocess_button_;
    QPushButton* cancel_button_;

    void InitWidget();

    private slots:
        void OnCancelButtonClicked();
        void OnSearchButtonClicked();
};

#endif