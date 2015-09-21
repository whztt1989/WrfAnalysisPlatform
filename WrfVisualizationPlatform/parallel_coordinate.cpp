#include "parallel_coordinate.h"

ParallelDataset::ParallelDataset()
    : is_edge_bundling_enabled(false), is_correlation_analysis_enabled(false), 
    is_cluster_enabled(false), is_range_filter_enabled(false), is_axis_weight_enabled(false){
}

ParallelDataset::~ParallelDataset(){

}

bool ParallelDataset::CompleteInput(){
    if ( subset_names.size() != subset_records.size() || axis_anchors.size() != axis_names.size() ) return false;

    if ( subset_colors.size() != subset_names.size() ){
        subset_colors.clear();
        /// TODO:select colors here
    }

    if ( is_subset_visible.size() != subset_names.size() ){
        is_subset_visible.clear();
        is_subset_visible.resize(subset_names.size(), true);
    }

    if ( subset_opacity.size() != subset_names.size() ){
        subset_opacity.clear();
        subset_opacity.resize(subset_names.size(), 1.0);
    }

    if ( mapped_axis.size() != axis_names.size() ){
        mapped_axis.resize(axis_names.size());
        for ( int i = 0; i < mapped_axis.size(); ++i ) mapped_axis[i] = i;
    }

    if ( axis_weights.size() != axis_names.size() ){
        axis_weights.clear();
        axis_weights.resize(axis_names.size(), 1.0);
    }

    if ( axis_value_filter_range.size() != axis_names.size() ){
        axis_value_filter_range.resize(axis_names.size());
        for ( int i = 0; i < axis_value_filter_range.size(); ++i ){
            axis_value_filter_range[i].resize(2);
            axis_value_filter_range[i][0] = 0.0;
            axis_value_filter_range[i][1] = 1.0;
        }
    }

    //if ( is_record_selected.size() != subset_names.size() ){
    //    is_record_selected.resize(subset_names.size());
    //    for ( int i = 0; i < is_record_selected.size(); ++i )
    //        if ( is_record_selected[i].size() != subset_records[i].size() ){
    //            is_record_selected[i].resize(subset_records[i].size());
    //            is_record_selected[i].assign(is_record_selected[i].size(), true);
    //            //memset(&is_record_selected[i][0], 0, subset_records[i].size());
    //        }
    //}

    if ( is_axis_selected.size() != axis_names.size() ){
        is_axis_selected.clear();
        is_axis_selected.resize(axis_names.size(), false);
    }

    return false;
}

bool ParallelDataset::ClearData(){
    this->subset_names.clear();
    for ( int i = 0; i < subset_records.size(); ++i )
        for ( int j = 0; j < subset_records[i].size(); ++j ) delete subset_records[i][j];
    subset_records.clear();
    axis_anchors.clear();
    axis_names.clear();
    record_color.clear();

    return true;
}


ParallelCoordinate::ParallelCoordinate()
    : dataset_(NULL){
    this->setMinimumSize(200, 200);
}

ParallelCoordinate::~ParallelCoordinate(){

}

void ParallelCoordinate::SetDataset(ParallelDataset* dataset_t){
    dataset_ = dataset_t;

    UpdateViewLayoutParameters();

    this->updateGL();
}

void ParallelCoordinate::UpdateViewLayoutParameters(){
    x_border_ = 40.0 / this->width();
    y_border_ = 10.0 / this->height();

    icon_width_ = 20.0 / this->width();
    icon_height_ = 20.0 / this->height();

    if ( dataset_ == NULL ) return;

    axis_name_height_ = 15.0 / this->height();
    range_text_height_ = 10.0 / this->height();
    axis_width_ = 2.0 / this->width();

    weight_circle_radius_ = 10.0;

    subset_rect_width_ = 40.0 / this->width();
    subset_rect_height_ = 15.0 / this->height();
    subset_rect_text_width_ = 40.0 / this->width();
    subset_rect_y_value_ = 1.0 - y_border_ - subset_rect_height_;

    if ( dataset_->is_axis_weight_enabled ){
        weight_circle_radius_ = 15.0 / this->height();
        weight_circle_center_y_value_ = 0.5 * y_border_ + weight_circle_center_y_value_;
    } else {
        weight_circle_radius_ = 0;
    }

    axis_top_y_value_ = 1.0 - y_border_ * 3 - range_text_height_ - subset_rect_height_;
    axis_bottom_y_value_ = y_border_ * 2.5 + axis_name_height_ + range_text_height_ + 2 * weight_circle_radius_;
    axis_y_size_ = axis_top_y_value_ - axis_bottom_y_value_;
    axis_x_pos_values_.resize(dataset_->axis_names.size());
    int axis_num = dataset_->axis_names.size();
    float begin_pos = x_border_ + axis_width_ / 2;
    float end_pos = 1.0 - x_border_ - axis_width_ / 2;
    float step_size = (end_pos - begin_pos) / (axis_num - 1);
    for ( int i = 0; i < axis_num; ++i ) axis_x_pos_values_[i] = begin_pos + step_size * i;

    axis_name_y_value_ = y_border_ + 2 * weight_circle_radius_;
    range_text_bottom_y_value_ = axis_name_y_value_ + axis_name_height_ + 0.5 * y_border_;
    range_text_top_y_value_ = axis_top_y_value_ + 3.0 / this->height();
}

