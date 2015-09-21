#include "wrf_ensemble_manager.h"
#include "wrf_data_manager.h"

WrfEnsembleManager* WrfEnsembleManager::instance_ = NULL;

WrfEnsembleManager* WrfEnsembleManager::GetInstance(){
    if ( instance_ == NULL ){
        instance_ = new WrfEnsembleManager;
    }

    return instance_;
}

bool WrfEnsembleManager::DeleteInstance(){
    if ( instance_ != NULL ){
        delete instance_;
        return true;
    } else {
        return false;
    }
}

WrfEnsembleManager::WrfEnsembleManager(){

}

WrfEnsembleManager::~WrfEnsembleManager(){

}

void WrfEnsembleManager::SetCurrentDate(QDateTime& date){
    current_date_ = date;
}

void WrfEnsembleManager::SetForecastingParameters(int fcst_hour, std::vector< WrfElementType >& ensemble_elements, std::vector< float >& normalize_values){
    fhour_ = fcst_hour;
    ens_elements_ = ensemble_elements;
    ens_element_normalize_values_ = normalize_values;
}

WrfGridValueMap* WrfEnsembleManager::GenerateUncertaintyMaps(WrfElementType element){
    std::vector< WrfGridValueMap* > ensemble_maps;
    if ( !WrfDataManager::GetInstance()->GetGridValueMap(current_date_, WRF_NCEP_ENSEMBLES, element, fhour_, ensemble_maps) ) return NULL;
    if ( ensemble_maps.size() <= 0 ) return NULL;

    WrfGridValueMap* un_map = ensemble_maps[0]->DeepCopy();
    un_map->model_type = WRF_ELEMENT_UNCERTAINTY;
    un_map->element_type = WRF_UNCERTAINTY;

    MapRange ens_map_range = ensemble_maps[0]->map_range;
    float normalize_value = 1.0;
    for ( int i = 0; i < ens_elements_.size(); ++i )
        if ( ens_elements_[i] == element ){
            normalize_value = ens_element_normalize_values_[i];
            break;
        }

    for ( int i = 0; i < ens_map_range.x_grid_number * ens_map_range.y_grid_number; ++i ){
        double average_value = 0.0;
        for ( int j = 0; j < ensemble_maps.size(); ++j ) average_value += ensemble_maps[j]->values[i];
        average_value /= ensemble_maps.size();

        double std_dev = 0.0;
        for ( int j = 0; j < ensemble_maps.size(); ++j ) std_dev += pow((ensemble_maps[j]->values[i] - average_value) / normalize_value, 2);
        std_dev = sqrt(std_dev / ensemble_maps.size());

        un_map->values[i] = std_dev;
    }

    for ( int i = 0; i < ensemble_maps.size(); ++i ) delete ensemble_maps[i];
    ensemble_maps.clear();

    return un_map;
}

WrfGridValueMap* WrfEnsembleManager::GetUncertaintyMap(WrfElementType element){
    std::map< WrfElementType, WrfGridValueMap* >::iterator iter = uncertainty_maps_.find(element);
    if ( iter != uncertainty_maps_.end() ){
        return iter->second;
    } else {
        WrfGridValueMap* value_map = GenerateUncertaintyMaps(element);
        if ( value_map != NULL ){
            uncertainty_maps_.insert(std::map< WrfElementType, WrfGridValueMap* >::value_type(element, value_map));
        }
        return value_map;
    }
}

WrfGridValueMap* WrfEnsembleManager::GenerateEnsembleForecastingMap(ProbabilisticPara& para){
    std::vector< float* > current_data;
    if ( !WrfDataManager::GetInstance()->GetGridData(current_date_, WRF_NCEP_ENSEMBLES, para.element, fhour_, current_data) ) return NULL;

    WrfGridValueMap* value_map = new WrfGridValueMap;
    value_map->model_type = WRF_ELEMENT_PROBABILISTIC;
    value_map->element_type = WRF_PROBABILISTIC;
    WrfDataManager::GetInstance()->GetEnsembleMapRange(value_map->map_range);

    value_map->values = new float[value_map->map_range.x_grid_number * value_map->map_range.y_grid_number];

    for ( int i = 0; i < value_map->map_range.x_grid_number * value_map->map_range.y_grid_number; ++i ){
        float accu_count = 0;
        for ( int j = 0; j < current_data.size(); ++j ){
            float temp_value = *(current_data[j] + i);
            if ( temp_value > para.thresh ) accu_count += 1;
        }
        accu_count /= current_data.size();
        value_map->values[i] = accu_count;
    }

    return value_map;
}

WrfGridValueMap* WrfEnsembleManager::GenerateAverageMap(WrfElementType element){
	std::vector< WrfGridValueMap* > ensemble_maps;
	if ( !WrfDataManager::GetInstance()->GetGridValueMap(current_date_, WRF_NCEP_ENSEMBLES, element, fhour_, ensemble_maps) ) return NULL;
	if ( ensemble_maps.size() <= 0 ) return NULL;

	WrfGridValueMap* average_map = ensemble_maps[0]->DeepCopy();
	average_map->model_type = WRF_NCEP_ENSEMBLE_MEAN;
	average_map->element_type = element;

	MapRange ens_map_range = ensemble_maps[0]->map_range;
	float normalize_value = 1.0;
	for ( int i = 0; i < ens_elements_.size(); ++i )
		if ( ens_elements_[i] == element ){
			normalize_value = ens_element_normalize_values_[i];
			break;
		}

	for ( int i = 0; i < ens_map_range.x_grid_number * ens_map_range.y_grid_number; ++i ){
		double average_value = 0.0;
		for ( int j = 0; j < ensemble_maps.size(); ++j ) average_value += ensemble_maps[j]->values[i];
		average_value /= ensemble_maps.size();

		average_map->values[i] = average_value;
	}

	for ( int i = 0; i < ensemble_maps.size(); ++i ) delete ensemble_maps[i];
	ensemble_maps.clear();

	return average_map;
}