#include "wrf_reforecast_manager.h"
#include "wrf_data_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_grid_rms_pack.h"

WrfReforecastManager* WrfReforecastManager::instance_ = NULL;

WrfReforecastManager* WrfReforecastManager::GetInstance(){
    if ( instance_ == NULL ){
        instance_ = new WrfReforecastManager;
    }

    return instance_;
}

bool WrfReforecastManager::DeleteInstance(){
    if ( instance_ != NULL ){
        delete instance_;
        return true;
    } else {
        return false;
    }
}

WrfReforecastManager::WrfReforecastManager()
    : retrieval_time_step_(24){
 
}

WrfReforecastManager::~WrfReforecastManager(){

}

void WrfReforecastManager::SetForecastingDate(QDateTime& date){
    forecasting_date_ = date;
}

void WrfReforecastManager::SetRetrievalParameters(int year_length, int day_length, int grid_size, int fcst_hour, int analog_num,
    std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements, 
    std::vector< float >& element_weights, std::vector< float >& normalize_values){

    retrieval_year_length_ = year_length;
    retrieval_day_length_ = day_length;
    retrieval_grid_size_ = grid_size;
    fcst_hour_ = fcst_hour;
    retrieval_analog_num_ = analog_num;
    retrieval_ens_elements_ = ensemble_elements;
    retrieval_ens_mean_elements_ = ensemble_mean_elements;
    
    retrieval_element_weights_ = element_weights;
    float normalize_weight = 0;
    for ( int i = 0; i < retrieval_element_weights_.size(); ++i )
        normalize_weight += retrieval_element_weights_[i];
    for ( int i = 0; i < retrieval_element_weights_.size(); ++i )
        retrieval_element_weights_[i] /= normalize_weight;

    retrieval_normalize_values_ = normalize_values;
}

void WrfReforecastManager::GetRetrievalElements(std::vector< WrfElementType >& elements){
    elements.clear();
    for ( int i = 0; i < retrieval_ens_elements_.size(); ++i ) elements.push_back(retrieval_ens_elements_[i]);
    for ( int i = 0; i < retrieval_ens_mean_elements_.size(); ++i ) elements.push_back(retrieval_ens_mean_elements_[i]);
}

void WrfReforecastManager::GetReferenceDateTime(std::vector< int >& date_time){
    date_time = retrieval_time_vec_;
}

void WrfReforecastManager::LoadData(){
    GenerateRetrievalTimeVec();

    LoadReferenceData();
}

void WrfReforecastManager::PreProcessDataForEvent(AxisData* data, std::vector< int >& selected_grid_index){
    data->rms_values.resize(retrieval_time_vec_.size());
    data->date_sort_index.resize(retrieval_time_vec_.size());
    data->sort_date_index.resize(retrieval_time_vec_.size());
    
    std::vector< int > elements;
    for ( int e = 0; e < data->variables.size(); ++e ){
        int temp_index = -1;
        for ( int k = 0; k < retrieval_ens_elements_.size(); ++k )
            if ( retrieval_ens_elements_[k] == data->variables[e] ){
                temp_index = k;
                break;
            }
        if ( temp_index == -1 ){
            for ( int k = 0; k < retrieval_ens_mean_elements_.size(); ++k )
                if ( retrieval_ens_mean_elements_[k] == data->variables[e] ){
                    temp_index = k;
                    break;
                }
            if ( temp_index != -1 ) temp_index += retrieval_ens_elements_.size();
        }
        elements.push_back(temp_index);
    }

    for ( int i = 0; i < retrieval_time_vec_.size(); ++i ){
        if ( !is_retrieval_data_completed_[i] ) {
            data->rms_values[i] = 9999.0;
            continue;
        }

        float accu_rms = 0;
        for ( int j = 0; j < selected_grid_index.size(); ++j ){
            int grid_index = selected_grid_index[j];
            int left = grid_index % retrieval_map_range_.x_grid_number;
            int bottom = grid_index / retrieval_map_range_.x_grid_number;

            float temp_distance = 0;
            for ( int e = 0; e < elements.size(); ++e ){
                float element_distance = 0;
                for ( int k = 0; k < retrieval_value_maps_[i][elements[e]].size(); ++k ){
                    float his_value = *(retrieval_value_maps_[i][elements[e]][k] + grid_index);
                    float current_value = *(current_value_maps_[elements[e]][k] + grid_index);
                    if ( his_value < -10000 || current_value < -10000 ) {
                        element_distance = 9999.0;
                        break;
                    }
                    element_distance += abs(current_value - his_value);
                }
                if ( element_distance > 9000 ){
                    temp_distance = element_distance;
                    break;
                }
                temp_distance += element_distance / retrieval_value_maps_[i][elements[e]].size() * data->weights[e] / data->normalize_values[e];
            }
            
            accu_rms += pow(temp_distance, 2);
        }
        data->rms_values[i] = sqrt(accu_rms / selected_grid_index.size());
    }

    for ( int i = 0; i < retrieval_time_vec_.size(); ++i ) data->sort_date_index[i] = i;
    WrfStatisticSolver::Sort(data->rms_values, data->sort_date_index);
    for ( int i = 0; i < retrieval_time_vec_.size(); ++i ) data->date_sort_index[data->sort_date_index[i]] = i;
}

