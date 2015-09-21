#ifndef WRF_LOAD_REANALYSIS_H_
#define WRF_LOAD_REANALYSIS_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <vector>
#include "wrf_data_common.h"

class WrfLoadReanalysisDialog : public QDialog{
    Q_OBJECT
public:
    WrfLoadReanalysisDialog();
    ~WrfLoadReanalysisDialog();

    void GetSelectionParas(WrfElementType& element, std::string& file_name);

private:
    std::vector< WrfElementType > candidate_elements_;

    std::vector< QCheckBox* > element_check_boxes_;

    QString file_name_;

    QPushButton* load_button_;
    QPushButton* cancel_button_;

    void InitWidget();

    private slots:
        void OnLoadButtonClicked();
};

#endif