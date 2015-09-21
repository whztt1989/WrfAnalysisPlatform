#include "site_map_view.h"
#include <QtGui/QMouseEvent>
#include "../WrfDataLoader/wrf_data_common.h"
#include "mean_error_matrix.h"
#include "hierarchical_cluster.h"

SiteMapView::SiteMapView()
    : value_map_(NULL), data_model_(NULL){
    bin_count_ = 3;
    pie_count_ = 8;
    radius_ = 2.0;

    selection_data_model_ = new ErrorMatrixDataModel;

    status_ = NORMAL | SELECTION_CONTOUR;
}

SiteMapView::~SiteMapView(){

}

void SiteMapView::SetData(WrfDiscreteValueMap* map, ErrorMatrixDataModel* model){
    value_map_ = map;
    data_model_ = model;

    // copy info to selected_data_model_
    selection_data_model_->axis_names.assign(data_model_->axis_names.begin(), data_model_->axis_names.end());
    selection_data_model_->axis_min_values.assign(data_model_->axis_min_values.begin(), data_model_->axis_min_values.end());
    selection_data_model_->axis_max_values.assign(data_model_->axis_max_values.begin(), data_model_->axis_max_values.end());
    selection_data_model_->axis_value_size.assign(data_model_->axis_value_size.begin(), data_model_->axis_value_size.end());
    selection_data_model_->axis_value_step.assign(data_model_->axis_value_step.begin(), data_model_->axis_value_step.end());
    selection_data_model_->values.resize(data_model_->values.size(), 0);
    selection_data_model_->min_value = 0;
    selection_data_model_->max_value = 1;

    is_highlight_.resize(map->values.size());
    is_highlight_.assign(map->values.size(), false);

    is_point_selected_.resize(map->values.size(), false);

    UpdateViewport();

    UpdateSampleValues();
    Triangulation();

    this->updateGL();
}

