#include "wrf_rendering_element_factory.h"
#include "wrf_rendering_element.h"
#include "wrf_grid_map_element.h"
#include "wrf_color_bar_element.h"
#include "wrf_image_element.h"
#include "wrf_iso_line_element.h"
#include <vtkContourFilter.h>
#include <vtkSmartPointer.h>
#include <vtkPlaneSource.h>
#include <vtkMarchingContourFilter.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkStripper.h>
#include <vtkCellArray.h>
#include <vtkContourTriangulator.h>
#include <iostream>
#include "wrf_utility.h"

WrfRenderingElementFactory::WrfRenderingElementFactory(){

}

WrfRenderingElementFactory::~WrfRenderingElementFactory(){

}

WrfRenderingElement* WrfRenderingElementFactory::GenerateMapElement(){
	WrfRenderingElement* element = new WrfGridMapELement();
	element->SetName(std::string("Geographical Map"));
    return element;
}

WrfRenderingElement* WrfRenderingElementFactory::GenerateRenderingElement(WrfValueMap* value_map){
    if ( value_map->type() == WrfValueMap::GRID_VALUE_MAP
        || value_map->type() == WrfValueMap::LAMBCC_VALUE_MAP ){
        WrfGridValueMap* grid_map = dynamic_cast< WrfGridValueMap* >(value_map);
        WrfGridMapELement* map_element = new WrfGridMapELement(grid_map);
		map_element->SetName(std::string("Value Map"));
        return map_element;
    }
    return NULL;
}

WrfRenderingElement* WrfRenderingElementFactory::GenerateRenderingElement(const char* image_path){
    WrfImageElement* image_element = new WrfImageElement(image_path);
	image_element->SetName(std::string("Geographical Background"));
    return image_element;
}

WrfRenderingElement* WrfRenderingElementFactory::GenerateColorBarElement(WrfElementType type){
    WrfColorBarElement* color_bar = NULL;
    switch ( type ){
    case WRF_ACCUMULATED_PRECIPITATION:
    case WRF_PRECIPITABLE_WATER:
        color_bar = new WrfColorBarElement(METEO_RAIN_MAPPING);
        break;
    case WRF_PROBABILISTIC:
        color_bar = new WrfColorBarElement(PROBABILISTIC_MAPPING);
        break;
    case WRF_UNCERTAINTY:
        color_bar = new WrfColorBarElement(UNCERTAINTY_MAPPING);
        break;
	case WRF_T2M:
    case WRF_TEMPERATURE_500HPA:
        color_bar = new WrfColorBarElement(METEO_TMP_MAPPING);
        break;
	case WRF_MSLP:
	case WRF_PRESSURE_SURFACE:
		color_bar = new WrfColorBarElement(METEO_PRES_MAPPING);
		break;
    case WRF_RMS:
        color_bar = new WrfColorBarElement(RMS_MAPPING);
        break;
    default:
        color_bar = new WrfColorBarElement(GRAY_MAPPING);
        break;
    }
	color_bar->SetName(std::string("Color Bar"));

    return color_bar;
}

WrfRenderingElement* WrfRenderingElementFactory::GenerateIsoLinePlot(int mode, WrfGridValueMap* value_map, float iso_value, int list_base){
	vtkSmartPointer< vtkMarchingContourFilter > contours = vtkSmartPointer< vtkMarchingContourFilter >::New();
	vtkSmartPointer< vtkPlaneSource > plane = vtkSmartPointer< vtkPlaneSource >::New();
	plane->SetXResolution(value_map->map_range.x_grid_number + 1);
	plane->SetYResolution(value_map->map_range.y_grid_number + 1);
	plane->SetCenter(0, 0, 0);
	plane->SetNormal(0, 0, 1);
	plane->Update();

	vtkSmartPointer< vtkFloatArray > map_scalars = vtkSmartPointer< vtkFloatArray >::New();
	map_scalars->SetNumberOfComponents(1);
	int temp_index = 0;
	for ( int i = 0; i <= value_map->map_range.y_grid_number + 1; ++i )
		for ( int j = 0; j <= value_map->map_range.x_grid_number + 1; ++j ){
			if ( i == 0 || i == value_map->map_range.y_grid_number + 1 
				|| j == 0 || j == value_map->map_range.x_grid_number + 1 )
				map_scalars->InsertNextTuple1(-1e10);
			else{
				map_scalars->InsertNextTuple1(*(value_map->values + temp_index));
				temp_index++;
			}
		}
	plane->GetOutput()->GetPointData()->SetScalars(map_scalars);

	contours->SetInputConnection(plane->GetOutputPort());
	contours->GenerateValues(1, iso_value, 9999);

	vtkSmartPointer<vtkStripper> contourStripper =
		vtkSmartPointer<vtkStripper>::New();
	contourStripper->SetInputConnection(contours->GetOutputPort());
	contourStripper->Update();

	int numberOfContourLines = contourStripper->GetOutput()->GetNumberOfCells();

	std::vector< float > iso_values;
	std::vector< std::vector< std::vector< float > > > iso_lines;
	iso_values.push_back(iso_value);
	iso_lines.resize(1);
	vtkPolyData* output_poly = contourStripper->GetOutput();
	iso_lines[0].resize(numberOfContourLines);
	for ( int i = 0; i < numberOfContourLines; ++i ){
		std::vector< float > line_point;
		vtkCell* cell = output_poly->GetCell(i);
		vtkIdList* id_list = cell->GetPointIds();

		for ( int j = 0; j < id_list->GetNumberOfIds(); ++j ){
			int id = id_list->GetId(j);
			double* pos = output_poly->GetPoint(id);

			float x = pos[0] * (float)(value_map->map_range.x_grid_number + 1) / (value_map->map_range.x_grid_number - 1);
			float y = pos[1] * (float)(value_map->map_range.y_grid_number + 1) / (value_map->map_range.y_grid_number - 1);

			x = (x + 0.5) * (value_map->map_range.x_grid_number - 1) * value_map->map_range.x_grid_space + value_map->map_range.start_x;
			y = (y + 0.5) * (value_map->map_range.y_grid_number - 1) * value_map->map_range.y_grid_space + value_map->map_range.start_y;

			line_point.push_back(x);
			line_point.push_back(y);
		}
		iso_lines[0][i] = line_point;
	}

	WrfIsoLineElement* line_element = new WrfIsoLineElement(iso_values, iso_lines);
	line_element->SetLineMode(mode);
	line_element->SetListBase(list_base);
	QString name_str(enum_element_to_string(value_map->element_type));
	name_str += QString(" ISO Line Value: %0").arg(iso_value);
	line_element->SetName(std::string(name_str.toLocal8Bit().data()));

	return line_element;
}