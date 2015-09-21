#include "wrf_grid_map_element.h"
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
#include "wrf_data_common.h"
#include "wrf_data_manager.h"
#include <iostream>

WrfGridMapELement::WrfGridMapELement(){
    value_map_ = NULL;

    LoadMapData();
}

WrfGridMapELement::WrfGridMapELement(WrfGridValueMap* value_map){
    value_map_ = value_map;
    map_range_ = value_map_->map_range;

    LoadMapData();

    if ( value_map_->element_type != WRF_UNCERTAINTY ){
        ConstructIsoLines();
    } else {
        ConstructRgbaValues(value_map_->values);
    }
}

WrfGridMapELement::WrfGridMapELement(WrfGridValueMap* value_map, MapRange& viewing_range){

}

WrfGridMapELement::~WrfGridMapELement(){

}

void WrfGridMapELement::UpdateMap(){
	LoadMapData();
}

void WrfGridMapELement::SetViewingRange(MapRange& range){
	map_range_ = range;
}

void WrfGridMapELement::GetRenderImage(int w, int h, WrfGridValueMap* value_map, MapRange& viewing_range, std::vector< QRgb >& pixel_values){
    if (glewInit() != GLEW_OK){
        std::cout << "Stamp generator glewInit error!" << std::endl;
        exit(0);
    }


    std::vector< std::vector< float > > map_polygons_;
    map_polygons_ = WrfDataManager::GetInstance()->GetMapInformation();


    WrfElementType element_type = value_map->element_type;
    ColorMappingType color_type;
	switch ( element_type ){
	case WRF_ACCUMULATED_PRECIPITATION:
	case WRF_PRECIPITABLE_WATER:
		color_type = METEO_RAIN_MAPPING;
		break;
	case WRF_PROBABILISTIC:
		color_type = PROBABILISTIC_MAPPING;
		break;
	case WRF_UNCERTAINTY:
		color_type = UNCERTAINTY_MAPPING;
		break;
	case WRF_T2M:
	case WRF_TEMPERATURE_500HPA:
		color_type = METEO_TMP_MAPPING;
		break;
	case WRF_MSLP:
	case WRF_PRESSURE_SURFACE:
		color_type = METEO_PRES_MAPPING;
		break;
	default:
		color_type = GRAY_MAPPING;
		break;
	}

    bool is_linear;
    std::vector< float > iso_values;
    std::vector< ColorMappingGenerator::ColorIndex > color_index;
    ColorMappingGenerator::GetInstance()->GetColorIndex(color_type, color_index, is_linear);
    for ( int i = 1; i < color_index.size() - 1; ++i ) iso_values.push_back(color_index[i].value_index);

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

    std::vector< std::vector< float > > vtk_triangles;
    vtk_triangles.resize(iso_values.size());
    for ( int i = 0; i < iso_values.size(); ++i ){
        contours->SetInputConnection(plane->GetOutputPort());
        contours->GenerateValues(1, iso_values[i], 9999);
        contours->Update();

        vtkSmartPointer< vtkContourTriangulator > triangulator = vtkSmartPointer< vtkContourTriangulator >::New();
        //triangulator->TriangulationErrorDisplayOn();
        triangulator->SetInputConnection(contours->GetOutputPort());
        triangulator->Update();

        vtkPolyData* polydata = triangulator->GetOutput();
        for ( int j = 0; j < polydata->GetNumberOfCells(); ++j ){
            vtkCell* cell = polydata->GetCell(j);
            vtkIdList* id_list = cell->GetPointIds();
            for ( int k = 0; k < id_list->GetNumberOfIds(); ++k ){
                int id = id_list->GetId(k);
                double* pos = polydata->GetPoint(id);
				float x = pos[0] * (float)(value_map->map_range.x_grid_number + 1) / (value_map->map_range.x_grid_number - 1);
				float y = pos[1] * (float)(value_map->map_range.y_grid_number + 1) / (value_map->map_range.y_grid_number - 1);
                vtk_triangles[i].push_back((x + 0.5) * (value_map->map_range.x_grid_number - 1) * value_map->map_range.x_grid_space + value_map->map_range.start_x);
                vtk_triangles[i].push_back((y + 0.5) * (value_map->map_range.y_grid_number - 1) * value_map->map_range.y_grid_space + value_map->map_range.start_y);
            }
        }
    }   

    GLenum err = glGetError();

    float start_longitude_, end_longitude_, start_latitude_, end_latitude_;
    float left_, right_, bottom_, top_;

    GLuint frame_buffer_, color_buffer_, depth_buffer_;
    GLuint VBO_, EBO_;

    start_longitude_ = viewing_range.start_x;
    end_longitude_ = viewing_range.end_x;
    start_latitude_ = viewing_range.start_y;
    end_latitude_ = viewing_range.end_y;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();

    glGenRenderbuffers(1, &color_buffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, color_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, w, h);

    glGenRenderbuffers(1, &depth_buffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);

    glGenFramebuffers(1, &frame_buffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);

    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    err = glGetError();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //glClearColor(1.0, 1.0, 1.0, 1.0);

    if ( abs(end_longitude_ - start_longitude_) / w >= abs(end_latitude_ - start_latitude_) / h ){
        float degree_per_pixel = abs(end_longitude_ - start_longitude_) / w;
        left_ = start_longitude_;
        right_ = end_longitude_;
        bottom_ = (end_latitude_ + start_latitude_) / 2 - degree_per_pixel * h / 2;
        top_ = (end_latitude_ + start_latitude_) / 2 + degree_per_pixel * h / 2;
    } else {
        float degree_per_pixel = abs(end_latitude_ - start_latitude_) / h;
        left_ = (end_longitude_ + start_longitude_) / 2 - degree_per_pixel * w / 2;
        right_ = (end_longitude_ + start_longitude_) / 2 + degree_per_pixel * w / 2;
        bottom_ = start_latitude_;
        top_ = end_latitude_;
    }

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left_, right_, bottom_, top_, 1, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2);

    glClear(GL_COLOR_BUFFER_BIT);
    
    for ( int i = 0; i < vtk_triangles.size(); ++i ){
        glColor3f(color_index[i + 1].color.redF(), color_index[i + 1].color.greenF(), color_index[i + 1].color.blueF());
        glBegin(GL_TRIANGLES);
        for ( int k = 0; k < vtk_triangles[i].size() / 2; ++k){
            glVertex3f(vtk_triangles[i][k * 2], vtk_triangles[i][k * 2 + 1], 0.0);
        }
        glEnd();
    }


    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(1.0);
    for ( int i = 0; i < map_polygons_.size(); ++i ){
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, (float*)map_polygons_[i].data());
        glDrawArrays(GL_LINE_LOOP, 0, map_polygons_[i].size() / 2);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

	/*glColor3f(0.4f, 0.4f, 0.4f);
	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
	for ( int i = 0; i < brush_path.size(); ++i )
		glVertex3f(brush_path[i].rx(), brush_path[i].ry(), 0);
	glEnd();*/

    err = glGetError();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer_);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    std::vector< unsigned char > temp_pixel_values;
    temp_pixel_values.resize(4 * w * h);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, temp_pixel_values.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteBuffers(1, &VBO_);
    glDeleteBuffers(1, &EBO_);
    glDeleteRenderbuffers(1, &color_buffer_);
    glDeleteRenderbuffers(1, &depth_buffer_);
    glDeleteFramebuffers(1, &frame_buffer_);

    glPopMatrix();
    glPopAttrib();

    err = glGetError();
    pixel_values.resize(w * h);
    for ( int i = 0; i < h; ++i )
        for ( int j = 0; j < w; ++j ){
            int temp_index1 = i * w * 4 + j * 4;
            int temp_index2 = (h - i - 1) * w + j;

            pixel_values[temp_index2] = qRgba(temp_pixel_values[temp_index1], temp_pixel_values[temp_index1 + 1], temp_pixel_values[temp_index1 + 2], temp_pixel_values[temp_index1 + 3]);
        }
}

