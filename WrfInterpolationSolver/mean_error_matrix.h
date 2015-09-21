#ifndef MEAN_ERROR_MATRIX_H_
#define MEAN_ERROR_MATRIX_H_

#include "gl/glew.h"
#include <string>
#include <QtOpenGL/QGLWidget>
#include "qcolor_bar_controller.h"

class ErrorMatrixDataModel
{
public:
    ErrorMatrixDataModel() {
        min_value = 1e10;
        max_value = -1e10;
    }
    ~ErrorMatrixDataModel() {}

    std::vector< std::string > axis_names;
    std::vector< float > axis_min_values;
    std::vector< float > axis_max_values;
    std::vector< float > axis_value_step;
    std::vector< int > axis_value_size;

    std::vector< std::vector< float > > volume_values;

    float min_value, max_value;
    std::vector< float > para_error_values;
    std::vector< float > values;
    std::vector< std::vector< int > > para_value_nodes;
};

class MeanErrorMatrix : public QGLWidget
{
    Q_OBJECT

public:
    MeanErrorMatrix();
    ~MeanErrorMatrix();

    void SetDataModel(ErrorMatrixDataModel* data_model_t);
    void SetColorMapMode(ColorBarType type);

    public slots:
        void OnDataModelChanged();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    ErrorMatrixDataModel* data_model_;
    ColorBarType color_type_;

    void PaintMatrix(int left, int top, int w, int h, int row, int column, float* values);
};

#endif