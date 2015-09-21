#ifndef SITE_MAP_VIEW_H_
#define SITE_MAP_VIEW_H_

#include "gl/glew.h"
#include <opencv2/core/core.hpp> 
#include <QtOpenGL/QGLWidget>
#include <vector>

#include "triangle.h"

class WrfDiscreteValueMap;
class ErrorMatrixDataModel;

class SiteMapView : public QGLWidget
{
    Q_OBJECT

public:
    SiteMapView();
    ~SiteMapView();

    void SetData(WrfDiscreteValueMap* map, ErrorMatrixDataModel* model);
    void SetParameterRange(std::vector< int >& para_ranges);
    ErrorMatrixDataModel* GetSelectionDataModel() { return selection_data_model_; }

    enum SiteMapStatus{
        NORMAL = 0x1,
        SELECTION_CONTOUR = 0x10
    };

signals:
    void ClusterSelected();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    WrfDiscreteValueMap* value_map_;
    ErrorMatrixDataModel* data_model_;

    ErrorMatrixDataModel* selection_data_model_;

    unsigned int status_;

    float left_, right_, bottom_, top_;
    std::vector< int > para_ranges_;
    int bin_count_, pie_count_;

    std::vector< bool > is_highlight_;

    float radius_;

    struct triangulateio triangle_in, triangle_mid, triangle_out, triangle_vorout;

    std::vector< std::vector< int > > sample_para_values_;
    std::vector< std::vector< float > > sample_pos_values_;
    std::vector< std::vector< float > > sample_radius_values_;
    std::vector< std::vector< bool > > connect_status_;

    std::vector< std::vector< float > > cluster_radius_values_;
    std::vector< std::vector< int > > cluster_nodes;
    std::vector< float > cluster_positions_;
    std::vector< float > optimized_positions_;

    std::vector< std::vector< float > > node_radius_variance_;
    std::vector< float > sample_radius_variances_;

    int current_selected_cluster_;

    std::vector< float > selecting_region_contour_;
    std::vector< bool > is_point_selected_;

    void UpdateViewport();
    void UpdateSampleValues();
    void ConvertIndex2ParaValues(int index, std::vector< float >& para_values);
    void Triangulation();
    void UpdateIconPosition();
    void UpdateNodeParaVariance();
    void ConvertViewPos2MapPos(int x, int y, float& longitude, float& latitude);

    void GetSelectingPointIds(std::vector< bool >& is_selected);

    void DrawColorBar();
    void DrawPlottingScale();
    void DrawSelectionContour();
};

#endif