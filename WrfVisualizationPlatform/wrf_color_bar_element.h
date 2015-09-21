#ifndef WRF_COLOR_BAR_ELEMENT_H_
#define WRF_COLOR_BAR_ELEMENT_H_

#include <vector>
#include "wrf_rendering_element.h"
#include "color_mapping_generator.h"

enum WrfColorBarType{
    VERTICAL = 0x0,
    HORIZONTAL
};

class WrfColorBarElement : public WrfRenderingElement{
public:
    WrfColorBarElement(ColorMappingType color_type, WrfColorBarType = VERTICAL);
    ~WrfColorBarElement();

    // need to set the absolute size of color bar
    virtual void Render(int left, int right, int bottom, int top);
    void Render(QPainter& painter, int left, int right, int bottom, int top);
    void SetTitle(QString title);

private:
    WrfColorBarType type_;
	ColorMappingType color_type_;
    QString title_;

    std::vector< ColorMappingGenerator::ColorIndex > color_index_;
    bool is_linear_;

    void RenderHorizontalBar(int left, int right, int bottom, int top);
    void RenderVerticalbar(int left, int right, int bottom, int top);
};

#endif