void SiteMapView::initializeGL(){
    if ( glewInit() != GLEW_OK ) exit(0);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void SiteMapView::resizeGL(int w, int h){
    UpdateViewport();
}

void SiteMapView::paintGL(){
   glViewport(0, 0, this->width(), this->height());
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(left_, right_, bottom_, top_, 1, 100);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -4);

   glClear(GL_COLOR_BUFFER_BIT);
   if ( value_map_ == NULL ) return;

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glPointSize(10);
   for ( int i = 0; i < value_map_->values.size(); ++i ){
       float x = value_map_->values[i].longitude;
       float y = value_map_->values[i].latitude;
    
       /*if ( is_point_selected_[i] )
           glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
       else
           glColor4f(1.0f, 0.0f, 0.0f, 1.0f);*/

       QColor color1 = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0, 1, sample_radius_variances_[i]);
       glColor3f(color1.redF(), color1.greenF(), color1.blueF());

       glBegin(GL_POINTS);
       glVertex3f(x, y, 0.0f);
       glEnd();
   }

   // draw pies
   /*for ( int i = 0; i < sample_radius_values_.size(); ++i ){
       for ( int bin = bin_count_ - 1; bin >= 0 ; --bin )
           for ( int pie = 0; pie < pie_count_; ++pie ){
               float value = sample_radius_values_[i][pie * bin_count_ + bin];
               glTranslatef(value_map_->values[i].longitude, value_map_->values[i].latitude, 0.0);
               glColor3f(value, value, value);
               float begin_theta = 2 * 3.14159 * pie / pie_count_;
               float end_theta = 2 * 3.14159 * (pie + 1) / pie_count_;
               float step_theta = (end_theta - begin_theta) / 3;
               for ( int k = 0; k < 3; ++k ){
                   float current_theta = begin_theta + k * step_theta;
                   float radius = (bin + 1.0) / bin_count_ * 1.0;
                   glBegin(GL_TRIANGLES);
                   glVertex3f(0, 0, 0);
                   glVertex3f(radius * cos(current_theta), radius * sin(current_theta), 0.0);
                   glVertex3f(radius * cos(current_theta + step_theta), radius * sin(current_theta + step_theta), 0.0);
                   glEnd();
               }
               glTranslatef(-1 * value_map_->values[i].longitude, -1 * value_map_->values[i].latitude, 0.0);
           }
   }*/
   

   /*for ( int i = 0; i < cluster_radius_values_.size(); ++i ){
       for ( int bin = bin_count_ - 1; bin >= 0 ; --bin )
           for ( int pie = 0; pie < pie_count_; ++pie ){
               float value = cluster_radius_values_[i][pie * bin_count_ + bin];
               glTranslatef(optimized_positions_[2 * i], optimized_positions_[2 * i + 1], 0.0);
               glColor4f(value, value, value, 1.0);
               float begin_theta = 2 * 3.14159 * pie / pie_count_;
               float end_theta = 2 * 3.14159 * (pie + 1) / pie_count_;
               float step_theta = (end_theta - begin_theta) / 3;
               for ( int k = 0; k < 3; ++k ){
                   float current_theta = begin_theta + k * step_theta;
                   float radius = (bin + 1.0) / bin_count_ * radius_;
                   glBegin(GL_TRIANGLES);
                   glVertex3f(0, 0, 0);
                   glVertex3f(radius * cos(current_theta), radius * sin(current_theta), 0.0);
                   glVertex3f(radius * cos(current_theta + step_theta), radius * sin(current_theta + step_theta), 0.0);
                   glEnd();
               }
               glTranslatef(-1 * optimized_positions_[2 * i], -1 * optimized_positions_[2 * i + 1], 0.0);
           }
   }

   GLUquadricObj *qobj;
   qobj = gluNewQuadric();
   for ( int i = 0; i < cluster_radius_values_.size(); ++i ){
       glColor4f(0.0, 0.0, 1.0, 1.0);
       glTranslatef(cluster_positions_[2 * i], cluster_positions_[2 * i + 1], 0.0);
       gluSphere(qobj, radius_ / 10, 25, 25);
       glTranslatef(-1 * cluster_positions_[2 * i], -1 * cluster_positions_[2 * i + 1], 0.0);

       glColor4f(0.0, 1.0, 0.0, 1.0);
       glBegin(GL_LINES);
       glVertex3f(cluster_positions_[2 * i], cluster_positions_[2 * i + 1], 0.0);
       glVertex3f(optimized_positions_[2 * i], optimized_positions_[2 * i + 1], 0.0);
       glEnd();
   }*/
    
    // draw triangle result
    glColor4f(1.0, 1.0, 0.0, 0.5);
    glBegin(GL_LINES);
    for ( int i = 0; i < triangle_out.numberoftriangles; ++i ){
        int id1 = triangle_out.trianglelist[i * 3];
        int id2 = triangle_out.trianglelist[i * 3 + 1];
        int id3 = triangle_out.trianglelist[i * 3 + 2];
        glVertex3f(triangle_out.pointlist[id1 * 2], triangle_out.pointlist[id1 * 2 + 1], 0.0);
        glVertex3f(triangle_out.pointlist[id2 * 2], triangle_out.pointlist[id2 * 2 + 1], 0.0);

        glVertex3f(triangle_out.pointlist[id2 * 2], triangle_out.pointlist[id2 * 2 + 1], 0.0);
        glVertex3f(triangle_out.pointlist[id3 * 2], triangle_out.pointlist[id3 * 2 + 1], 0.0);

        glVertex3f(triangle_out.pointlist[id3 * 2], triangle_out.pointlist[id3 * 2 + 1], 0.0);
        glVertex3f(triangle_out.pointlist[id1 * 2], triangle_out.pointlist[id1 * 2 + 1], 0.0);
    }
    glEnd();

    // draw triangle variance
    /*glBegin(GL_TRIANGLES);
    for ( int i = 0; i < triangle_out.numberoftriangles; ++i ){
        int id1 = triangle_out.trianglelist[i * 3];
        int id2 = triangle_out.trianglelist[i * 3 + 1];
        int id3 = triangle_out.trianglelist[i * 3 + 2];

        QColor color1 = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0, 1, sample_radius_variances_[id1]);
        glColor3f(color1.redF(), color1.greenF(), color1.blueF());
        glVertex3f(triangle_out.pointlist[id1 * 2], triangle_out.pointlist[id1 * 2 + 1], 0.0);
        QColor color2 = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0, 1, sample_radius_variances_[id2]);
        glColor3f(color2.redF(), color2.greenF(), color2.blueF());
        glVertex3f(triangle_out.pointlist[id2 * 2], triangle_out.pointlist[id2 * 2 + 1], 0.0);
        QColor color3 = QColorBarController::GetInstance(HEAT_MAP)->GetColor(0, 1, sample_radius_variances_[id3]);
        glColor3f(color3.redF(), color3.greenF(), color3.blueF());
        glVertex3f(triangle_out.pointlist[id3 * 2], triangle_out.pointlist[id3 * 2 + 1], 0.0);
    }
    glEnd();*/

    /*glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINES);
    for ( int i = 0; i < triangle_vorout.numberofedges; ++i ){
        int id1 = triangle_vorout.edgelist[i * 2];
        int id2 = triangle_vorout.edgelist[i * 2 + 1];
        glVertex3f(triangle_vorout.pointlist[id1 * 2], triangle_vorout.pointlist[id1 * 2 + 1], 0.0);
        glVertex3f(triangle_vorout.pointlist[id2 * 2], triangle_vorout.pointlist[id2 * 2 + 1], 0.0);
    }
    glEnd();*/

   if ( status_ ^ SELECTION_CONTOUR != 0 ) DrawSelectionContour();

    glDisable(GL_BLEND);

    DrawPlottingScale();
}

