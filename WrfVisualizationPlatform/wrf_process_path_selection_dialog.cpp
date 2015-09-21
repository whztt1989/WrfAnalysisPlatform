#include "wrf_process_path_selection_dialog.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>

WrfProcessPathSelectionDialog::WrfProcessPathSelectionDialog(){
    InitWidget();
}

WrfProcessPathSelectionDialog::~WrfProcessPathSelectionDialog(){

}

int WrfProcessPathSelectionDialog::GetSelection(){
    if ( ensemble_check_box_->isChecked() )
        return 0;
    else
        return 1;
}

void WrfProcessPathSelectionDialog::InitWidget(){
    this->setWindowTitle("Select Model for Adjustment");

    QGroupBox* path_group = new QGroupBox(tr("Models"));
    ensemble_check_box_ = new QCheckBox(tr("Ensemble"));
    reforecast_check_box_ = new QCheckBox(tr("Reforecast"));
    QHBoxLayout* check_layout = new QHBoxLayout;
    check_layout->setAlignment(Qt::AlignLeft);
    check_layout->addWidget(ensemble_check_box_);
    check_layout->addWidget(reforecast_check_box_);
    QButtonGroup* model_buttons = new QButtonGroup;
    model_buttons->addButton(ensemble_check_box_);
    model_buttons->addButton(reforecast_check_box_);
    model_buttons->setExclusive(true);
    path_group->setLayout(check_layout);

    QHBoxLayout* button_layout = new QHBoxLayout;
    button_layout->setAlignment(Qt::AlignRight);
    exec_button_ = new QPushButton(tr("Import"));
    cancel_button_ = new QPushButton(tr("Cancel"));
    button_layout->addWidget(cancel_button_);
    button_layout->addWidget(exec_button_);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(path_group);
    main_layout->addLayout(button_layout);
    this->setLayout(main_layout);

    connect(exec_button_, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel_button_, SIGNAL(clicked()), this, SLOT(reject()));
}