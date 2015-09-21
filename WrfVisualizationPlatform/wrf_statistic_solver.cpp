#include "wrf_statistic_solver.h"
#include "wrf_data_common.h"
#include "wrf_data_manager.h"

WrfStatisticSolver::WrfStatisticSolver(){

}

WrfStatisticSolver::~WrfStatisticSolver(){

}

WrfGridValueMap* WrfStatisticSolver::GetUncertaintyMap(std::vector< WrfGridValueMap* >& maps, float normalize_value /* = 1.0 */){
    if ( maps.size() == 0 ) return NULL;

    WrfGridValueMap* uncertainty_map = new WrfGridValueMap(*maps[0]);
    uncertainty_map->model_type = WRF_ELEMENT_UNCERTAINTY;
    uncertainty_map->element_type = WRF_UNCERTAINTY;

    for ( int i = 0; i < maps[0]->map_range.x_grid_number * maps[0]->map_range.y_grid_number; ++i ){
        double average_value = 0.0;
        for ( int j = 0; j < maps.size(); ++j ) average_value += maps[j]->values[i];
        average_value /= maps.size();

        double std_dev = 0.0;
        for ( int j = 0; j < maps.size(); ++j ) std_dev += pow((maps[j]->values[i] - average_value) / normalize_value, 2);
        std_dev = sqrt(std_dev / maps.size());

        uncertainty_map->values[i] = std_dev;
    }

    return uncertainty_map;
}

WrfLevelImage* WrfStatisticSolver::GenerateLevelImage(WrfGridValueMap* map, std::vector< float >& colors, std::vector< float >& level_values){
    return NULL;
}

bool WrfStatisticSolver::GenerateCluster(std::vector< WrfGridValueMap* >& value_maps, std::vector< std::vector< int > >& cluster_index){
    if ( value_maps.size() == 0 ) return false;

    // convert maps to volume
    std::vector< std::vector< std::vector< float > > > volume_values;
    volume_values.resize(value_maps[0]->map_range.y_grid_number);
    for ( int i = 0; i < volume_values.size(); ++i ) volume_values[i].resize(value_maps[0]->map_range.x_grid_number);
    for ( int i = 0; i < volume_values.size(); ++i )
        for ( int j = 0; j < volume_values[i].size(); ++j )
            for ( int k = 0; k < value_maps.size(); ++k )
                volume_values[i][j].push_back(value_maps[k]->values[i * value_maps[k]->map_range.x_grid_number + j]);

    cluster_index.resize(value_maps[0]->map_range.y_grid_number);
    for ( int i = 0; i < cluster_index.size(); ++i ){
        cluster_index[i].resize(value_maps[0]->map_range.x_grid_number);
        cluster_index[i].assign(cluster_index[i].size(), -1);
    }

    return Cluster(volume_values, cluster_index);
}

