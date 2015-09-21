#ifndef WRF_VARIABLE_SELECTION_DIALOG_H_
#define WRF_VARIABLE_SELECTION_DIALOG_H_

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QSpinBox>
#include <vector>
#include "wrf_data_common.h"
#include "ui_variable_selection_dialog.h"

class WrfVariableSelectionDialog : public QDialog{
	Q_OBJECT
public:
	WrfVariableSelectionDialog();
	~WrfVariableSelectionDialog();

	void GetSelectionParas(std::vector< WrfElementType >& variables);

private:
	Ui::VariableSelectionDialog dialog_ui_;
	std::vector< WrfElementType > elements_;
	std::vector< QCheckBox* > check_vec_;

	void InitWidget();
};

#endif