void ParallelCoordinate::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        std::cout << "Error initialize PCP error!" << std::endl;
        return;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);

    QImage image("../Plots/Resources/setting.png");
    setting_texture_ = this->bindTexture(image, GL_TEXTURE_2D, GL_RGBA);
    if ( setting_texture_ == -1 ) exit(0);
}

void ParallelCoordinate::resizeGL(int w, int h){
    UpdateViewLayoutParameters();
}

void ParallelCoordinate::paintGL(){
    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, 0, 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -1.0);

    glClear(GL_COLOR_BUFFER_BIT);
    glShadeModel(GL_SMOOTH);

    if ( dataset_ == NULL ) return;

    PaintSubsetIdentifyItems();
    PaintLines();
    PaintText();
    PaintCoordinate();
}

void ParallelCoordinate::PaintSettingIcon(){
    this->drawTexture(QRectF(1.0 - icon_width_, 1.0 - icon_height_, icon_width_, icon_height_), setting_texture_);
}

void ParallelCoordinate::PaintCoordinate(){
    glColor3f(0.0, 0.0, 0.0);

    for ( int i = 0; i < axis_x_pos_values_.size(); ++i ){
        glRectf(axis_x_pos_values_[i] - axis_width_ / 2, axis_bottom_y_value_, axis_x_pos_values_[i] + axis_width_ / 2, axis_top_y_value_);
    }
}

void ParallelCoordinate::PaintLines(){
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for ( int i = 0; i < dataset_->subset_records.size(); ++i ){
        if ( i != 0 ) 
            glLineWidth(2.0);
        else
            glLineWidth(1.0);
        //glColor4f(dataset_->subset_colors[i].redF(), dataset_->subset_colors[i].greenF(), dataset_->subset_colors[i].blueF(), dataset_->subset_opacity[i]);
        for ( int j = 0; j < dataset_->subset_records[i].size(); ++j ){
            ParallelRecord* record = dataset_->subset_records[i][j];
            if ( record == NULL || record->values.size() != dataset_->axis_names.size() ) continue;
            if ( dataset_->is_record_selected.size() != 0 && dataset_->record_color.size() != 0 ){
                if ( dataset_->is_record_selected[i][j] ){
                    glColor4f(dataset_->record_color[i][j].redF(), dataset_->record_color[i][j].greenF(), dataset_->record_color[i][j].blueF(), 1.0);
                } else {
                    glColor4f(dataset_->record_color[i][j].redF(), dataset_->record_color[i][j].greenF(), dataset_->record_color[i][j].blueF(), 0.0);
                }
            }
            glBegin(GL_LINE_STRIP);
            for ( int k = 0; k < record->values.size(); ++k )
                glVertex3f(axis_x_pos_values_[k], axis_bottom_y_value_ + record->values[k] * axis_y_size_, 0);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
}

void ParallelCoordinate::PaintText(){
    int y_axis_name = (int)((1.0 - axis_name_y_value_) * this->height());
    int y_max = (int)((1.0 - range_text_top_y_value_) * this->height());
    int y_min = (int)((1.0 - range_text_bottom_y_value_) * this->height());

    glColor3f(0.0, 0.0, 0.0);
    for ( int i = 0; i < axis_x_pos_values_.size(); ++i ){
        int axis_x = (int)(axis_x_pos_values_[i] * this->width());
        this->renderText(axis_x - dataset_->axis_names[i].length() * 3, y_axis_name, dataset_->axis_names[i]);
        this->renderText(axis_x - dataset_->axis_anchors[i][1].length() * 3, y_max, dataset_->axis_anchors[i][1]);
        this->renderText(axis_x - dataset_->axis_anchors[i][0].length() * 3, y_min, dataset_->axis_anchors[i][0]);
    }
}

void ParallelCoordinate::PaintSubsetIdentifyItems(){
    for ( int i = 0; i < dataset_->subset_names.size(); ++i ){
        float x = i * (subset_rect_width_ + subset_rect_text_width_ + x_border_) + x_border_;
        glColor3f(dataset_->subset_colors[i].redF(), dataset_->subset_colors[i].greenF(), dataset_->subset_colors[i].blueF());
        glRectf(x, subset_rect_y_value_, x + subset_rect_width_, subset_rect_y_value_ + subset_rect_height_);

        int text_x = (x + subset_rect_width_) * this->width() + 5;
        int text_y = (1.0 - subset_rect_y_value_) * this->height();
        glColor3f(0.0, 0.0, 0.0);
        this->renderText(text_x, text_y, dataset_->subset_names[i]);
    }
}

void ParallelCoordinate::PaintWeightCircles(){

}