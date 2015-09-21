#include "wrf_iso_line_element.h"
#include <iostream>

WrfIsoLineElement::WrfIsoLineElement(std::vector< float >& iso_values, std::vector< std::vector< std::vector< float > > >& iso_lines){
	iso_values_ = iso_values;
	iso_lines_ = iso_lines;

	line_mode_ = 0;
	list_base_ = -1;
}

WrfIsoLineElement::~WrfIsoLineElement(){

}

void WrfIsoLineElement::SetLineMode(int mode){
	line_mode_ = mode;
}

void WrfIsoLineElement::SetListBase(int base){
	list_base_ = base;
}

void WrfIsoLineElement::Render(int left, int right, int bottom, int top){
	if ( !is_visible_ ) return;

	glLineWidth(1.0);

	glColor3f(0.4, 0.4, 0.4);
	for ( int i = 0; i < iso_values_.size(); ++i ){
		// render lines
		for ( int j = 0; j < iso_lines_[i].size(); ++j )
			if ( iso_lines_[i][j].size() > 0 ){
				if ( line_mode_ != 0 ){
					glEnable(GL_LINE_STIPPLE);
					glLineStipple(1, 0x3F07);
				}

				glBegin(GL_LINE_STRIP);
				for ( int k = 0; k < iso_lines_[i][j].size() / 2; ++k ){
					glVertex3f(iso_lines_[i][j][k * 2], iso_lines_[i][j][2 * k + 1], 0);
				}
				glEnd();

				if ( line_mode_!= 0 ){
					glDisable(GL_LINE_STIPPLE);
				}
				
				// render value
				int mid_index = iso_lines_[i][j].size() / 2 - 1;

				glPushAttrib(GL_LIST_BIT);
				if ( list_base_ != -1 ){

					glListBase(list_base_);
					glPushMatrix();
					glTranslatef(iso_lines_[i][j][mid_index], iso_lines_[i][j][mid_index + 1], 0);
					glRasterPos2f(0, 0);
					char value[256];
					itoa((int)(iso_values_[i]), value, 10);
					glCallLists((GLsizei)strlen(value), GL_UNSIGNED_BYTE, value);
					glPopMatrix();
				}
				glPopAttrib();
			}
	}
}