void SiteMapView::UpdateViewport(){
    if ( value_map_ != NULL ){
        left_ = 1e10;
        right_ = -1e10;
        bottom_ = 1e10;
        top_ = -1e10;
        for ( int i = 0; i < value_map_->values.size(); ++i ){
            float x = value_map_->values[i].longitude;
            float y = value_map_->values[i].latitude;
            if ( x < left_ ) left_ = x;
            if ( x > right_ ) right_ = x;
            if ( y < bottom_ ) bottom_ = y;
            if ( y > top_ ) top_ = y;
        }

        float width = right_ - left_;
        float height = top_ - bottom_;
        if ( width / this->width() > height / this->height() ){
            height = width / this->width() * this->height();
        } else {
            width = height / this->height() * this->width();
        }

        float w_center = (left_ + right_) / 2;
        left_ = w_center - width * 0.6;
        right_ = w_center + width * 0.6;
        float h_center = (top_ + bottom_) / 2;
        top_ = h_center + height * 0.6;
        bottom_ = h_center - height * 0.6;
    } else {
        left_ = 0;
        right_ = 1;
        bottom_ = 0;
        top_ = 1;
    }
}

void SiteMapView::SetParameterRange(std::vector< int >& para_ranges){
    para_ranges_.assign(para_ranges.begin(), para_ranges.end());

    UpdateSampleValues();
}