bool WrfStatisticSolver::Cluster(std::vector< std::vector< std::vector< float > > >& volume_values, std::vector< std::vector< int > >& cluster_index){
    std::vector< std::vector< float > > cluster_centers;
    std::vector< std::vector< bool > > linking_status;
    cluster_centers.resize(volume_values.size() * volume_values[0].size());
    linking_status.resize(volume_values.size() * volume_values[0].size());
    for ( int i = 0; i < volume_values.size(); ++i )
        for ( int j = 0; j < volume_values[i].size(); ++j ){
            cluster_centers[i * volume_values[0].size() + j] = volume_values[i][j];
            cluster_index[i][j] = i * volume_values[0].size() + j;
            linking_status[i * volume_values[0].size() + j].resize(volume_values.size() * volume_values[0].size(), false);
        }
    for ( int i = 0; i < volume_values.size() - 1; ++i )
        for ( int j = 0; j < volume_values[i].size() - 1; ++j ){
            int index1 = i * volume_values[0].size() + j;
            int index2 = i * volume_values[0].size() + j + 1;
            int index3 = (i + 1) * volume_values[0].size() + j;
            linking_status[index1][index2] = true;
            linking_status[index1][index3] = true;
            linking_status[index2][index1] = true;
            linking_status[index3][index1] = true;
        }

    float threshold = 2.0;
    int max_cluster_num = 10;

    float current_distance_value = -10;

    while ( current_distance_value < threshold && cluster_centers.size() > max_cluster_num ){
        current_distance_value = 1e10;

        // find the nearest cluster
        float temp_distance_value;
        int min_dis_cluster_index[2];
        min_dis_cluster_index[0] = -1;
        min_dis_cluster_index[1] = -1;
        for ( int i = 0; i < cluster_centers.size() - 1; ++i )
            for ( int j = i + 1; j < cluster_centers.size(); ++j )
                if ( linking_status[i][j] ){
                    temp_distance_value = 0.0;
                    for ( int k = 0; k < cluster_centers[i].size(); ++k )
                        temp_distance_value += abs(cluster_centers[i][k] - cluster_centers[j][k]);
                    if ( temp_distance_value < current_distance_value ){
                        current_distance_value = temp_distance_value;
                        min_dis_cluster_index[0] = i;
                        min_dis_cluster_index[1] = j;
                    }
                }

        if ( min_dis_cluster_index[0] == -1 || min_dis_cluster_index[1] == -1 ) break;

        // combine cluster
        int cluster_size[2];
        cluster_size[0] = 0;
        cluster_size[1] = 0;
        for ( int i = 0; i < cluster_index.size(); ++i )
            for ( int j = 0; j < cluster_index[i].size(); ++j ){
                if ( cluster_index[i][j] == min_dis_cluster_index[1] ) {
                    cluster_index[i][j] = min_dis_cluster_index[0];
                    cluster_size[1]++;
                }
                if ( cluster_index[i][j] == min_dis_cluster_index[0] ) cluster_size[0]++;
            }
        for ( int i = 0; i < cluster_centers[min_dis_cluster_index[0]].size(); ++i )
            cluster_centers[min_dis_cluster_index[0]][i] = (cluster_centers[min_dis_cluster_index[0]][i] * cluster_size[0] + cluster_centers[min_dis_cluster_index[1]][i] * cluster_size[1]) / (cluster_size[0] + cluster_size[1]);
        for ( int i = min_dis_cluster_index[1]; i < cluster_centers.size() - 1; ++i )
            cluster_centers[i] = cluster_centers[i + 1];
        for ( int i = 0; i < cluster_index.size(); ++i )
            for ( int j = 0; j < cluster_index[i].size(); ++j )
                if ( cluster_index[i][j] > min_dis_cluster_index[1] ) cluster_index[i][j]--;

        // update linking status
        for ( int i = 0; i < linking_status.size(); ++i ) linking_status[i].assign(linking_status[i].size(), false);
        for ( int i = 0; i < volume_values.size() - 1; ++i )
            for ( int j = 0; j < volume_values[i].size() - 1; ++j ){
                int index1 = cluster_index[i][j];
                int index2 = cluster_index[i][j + 1];
                int index3 = cluster_index[i + 1][j];;
                linking_status[index1][index2] = true;
                linking_status[index1][index3] = true;
                linking_status[index2][index1] = true;
                linking_status[index3][index1] = true;
            }
    }

    return true;
}

WrfLevelImage* WrfStatisticSolver::GenerateCluster(std::vector< WrfLevelImage* >& cluster_images){

    return NULL;
}

