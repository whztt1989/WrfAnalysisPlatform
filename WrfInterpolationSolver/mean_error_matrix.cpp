#include "mean_error_matrix.h"

MeanErrorMatrix::MeanErrorMatrix()
    : data_model_(NULL){
    this->setFixedSize(500, 500);
    color_type_ = HEAT_MAP;
}

MeanErrorMatrix::~MeanErrorMatrix(){

}

void MeanErrorMatrix::SetDataModel(ErrorMatrixDataModel* data_model_t){
    if ( data_model_ != NULL ) delete data_model_;
    data_model_ = data_model_t;

    this->updateGL();
}

void MeanErrorMatrix::OnDataModelChanged(){
    this->updateGL();
}

void MeanErrorMatrix::SetColorMapMode(ColorBarType type){
    color_type_ = type;
}

void MeanErrorMatrix::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        exit(0);
    }
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void MeanErrorMatrix::resizeGL(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2);
}

void MeanErrorMatrix::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT);

    if ( data_model_ == NULL ) return;

    int border_size = 10;
    int text_size = 40;
    int block_margin = 20;
    int block_width = (this->width() - 2 * border_size - text_size - (data_model_->axis_names.size() - 1) * block_margin) / data_model_->axis_names.size();

    // paint the axis names
    glColor3f(0.0, 0.0, 0.0);
    for ( int i = 0; i < data_model_->axis_names.size(); ++i ){
        int block_pos = border_size + text_size + (block_width + block_margin) * i;
        QString name = QString::fromLocal8Bit(data_model_->axis_names[i].c_str());
        this->renderText(block_pos, border_size + text_size - 10, name);
        this->renderText(border_size, block_pos + 30, name);
    }

    // paint the matrix
    int accu_index = 0;
    for ( int i = 0; i < data_model_->axis_names.size() - 1; ++i )
        for ( int j = i + 1; j < data_model_->axis_names.size(); ++j ){
            int left = border_size + text_size + (block_width + block_margin) * j;
            int top = border_size + text_size + (block_width + block_margin) * i;
            int w = block_width;
            int h = block_width;
            int row = data_model_->axis_value_size[i];
            int column = data_model_->axis_value_size[j];

            PaintMatrix(left, top, w, h, row, column, &data_model_->values[accu_index]);

            accu_index += row * column;
        }
}

void MeanErrorMatrix::PaintMatrix(int left, int top, int w, int h, int row, int column, float* values){
    float rect_left, rect_bottom, rect_width, rect_height;
    rect_width = (float)w / column;
    rect_height = (float)h / row;
    rect_left = left;
    rect_bottom = this->height() - top - rect_height;

    // paint the color rectangle
    int current_index = 0;
    for ( int i = 0; i < row; ++i ){
        rect_left = left;
        for ( int j = 0; j < column; ++j ){
            QColor color = QColorBarController::GetInstance(color_type_)->GetColor(data_model_->min_value, data_model_->max_value, values[current_index]);
            glColor3f(color.redF(), color.greenF(), color.blueF());
            glBegin(GL_QUADS);
            glVertex3f(rect_left, rect_bottom, 0);
            glVertex3f(rect_left + rect_width, rect_bottom, 0);
            glVertex3f(rect_left + rect_width, rect_bottom + rect_height, 0);
            glVertex3f(rect_left, rect_bottom + rect_height, 0);

            glVertex3f(this->height() - rect_bottom, this->height() - rect_left, 0);
            glVertex3f(this->height() - rect_bottom, this->height() - rect_left - rect_width, 0);
            glVertex3f(this->height() - rect_bottom - rect_height, this->height() - rect_left - rect_width, 0);
            glVertex3f(this->height() - rect_bottom - rect_height, this->height() - rect_left, 0);
            glEnd();

            rect_left += rect_width;
            current_index++;
        }
        rect_bottom -= rect_height;
    }

    // paint the lines
    glColor3f(0.7, 0.7, 0.7);
    rect_bottom = this->height() - top;
    for ( int i = 0; i < row + 1; ++i ){
        glBegin(GL_LINES);
        glVertex3f(left, rect_bottom - i * rect_height, 0);
        glVertex3f(left + w, rect_bottom - i * rect_height, 0);

        glVertex3f(this->height() - rect_bottom + i * rect_height, this->height() - left, 0);
        glVertex3f(this->height() - rect_bottom + i * rect_height, this->height() - left - w, 0);
        glEnd();
    }   
    for ( int i = 0; i < column + 1; ++i ){
        glBegin(GL_LINES);
        glVertex3f(left + i * rect_width, this->height() - top, 0);
        glVertex3f(left + i * rect_width, this->height() - top - h, 0);

        glVertex3f(top, this->height() - left - i * rect_width, 0);
        glVertex3f(top + h, this->height() - left - i * rect_width, 0);
        glEnd();
    }
}