void SiteMapView::UpdateSampleValues(){
    float sample_rate = 0.2;
    int sample_size = (int)data_model_->volume_values[0].size() * sample_rate;

    sample_para_values_.resize(value_map_->values.size());
    for ( int i = 0; i < sample_para_values_.size(); ++i ) {
        sample_para_values_[i].resize(sample_size);
    }

    std::vector< int > sort_index;
    sort_index.resize(data_model_->volume_values[0].size());

    for ( int t = 0; t < data_model_->volume_values.size(); ++t ){
        for ( int k = 0; k < sort_index.size(); ++k ) sort_index[k] = k;
        // sort the values
        for ( int i = 0; i < data_model_->volume_values[t].size() - 1; ++i )
            for ( int j = i + 1; j < data_model_->volume_values[t].size(); ++j )
                if ( data_model_->volume_values[t][sort_index[i]] > data_model_->volume_values[t][sort_index[j]] ){
                    int temp_index = sort_index[j];
                    sort_index[j] = sort_index[i];
                    sort_index[i] = temp_index;
                }
         
        // copy the first sample_size indexes
        for ( int i = 0; i < sample_size; ++i ) sample_para_values_[t][i] = sort_index[i];
    }

    // MDS
    cv::Mat data_map(sample_para_values_.size() * sample_para_values_[0].size(), data_model_->axis_names.size(), CV_32F);

    int data_map_index = 0;
    std::vector< float > temp_para_values;
    temp_para_values.resize(data_model_->axis_names.size());
    for ( int i = 0; i < sample_para_values_.size(); ++i )
        for ( int j = 0; j < sample_para_values_[0].size(); ++j ){
            ConvertIndex2ParaValues(sample_para_values_[i][j], temp_para_values);
            for ( int k = 0; k < data_model_->axis_names.size(); ++k )
                data_map.at<float>(data_map_index, k) = temp_para_values[k];
        }
    cv::PCA pca(data_map, cv::Mat(), 0);
    std::vector< std::vector< float > > scaled_vec;
    scaled_vec.resize(2);
    for ( int i = 0; i < scaled_vec.size(); ++i ) scaled_vec[i].resize(data_model_->axis_names.size());
    for ( int i = 0; i < 2; ++i )
        for ( int j = 0; j < data_model_->axis_names.size(); ++j )
            scaled_vec[i][j] = pca.eigenvectors.at<float>(i, j);

    // acquire the 2d representation value for each sample point
    float min_value[2];
    min_value[0] = 1e10; min_value[1] = 1e10;
    float max_value[2];
    max_value[0] = -1e10; max_value[1] = -1e10;
    
    sample_pos_values_.resize(sample_para_values_.size());
    for ( int i = 0; i < sample_para_values_.size(); ++i ){
        sample_pos_values_[i].resize(sample_para_values_[i].size() * 3);

        for ( int j = 0; j < sample_para_values_[i].size(); ++j ){
            int temp_index = sample_para_values_[i][j];
            ConvertIndex2ParaValues(temp_index, temp_para_values);
            int temp_value[2];
            temp_value[0] = 0;
            temp_value[1] = 0;
            for ( int k = 0; k < temp_para_values.size(); ++k ){
                temp_value[0] += temp_para_values[k] * scaled_vec[0][k];
                temp_value[1] += temp_para_values[k] * scaled_vec[1][k];
            }
            if ( min_value[0] > temp_value[0] ) min_value[0] = temp_value[0];
            if ( max_value[0] < temp_value[0] ) max_value[0] = temp_value[0];
            if ( min_value[1] > temp_value[1] ) min_value[1] = temp_value[1];
            if ( max_value[1] < temp_value[1] ) max_value[1] = temp_value[1];
            sample_pos_values_[i][j * 3] = temp_value[0];
            sample_pos_values_[i][j * 3 + 1] = temp_value[1];
            sample_pos_values_[i][j * 3 + 2] = data_model_->volume_values[i][temp_index];
        }
    }

    sample_radius_values_.resize(sample_para_values_.size());
    for (int i = 0; i < sample_para_values_.size(); ++i ) {
        sample_radius_values_[i].resize(bin_count_ * pie_count_ + 2);
        memset(sample_radius_values_[i].data(), 0, sizeof(float) * sample_radius_values_[i].size());
    }

    float value_range0 = max_value[0] - min_value[0];
    float value_range1 = max_value[1] - min_value[1];
    for ( int i = 0; i < sample_para_values_.size(); ++i ){
        for ( int j = 0; j < sample_para_values_[i].size(); ++j ){
            sample_pos_values_[i][j * 3] = (sample_pos_values_[i][j * 3] - min_value[0]) / value_range0;
            sample_pos_values_[i][j * 3 + 1] = (sample_pos_values_[i][j * 3 + 1] - min_value[1]) / value_range1;
            
            int bin_index = (int)(sample_pos_values_[i][j * 3] * bin_count_);
            if ( bin_index > bin_count_ - 1 ) bin_index = bin_count_ - 1;
            int pie_index = (int)(sample_pos_values_[i][j * 3 + 1] * pie_count_);
            if ( pie_index > pie_count_ - 1 ) pie_index = pie_count_ - 1;

            sample_radius_values_[i][pie_index * bin_count_ + bin_index] += 1;
        }
        for ( int j = 0; j < sample_radius_values_[i].size(); ++j )
            sample_radius_values_[i][j] /= sample_para_values_[i].size();
    }

    for ( int i = 0; i < sample_radius_values_.size(); ++i ){
        sample_radius_values_[i][sample_radius_values_[i].size() - 2] = (value_map_->values[i].longitude - value_map_->min_longitude) / (value_map_->max_longitude - value_map_->min_longitude);
        sample_radius_values_[i][sample_radius_values_[i].size() - 1] = (value_map_->values[i].latitude - value_map_->min_latitude) / (value_map_->max_latitude - value_map_->min_latitude);
    }
}

