#include "stacked_image_view.h"

StackedImageView::StackedImageView(QWidget* parent /* = 0 */){
    this->setMinimumSize(200, 200);
}

StackedImageView::~StackedImageView(){

}

void StackedImageView::SetData(std::vector< BasicImage* >& images){
	view_images_.assign(images.begin(), images.end());

	this->updateGL();
}

void StackedImageView::initializeGL(){

}

void StackedImageView::resizeGL(int w, int h){

}

void StackedImageView::paintGL(){

}