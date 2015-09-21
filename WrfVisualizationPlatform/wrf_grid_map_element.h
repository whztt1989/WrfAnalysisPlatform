#ifndef WRF_GRID_MAP_ELEMENT_H_
#define WRF_GRID_MAP_ELEMENT_H_

#include "wrf_rendering_element.h"
#include <QtGui/QPixmap>
#include <vector>
#include "wrf_data_common.h"
#include "color_mapping_generator.h"

class WrfGridMapELement : public WrfRenderingElement{
    Q_OBJECT

public:
    WrfGridMapELement();
    WrfGridMapELement(WrfGridValueMap* value_map);
    WrfGridMapELement(WrfGridValueMap* value_map, MapRange& viewing_range);
    ~WrfGridMapELement();

    virtual void Render(int left = 0, int right = 0, int bottom = 0, int top = 0);
    static void GetRenderImage(int w, int h, WrfGridValueMap* value_map, MapRange& viewing_range, std::vector< QRgb >& pixel_values);
	void SetViewingRange(MapRange& range);
	void UpdateMap();

private:
    WrfGridValueMap* value_map_;
    std::vector< unsigned char > rgba_values_;
    MapRange map_range_;

    std::vector< std::vector< float > > map_polygons_;

    std::vector< float > iso_values_;
    std::vector< ColorMappingGenerator::ColorIndex > color_index_;
    std::vector< std::vector< float > > vtk_triangles_;

    void ConstructRgbaValues(float* values);
    void ConstructIsoLines();
    void LoadMapData();

    void RenderStructuredGridMap();
    void RenderLambccGridMap();
};

#endif