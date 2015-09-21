#ifndef WRF_PROCESS_PATH_SELECTION_DIALOG_H_
#define WRF_PROCESS_PATH_SELECTION_DIALOG_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

class WrfProcessPathSelectionDialog : public QDialog{
public:
    WrfProcessPathSelectionDialog();
    ~WrfProcessPathSelectionDialog();

    int GetSelection();

private:
    QCheckBox* ensemble_check_box_;
    QCheckBox* reforecast_check_box_;

    QPushButton* exec_button_;
    QPushButton* cancel_button_;
    void InitWidget();
};

#endif