void SiteMapView::ConvertIndex2ParaValues(int index, std::vector< float >& para_values){
    int current_index = data_model_->axis_names.size() - 1;
    while ( index != 0 ){
        para_values[current_index] = index % data_model_->axis_value_size[current_index];
        index /= data_model_->axis_value_size[current_index];
        current_index--;
    }
}

void SiteMapView::Triangulation(){
    triangle_in.numberofpoints = value_map_->values.size();
    triangle_in.numberofpointattributes = 0;
    triangle_in.pointlist = (float *)malloc(triangle_in.numberofpoints * 2 * sizeof(float));
    for ( int i = 0; i < value_map_->values.size(); ++i ){
        triangle_in.pointlist[i * 2] = value_map_->values[i].longitude;
        triangle_in.pointlist[i * 2 + 1] = value_map_->values[i].latitude;
    }
    triangle_in.pointattributelist = NULL;
    triangle_in.pointmarkerlist = NULL;

    triangle_in.numberofsegments = 0;
    triangle_in.numberofholes = 0;
    triangle_in.numberofregions = 1;
    triangle_in.regionlist = (float *) malloc(triangle_in.numberofregions * 4 * sizeof(float));
    triangle_in.regionlist[0] = 0.5;
    triangle_in.regionlist[1] = 5.0;
    triangle_in.regionlist[2] = 7.0;            /* Regional attribute (for whole mesh). */
    triangle_in.regionlist[3] = 0.1;          /* Area constraint that will not be used. */

    triangle_mid.pointlist = (float *) NULL;            /* Not needed if -N switch used. */
    /* Not needed if -N switch used or number of point attributes is zero: */
    triangle_mid.pointattributelist = (float *) NULL;
    triangle_mid.pointmarkerlist = (int *) NULL; /* Not needed if -N or -B switch used. */
    triangle_mid.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
    /* Not needed if -E switch used or number of triangle attributes is zero: */
    triangle_mid.triangleattributelist = (float *) NULL;
    triangle_mid.neighborlist = (int *) NULL;         /* Needed only if -n switch used. */
    /* Needed only if segments are output (-p or -c) and -P not used: */
    triangle_mid.segmentlist = (int *) NULL;
    /* Needed only if segments are output (-p or -c) and -P and -B not used: */
    triangle_mid.segmentmarkerlist = (int *) NULL;
    triangle_mid.edgelist = (int *) NULL;             /* Needed only if -e switch used. */
    triangle_mid.edgemarkerlist = (int *) NULL;   /* Needed if -e used and -B not used. */

    triangle_vorout.pointlist = (float *) NULL;        /* Needed only if -v switch used. */
    /* Needed only if -v switch used and number of attributes is not zero: */
    triangle_vorout.pointattributelist = (float *) NULL;
    triangle_vorout.edgelist = (int *) NULL;          /* Needed only if -v switch used. */
    triangle_vorout.normlist = (float *) NULL;         /* Needed only if -v switch used. */

    triangulate("pczAevn", &triangle_in, &triangle_mid, &triangle_vorout);

    triangle_out.pointlist = (float *) NULL;            /* Not needed if -N switch used. */
    /* Not needed if -N switch used or number of attributes is zero: */
    triangle_out.pointattributelist = (float *) NULL;
    triangle_out.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
    /* Not needed if -E switch used or number of triangle attributes is zero: */
    triangle_out.triangleattributelist = (float *) NULL;

    /* Refine the triangulation according to the attached */
    /*   triangle area constraints.                       */

    triangulate("pzBP", &triangle_mid, &triangle_out, (struct triangulateio *) NULL);

    connect_status_.resize(value_map_->values.size());
    for ( int i = 0; i < connect_status_.size(); ++i ){
        connect_status_[i].resize(value_map_->values.size(), false);
    }
    for ( int i = 0; i < triangle_out.numberoftriangles; ++i ){
        int id1 = triangle_out.trianglelist[i * 3];
        int id2 = triangle_out.trianglelist[i * 3 + 1];
        int id3 = triangle_out.trianglelist[i * 3 + 2];
        connect_status_[id1][id2] = true;
        connect_status_[id2][id1] = true;
        connect_status_[id2][id3] = true;
        connect_status_[id3][id2] = true;
        connect_status_[id1][id3] = true;
        connect_status_[id3][id1] = true;
    }

    HierarchicalCluster::Cluster(value_map_, sample_radius_values_, connect_status_, 15, cluster_radius_values_, cluster_nodes);
    cluster_positions_.resize(cluster_radius_values_.size() * 2);
    for (int i = 0; i < cluster_radius_values_.size(); ++i ){
        float x = 0;
        float y = 0;
        for ( int j = 0; j < cluster_nodes[i].size(); ++j ){
            x += value_map_->values[cluster_nodes[i][j]].longitude;
            y += value_map_->values[cluster_nodes[i][j]].latitude;
        }

        x /= cluster_nodes[i].size();
        y /= cluster_nodes[i].size();
        cluster_positions_[2 * i] = x;
        cluster_positions_[2 * i + 1] = y;
    }

    UpdateIconPosition();
    UpdateNodeParaVariance();
}

