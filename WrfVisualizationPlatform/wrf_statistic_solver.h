#ifndef WRF_STATISTIC_SOLVER_H_
#define WRF_STATISTIC_SOLVER_H_

#include <vector>
#include <QtCore/QPointF>
#include "wrf_data_common.h"

class WrfLevelImage;

class WrfStatisticSolver 
{
public:
    WrfStatisticSolver();
    ~WrfStatisticSolver();

    // Calculate the uncertainty map
    static WrfGridValueMap* GetUncertaintyMap(std::vector< WrfGridValueMap* >& maps, float normalize_value = 1.0);

    // Generate the level image using specified colors
    static WrfLevelImage* GenerateLevelImage(WrfGridValueMap* map, std::vector< float >& colors, std::vector< float >& level_values);

    // Cluster images
    static bool GenerateCluster(std::vector< WrfGridValueMap* >& value_maps, std::vector< std::vector< int > >& cluster_index);
    static bool Cluster(std::vector< std::vector< std::vector< float > > >& volume_values, std::vector< std::vector< int > >& cluster_index);

    // Generate overall cluster image based on element cluster images
    static WrfLevelImage* GenerateCluster(std::vector< WrfLevelImage* >& cluster_images);

    static WrfGridValueMap* Convert2Map(WrfGridValueMap* map, MapRange& new_map_range);

    static float GetSimilarity(WrfGridValueMap* map_one, WrfGridValueMap* map_two, std::vector< int > selected_grid_index);

    static void GetSelectedGridIndex(std::vector< QPointF >& region_contour_, 
        float start_x, float start_y, float space_x, float space_y, int x_grid, int y_grid, 
        std::vector< int >& selected_index);

    static void Sort(std::vector< float >& value, std::vector< int >& index);
    static void Sort(std::vector< float >& value, int* index);

    static WrfGridValueMap* GeneratePqpfResult(std::vector< int >& retrieval_time_vec, ProbabilisticPara& para, int fcst_hour);

    static WrfGridValueMap* GenerateAverageResult(std::vector< int >& retrieval_time_vec, WrfElementType element, int fcst_hour);

	static void GenerateLineCurvature(std::vector< float >& values, int window_size, std::vector< float >& curvature);

	static void GaussianSmooth(std::vector< float >& value, int window_size);
};

#endif