void WrfReforecastManager::PreProcessData(){
    std::vector< int > temp_grid_vec;
    for ( int i = 0; i < retrieval_map_range_.x_grid_number * retrieval_map_range_.y_grid_number; ++i )
        temp_grid_vec.push_back(i);
    WrfGridRmsPack* grid_rms_pack = GenerateSimilarityPack(retrieval_grid_size_, temp_grid_vec);

    for ( int i = 0; i < grid_rms_pack->sorted_index.size(); ++i ){
        for ( int j = 0; j < retrieval_analog_num_; ++j )
            grid_rms_pack->is_date_selected[i][grid_rms_pack->sorted_index[i][j]] = true;
    }

    grid_rms_pack_map_.insert(std::map< int, WrfGridRmsPack* >::value_type(retrieval_grid_size_, grid_rms_pack));

    UpdateDateSelection();
}

void WrfReforecastManager::GenerateRetrievalTimeVec(){
    retrieval_time_vec_.clear();

    int temp_time = forecasting_date_.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
    for ( int i = 0; i < retrieval_day_length_; ++i ){
        retrieval_time_vec_.push_back(temp_time - (retrieval_day_length_ - i) * retrieval_time_step_);
    }

    for ( int i = 1; i <= retrieval_year_length_; ++i ){
        QDateTime temp_date = forecasting_date_.addYears(-1 * i);
        temp_time = temp_date.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
        for ( int j = -1 * retrieval_day_length_; j <= retrieval_day_length_; ++j ){
            retrieval_time_vec_.push_back(temp_time + retrieval_time_step_ * j);
        }
    }
}

void WrfReforecastManager::LoadReferenceData(){
    // load retrieval data
    retrieval_value_maps_.clear();
    retrieval_value_maps_.resize(retrieval_time_vec_.size());
    is_retrieval_data_completed_.resize(retrieval_time_vec_.size());

    for ( int i = 0; i < retrieval_time_vec_.size(); ++i ){
        retrieval_value_maps_[i].resize(retrieval_ens_elements_.size() + retrieval_ens_mean_elements_.size());

        bool is_data_completed = true;
        for ( int j = 0; j < retrieval_ens_elements_.size(); ++j ){
            is_data_completed = is_data_completed && 
                WrfDataManager::GetInstance()->GetGridData(retrieval_time_vec_[i], WRF_NCEP_ENSEMBLES, 
                                                        retrieval_ens_elements_[j], fcst_hour_, retrieval_value_maps_[i][j]);
        }

        for ( int j = 0; j < retrieval_ens_mean_elements_.size(); ++j ){
            is_data_completed = is_data_completed && 
                WrfDataManager::GetInstance()->GetGridData(retrieval_time_vec_[i], WRF_NCEP_ENSEMBLE_MEAN, 
                                                        retrieval_ens_mean_elements_[j], fcst_hour_, 
                                                        retrieval_value_maps_[i][j + retrieval_ens_elements_.size()]);
        }

        is_retrieval_data_completed_[i] = is_data_completed;
    }



    // load current data
    current_value_maps_.clear();
    current_value_maps_.resize(retrieval_ens_elements_.size() + retrieval_ens_mean_elements_.size());
    for ( int i = 0; i < retrieval_ens_elements_.size(); ++i ){
        WrfDataManager::GetInstance()->GetGridData(forecasting_date_, WRF_NCEP_ENSEMBLES, 
                                                retrieval_ens_elements_[i], fcst_hour_, current_value_maps_[i]);
    }

    for ( int i = 0; i < retrieval_ens_mean_elements_.size(); ++i ){
        WrfDataManager::GetInstance()->GetGridData(forecasting_date_, WRF_NCEP_ENSEMBLE_MEAN, 
                                                retrieval_ens_mean_elements_[i], fcst_hour_, 
                                                current_value_maps_[i + retrieval_ens_elements_.size()]);
    }

    // get the retrieval map range
    if ( retrieval_ens_elements_.size() != 0 )
        WrfDataManager::GetInstance()->GetModelMapRange(WRF_NCEP_ENSEMBLES, retrieval_map_range_);
    else
        WrfDataManager::GetInstance()->GetModelMapRange(WRF_NCEP_ENSEMBLE_MEAN, retrieval_map_range_);
}