void SiteMapView::UpdateIconPosition(){
    optimized_positions_.assign(cluster_positions_.begin(), cluster_positions_.end());

    bool is_collision_happened = true;
    int optimized_run = 0;
    std::vector< std::vector< float > > step_translation_force;
    step_translation_force.resize(cluster_positions_.size() / 2);
    while ( is_collision_happened && optimized_run < 5 ){
        is_collision_happened = false;
        for ( int i = 0; i < step_translation_force.size(); ++i ) step_translation_force[i].clear();
        for ( int i = 0; i < optimized_positions_.size() / 2 - 1; ++i )
            for ( int j = i + 1; j < optimized_positions_.size() / 2; ++j ){
                float x_t = optimized_positions_[2 * i] - optimized_positions_[2 * j];
                float y_t = optimized_positions_[2 * i + 1] - optimized_positions_[2 * j + 1];
                float dis = sqrt(pow(x_t, 2) + pow(y_t, 2));
                if ( dis < 2 * radius_ ){
                    is_collision_happened = true;
                    if ( dis < 1e-5 ){
                        x_t = 1.0;
                        y_t = 0.0;
                    } else {
                        x_t /= dis;
                        y_t /= dis;
                    }
                    step_translation_force[i].push_back(x_t * (radius_ * 1.5 - dis / 2));
                    step_translation_force[i].push_back(y_t * (radius_ * 1.5 - dis / 2));
                    step_translation_force[i].push_back(dis);

                    step_translation_force[j].push_back(-1 * x_t * (radius_ * 1.5 - dis / 2));
                    step_translation_force[j].push_back(-1 * y_t * (radius_ * 1.5 - dis / 2));
                    step_translation_force[j].push_back(dis);
                }
            }
        for ( int i = 0; i < step_translation_force.size(); ++i ){
            if ( step_translation_force[i].size() == 0 ) continue;
            float accu_x = 0, accu_y = 0, accu_weight = 0;
            for ( int j = 0; j < step_translation_force[i].size() / 3; ++j ){
                float temp_weight = 1.0 / (step_translation_force[i][j * 3 + 2] + 0.1);
                accu_x += step_translation_force[i][j * 3] * temp_weight;
                accu_y += step_translation_force[i][j * 3 + 1] * temp_weight;
                accu_weight += temp_weight;
            }
            accu_x /= accu_weight;
            accu_y /= accu_weight;

            optimized_positions_[i * 2] += accu_x;
            optimized_positions_[i * 2 + 1] += accu_y;
        }

        optimized_run++;
    }
}

