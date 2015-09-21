#include "volume_renderer.h"
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QImage>
#include "transfer_function_1d_widget.h"
#include "volume_render_widget.h"

VolumeRenderer::VolumeRenderer(){
    InitializeWidget();
}

VolumeRenderer::~VolumeRenderer(){

}

void VolumeRenderer::SetData(int* sizes_t, float* spacings_t, GLenum data_format_t, void* data_t){
    transfer_function_1d_widget_->SetData(sizes_t[0] * sizes_t[1] * sizes_t[2], (float*)data_t);
    OnTransferFunctionChanged();

    win_center_slider->setRange(transfer_function_1d_widget_->min_value(), transfer_function_1d_widget_->max_value());
    win_center_slider->setValue((transfer_function_1d_widget_->min_value() + transfer_function_1d_widget_->max_value()) / 2);

    win_width_slider->setRange(1, transfer_function_1d_widget_->max_value() - transfer_function_1d_widget_->min_value());
    win_width_slider->setValue(transfer_function_1d_widget_->max_value() - transfer_function_1d_widget_->min_value());

    QImage image("china.bmp");
    std::vector< float > pixel_data;
    pixel_data.resize(image.width() * image.height() * 4, 1.0);
    for ( int i = 0; i < image.height(); ++i )
        for ( int j = 0; j < image.width(); ++j ){
            QColor color = image.pixel(j, image.height() - 1 - i);
            int index = (i * image.width() + j) * 4;
            pixel_data[index] = color.redF();
            pixel_data[index + 1] = color.greenF();
            pixel_data[index + 2] = color.blueF();
            pixel_data[index + 3] = color.alphaF();
        }

    volume_render_widget_->SetData(sizes_t, spacings_t, data_format_t, data_t);
    volume_render_widget_->SetBackgroundData(image.width(), image.height(), pixel_data);
}

void VolumeRenderer::InitializeWidget(){
    transfer_function_1d_widget_ = new TransferFunction1DWidget;
    volume_render_widget_ = new VolumeRenderWidget;

    QWidget* function_widget = new QWidget;

    QWidget* parameter_widget = new QWidget;
    QLabel* win_center_label = new QLabel(tr("Window Center: "));
    QLabel* win_width_label = new QLabel(tr("Window Width: "));
    QLabel* step_size_label = new QLabel(tr("Step Size: "));
    win_center_slider = new QSlider(Qt::Horizontal);
    win_center_slider->setRange(0, 10);
    win_width_slider = new QSlider(Qt::Horizontal);
    win_width_slider->setRange(0, 10);
    QSlider* step_size_slider = new QSlider(Qt::Horizontal);
    step_size_slider->setRange(0, 10);
    QPushButton* save_button = new QPushButton(tr("Save TF"));
    QPushButton* load_button = new QPushButton(tr("Load TF"));
    QPushButton* optimize_button = new QPushButton(tr("Optimize"));
    QGridLayout* parameter_layout = new QGridLayout;
    parameter_layout->addWidget(win_center_label, 0, 0, 1, 1);
    parameter_layout->addWidget(win_center_slider, 0, 1, 1, 5);
    parameter_layout->addWidget(win_width_label, 1, 0, 1, 1);
    parameter_layout->addWidget(win_width_slider, 1, 1, 1, 5);
    parameter_layout->addWidget(step_size_label, 2, 0, 1, 1);
    parameter_layout->addWidget(step_size_slider, 2, 1, 1, 5);
    parameter_layout->addWidget(save_button, 3, 0, 1, 2);
    parameter_layout->addWidget(load_button, 3, 2, 1, 2);
    parameter_layout->addWidget(optimize_button, 3, 4, 1, 2);
    parameter_widget->setLayout(parameter_layout);
    parameter_widget->setFixedWidth(400);

    QHBoxLayout* function_layout = new QHBoxLayout;
    function_layout->addWidget(parameter_widget);
    function_layout->addWidget(transfer_function_1d_widget_);
    function_widget->setLayout(function_layout);
    function_widget->setFixedHeight(250);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(volume_render_widget_);
    main_layout->addWidget(function_widget);

    this->setLayout(main_layout);

    connect(transfer_function_1d_widget_, SIGNAL(ValueChanged()), this, SLOT(OnTransferFunctionChanged()));
    connect(optimize_button, SIGNAL(clicked()), this, SLOT(OnOptimizationTriggered()));
    connect(win_center_slider, SIGNAL(valueChanged(int)), this, SLOT(OnWinCenterChanged(int)));
    connect(win_width_slider, SIGNAL(valueChanged(int)), this, SLOT(OnWinWidthChanged(int)));
}

void VolumeRenderer::OnTransferFunctionChanged(){
    std::vector< float > tf_values;
    transfer_function_1d_widget_->GetTfColorAndAlpha(256, tf_values);
    volume_render_widget_->SetTransferFunction(tf_values.data(), 256);
}

void VolumeRenderer::OnWinCenterChanged(int value){
    float win_center = win_center_slider->value();
    float win_width = win_width_slider->value();

    transfer_function_1d_widget_->SetViewWindow(win_center, win_width);
    volume_render_widget_->SetViewWindow(win_center, win_width);
}

void VolumeRenderer::OnWinWidthChanged(int value){
    float win_center = win_center_slider->value();
    float win_width = win_width_slider->value();

    transfer_function_1d_widget_->SetViewWindow(win_center, win_width);
    volume_render_widget_->SetViewWindow(win_center, win_width);
}

void VolumeRenderer::OnOptimizationTriggered(){
    volume_render_widget_->OptimizeResult();
}