void WrfGridMapELement::ConstructRgbaValues(float* values){
    rgba_values_.resize(map_range_.x_grid_number * map_range_.y_grid_number * 4);

    WrfModelType model_type = value_map_->model_type;
    WrfElementType element_type = value_map_->element_type;
    float min_value, max_value;
    ColorMappingType color_type;
    switch ( element_type ){
    case WRF_ACCUMULATED_PRECIPITATION:
    case WRF_PRECIPITABLE_WATER:
        min_value = 0;
        max_value = 200;
        color_type = METEO_RAIN_MAPPING;
        break;
    case WRF_PROBABILISTIC:
        min_value = 0;
        max_value = 1;
        color_type = PROBABILISTIC_MAPPING;
        break;
    case WRF_UNCERTAINTY:
        min_value = 0;
        max_value = 1;
        color_type = UNCERTAINTY_MAPPING;
        break;
	case WRF_T2M:
    case WRF_TEMPERATURE_500HPA:
        min_value = 0;
        max_value = 400;
        color_type = METEO_TMP_MAPPING;
        break;
	case WRF_MSLP:
	case WRF_PRESSURE_SURFACE:
		min_value = 0;
		max_value = 1000000;
		color_type = METEO_PRES_MAPPING;
		break;
    default:
        min_value = 0;
        max_value = 1;
        color_type = GRAY_MAPPING;
        break;
    }

    for ( int i = 0; i < map_range_.y_grid_number * map_range_.x_grid_number; ++i ){
            QColor color = ColorMappingGenerator::GetInstance()->GetColor(color_type, values[i], min_value, max_value);
            rgba_values_[4 * i] = color.red();
            rgba_values_[4 * i + 1] = color.green();
            rgba_values_[4 * i + 2] = color.blue();
            rgba_values_[4 * i + 3] = color.alpha();
        }

    value_map_->is_updated = false;
}

