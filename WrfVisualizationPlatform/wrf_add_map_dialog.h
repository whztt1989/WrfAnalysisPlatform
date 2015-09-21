#ifndef WRF_ADD_MAP_DIALOG_H_
#define WRF_ADD_MAP_DIALOG_H_

#include <QtGui/QDialog>
#include "ui_add_map_dialog.h"

class WrfAddMapDialog : public QDialog{
	Q_OBJECT

public:
	WrfAddMapDialog();
	~WrfAddMapDialog();

	void GetSelectionPara(int& mode, float& threshold);

private:
	Ui::AddMapDialog add_map_ui_;
};

#endif