WrfGridRmsPack* WrfReforecastManager::GenerateSimilarityPack(int grid_size, std::vector< int >& selected_grid_point_index){
    WrfGridRmsPack* pack = new WrfGridRmsPack;
    pack->SetRetrievalGridSize(grid_size);
    pack->SetSelectedGridPointIndex(selected_grid_point_index);

    pack->date_grid_point_rms.resize(retrieval_value_maps_.size());
    // date 
    for ( int i = 0; i < retrieval_time_vec_.size(); ++i ){
        pack->date_grid_point_rms[i].resize(selected_grid_point_index.size());

        // grid point
        for ( int j = 0; j < pack->date_grid_point_rms[i].size(); ++j ){
            pack->date_grid_point_rms[i][j].resize(retrieval_ens_elements_.size() + retrieval_ens_mean_elements_.size());

            // element
            for ( int k = 0; k < pack->date_grid_point_rms[i][j].size(); ++k ){
                if ( !is_retrieval_data_completed_[i] ) 
                    pack->date_grid_point_rms[i][j][k] = 9999.0;
                else
                    pack->date_grid_point_rms[i][j][k] = GetRetrievalElementRms(i, k, selected_grid_point_index[j], grid_size);
            }
        }
    }

    pack->aggregated_grid_point_rms.resize(selected_grid_point_index.size());
    // grid point
    for ( int i = 0; i < pack->aggregated_grid_point_rms.size(); ++i ){
        pack->aggregated_grid_point_rms[i].resize(retrieval_value_maps_.size());

        // date
        for ( int j = 0; j < pack->aggregated_grid_point_rms[i].size(); ++j ){
            pack->aggregated_grid_point_rms[i][j] = 9999.0;

            if ( !is_retrieval_data_completed_[j] ) continue;

            pack->aggregated_grid_point_rms[i][j] = 0.0;
            // element
            for ( int k = 0; k < retrieval_ens_elements_.size() + retrieval_ens_mean_elements_.size(); ++k )
                pack->aggregated_grid_point_rms[i][j] += pack->date_grid_point_rms[j][i][k] * retrieval_element_weights_[k] / retrieval_normalize_values_[k];
        }
    }

    pack->sorted_aggregated_rms = pack->aggregated_grid_point_rms;
    pack->sorted_index.resize(pack->aggregated_grid_point_rms.size());
    for ( int i = 0; i < pack->sorted_index.size(); ++i ){
        pack->sorted_index[i].resize(pack->aggregated_grid_point_rms[i].size());

        for ( int j = 0; j < pack->sorted_index[i].size(); ++j ) pack->sorted_index[i][j] = j;
    }
    for ( int i = 0; i < pack->sorted_index.size(); ++i ) WrfStatisticSolver::Sort(pack->sorted_aggregated_rms[i], pack->sorted_index[i]);

    pack->is_date_selected.resize(selected_grid_point_index.size());
    for ( int i = 0; i < pack->is_date_selected.size(); ++i )
        pack->is_date_selected[i].resize(retrieval_time_vec_.size(), false);

    return pack;
}

