#include "wrf_forecasting_manager.h"
#include "wrf_data_manager.h"
#include "wrf_ensemble_manager.h"
#include "wrf_reforecast_manager.h"

WrfForecastingManager* WrfForecastingManager::instance_ = NULL;

WrfForecastingManager* WrfForecastingManager::GetInstance(){
    if ( instance_ == NULL ){
        instance_ = new WrfForecastingManager;
    }

    return instance_;
}

bool WrfForecastingManager::DeleteInstance(){
    if ( instance_ != NULL ){
        delete instance_;
        return true;
    } else {
        return false;
    }
}

WrfForecastingManager::WrfForecastingManager(){
    maps_.resize(5);
}

WrfForecastingManager::~WrfForecastingManager(){

}

void WrfForecastingManager::AddProbabilisticForecast(ProbabilisticPara& para){
    paras_.push_back(para);

    // add ensemble probabilistic forecast
    WrfGridValueMap* ens_map = WrfEnsembleManager::GetInstance()->GenerateEnsembleForecastingMap(para);
    maps_[WRF_ENSEMBLER_PQPF].push_back(ens_map);

    // add reforecast based probabilistic forecast
    WrfGridValueMap* refcst_map = WrfReforecastManager::GetInstance()->GenerateForecastingMap(para);
    maps_[WRF_REFORECAST_PQPF].push_back(refcst_map);

	WrfGridValueMap* ens_average_map = WrfEnsembleManager::GetInstance()->GenerateAverageMap(para.element);
	maps_[WRF_ENSEMBLE_AVERAGE].push_back(ens_average_map);

    WrfGridValueMap* adapted_refcst_map = refcst_map->DeepCopy();
    maps_[WRF_ADAPTED_REFORECAST_PQPF].push_back(adapted_refcst_map);
}

WrfGridValueMap* WrfForecastingManager::GetForecastingMap(WrfForecastingType forecasting_type, ProbabilisticPara& para){
    for ( int i = 0; i < paras_.size(); ++i )
        if ( paras_[i].element == para.element && paras_[i].thresh == para.thresh ){
            return maps_[forecasting_type][i];
        }
    return NULL;
}

void WrfForecastingManager::GetForecastingMaps(WrfForecastingType forecasting_type, std::vector< ProbabilisticPara >& paras, std::vector< WrfGridValueMap* >& maps){
    if ( forecasting_type != WRF_REALISTIC ){
        paras = paras_;
        maps = maps_[forecasting_type];
    }
}