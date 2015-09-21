#include "consistency_common.h"
#include "gl/glew.h"
#include <QtGui/QImage>
#include <QtGui/QColor>
#include "qcolor_bar_controller.h"

BasicImage::BasicImage(int w, int h, float* data)
    : width_(w), height_(h), alpha_(0), image_type_(BASIC_IMAGE), rendering_mode_(MAP_LEVLE){
    data_.resize(width_ * height_);
    memcpy(data_.data(), data, sizeof(float) * width_ * height_);

    rgba_values_.resize(w * h * 4);
    for ( int i = 0; i < w * h; ++i ){
        int temp_index = i * 4;
        QColor color = QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(0, 1, data[i]);
        rgba_values_[temp_index] = color.redF();
        rgba_values_[temp_index + 1] = color.greenF();
        rgba_values_[temp_index + 2] = color.blueF();
        rgba_values_[temp_index + 3] = 1.0;
    }

	is_alpha_set_ = false;
}

BasicImage::BasicImage(int w, int h, float* data, float* alpha)
    : width_(w), height_(h), image_type_(BASIC_IMAGE), rendering_mode_(MAP_LEVLE){
    data_.resize(width_ * height_);
    memcpy(data_.data(), data, sizeof(float) * width_ * height_);
    alpha_.resize(width_ * height_);
    memcpy(alpha_.data(), alpha, sizeof(float) * width_ * height_);

    rgba_values_.resize(w * h * 4);
    for ( int i = 0; i < w * h; ++i ){
        int temp_index = i * 4;
        QColor color = QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(0, 1, data[i]);
        rgba_values_[temp_index] = color.redF();
        rgba_values_[temp_index + 1] = color.greenF();
        rgba_values_[temp_index + 2] = color.blueF();
        rgba_values_[temp_index + 3] = alpha_[i];
    }

	is_alpha_set_ = false;
}

BasicImage::~BasicImage(){

}

void BasicImage::set_rendering_mode(RenderingMode mode){
    rendering_mode_ = mode;
}

void BasicImage::set_alpha(float* alpha){
	if ( !is_alpha_set_ ){
		alpha_.resize(width_ * height_);
		memcpy(alpha_.data(), alpha, sizeof(float) * width_ * height_);
		is_alpha_set_ = true;
	} else {
		for ( int i = 0; i <  width_ * height_; ++i ) alpha_[i] = (alpha_[i] + alpha[i]) / 2;
	}

    for ( int i = 0; i < width_ * height_; ++i ){
        int temp_index = i * 4;
        rgba_values_[temp_index + 3] = alpha_[i];
    }
}

void BasicImage::Render(float left, float right, float bottom, float top){

}

bool BasicImage::Save(QString file_name){
    QImage image(width_, height_, QImage::Format_ARGB32);
    for ( int i = 0; i < height_; ++i )
        for ( int j = 0; j < width_; ++j ){
            int temp_index = i * width_ + j;
            QColor color;
            //color.setRgba(qRgba(data_[temp_index] * 255, 0, 0, 255));
			color.setRgba(qRgba(rgba_values_[temp_index * 4] * 255, rgba_values_[temp_index * 4 + 1] * 255, rgba_values_[temp_index * 4 + 2] * 255, rgba_values_[temp_index * 4 + 3] * 255));
            if ( alpha_.size() != 0 ) color.setAlphaF(alpha_[temp_index]);
            image.setPixel(j, i, color.rgba());
        }
    return image.save(file_name);
}

LevelImage::LevelImage(int w, int h, float* data)
    : BasicImage(w, h, data){

}

LevelImage::~LevelImage(){
}

void LevelImage::Render(float left, float right, float bottom, float top){
    GLuint temp_tex;
    
    glGenTextures(1, &temp_tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width(), this->height(), 0, GL_RGBA, GL_FLOAT, rgba_values_.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(left, bottom, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(left, top, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(right, top, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(right, bottom, 0.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &temp_tex);
}

AlphaImage::AlphaImage(BasicImage* image)
    : BasicImage(image->width(), image->height(), image->data()){
	alpha_.resize(image->width() * image->height());
	alpha_.assign(image->width() * image->height(), 1.0);
}

AlphaImage::~AlphaImage(){

}

void AlphaImage::Render(float left, float right, float bottom, float top){
    GLuint temp_tex;

    glGenTextures(1, &temp_tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width(), this->height(), 0, GL_RGBA, GL_FLOAT, rgba_values_.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(left, bottom, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(left, top, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(right, top, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(right, bottom, 0.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &temp_tex);
}

AggregateImage::AggregateImage(int w, int h, float* data)
    : BasicImage(w, h, data){

}

AggregateImage::~AggregateImage(){

}

void AggregateImage::Render(float left, float right, float bottom, float top){
    GLuint temp_tex;

    glGenTextures(1, &temp_tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width(), this->height(), 0, GL_RGBA, GL_FLOAT, rgba_values_.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(left, bottom, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(left, top, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(right, top, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(right, bottom, 0.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &temp_tex);
}

BlendImage::BlendImage(int w, int h, float* data)
    : BasicImage(w, h, data){

}

BlendImage::~BlendImage(){

}

void BlendImage::Render(float left, float right, float bottom, float top){
    GLuint temp_tex;

    glGenTextures(1, &temp_tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width(), this->height(), 0, GL_RGBA, GL_FLOAT, rgba_values_.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(left, bottom, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(left, top, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(right, top, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(right, bottom, 0.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &temp_tex);
}

ClusterImage::ClusterImage(int w, int h, float* data)
    : BasicImage(w, h, data){

}

ClusterImage::~ClusterImage(){

}

void ClusterImage::Render(float left, float right, float bottom, float top){
    GLuint temp_tex;

    glGenTextures(1, &temp_tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width(), this->height(), 0, GL_RGBA, GL_FLOAT, rgba_values_.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(left, bottom, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(left, top, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(right, top, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(right, bottom, 0.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &temp_tex);
}

Node::Node(BasicImage* image)
    : image(image), id(-1){

}

Node::~Node(){
    delete image;
}