float WrfReforecastManager::GetRetrievalElementRms(int date_index, int element, int grid_index, int grid_size){
    int left = grid_index % retrieval_map_range_.x_grid_number;
    int bottom = grid_index / retrieval_map_range_.x_grid_number;
    int half_size = grid_size / 2;

    int accu_count = 0;
    float accu_rms = 0;

    for ( int y = bottom - half_size + 1; y < bottom + half_size; ++y ){
        if ( y < 0 || y >= retrieval_map_range_.y_grid_number ) continue;

        for ( int x = left - half_size + 1; x < left + half_size; ++x ){
            if ( x < 0 || x >= retrieval_map_range_.x_grid_number ) continue;

            int temp_index = y * retrieval_map_range_.x_grid_number + x;

            float temp_distance = 0;
            for ( int i = 0; i < retrieval_value_maps_[date_index][element].size(); ++i ){
                float his_value = *(retrieval_value_maps_[date_index][element][i] + temp_index);
                float current_value = *(current_value_maps_[element][i] + temp_index);
                if ( his_value < -10000 || current_value < -10000 ) return 9999.0;
                temp_distance += abs(current_value - his_value);
            }

            accu_rms += pow(temp_distance / retrieval_value_maps_[date_index][element].size(), 2);

            accu_count++;
        }
    }

    return sqrt(accu_rms / accu_count);
}

void WrfReforecastManager::PreProcessFocusGrid(std::vector< int >& selected_index, std::vector< int >& selected_size){
    for ( int i = 0; i < selected_size.size(); ++i )
        if ( selected_size[i] != retrieval_grid_size_ ) {
            WrfGridRmsPack* grid_pack = GenerateSimilarityPack(selected_size[i], selected_index); 

            for ( int j = 0; j < grid_pack->is_date_selected.size(); ++j )
                grid_pack->is_date_selected[j].assign(grid_pack->is_date_selected[j].size(), true);

            std::map< int, WrfGridRmsPack* >::iterator iter = grid_rms_pack_map_.find(selected_size[i]);
            if ( iter != grid_rms_pack_map_.end() ){
                delete iter->second;
                grid_rms_pack_map_.erase(iter);
            }
            grid_rms_pack_map_.insert(std::map< int, WrfGridRmsPack* >::value_type(selected_size[i], grid_pack));
        }

	// update focus grids' climatological distribution

	climate_distribution_.resize(selected_index.size());
	std::vector< float > historical_data_vec;
	for ( int i = 0; i < selected_index.size(); ++i ){
		historical_data_vec.resize(retrieval_time_vec_.size());
		for ( int j = 0; j < retrieval_time_vec_.size(); ++j ){
			float temp_value = 0.0;
			for ( int k = 0; k < retrieval_value_maps_[j][0].size(); ++k ){
				float his_value = *(retrieval_value_maps_[j][0][k] + selected_index[i]);

				if ( his_value < -10000 ) his_value = 9999.0;
				temp_value += his_value;
			}
			temp_value /= retrieval_value_maps_[j][0].size();
			historical_data_vec[j] = temp_value;
		}

		float current_value = 0.0;
		for ( int k = 0; k < current_value_maps_[0].size(); ++k ){
			float temp_value = *(current_value_maps_[0][k] + selected_index[i]);

			if ( temp_value < -10000 ) temp_value = 9999.0;
			current_value += temp_value;
		}
		current_value /= current_value_maps_[0].size();

		sort(historical_data_vec.begin(), historical_data_vec.end());

		int temp_index = 0;
		while ( historical_data_vec[temp_index] < current_value && temp_index < historical_data_vec.size() ) temp_index++;
		climate_distribution_[i] = (float)temp_index / historical_data_vec.size();
	}
}

bool WrfReforecastManager::UpdateDateSelection(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< bool > >& is_date_selected){
    std::map< int, WrfGridRmsPack* >::iterator iter = grid_rms_pack_map_.find(grid_size);
    if ( iter == grid_rms_pack_map_.end() ) return false;
    iter->second->UpdateDateSelection(selected_index, is_date_selected);

    UpdateDateSelection();

    return true;
}


void WrfReforecastManager::GetNormalizedValues(std::vector< float >& normalize_value){
    normalize_value = retrieval_normalize_values_;
}