void WrfGridMapELement::ConstructIsoLines(){    
    WrfElementType element_type = value_map_->element_type;
	ColorMappingType color_type;
	switch ( element_type ){
	case WRF_ACCUMULATED_PRECIPITATION:
	case WRF_PRECIPITABLE_WATER:
		color_type = METEO_RAIN_MAPPING;
		break;
	case WRF_PROBABILISTIC:
		color_type = PROBABILISTIC_MAPPING;
		break;
	case WRF_UNCERTAINTY:
		color_type = UNCERTAINTY_MAPPING;
		break;
	case WRF_T2M:
	case WRF_TEMPERATURE_500HPA:
		color_type = METEO_TMP_MAPPING;
		break;
	case WRF_MSLP:
	case WRF_PRESSURE_SURFACE:
		color_type = METEO_PRES_MAPPING;
		break;
	default:
		color_type = GRAY_MAPPING;
		break;
	}

    bool is_linear;
    ColorMappingGenerator::GetInstance()->GetColorIndex(color_type, color_index_, is_linear);
    for ( int i = 1; i < color_index_.size() - 1; ++i ) iso_values_.push_back(color_index_[i].value_index - 1e-3);

    vtkSmartPointer< vtkMarchingContourFilter > contours = vtkSmartPointer< vtkMarchingContourFilter >::New();
    vtkSmartPointer< vtkPlaneSource > plane = vtkSmartPointer< vtkPlaneSource >::New();
    plane->SetXResolution(value_map_->map_range.x_grid_number + 1);
    plane->SetYResolution(value_map_->map_range.y_grid_number + 1);
    plane->SetCenter(0, 0, 0);
    plane->SetNormal(0, 0, 1);
    plane->Update();

    vtkSmartPointer< vtkFloatArray > map_scalars = vtkSmartPointer< vtkFloatArray >::New();
    map_scalars->SetNumberOfComponents(1);
    int temp_index = 0;
    for ( int i = 0; i <= value_map_->map_range.y_grid_number + 1; ++i )
        for ( int j = 0; j <= value_map_->map_range.x_grid_number + 1; ++j ){
            if ( i == 0 || i == value_map_->map_range.y_grid_number + 1 
                || j == 0 || j == value_map_->map_range.x_grid_number + 1 )
                map_scalars->InsertNextTuple1(-1e10);
            else{
                map_scalars->InsertNextTuple1(*(value_map_->values + temp_index));
                temp_index++;
            }
        }
    plane->GetOutput()->GetPointData()->SetScalars(map_scalars);

    vtk_triangles_.clear();
    vtk_triangles_.resize(iso_values_.size());
    for ( int i = 0; i < iso_values_.size(); ++i ){
        contours->SetInputConnection(plane->GetOutputPort());
        contours->GenerateValues(1, iso_values_[i], 9999);
        contours->Update();

        vtkSmartPointer< vtkContourTriangulator > triangulator = vtkSmartPointer< vtkContourTriangulator >::New();
        //triangulator->TriangulationErrorDisplayOn();
        triangulator->SetInputConnection(contours->GetOutputPort());
        triangulator->Update();

        vtkPolyData* polydata = triangulator->GetOutput();
        for ( int j = 0; j < polydata->GetNumberOfCells(); ++j ){
            vtkCell* cell = polydata->GetCell(j);
            vtkIdList* id_list = cell->GetPointIds();
            for ( int k = 0; k < id_list->GetNumberOfIds(); ++k ){
                int id = id_list->GetId(k);
                double* pos = polydata->GetPoint(id);
				float x = pos[0] * (float)(value_map_->map_range.x_grid_number + 1) / (value_map_->map_range.x_grid_number - 1);
				float y = pos[1] * (float)(value_map_->map_range.y_grid_number + 1) / (value_map_->map_range.y_grid_number - 1);
                vtk_triangles_[i].push_back((x + 0.5) * (value_map_->map_range.x_grid_number - 1) * value_map_->map_range.x_grid_space + value_map_->map_range.start_x);
                vtk_triangles_[i].push_back((y + 0.5) * (value_map_->map_range.y_grid_number - 1) * value_map_->map_range.y_grid_space + value_map_->map_range.start_y);
            }
        }
    }   
    value_map_->is_updated = false;
}

