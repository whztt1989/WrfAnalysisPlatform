#include "wrf_uncertainty_glyph_element.h"

WrfUncertaintyGlyphElement::WrfUncertaintyGlyphElement(){

}

WrfUncertaintyGlyphElement::~WrfUncertaintyGlyphElement(){

}

void WrfUncertaintyGlyphElement::SetUncertaintyData(std::vector< UnGlyphUnit >& data){
	glyph_data_ = data;
}

void WrfUncertaintyGlyphElement::Render(int left, int right, int bottom, int top){
	if ( !is_visible_ ) return;

	// render background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1.0, 0.6, 0.38, 0.6);

	for ( int i = 0; i < glyph_data_.size(); ++i ){
		RenderGridInfoPie(i);
	}
}

void WrfUncertaintyGlyphElement::RenderGridInfoPie(int index){

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTranslatef(glyph_data_[index].lon, glyph_data_[index].lat, 0.0);

	float radius_scale = glyph_data_[index].radius;
	QColor color = QColor(glyph_data_[index].r, glyph_data_[index].g, glyph_data_[index].b);
	glColor4f(color.redF(), color.greenF(), color.blueF(), 1.0);
	RenderArc(radius_scale, 0, 2 * 3.14159, false);
	glTranslatef(-1 * glyph_data_[index].lon, -1 * glyph_data_[index].lat, 0.0);

	glDisable(GL_BLEND);
}

void WrfUncertaintyGlyphElement::RenderArc(float radius, float begin_theta, float end_theta, bool is_focused){
    int pie_count = (end_theta - begin_theta) / 0.2;
    float temp_theta = begin_theta;

    glBegin(GL_TRIANGLES);
    while ( begin_theta + 0.2 < end_theta ){
        glVertex3f(0, 0, 0);
        glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
        glVertex3f(radius * cos(begin_theta + 0.2), radius * sin(begin_theta + 0.2), 0.0);
        begin_theta += 0.2;
    }
    glVertex3f(0, 0, 0);
    glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
    glVertex3f(radius * cos(end_theta), radius * sin(end_theta), 0.0);
    glEnd();
}