float WrfReforecastManager::GetCurrentGridAveragesValues(int grid_size, WrfElementType element, int grid_index){
    float average = 0;

    int left = grid_index % retrieval_map_range_.x_grid_number;
    int bottom = grid_index / retrieval_map_range_.x_grid_number;
    int half_size = grid_size / 2;

    int element_index = -1;
    for ( int i = 0; i < retrieval_ens_elements_.size(); ++i )
        if ( retrieval_ens_elements_[i] == element ){
            element_index = i;
            break;
        }

    if( element_index != -1 ){
        int accu_count = 0, temp_index;
        for ( int y = bottom - half_size + 1; y <= bottom + half_size; ++y ){
            if ( y < 0 || y >= retrieval_map_range_.y_grid_number ) continue;
            for ( int x = left - half_size + 1; x <= left + half_size; ++x ){
                if ( x < 0 || x >= retrieval_map_range_.x_grid_number ) continue;
                temp_index = y * retrieval_map_range_.x_grid_number + x;
                float temp_distance = 0;
                for ( int m = 0; m < current_value_maps_[element_index].size(); ++m ){
                    temp_distance += *(current_value_maps_[element_index][m] + temp_index);
                }
                average += temp_distance / current_value_maps_[element_index].size();
                accu_count++;
            }
        }

        return average / accu_count;
    }

    for ( int i = 0; i < retrieval_ens_mean_elements_.size(); ++i )
        if ( retrieval_ens_mean_elements_[i] == element ){
            element_index = i + retrieval_ens_elements_.size(); 
            break;
        }

    if( element_index != -1 ){
        int accu_count = 0, temp_index;
        for ( int y = bottom - half_size + 1; y <= bottom + half_size; ++y ){
            if ( y < 0 || y >= retrieval_map_range_.y_grid_number ) continue;
            for ( int x = left - half_size + 1; x <= left + half_size; ++x ){
                if ( x < 0 || x >= retrieval_map_range_.x_grid_number ) continue;
                temp_index = y * retrieval_map_range_.x_grid_number + x;
                float temp_distance = 0;
                for ( int m = 0; m < current_value_maps_[element_index].size(); ++m ){
                    temp_distance += *(current_value_maps_[element_index][m] + temp_index);
                }
                average += temp_distance / current_value_maps_[element_index].size();
                accu_count++;
            }
        }

        return average / accu_count;
    }
    
    return -1;
}

WrfGridValueMap* WrfReforecastManager::GenerateForecastingMap(ProbabilisticPara& para){
	if ( para.analog_number != -1 ) return GenerateRawForecastingMap(para);

    // load observation maps of para.element
    std::vector< float* > observation_maps;
    observation_maps.resize(retrieval_time_vec_.size());

    std::vector< float* > temp_map;
    for ( int i = 0; i < retrieval_time_vec_.size(); ++i ){
        temp_map.clear();
        WrfDataManager::GetInstance()->GetGridData(retrieval_time_vec_[i], WRF_REANALYSIS, para.element, fcst_hour_, temp_map);
        observation_maps[i] = temp_map[0];
    }

    MapRange reanalysis_map_range;
    WrfDataManager::GetInstance()->GetModelMapRange(WRF_REANALYSIS, reanalysis_map_range);

    std::vector< WrfGridValueMap* > reanalysis_maps;
    WrfDataManager::GetInstance()->GetGridValueMap(retrieval_time_vec_[0], WRF_REANALYSIS, para.element, fcst_hour_, reanalysis_maps);

    if ( reanalysis_maps[0]->type() == WrfValueMap::LAMBCC_VALUE_MAP ){
        WrfLambCcValueMap* temp_lambcc_map = dynamic_cast< WrfLambCcValueMap* >(reanalysis_maps[0]);
        WrfLambCcValueMap* value_map = temp_lambcc_map->DeepCopy();
        value_map->model_type = WRF_ELEMENT_PROBABILISTIC;
        value_map->element_type = WRF_PROBABILISTIC;

        for ( int i = 0; i < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++i ){
            float lon = value_map->lon[i];
            float lat = value_map->lat[i];
            int x = (int)((lon - retrieval_map_range_.start_x) / retrieval_map_range_.x_grid_space);
            if ( x < 0 ) x = 0;
            if ( x >= retrieval_map_range_.x_grid_number ) x = retrieval_map_range_.x_grid_number - 1;
            int y = (int)((lat - retrieval_map_range_.start_y) / retrieval_map_range_.y_grid_space);
            if ( y < 0 ) y = 0;
            if ( y >= retrieval_map_range_.y_grid_number ) y = retrieval_map_range_.y_grid_number - 1;

            int grid_index = y * retrieval_map_range_.x_grid_number + x;

            int accu_count = 0;
            int total_count = 0;
            for ( int j = 0; j < is_date_selected_[grid_index].size(); ++j )
                if ( is_date_selected_[grid_index][j] ){
                    total_count++;
                    if ( observation_maps[j][i] > para.thresh ) accu_count++;
                }

            if ( total_count != 0 )
                value_map->values[i] = (float)accu_count / total_count;
            else
                value_map->values[i] = 0;
        }

        return value_map;
    } else {
        WrfGridValueMap* value_map = reanalysis_maps[0]->DeepCopy();
        value_map->model_type = WRF_ELEMENT_PROBABILISTIC;
        value_map->element_type = WRF_PROBABILISTIC;

        for ( int i = 0; i < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++i ){
            float lon = reanalysis_map_range.start_x + i % reanalysis_map_range.x_grid_number * reanalysis_map_range.x_grid_space;
            float lat = reanalysis_map_range.start_y + i / reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_space;
            int x = (int)((lon - retrieval_map_range_.start_x) / retrieval_map_range_.x_grid_space);
            if ( x < 0 ) x = 0;
            if ( x >= retrieval_map_range_.x_grid_number ) x = retrieval_map_range_.x_grid_number - 1;
            int y = (int)((lat - retrieval_map_range_.start_y) / retrieval_map_range_.y_grid_space);
            if ( y < 0 ) y = 0;
            if ( y >= retrieval_map_range_.y_grid_number ) y = retrieval_map_range_.y_grid_number - 1;

            int grid_index = y * retrieval_map_range_.x_grid_number + x;

            int accu_count = 0;
            int total_count = 0;
            for ( int j = 0; j < is_date_selected_[grid_index].size(); ++j )
                if ( is_date_selected_[grid_index][j] ){
                    total_count++;
                    if ( observation_maps[j][i] > para.thresh ) accu_count++;
                }

                if ( total_count != 0 )
                    value_map->values[i] = (float)accu_count / total_count;
                else
                    value_map->values[i] = 0;
        }

        return value_map;
    }
}

