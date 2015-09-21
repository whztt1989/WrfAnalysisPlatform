#include "wrf_parameter_widget.h"
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include "wrf_data_manager.h"

WrfParameterWidget::WrfParameterWidget(){
	InitWidget();
}

WrfParameterWidget::~WrfParameterWidget(){

}

void WrfParameterWidget::InitWidget(){
	QLabel* his_length_label = new QLabel(QString(tr("History Length:")));
	his_length_label->setFixedWidth(100);
	his_length_view_label = new QLabel(QString(tr("")));
	his_length_view_label->setFixedWidth(15);
	QSlider* his_length_slider = new QSlider(Qt::Horizontal);
	his_length_slider->setRange(2, 7);
	his_length_slider->setValue(3);
	his_length_slider->setSingleStep(1);
	his_length_slider->setTracking(false);
	QHBoxLayout* layout1 = new QHBoxLayout;
	layout1->addWidget(his_length_label);
	layout1->addWidget(his_length_view_label);
	layout1->addWidget(his_length_slider);
	connect(his_length_slider, SIGNAL(valueChanged(int)), this, SLOT(OnHistoryLengthChanged(int)));

	QLabel* wind_variance_label = new QLabel(QString(tr("Wind Variance")));
	wind_variance_label->setFixedWidth(100);
	QSlider* variance_slider = new QSlider(Qt::Horizontal);
	variance_slider->setRange(0, 100);
	variance_slider->setSingleStep(1);
	variance_slider->setTracking(true);
	QHBoxLayout* layout_wind = new QHBoxLayout;
	layout_wind->addWidget(wind_variance_label);
	layout_wind->addWidget(variance_slider);
	connect(variance_slider, SIGNAL(valueChanged(int)), this, SLOT(OnWindVarianceThreshChanged(int)));

	QHBoxLayout* layout5 = new QHBoxLayout;
	layout5->addLayout(layout1);
	layout5->addLayout(layout_wind);

	QHBoxLayout* layout6 = new QHBoxLayout;
	QLabel* history_weight_label = new QLabel("History Weight");
	history_weight_label->setFixedWidth(100);
	QSlider* history_weight_slider = new QSlider(Qt::Horizontal);
	history_weight_slider->setRange(0, 100);
	history_weight_slider->setValue(50);
	history_weight_slider->setSingleStep(1);
	history_weight_slider->setTracking(false);
	layout6->addWidget(history_weight_label);
	layout6->addWidget(history_weight_slider);
	connect(history_weight_slider, SIGNAL(valueChanged(int)), this, SLOT(OnHistoricalWeightChanged(int)));


	QHBoxLayout* layout7 = new QHBoxLayout;
	QLabel* current_weight_label = new QLabel("Change Weight:");
	current_weight_label->setFixedWidth(100);
	QSlider* current_weight_slider = new QSlider(Qt::Horizontal);
	current_weight_slider->setRange(0, 100);
	current_weight_slider->setValue(50);
	current_weight_slider->setSingleStep(1);
	current_weight_slider->setTracking(false);
	layout7->addWidget(current_weight_label);
	layout7->addWidget(current_weight_slider);
	connect(current_weight_slider, SIGNAL(valueChanged(int)), this, SLOT(OnCurrentWeightChanged(int)));

	QHBoxLayout* layout8 = new QHBoxLayout;
	layout8->addLayout(layout6);
	layout8->addLayout(layout7);

	QLabel* start_long = new QLabel(QString(tr("Start Longitude:")));
	start_long->setFixedWidth(100);
	QLineEdit* start_long_edit = new QLineEdit;
	start_long_edit->setText(QString("60"));
	QLabel* end_long = new QLabel(QString(tr("End Longitude:")));
	end_long->setFixedWidth(100);
	QLineEdit* end_long_edit = new QLineEdit;
	end_long_edit->setText(QString("150"));
	QHBoxLayout* layout2 = new QHBoxLayout;
	layout2->addWidget(start_long);
	layout2->addWidget(start_long_edit);
	layout2->addWidget(end_long);
	layout2->addWidget(end_long_edit);

	QLabel* start_lati = new QLabel(QString(tr("Start Latitude:")));
	start_lati->setFixedWidth(100);
	QLineEdit* start_lati_edit = new QLineEdit;
	start_lati_edit->setText(QString("15"));
	QLabel* end_lati = new QLabel(QString(tr("End Latitude:")));
	end_lati->setFixedWidth(100);
	QLineEdit* end_lati_edit = new QLineEdit;
	end_lati_edit->setText(QString("60"));
	QHBoxLayout* layout3 = new QHBoxLayout;
	layout3->addWidget(start_lati);
	layout3->addWidget(start_lati_edit);
	layout3->addWidget(end_lati);
	layout3->addWidget(end_lati_edit);

	QLabel* grid_size_label = new QLabel(QString(tr("Grid Size:")));
	grid_size_label->setFixedWidth(100);
	QLineEdit* grid_size_edit = new QLineEdit;
	grid_size_edit->setFixedWidth(100);
	grid_size_edit->setText("0.25");
	QHBoxLayout* layout4 = new QHBoxLayout;
	layout4->addWidget(grid_size_label);
	layout4->addWidget(grid_size_edit);
	layout4->setAlignment(Qt::AlignLeft);

	QVBoxLayout* widget_layout = new QVBoxLayout;
	widget_layout->addLayout(layout5);
	widget_layout->addLayout(layout8);
	widget_layout->addLayout(layout2);
	widget_layout->addLayout(layout3);
	widget_layout->addLayout(layout4);

	this->setLayout(widget_layout);
}

void WrfParameterWidget::OnWindVarianceThreshChanged(int value){
	WrfDataManager::GetInstance()->set_wind_var_threshold(value / 100.0);

	emit WindVarThresholdChanged();
}

void WrfParameterWidget::OnHistoricalWeightChanged(int weight){
	WrfDataManager::GetInstance()->SetHistoryWeight(weight / 100.0);

	emit BiasWeightChanged();
}

void WrfParameterWidget::OnCurrentWeightChanged(int weight){
	WrfDataManager::GetInstance()->SetCurrentWeight(weight / 100.0);

	emit BiasWeightChanged();
}

void WrfParameterWidget::OnHistoryLengthChanged(int length){
	his_length_view_label->setText(QString("%0").arg(length));

	emit HistoryLengthChanged(length);
}