void WrfGridMapELement::LoadMapData(){
    map_polygons_ = WrfDataManager::GetInstance()->GetMapInformation();
}

void WrfGridMapELement::Render(int left /* = 0 */, int right /* = 0 */, int bottom /* = 0 */, int top /* = 0 */){
	if ( !is_visible_ ) return;

    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if ( value_map_ != NULL ){
        if ( value_map_->is_updated ) {
            if ( value_map_->element_type != WRF_UNCERTAINTY ){
                ConstructIsoLines();
            } else {
                ConstructRgbaValues(value_map_->values);
            }
        }

        glBegin(GL_QUADS);
        glVertex3f(value_map_->map_range.start_x, value_map_->map_range.start_y, 0.0);
        glVertex3f(value_map_->map_range.end_x, value_map_->map_range.start_y, 0.0);
        glVertex3f(value_map_->map_range.end_x, value_map_->map_range.end_y, 0.0);
        glVertex3f(value_map_->map_range.start_x, value_map_->map_range.end_y, 0.0);
        glEnd();

        if ( value_map_->element_type != WRF_UNCERTAINTY ){
            for ( int i = 0; i < vtk_triangles_.size(); ++i ){
                glColor3f(color_index_[i + 1].color.redF(), color_index_[i + 1].color.greenF(), color_index_[i + 1].color.blueF());
                glBegin(GL_TRIANGLES);
                for ( int k = 0; k < vtk_triangles_[i].size() / 2; ++k){
                    glVertex3f(vtk_triangles_[i][k * 2], vtk_triangles_[i][k * 2 + 1], 0.0);
                }
                glEnd();
            }

        } else {
            glColor4f(1.0, 1.0, 1.0, 1.0);

            switch ( value_map_->type() ){
            case WrfValueMap::GRID_VALUE_MAP:
                RenderStructuredGridMap();
                break;
            case WrfValueMap::LAMBCC_VALUE_MAP:
                RenderLambccGridMap();
                break;
            default:
                break;
            }
        }
    } else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0, 1.0, 1.0, 0.0);
		glBegin(GL_QUADS);
		glVertex3f(map_range_.start_x, map_range_.start_y, 0.0);
		glVertex3f(map_range_.end_x, map_range_.start_y, 0.0);
		glVertex3f(map_range_.end_x, map_range_.end_y, 0.0);
		glVertex3f(map_range_.start_x, map_range_.end_y, 0.0);
		glEnd();
		glDisable(GL_BLEND);
	}
    
    glDepthFunc(GL_GEQUAL);

    //if ( value_map_ == NULL ) glDepthFunc(GL_LEQUAL);

    glColor3f(0.7, 0.7, 0.7);
    glLineWidth(2.0);
    for ( int i = 0; i < map_polygons_.size(); ++i ){
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, (float*)map_polygons_[i].data());
        glDrawArrays(GL_LINE_LOOP, 0, map_polygons_[i].size() / 2);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    glDisable(GL_DEPTH_TEST);
}

