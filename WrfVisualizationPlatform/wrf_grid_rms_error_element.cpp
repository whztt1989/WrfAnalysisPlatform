#include "wrf_grid_rms_error_element.h"
#include "color_mapping_generator.h"

WrfGridRmsErrorElement::WrfGridRmsErrorElement(){
    focus_indicator_index_ = -1;
    retrieval_size_ = 1.0;

	name_ = "Region RMS Glyph";
}

WrfGridRmsErrorElement::~WrfGridRmsErrorElement(){

}

void WrfGridRmsErrorElement::SetRmsUnits(std::vector< RmsUnit >& units){
    rms_units_ = units;
    is_selected_.resize(rms_units_.size());
    is_selected_.assign(is_selected_.size(), true);

    max_size_ = 1.0;
    max_size_ /= 2.5;
    min_size_ = 0.2 * max_size_;

    focus_indicator_index_ = -1;
}

void WrfGridRmsErrorElement::SetMaxSize(float size){
    max_size_ = size;
    max_size_ /= 2.5;
    min_size_ = 0.1;
}

void WrfGridRmsErrorElement::SetSelectedIndex(std::vector< bool >& is_selected){
    if ( is_selected.size() != rms_units_.size() ) return;
    is_selected_ = is_selected;

    UpdateBackGroundSelection();
}

void WrfGridRmsErrorElement::SetRetrievalSize(int size){
    retrieval_size_ = size;
}

void WrfGridRmsErrorElement::SetIndicatorIndex(int index){
    focus_indicator_index_ = index;;

    UpdateBackGroundSelection();
}

void WrfGridRmsErrorElement::SetRetrievalMapRange(MapRange& range){
    retrieval_map_range_ = range;

    is_background_selected_.resize(range.y_grid_number - 1);
    for ( int i = 0; i < is_background_selected_.size(); ++i ){
        is_background_selected_[i].resize(range.x_grid_number - 1);
    }
}

void WrfGridRmsErrorElement::UpdateBackGroundSelection(){
    for ( int i = 0; i < is_background_selected_.size(); ++i ){
        is_background_selected_[i].assign(is_background_selected_[i].size(), false);
    }

    int half_size = retrieval_size_ / 2;
    if ( focus_indicator_index_ != -1 ){
        int x_index = rms_units_[focus_indicator_index_].grid_point_index % retrieval_map_range_.x_grid_number;
        int y_index = rms_units_[focus_indicator_index_].grid_point_index / retrieval_map_range_.x_grid_number;
		is_background_selected_[y_index][x_index] = true;
        /*for ( int y = -1 * half_size + 1; y < half_size; ++y ){
            if ( y + y_index < 0 || y + y_index >= retrieval_map_range_.y_grid_number - 1 ) continue;
            for ( int x = -1 * half_size + 1; x < half_size; ++x ){
                if ( x + x_index < 0 || x + x_index > retrieval_map_range_.x_grid_number - 1 ) continue;
                is_background_selected_[y + y_index][x + x_index] = true;
            }
        }*/

    } 
	/*else {
        for ( int i = 0; i < rms_units_.size(); ++i )
            if ( is_selected_[i] ){
                int x_index = rms_units_[i].grid_point_index % retrieval_map_range_.x_grid_number;
                int y_index = rms_units_[i].grid_point_index / retrieval_map_range_.x_grid_number;
                for ( int y = -1 * half_size + 1; y < half_size; ++y ){
                    if ( y + y_index < 0 || y + y_index >= retrieval_map_range_.y_grid_number - 1 ) continue;
                    for ( int x = -1 * half_size + 1; x < half_size; ++x ){
                        if ( x + x_index < 0 || x + x_index > retrieval_map_range_.x_grid_number - 1 ) continue;
                        is_background_selected_[y + y_index][x + x_index] = true;
                    }
                }
            }
    }*/
}

