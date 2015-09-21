#ifndef WRF_GRID_RMS_ERROR_ELEMENT_H_
#define WRF_GRID_RMS_ERROR_ELEMENT_H_

#include <vector>
#include "wrf_rendering_element.h"
#include "wrf_data_common.h"

class WrfGridRmsErrorElement : public WrfRenderingElement{
    Q_OBJECT
public:
    WrfGridRmsErrorElement();
    ~WrfGridRmsErrorElement();

    void SetRmsUnits(std::vector< RmsUnit >& units);
    void SetMaxSize(float size);
    void SetSelectedIndex(std::vector< bool >& is_selected);
    void SetIndicatorIndex(int index);
    void SetRetrievalSize(int size);
    void SetRetrievalMapRange(MapRange& range);

    virtual void Render(int left /* = 0 */, int right /* = 0 */, int bottom /* = 0 */, int top /* = 0 */);
    void RenderGridInfoPie(int index, bool is_selected, bool is_focused);

private:
    std::vector< RmsUnit > rms_units_;
    std::vector< bool > is_selected_;
    int focus_indicator_index_;
    int retrieval_size_;
    MapRange retrieval_map_range_;
    std::vector< std::vector< bool > > is_background_selected_;

    float max_size_, min_size_;

    void RenderArc(float radius, float begin_theta, float end_theta, bool is_focused);
    void UpdateBackGroundSelection();
};

#endif