WrfGridValueMap* WrfReforecastManager::GenerateRawForecastingMap(ProbabilisticPara& para){
	std::vector< float* > observation_maps;
	observation_maps.resize(retrieval_time_vec_.size());

	std::vector< float* > temp_map;
	for ( int i = 0; i < retrieval_time_vec_.size(); ++i ){
		temp_map.clear();
		WrfDataManager::GetInstance()->GetGridData(retrieval_time_vec_[i], WRF_REANALYSIS, para.element, fcst_hour_, temp_map);
		observation_maps[i] = temp_map[0];
	}

	MapRange reanalysis_map_range;
	WrfDataManager::GetInstance()->GetModelMapRange(WRF_REANALYSIS, reanalysis_map_range);

	std::vector< WrfGridValueMap* > reanalysis_maps;
	WrfDataManager::GetInstance()->GetGridValueMap(retrieval_time_vec_[0], WRF_REANALYSIS, para.element, fcst_hour_, reanalysis_maps);

	WrfGridValueMap* value_map = reanalysis_maps[0]->DeepCopy();
	value_map->model_type = WRF_ELEMENT_PROBABILISTIC;
	value_map->element_type = WRF_PROBABILISTIC;

	WrfGridRmsPack* grid_pack = grid_rms_pack_map_[retrieval_grid_size_];

	for ( int i = 0; i < reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_number; ++i ){
		float lon = reanalysis_map_range.start_x + i % reanalysis_map_range.x_grid_number * reanalysis_map_range.x_grid_space;
		float lat = reanalysis_map_range.start_y + i / reanalysis_map_range.x_grid_number * reanalysis_map_range.y_grid_space;
		int x = (int)((lon - retrieval_map_range_.start_x) / retrieval_map_range_.x_grid_space);
		if ( x < 0 ) x = 0;
		if ( x >= retrieval_map_range_.x_grid_number ) x = retrieval_map_range_.x_grid_number - 1;
		int y = (int)((lat - retrieval_map_range_.start_y) / retrieval_map_range_.y_grid_space);
		if ( y < 0 ) y = 0;
		if ( y >= retrieval_map_range_.y_grid_number ) y = retrieval_map_range_.y_grid_number - 1;

		int grid_index = y * retrieval_map_range_.x_grid_number + x;

		int accu_count = 0;
		for ( int j = 0; j < para.analog_number; ++j )
			if ( observation_maps[grid_pack->sorted_index[grid_index][j]][i] > para.thresh ) accu_count++;
		value_map->values[i] = (float)accu_count / para.analog_number;
	}

	return value_map;
}