WrfGridValueMap* WrfStatisticSolver::Convert2Map(WrfGridValueMap* map, MapRange& new_map_range){
    WrfGridValueMap* new_map = new WrfGridValueMap();
    new_map->map_range.start_y = new_map_range. start_y;
    new_map->map_range.end_y = new_map_range.end_y;
    new_map->map_range.start_x = new_map_range.start_x;
    new_map->map_range.end_x = new_map_range.end_x;
    new_map->map_range.x_grid_number = new_map_range.x_grid_number;
    new_map->map_range.x_grid_space = new_map_range.x_grid_space;
    new_map->map_range.y_grid_number = new_map_range.y_grid_number;
    new_map->map_range.y_grid_space = new_map_range.y_grid_space;
    new_map->model_type = map->model_type;
    new_map->element_type = map->element_type;
    new_map->values = new float[new_map_range.y_grid_number * new_map_range.x_grid_number];

    int longitude_index, longitude_next_index, latitude_index, latitude_next_index;
    float temp_longitude_value[2];
    float longitude, latitude, longitude_scale, latitude_scale;
    latitude = new_map->map_range.start_y;
    for ( int i = 0; i < new_map->map_range.y_grid_number; ++i ){
        longitude = new_map->map_range.start_x;
        for ( int j = 0; j < new_map->map_range.x_grid_number; ++j ){
            longitude_index = (int)((longitude - map->map_range.start_x) / map->map_range.x_grid_space);
            longitude_next_index = longitude_index + 1;
            if ( longitude_next_index >= map->map_range.x_grid_number ) longitude_next_index = map->map_range.x_grid_number - 1;
            latitude_index = (int)((latitude - map->map_range.start_y) / map->map_range.y_grid_space);
            latitude_next_index = latitude_index + 1;
            if ( latitude_next_index >= map->map_range.y_grid_number ) latitude_next_index = map->map_range.y_grid_number - 1;

            if ( longitude_index >= map->map_range.x_grid_number - 1 ) 
                longitude_scale = 1.0;
            else 
                longitude_scale = (longitude - (map->map_range.start_x + map->map_range.x_grid_space * longitude_index)) / map->map_range.x_grid_space;

            int test = (int)(-0.5);

            if ( latitude_index >= map->map_range.y_grid_number - 1 )
                latitude_scale = 1.0;
            else
                latitude_scale = (latitude - (map->map_range.start_y + map->map_range.y_grid_space * latitude_index)) / map->map_range.y_grid_space;

            temp_longitude_value[0] = (1.0 - longitude_scale) * map->values[latitude_index * map->map_range.x_grid_number + longitude_index]
                                    + longitude_scale * map->values[latitude_index * map->map_range.x_grid_number + longitude_next_index];
            temp_longitude_value[1] = (1.0 - longitude_scale) * map->values[latitude_next_index * map->map_range.x_grid_number + longitude_index]
                                    + longitude_scale * map->values[latitude_next_index * map->map_range.x_grid_number + longitude_next_index];
            new_map->values[i * new_map->map_range.x_grid_number + j] = (1.0 - latitude_scale) * temp_longitude_value[0] + latitude_scale * temp_longitude_value[1];

            longitude += new_map->map_range.x_grid_space;
        }
        latitude += new_map->map_range.y_grid_space;
    }

    return new_map;
}

float WrfStatisticSolver::GetSimilarity(WrfGridValueMap* map_one, WrfGridValueMap* map_two, std::vector< int > selected_grid_index){
    float normalized_value = 1.0;

    switch ( map_one->element_type ){
    case WRF_ACCUMULATED_PRECIPITATION:
        normalized_value = 5;
        break;
    case WRF_TEMPERATURE_500HPA:
        normalized_value = 2.0;
        break;
    case WRF_HEIGHT_500HPA:
        normalized_value = 2.0;
        break;
    case WRF_RELATIVE_HUMIDITY_500HPA:
        normalized_value = 10.0;
        break;
    default:
        normalized_value = 1.0;
        break;
    }

    float similarity = 0.0;
    for ( int i = 0; i < selected_grid_index.size(); ++i ){
        similarity += pow(abs(map_one->values[selected_grid_index[i]] - map_two->values[selected_grid_index[i]]) / normalized_value, 2);
    }
    if ( selected_grid_index.size() != 0 ){
        similarity = sqrt(similarity / selected_grid_index.size());
    }

    return similarity;
}

void WrfStatisticSolver::GetSelectedGridIndex(std::vector< QPointF >& region_contour_, 
    float start_x, float start_y, float space_x, float space_y, int x_grid, int y_grid, 
    std::vector< int >& selected_index){

    selected_index.clear();

    float current_x_index, current_y_index, next_x_index, next_y_index;
    int x_grid_index, y_grid_index;

    std::vector< bool > is_selected;
    is_selected.resize(x_grid * y_grid, false);

    current_x_index = (region_contour_[0].rx() - start_x) / space_x;
    current_y_index = (region_contour_[0].ry() - start_y) / space_y;
    for ( int j = 1; j <= region_contour_.size(); ++j ){
        next_x_index = (region_contour_[j % region_contour_.size()].rx() - start_x) / space_x;
        next_y_index = (region_contour_[j % region_contour_.size()].ry() - start_y) / space_y;

        if ( abs(current_x_index - next_x_index) < abs(current_y_index - next_y_index) ){
            float step = (next_x_index - current_x_index) / (next_y_index - current_y_index);
            int scale = current_y_index < next_y_index ? 1:-1;
            y_grid_index = (int)(current_y_index + 0.5);
            if ( y_grid_index < 0 ) y_grid_index = 0;
            if ( y_grid_index >= y_grid ) y_grid_index = y_grid - 1;
            while ( y_grid_index * scale < next_y_index * scale ){
                x_grid_index = (int)(current_x_index + step * (y_grid_index - current_y_index) + 0.5);
                if ( x_grid_index < 0 ) x_grid_index = 0;
                if ( x_grid_index >= x_grid ) x_grid_index = x_grid - 1;
                is_selected[y_grid_index * x_grid + x_grid_index] = true;

                y_grid_index += scale;
            }
        } else {
            float step = (next_y_index - current_y_index) / (next_x_index - current_x_index);
            int scale = current_x_index < next_x_index ? 1:-1;
            x_grid_index = (int)(current_x_index + 0.5);
            if ( x_grid_index < 0 ) x_grid_index = 0;
            if ( x_grid_index >= x_grid ) x_grid_index = x_grid - 1;
            while ( x_grid_index * scale < next_x_index * scale ){
                y_grid_index = (int)(current_y_index + step * (x_grid_index - current_x_index) + 0.5);
                if ( y_grid_index < 0 ) y_grid_index = 0;
                if ( y_grid_index >= y_grid ) y_grid_index = y_grid - 1;
                is_selected[y_grid_index * x_grid + x_grid_index] = true;

                x_grid_index += scale;
            }
        }

        current_x_index = next_x_index;
        current_y_index = next_y_index;
    }

    int left, right;
    for ( int r = 0; r < y_grid; ++r ){
        left = 100000;
        right = -1;
        for ( int c = 0; c < x_grid; ++c ){
            int temp_index = r * x_grid + c;
            if ( is_selected[temp_index] && c < left ) left = c;
            if ( is_selected[temp_index] && c > right ) right = c;
        }
        if ( left != 100000 && right != -1 )
            for ( int k = left; k <= right; ++k ) {
                selected_index.push_back(x_grid * r + k);
            }
    }
}

