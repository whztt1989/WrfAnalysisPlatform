#ifndef WRF_IMAGE_ELEMENT_H_
#define WRF_IMAGE_ELEMENT_H_

#include "wrf_rendering_element.h"
#include "wrf_data_common.h"

class WrfImageElement : public WrfRenderingElement {
public:
    WrfImageElement(const char* file_name);
    ~WrfImageElement();

    virtual void Render(int left, int right, int bottom, int top);
	void SetViewingRange(MapRange& range) { map_range_ = range; }

private:
    int image_width_, image_height_;
    std::vector< unsigned char > rgba_values_;
	MapRange map_range_;
};

#endif