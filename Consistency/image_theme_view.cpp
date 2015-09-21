#include "image_theme_view.h"
#include "segment_image.h"

ImageThemeView::ImageThemeView(){
    selection_index_ = -1;
}

ImageThemeView::~ImageThemeView(){

}

void ImageThemeView::SetData(std::vector< SegmentImage* > images){
    forecasting_images_.assign(images.begin(), images.end());

    // accuracy

    for ( int i = 0; i < images[0]->region_count.size(); ++i )
        for ( int j = 0; j < images[0]->region_count[i]; ++j ){

        }
}

void ImageThemeView::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        exit(-1);
    }
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void ImageThemeView::resizeGL(int w, int h){
    float width = (float)w / h;
    view_rect_.setLeft(-1 * width);
    view_rect_.setRight(width);
    view_rect_.setTop(1.0);
    view_rect_.setBottom(-1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1 * width, width, -1, 1, 0.1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -0.5);
}

void ImageThemeView::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT);
    if ( forecasting_images_.size() ) return;

    if ( selection_index_ != -1 )
        PaintSelection();
    else 
        PaintOverall();
}

void ImageThemeView::PaintOverall(){
    float border = 10.0 / this->height();

    float width = view_rect_.right() - view_rect_.left() - 2 * border;
    float width_per_image = width / forecasting_images_.size();
    float height_per_image = width_per_image * forecasting_images_[0]->width() / forecasting_images_[0]->height();
    float image_width = width_per_image * 0.8;
    float image_height = image_width * forecasting_images_[0]->width() / forecasting_images_[0]->height();

    float view_height = 500.0f / this->height();
    if ( this->height() < view_height ) view_height = view_rect_.top() - view_rect_.bottom();
    float view_bottom = view_rect_.top() - view_height - border;
    float view_top = view_rect_.top() - border;
    float view_left = view_rect_.left() + border;
    float view_right = view_rect_.left() + border + width;

    // paint the pictures with segment identification
    // 1. the ground truth images
    // 2. the forecasting images
    float temp_left = view_rect_.left() + border;
    for ( int i = 0; i < forecasting_images_.size(); ++i ){
        forecasting_images_[i]->Render(temp_left, temp_left + image_width, view_top - image_height, view_top);
        temp_left += width_per_image;
    }

    // paint the error theme river
    // 1. paint the time line
    glLineWidth(3.0);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(view_rect_.left() + border + width_per_image, view_rect_.bottom() + border, 0);
    glVertex3f(view_rect_.right() - border, view_rect_.bottom() + border, 0);
    glEnd();
    // 2. paint the theme item
    std::vector< std::vector< float > > accu_height;
    accu_height.resize(change_uncertainty_values_.size());
    for ( int i = 0; i < accu_height.size(); ++i ) {
        accu_height[i].resize(change_uncertainty_values_[i].size());
        accu_height[i].assign(accu_height[i].size(), 0);
    }
    for ( int i = 0; i < change_uncertainty_values_.size(); ++i )
        for ( int j = 0; j < change_uncertainty_values_[i].size(); ++i )
            accu_height[i][j] = change_uncertainty_values_[i][j] + accuracy_uncertainty_values_[i][j];

    for ( int i = 0; i < accu_height.size(); ++i )
        for ( int j = 1; j < accu_height[i].size(); ++j )
            accu_height[i][j] = accu_height[i][j] + accu_height[i][j - 1];

    float max_value = -1e10;
    for ( int i = 0; i < accu_height.size(); ++i )
        if ( accu_height[i][accu_height[i].size() - 1] > max_value ) 
            max_value = accu_height[i][accu_height[i].size() - 1];
    
    for ( int i = 0; i < accu_height[0].size(); ++i ){
        temp_left = view_rect_.left() + border + width_per_image;
        glBegin(GL_LINE_STRIP);
        for ( int j = 0; j < accu_height.size(); ++j ){
            float temp_top = view_rect_.bottom() + border + accu_height[j][i] / max_value * (view_top - view_bottom - height_per_image);
            glVertex3f(temp_left, temp_top, 0.0);
            temp_left += width_per_image;
        }
        glEnd();
    }
}

void ImageThemeView::PaintSelection(){
    float border = 10.0 / this->height();

    float width = view_rect_.right() - view_rect_.left() - 2 * border;
    float width_per_image = width / forecasting_images_.size();
    float height_per_image = width_per_image * forecasting_images_[0]->width() / forecasting_images_[0]->height();
    float image_width = width_per_image * 0.8;
    float image_height = image_width * forecasting_images_[0]->width() / forecasting_images_[0]->height();

    float view_height = 500.0f / this->height();
    if ( this->height() < view_height ) view_height = view_rect_.top() - view_rect_.bottom();
    float view_bottom = view_rect_.top() - view_height - border;
    float view_top = view_rect_.top() - border;
    float view_left = view_rect_.left() + border;
    float view_right = view_rect_.left() + border + width;

    // paint the pictures with segment identification
    // 1. the ground truth images
    // 2. the forecasting images
    float temp_left = view_rect_.left() + border;
    for ( int i = 0; i < forecasting_images_.size(); ++i ){
        forecasting_images_[i]->RenderSegment(temp_left, temp_left + image_width, view_top - image_height, view_top, corr_segment_index_[i]);
        temp_left += width_per_image;
    }

    // paint the error theme river
    std::vector< std::vector< float > > accu_height;
    accu_height.resize(change_uncertainty_values_.size());
    for ( int i = 0; i < accu_height.size(); ++i ) {
        accu_height[i].resize(change_uncertainty_values_[i].size());
        accu_height[i].assign(accu_height[i].size(), 0);
    }
    for ( int i = 0; i < change_uncertainty_values_.size(); ++i )
        for ( int j = 0; j < change_uncertainty_values_[i].size(); ++i )
            accu_height[i][j] = change_uncertainty_values_[i][j] + accuracy_uncertainty_values_[i][j];
    for ( int i = 0; i < accu_height.size(); ++i )
        for ( int j = 1; j < accu_height[i].size(); ++j )
            accu_height[i][j] = accu_height[i][j] + accu_height[i][j - 1];
    // 1. paint the time line
    glLineWidth(3.0);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(view_rect_.left() + border + width_per_image, view_rect_.bottom() + border, 0);
    glVertex3f(view_rect_.right() - border, view_rect_.bottom() + border, 0);
    glEnd();
    // 2. paint the theme item
    
}

void ImageThemeView::mousePressEvent(QMouseEvent* event){
    // collision with the ground truth segment

    // collision with the forecasting segment

    // collision with the theme path
}

void ImageThemeView::mouseReleaseEvent(QMouseEvent* event){
   
}

void ImageThemeView::mouseDoubleClickEvent(QMouseEvent* event){

}