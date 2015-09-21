#ifndef WRF_ISO_LINE_ELEMENT_H_
#define WRF_ISO_LINE_ELEMENT_H_

#include "wrf_rendering_element.h"
#include <gl/glew.h>

class WrfIsoLineElement : public WrfRenderingElement{
public:
	WrfIsoLineElement(std::vector< float >& iso_values, std::vector< std::vector< std::vector< float > > >& iso_lines);
	~WrfIsoLineElement();

	virtual void Render(int left, int right, int bottom, int top);
	void SetLineMode(int mode);
	void SetListBase(int base);

private:
	int line_mode_;
	GLuint list_base_;
	std::vector< float > iso_values_;
	std::vector< std::vector< std::vector< float > > > iso_lines_;
};

#endif