void SiteMapView::UpdateNodeParaVariance(){
    node_radius_variance_.resize(value_map_->values.size());
    for ( int i = 0; i < triangle_out.numberoftriangles; ++i ){
        int id1 = triangle_out.trianglelist[i * 3];
        int id2 = triangle_out.trianglelist[i * 3 + 1];
        int id3 = triangle_out.trianglelist[i * 3 + 2];

        double var1 = 0, var2 = 0, var3 = 0;
        for ( int j = 0;  j < sample_radius_values_[0].size() - 2; ++j ){
            var1 += abs(sample_radius_values_[id1][j] - sample_radius_values_[id2][j]);
            var2 += abs(sample_radius_values_[id1][j] - sample_radius_values_[id3][j]);
            var3 += abs(sample_radius_values_[id2][j] - sample_radius_values_[id3][j]);
        }
        var1 = pow(var1 / 2, 2);
        var2 = pow(var2 / 2, 2);
        var3 = pow(var3 / 2, 2);

        node_radius_variance_[id1].push_back(var1);
        node_radius_variance_[id1].push_back(var2);

        node_radius_variance_[id2].push_back(var1);
        node_radius_variance_[id2].push_back(var3);

        node_radius_variance_[id3].push_back(var2);
        node_radius_variance_[id3].push_back(var3);
    }

    sample_radius_variances_.resize(value_map_->values.size(), 0);
    for ( int i = 0; i < value_map_->values.size(); ++i ){
        sample_radius_variances_[i] = 0;
        for ( int j = 0; j < node_radius_variance_[i].size(); ++j )
            sample_radius_variances_[i] += node_radius_variance_[i][j];
        sample_radius_variances_[i] /= node_radius_variance_[i].size();
        sample_radius_variances_[i] = sqrt(sample_radius_variances_[i]);
    }
}

void SiteMapView::mouseDoubleClickEvent(QMouseEvent *event){
    float x = event->x();
    float y = event->y();

    x = x / this->width() * (right_ - left_) + left_;
    y = top_ - y / this->height() * (top_ - bottom_);

    float min_distance = 1e10;
    int min_index = -1;
    for ( int i = 0; i < cluster_positions_.size() / 2; ++i ){
        float temp_dis = abs(x - cluster_positions_[2 * i]) + abs(y - cluster_positions_[2 * i + 1]);
        if ( temp_dis < min_distance ){
            min_distance = temp_dis;
            min_index = i;
        }
    }

    current_selected_cluster_ = min_index;

    // update selected_data_model_
    std::vector< std::vector< int > > accu_size;
    accu_size.resize(selection_data_model_->axis_names.size() - 1);
    for ( int i = 0; i < accu_size.size(); ++i ) accu_size[i].resize(selection_data_model_->axis_names.size());
    for ( int i = 0; i < selection_data_model_->axis_names.size() - 1; ++i )
        for ( int j = i + 1; j < selection_data_model_->axis_names.size(); ++j ){
            if ( j == i + 1 ){ 
                if ( i != 0 ){
                    accu_size[i][j] = selection_data_model_->axis_value_size[i - 1] * selection_data_model_->axis_value_size[selection_data_model_->axis_names.size() - 1];
                    accu_size[i][j] += accu_size[i - 1][selection_data_model_->axis_names.size() - 1];
                } else {
                    accu_size[i][j] = 0;
                }
            } else {
                accu_size[i][j] = selection_data_model_->axis_value_size[i] * selection_data_model_->axis_value_size[j - 1];
                accu_size[i][j] += accu_size[i][j - 1];
            }
        }

    selection_data_model_->min_value = 1e10;
    selection_data_model_->max_value = -1e10;
    memset(selection_data_model_->values.data(), 0, selection_data_model_->values.size() * sizeof(float));
    std::vector< int > para_vec;
    para_vec.resize(selection_data_model_->axis_names.size());
    for ( int i = 0; i < cluster_nodes[current_selected_cluster_].size(); ++i ){
        int node_index = cluster_nodes[current_selected_cluster_][i];
        for ( int j = 0; j < sample_para_values_[node_index].size(); ++j ){
            int para_index = sample_para_values_[node_index][j];
            memset(para_vec.data(), 0, para_vec.size() * sizeof(float));
            int current_para_pos = para_vec.size() - 1;
            while ( para_index != 0 ){
                para_vec[current_para_pos] = para_index % selection_data_model_->axis_value_size[current_para_pos];
                para_index /= selection_data_model_->axis_value_size[current_para_pos];
                current_para_pos--;
            }

            for ( int pre = 0; pre < selection_data_model_->axis_names.size() - 1; ++pre)
                for ( int next = pre + 1; next < selection_data_model_->axis_names.size(); ++next){
                    selection_data_model_->values[accu_size[pre][next] + para_vec[pre] * selection_data_model_->axis_value_size[next] + para_vec[next]]++;
                }
        }
    }

    for ( int i = 0; i < selection_data_model_->values.size(); ++i ){
        if ( selection_data_model_->values[i] > selection_data_model_->max_value ) selection_data_model_->max_value = selection_data_model_->values[i];
        if ( selection_data_model_->values[i] < selection_data_model_->min_value ) selection_data_model_->min_value = selection_data_model_->values[i];
    }

    if ( current_selected_cluster_ != -1 ){
        is_highlight_.assign(is_highlight_.size(), false);
        for ( int i = 0; i < cluster_nodes[current_selected_cluster_].size(); ++i )
            is_highlight_[cluster_nodes[current_selected_cluster_][i]] = true;
        emit ClusterSelected();
    }

    this->updateGL();
}