float WrfReforecastManager::GetGridPointRmsValue(int grid_index, int date_index, int grid_size){
    WrfGridRmsPack* pack = grid_rms_pack_map_[grid_size];

    int pack_grid_index = pack->FindGrid(grid_index);
    if ( pack_grid_index != -1 )
        return pack->aggregated_grid_point_rms[pack_grid_index][date_index];
    else
        return 9999.0;
}

void WrfReforecastManager::GetElementRmsValues(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< std::vector< float > > >& rms_values){
    WrfGridRmsPack* pack = grid_rms_pack_map_[grid_size];

    rms_values.clear();
    rms_values.resize(retrieval_time_vec_.size());
    for ( int i = 0; i < rms_values.size(); ++i ){
        rms_values[i].resize(selected_index.size());

        for ( int j = 0; j < selected_index.size(); ++j ){
            int temp_index = pack->FindGrid(selected_index[j]);
            if ( temp_index != -1 ) 
                rms_values[i][j] = pack->date_grid_point_rms[i][temp_index];
        }
    }
}


void WrfReforecastManager::GetSortedAggregatedRmsValues(int grid_size, std::vector< int >& selected_index, 
                                                    std::vector< std::vector< float > >& sorted_rms_values, 
                                                    std::vector< std::vector< int > >& sorted_index){
    WrfGridRmsPack* pack = grid_rms_pack_map_[grid_size];

    sorted_rms_values.clear();
    sorted_index.clear();

    sorted_rms_values.resize(selected_index.size());
    sorted_index.resize(selected_index.size());
    for ( int i = 0; i < selected_index.size(); ++i ){
        int temp_index = pack->FindGrid(selected_index[i]);
        if ( temp_index != -1 ) {
            sorted_rms_values[i] = pack->sorted_aggregated_rms[temp_index];
            sorted_index[i] = pack->sorted_index[temp_index];
        }
    }
}

void WrfReforecastManager::GetSortedSelectedAggregatedRmsValues(std::vector< std::vector< float > >& sorted_rms_values){
    WrfGridRmsPack* pack = grid_rms_pack_map_[retrieval_grid_size_];

    sorted_rms_values.clear();
    sorted_rms_values.resize(pack->sorted_index.size());
    for ( int i = 0; i < pack->sorted_index.size(); ++i ){
        for ( int j = 0; j < pack->sorted_index[i].size(); ++j )
            if ( pack->is_date_selected[i][pack->sorted_index[i][j]] )
                sorted_rms_values[i].push_back(pack->sorted_aggregated_rms[i][j]);
    }
}

void WrfReforecastManager::GetSelectedClimatologicalDistribution(std::vector< float >& distribution){
	distribution = climate_distribution_;
}

void WrfReforecastManager::GetDateSelection(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< bool > >& selection){
    WrfGridRmsPack* pack = grid_rms_pack_map_[grid_size];

    selection.resize(selected_index.size());
    for ( int i = 0; i < selected_index.size(); ++i ){
        int temp_index = pack->FindGrid(selected_index[i]);
        selection[i] = pack->is_date_selected[temp_index];
    }
}

void WrfReforecastManager::UpdateDateSelection(){
    is_date_selected_.clear();
    is_date_selected_.resize(retrieval_map_range_.x_grid_number * retrieval_map_range_.y_grid_number);
    for ( int i = 0; i < retrieval_map_range_.x_grid_number * retrieval_map_range_.y_grid_number; ++i )
        is_date_selected_[i].resize(retrieval_time_vec_.size(), true);
    std::map< int, WrfGridRmsPack* >::iterator iter = grid_rms_pack_map_.begin();
    while ( iter != grid_rms_pack_map_.end() ){
        WrfGridRmsPack* pack = iter->second;
        std::map< int, int >::iterator index_iter = pack->index_map.begin();
        while ( index_iter != pack->index_map.end() ){
            for ( int i = 0; i < retrieval_time_vec_.size(); ++i )
                is_date_selected_[index_iter->first][i] = is_date_selected_[index_iter->first][i] && pack->is_date_selected[index_iter->second][i];
            index_iter++;
        }
        iter++;
    }
}