void WrfGridRmsErrorElement::Render(int left , int right , int bottom , int top){

	if ( !is_visible_ ) return;

    // render background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(1.0, 0.6, 0.38, 0.6);
    glBegin(GL_QUADS);
    for ( int i = 0; i < is_background_selected_.size(); ++i )
        for ( int j = 0; j < is_background_selected_[i].size(); ++j )
            if ( is_background_selected_[i][j] ) {
                glVertex3f(retrieval_map_range_.start_x + j * retrieval_map_range_.x_grid_space, retrieval_map_range_.start_y + i * retrieval_map_range_.y_grid_space, 0);
                glVertex3f(retrieval_map_range_.start_x + (j + 1) * retrieval_map_range_.x_grid_space, retrieval_map_range_.start_y + i * retrieval_map_range_.y_grid_space, 0);
                glVertex3f(retrieval_map_range_.start_x + (j + 1) * retrieval_map_range_.x_grid_space, retrieval_map_range_.start_y + (i + 1) * retrieval_map_range_.y_grid_space, 0);
                glVertex3f(retrieval_map_range_.start_x + j * retrieval_map_range_.x_grid_space, retrieval_map_range_.start_y + (i + 1) * retrieval_map_range_.y_grid_space, 0);
            }
    glEnd();

    glDisable(GL_BLEND);

    for ( int i = 0; i < rms_units_.size(); ++i ){
        if ( i == focus_indicator_index_ )
            RenderGridInfoPie(i, is_selected_[i], true);
        else
            RenderGridInfoPie(i, is_selected_[i], false);
    }
}

void WrfGridRmsErrorElement::RenderGridInfoPie(int index, bool is_selected, bool is_focused){

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTranslatef(rms_units_[index].lon, rms_units_[index].lat, 0.0);

    float radius_scale = rms_units_[index].values.size() / 50.0;
    if ( radius_scale > 1.0 ) radius_scale = 1.0;
    float radius = min_size_ + (max_size_ - min_size_) * radius_scale;
    const float PI = 3.14159265;
    float delta_theta = 2 * PI / rms_units_[index].values.size();
    float theta = 0;
    for ( int i = 0; i < rms_units_[index].values.size(); ++i ){
        QColor color = ColorMappingGenerator::GetInstance()->GetColor(RMS_MAPPING, rms_units_[index].values[i], 0, 99);
        if ( is_selected )
            glColor4f(color.redF(), color.greenF(), color.blueF(), 1.0);
        else
            glColor4f(color.redF(), color.greenF(), color.blueF(), 0.1);
        RenderArc(radius, theta, theta + delta_theta, is_focused);

        theta += delta_theta;
    }

    glTranslatef(-1 * rms_units_[index].lon, -1 * rms_units_[index].lat, 0.0);

    glDisable(GL_BLEND);
}

void WrfGridRmsErrorElement::RenderArc(float radius, float begin_theta, float end_theta, bool is_focused){
    int pie_count = (end_theta - begin_theta) / 0.2;
    float temp_theta = begin_theta;

    glBegin(GL_TRIANGLES);
    while ( begin_theta + 0.2 < end_theta ){
        glVertex3f(0, 0, 0);
        glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
        glVertex3f(radius * cos(begin_theta + 0.2), radius * sin(begin_theta + 0.2), 0.0);
        begin_theta += 0.2;
    }
    glVertex3f(0, 0, 0);
    glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
    glVertex3f(radius * cos(end_theta), radius * sin(end_theta), 0.0);
    glEnd();

    /*if ( is_focused ){
        glColor4f(0.8, 0.4, 0.38, 1.0);
        glLineWidth(2.0);
        begin_theta = temp_theta;
        while ( begin_theta + 0.2 < end_theta ){
            glBegin(GL_LINES);
            glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
            glVertex3f(radius * cos(begin_theta + 0.2), radius * sin(begin_theta + 0.2), 0.0);
            glEnd();

            begin_theta += 0.2;
        }

        glBegin(GL_LINES);
        glVertex3f(radius * cos(begin_theta), radius * sin(begin_theta), 0.0);
        glVertex3f(radius * cos(end_theta), radius * sin(end_theta), 0.0);
        glEnd();

        glColor4f(0.5, 0.5, 0.5, 1.0);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.5 * retrieval_map_range_.x_grid_space, -0.5 * retrieval_map_range_.y_grid_space, 0);
        glVertex3f(0.5 * retrieval_map_range_.x_grid_space, -0.5 * retrieval_map_range_.y_grid_space, 0);
        glVertex3f(0.5 * retrieval_map_range_.x_grid_space, 0.5 * retrieval_map_range_.y_grid_space, 0);
        glVertex3f(-0.5 * retrieval_map_range_.x_grid_space, 0.5 * retrieval_map_range_.y_grid_space, 0);
        glEnd();
    }*/
}