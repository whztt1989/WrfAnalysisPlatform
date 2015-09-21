#ifndef WRF_LOAD_ENSEMBLE_DIALOG_H_
#define WRF_LOAD_ENSEMBLE_DIALOG_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <vector>
#include "wrf_data_common.h"

class WrfLoadEnsembleDialog : public QDialog{
    Q_OBJECT
public:
    WrfLoadEnsembleDialog();
    ~WrfLoadEnsembleDialog();

    void GetSelectionParas(WrfModelType& model, WrfElementType& element, std::string& file_name);

private:
    std::vector< WrfModelType > candidate_models_;
    std::vector< WrfElementType > candidate_elements_;

    std::vector< QCheckBox* > model_check_boxes_;
    std::vector< QCheckBox* > element_check_boxes_;

    QString file_name_;

    QPushButton* load_button_;
    QPushButton* cancel_button_;

    void InitWidget();

    private slots:
        void OnLoadButtonClicked();
};

#endif