void SiteMapView::mousePressEvent(QMouseEvent *event){
    if ( status_ ^ SELECTION_CONTOUR != 0 ){
        selecting_region_contour_.clear();
        int x, y;
        float longitude, latitude;
        x = event->x();
        y = event->y();
        ConvertViewPos2MapPos(x, y, longitude, latitude);
        selecting_region_contour_.push_back(longitude);
        selecting_region_contour_.push_back(latitude);
    }
}

void SiteMapView::mouseMoveEvent(QMouseEvent *event){
    if ( status_ ^ SELECTION_CONTOUR != 0 ){
        int x, y;
        float longitude, latitude;
        x = event->x();
        y = event->y();
        ConvertViewPos2MapPos(x, y, longitude, latitude);
        selecting_region_contour_.push_back(longitude);
        selecting_region_contour_.push_back(latitude);
    }

    this->updateGL();
}

void SiteMapView::mouseReleaseEvent(QMouseEvent *event){
    if ( status_ ^ SELECTION_CONTOUR != 0 ){
        this->GetSelectingPointIds(is_point_selected_);

        this->updateGL();
    }
}

void SiteMapView::ConvertViewPos2MapPos(int x, int y, float& longitude, float& latitude){
    longitude = left_ + (float)x / this->width() * (right_ - left_);
    latitude = bottom_ + (float)(this->height() - y) / this->height() * (top_ - bottom_);
}

void SiteMapView::DrawColorBar(){

}

void SiteMapView::DrawSelectionContour(){
    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for ( int i = 0; i < selecting_region_contour_.size() / 2; ++i ){
        glVertex3f(selecting_region_contour_[i * 2], selecting_region_contour_[i * 2 + 1], 0.0);
    }
    glEnd();
}

void SiteMapView::DrawPlottingScale(){
    glLineWidth(4.0);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(left_ + 10, bottom_ + 10, 0);
    glVertex3f(left_ + 10 + data_model_->axis_min_values[0], bottom_ + 10, 0);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(left_ + 10 + data_model_->axis_min_values[0], bottom_ + 10, 0);
    glVertex3f(left_ + 10 + data_model_->axis_max_values[0], bottom_ + 10, 0);
    glEnd();
}

void SiteMapView::GetSelectingPointIds(std::vector< bool >& is_selected){
    is_selected.resize(value_map_->values.size());
    is_selected.assign(is_selected.size(), false);

    for ( int i = 0; i < value_map_->values.size(); ++i ){
        float longitude = value_map_->values[i].longitude;
        float latitude = value_map_->values[i].latitude;
        int left_count = 0;
        for ( int j = 0; j < selecting_region_contour_.size() / 2; ++j ){
            float y1 = selecting_region_contour_[2 * j + 1];
            float y2 = selecting_region_contour_[2 * ((j + 1) % (selecting_region_contour_.size() / 2)) + 1];
            if ( (y1 - latitude) * (y2 - latitude) < 0 ){
                float rate = (latitude - y1) / (y2 - y1);
                float x = selecting_region_contour_[2 * j] + rate * (selecting_region_contour_[2 *((j + 1) % (selecting_region_contour_.size() / 2))] - selecting_region_contour_[2 * j]);
                if ( x <= longitude ) left_count++;
            }
        }

        if ( left_count % 2 != 0 ) is_selected[i] = true;
    }
}