void WrfStatisticSolver::Sort(std::vector< float >& value, std::vector< int >& index){
    for ( int i = 0; i < value.size() - 1; ++i )
        for ( int j = i + 1; j < value.size(); ++j )
            if ( value[i] > value[j] ){
                float temp_value = value[j];
                value[j] = value[i];
                value[i] = temp_value;

                int temp_index = index[i];
                index[i] = index[j];
                index[j] = temp_index;
            }
}

void WrfStatisticSolver::Sort(std::vector< float >& value, int* index){
    for ( int i = 0; i < value.size() - 1; ++i )
        for ( int j = i + 1; j < value.size(); ++j )
            if ( value[i] > value[j] ){
                float temp_value = value[j];
                value[j] = value[i];
                value[i] = temp_value;

                int temp_index = index[i];
                index[i] = index[j];
                index[j] = temp_index;
            }
}

WrfGridValueMap* WrfStatisticSolver::GeneratePqpfResult(std::vector< int >& retrieval_time_vec, ProbabilisticPara& para, int fcst_hour){
    std::vector< float* > observation_maps;
    observation_maps.resize(retrieval_time_vec.size());

    std::vector< float* > temp_map;
    for ( int i = 0; i < retrieval_time_vec.size(); ++i ){
        temp_map.clear();
        WrfDataManager::GetInstance()->GetGridData(retrieval_time_vec[i], WRF_REANALYSIS, para.element, fcst_hour, temp_map);
        observation_maps[i] = temp_map[0];
    }

    MapRange reanalysis_map_range;
    WrfDataManager::GetInstance()->GetModelMapRange(WRF_REANALYSIS, reanalysis_map_range);
    MapRange retrieval_map_range_;
    WrfDataManager::GetInstance()->GetEnsembleMapRange(retrieval_map_range_);

    std::vector< WrfGridValueMap* > reanalysis_maps;
    WrfDataManager::GetInstance()->GetGridValueMap(retrieval_time_vec[0], WRF_REANALYSIS, para.element, fcst_hour, reanalysis_maps);

    if ( reanalysis_maps[0]->type() == WrfValueMap::LAMBCC_VALUE_MAP ){
        WrfLambCcValueMap* temp_lambcc_map = dynamic_cast< WrfLambCcValueMap* >(reanalysis_maps[0]);
        WrfLambCcValueMap* value_map = temp_lambcc_map->DeepCopy();
        value_map->model_type = WRF_ELEMENT_PROBABILISTIC;
        value_map->element_type = WRF_PROBABILISTIC;
        value_map->values = new float[value_map->map_range.x_grid_number * value_map->map_range.y_grid_number];

        for ( int i = 0; i < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++i ){
            int accu_count = 0;
            for ( int j = 0; j < retrieval_time_vec.size(); ++j )
                if ( observation_maps[j][i] > para.thresh ) accu_count++;

                if ( retrieval_time_vec.size() != 0 )
                    value_map->values[i] = (float)accu_count / retrieval_time_vec.size();
                else
                    value_map->values[i] = 0;
        }

        return value_map;
    } else {
        WrfGridValueMap* value_map = reanalysis_maps[0]->DeepCopy();
        value_map->model_type = WRF_ELEMENT_PROBABILISTIC;
        value_map->element_type = WRF_PROBABILISTIC;
        value_map->values = new float[value_map->map_range.x_grid_number * value_map->map_range.y_grid_number];

        for ( int i = 0; i < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++i ){
            int accu_count = 0;
            for ( int j = 0; j < retrieval_time_vec.size(); ++j )
                if ( observation_maps[j][i] > para.thresh ) accu_count++;

            if ( retrieval_time_vec.size() != 0 )
                value_map->values[i] = (float)accu_count / retrieval_time_vec.size();
            else
                value_map->values[i] = 0;
        }

        return value_map;
    }
}

