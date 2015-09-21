#ifndef WRF_RENDERING_ELEMENT_FACTORY_H_
#define WRF_RENDERING_ELEMENT_FACTORY_H_

#include "wrf_data_common.h"
#include "color_mapping_generator.h"

class WrfRenderingElement;
class WrfValueMap;

class WrfRenderingElementFactory{
public:
    WrfRenderingElementFactory();
    ~WrfRenderingElementFactory();

    static WrfRenderingElement* GenerateRenderingElement(WrfValueMap* value_map);
    static WrfRenderingElement* GenerateRenderingElement(const char* image_path);
    static WrfRenderingElement* GenerateMapElement();
    static WrfRenderingElement* GenerateColorBarElement(WrfElementType type);
	static WrfRenderingElement* GenerateIsoLinePlot(int mode, WrfGridValueMap* value_map, float iso_value, int list_base);
};

#endif