void WrfGridMapELement::RenderStructuredGridMap(){
    std::vector< float > vertex_pos;
    vertex_pos.resize(value_map_->map_range.x_grid_number * value_map_->map_range.y_grid_number * 2);
    for ( int i = 0; i < vertex_pos.size() / 2; ++i ){
        int x = i % value_map_->map_range.x_grid_number;
        int y = i / value_map_->map_range.x_grid_number;
        vertex_pos[i * 2] = value_map_->map_range.start_x + value_map_->map_range.x_grid_space * x;
        vertex_pos[i * 2 + 1] = value_map_->map_range.start_y + value_map_->map_range.y_grid_space * y;
    }
    std::vector< int > tri_index;
    tri_index.resize((value_map_->map_range.y_grid_number - 1) * (value_map_->map_range.x_grid_number - 1) * 6);

    for ( int i = 0; i < value_map_->map_range.y_grid_number; ++i ){
        for ( int j = 0; j < value_map_->map_range.x_grid_number; ++j ){
            if ( i == 0 || j == 0 ) continue;
            int left_bottom = (i - 1) * value_map_->map_range.x_grid_number + j - 1;
            int left_top = i * value_map_->map_range.x_grid_number + j - 1;
            int right_bottom = (i - 1) * value_map_->map_range.x_grid_number + j;
            int right_top = i * value_map_->map_range.x_grid_number + j;
            int temp_index = ((i - 1) * (value_map_->map_range.x_grid_number - 1) + j - 1) * 6;
            tri_index[temp_index] = left_top;
            tri_index[temp_index + 1] = left_bottom;
            tri_index[temp_index + 2] = right_bottom;
            tri_index[temp_index + 3] = left_top;
            tri_index[temp_index + 4] = right_bottom;
            tri_index[temp_index + 5] = right_top;
        }
    }

    //glColor3f(1.0, 0.0, 0.0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertex_pos.data());
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, rgba_values_.data());
    glDrawElements(GL_TRIANGLES, tri_index.size(), GL_UNSIGNED_INT, (GLvoid*)tri_index.data());
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void WrfGridMapELement::RenderLambccGridMap(){
    WrfLambCcValueMap* lambcc_map = dynamic_cast< WrfLambCcValueMap* >(value_map_);

    std::vector< float > vertex_pos;
    vertex_pos.resize(lambcc_map->lon.size() * 2);
    for ( int i = 0; i < lambcc_map->lon.size(); ++i ){
        vertex_pos[i * 2] = lambcc_map->lon[i];
        vertex_pos[i * 2 + 1] = lambcc_map->lat[i];
    }
    std::vector< int > tri_index;
    tri_index.resize((lambcc_map->map_range.y_grid_number - 1) * (lambcc_map->map_range.x_grid_number - 1) * 6);

    for ( int i = 0; i < lambcc_map->map_range.y_grid_number; ++i ){
        for ( int j = 0; j < lambcc_map->map_range.x_grid_number; ++j ){
            if ( i == 0 || j == 0 ) continue;
            int left_bottom = (i - 1) * lambcc_map->map_range.x_grid_number + j - 1;
            int left_top = i * lambcc_map->map_range.x_grid_number + j - 1;
            int right_bottom = (i - 1) * lambcc_map->map_range.x_grid_number + j;
            int right_top = i * lambcc_map->map_range.x_grid_number + j;
            int temp_index = ((i - 1) * (lambcc_map->map_range.x_grid_number - 1) + j - 1) * 6;
            tri_index[temp_index] = left_top;
            tri_index[temp_index + 1] = left_bottom;
            tri_index[temp_index + 2] = right_bottom;
            tri_index[temp_index + 3] = left_top;
            tri_index[temp_index + 4] = right_bottom;
            tri_index[temp_index + 5] = right_top;
        }
    }

    //glColor3f(1.0, 0.0, 0.0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertex_pos.data());
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, rgba_values_.data());
    glDrawElements(GL_TRIANGLES, tri_index.size(), GL_UNSIGNED_INT, (GLvoid*)tri_index.data());
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}