WrfGridValueMap* WrfStatisticSolver::GenerateAverageResult(std::vector< int >& retrieval_time_vec, WrfElementType element, int fcst_hour){
    std::vector< float* > observation_maps;
    observation_maps.resize(retrieval_time_vec.size());

    std::vector< float* > temp_map;
    for ( int i = 0; i < retrieval_time_vec.size(); ++i ){
        temp_map.clear();
        WrfDataManager::GetInstance()->GetGridData(retrieval_time_vec[i], WRF_REANALYSIS, element, fcst_hour, temp_map);
        observation_maps[i] = temp_map[0];
    }

    MapRange reanalysis_map_range;
    WrfDataManager::GetInstance()->GetModelMapRange(WRF_REANALYSIS, reanalysis_map_range);

    std::vector< WrfGridValueMap* > reanalysis_maps;
    WrfDataManager::GetInstance()->GetGridValueMap(retrieval_time_vec[0], WRF_REANALYSIS, element, fcst_hour, reanalysis_maps);

    for ( int i = 1; i < retrieval_time_vec.size(); ++i ){
        for ( int j = 0; j < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++j )
            reanalysis_maps[0]->values[j] += observation_maps[i][j];
    }
    for ( int i = 0; i < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++i )
        reanalysis_maps[0]->values[i] /= retrieval_time_vec.size();

    return reanalysis_maps[0];
}

void WrfStatisticSolver::GaussianSmooth(std::vector< float >& value, int window_size){
	std::vector< float > temp_value;
	temp_value.assign(value.begin(), value.end());

	std::vector< float > gassian_factor;
	gassian_factor.resize(window_size);
	for ( int i = 0; i < window_size; ++i ){
		gassian_factor[i] = exp((double)(-1 * i * i));
	}

	for ( int i = 0; i < value.size(); ++i ){
		float accu_sum = 0;
		value[i] = 0;
		for ( int j = i - window_size + 1; j < i + window_size; ++j )
			if ( j >= 0 && j < value.size() ){
				value[i] += gassian_factor[abs(i - j)] * temp_value[j];
				accu_sum += gassian_factor[abs(i - j)];
			}
		value[i] /= accu_sum;
	}
}

void WrfStatisticSolver::GenerateLineCurvature(std::vector< float >& values, int window_size, std::vector< float >& curvature){
	curvature.resize(values.size());
	curvature.assign(curvature.size(), 0);

	for ( int i = 0; i < 3; ++i ) GaussianSmooth(values, 5);

	std::vector< float > temp_gradient;
	temp_gradient.resize(values.size(), 0);
	float max_gradient = -1e10;
	for ( int i = window_size; i < values.size() - window_size; ++i ){
		for ( int j = 1; j <= window_size; ++j )
			temp_gradient[i] += abs(values[i + j] - values[i - j]) / (2 * j);
		temp_gradient[i] /= window_size;
		if ( temp_gradient[i] > max_gradient ) max_gradient = temp_gradient[i];
	}

	/*float max_curvature = -1e10;
	for ( int i = window_size + 1; i < values.size() - window_size - 1; ++i ){
		curvature[i] = abs(temp_gradient[i + 1] - temp_gradient[i - 1]) / 2;
		if ( curvature[i] > max_curvature ) max_curvature = curvature[i];
	}

	if ( max_curvature != 0 )
		for ( int i = 0; i < curvature.size(); ++i ) curvature[i] /= max_curvature;*/
	for ( int i = 0; i < temp_gradient.size(); ++i ) temp_gradient[i] /= max_gradient;
	curvature.assign(temp_gradient.begin(), temp_gradient.end());

	//curvature.assign(values.begin(), values.end());
}