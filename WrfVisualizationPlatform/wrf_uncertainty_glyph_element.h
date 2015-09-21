#ifndef WRF_UNCERTAINTY_GLYPH_ELEMENT_H_
#define WRF_UNCERTAINTY_GLYPH_ELEMENT_H_

#include <vector>
#include "wrf_data_common.h"
#include "wrf_rendering_element.h"

class WrfUncertaintyGlyphElement : public WrfRenderingElement{
	Q_OBJECT
public:
	WrfUncertaintyGlyphElement();
	~WrfUncertaintyGlyphElement();

	struct UnGlyphUnit{
		float mean;
		float var;
		float lon;
		float lat;
		float radius;
		int node_num;
		int r, g, b;
	};

	void SetUncertaintyData(std::vector< UnGlyphUnit >& data);
	virtual void Render(int left /* = 0 */, int right /* = 0 */, int bottom /* = 0 */, int top /* = 0 */);

private:
	std::vector< UnGlyphUnit > glyph_data_;

	void RenderGridInfoPie(int index);
	void RenderArc(float radius, float begin_theta, float end_theta, bool is_focused);
};

#endif