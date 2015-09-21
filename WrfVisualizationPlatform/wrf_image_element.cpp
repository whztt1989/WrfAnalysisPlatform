#include "wrf_image_element.h"

WrfImageElement::WrfImageElement(const char* file_name){
    QImage image(file_name);

    image_width_ = image.width();
    image_height_ = image.height();

    rgba_values_.resize(image_width_ * image_height_ * 4);
    int temp_index = 0;
    for ( int i = 0; i < image_height_; ++i )
        for ( int j = 0; j < image_width_; ++j ){
            QRgb rgb = image.pixel(j, i);
            rgba_values_[temp_index * 4] = (rgb >> 16) & 0xFF;
            rgba_values_[temp_index * 4 + 1] = (rgb >> 8) & 0xFF;
            rgba_values_[temp_index * 4 + 2] = (rgb >> 0) & 0xFF;
            rgba_values_[temp_index * 4 + 3] = (rgb >> 24) & 0xFF;
            //rgba_values_[temp_index * 4 + 3] = 128;

            temp_index++;
        }
	map_range_.start_x = 0;
	map_range_.end_x = 360;
	map_range_.start_y = -90;
	map_range_.end_y = 90;
}

WrfImageElement::~WrfImageElement(){

}

void WrfImageElement::Render(int left, int right, int bottom, int top){
	if ( !is_visible_ ) return;

	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	glVertex3f(map_range_.start_x, map_range_.start_y, 0.0);
	glVertex3f(map_range_.end_x, map_range_.start_y, 0.0);
	glVertex3f(map_range_.end_x, map_range_.end_y, 0.0);
	glVertex3f(map_range_.start_x, map_range_.end_y, 0.0);
	glEnd();
	glDisable(GL_BLEND);

	glDepthFunc(GL_GEQUAL);

    GLuint temp_tex;

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenTextures(1, &temp_tex);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width_, image_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_values_.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(180, 90, 0.0);
    glTexCoord2f(0.5, 0.0); glVertex3f(360, 90, 0.0);
    glTexCoord2f(0.5, 1.0); glVertex3f(360, -90, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(180, -90, 0.0);

    glTexCoord2f(0.5, 0.0); glVertex3f(0, 90, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(180, 90, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(180, -90, 0.0);
    glTexCoord2f(0.5, 1.0); glVertex3f(0, -90, 0.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &temp_tex);

    //glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}