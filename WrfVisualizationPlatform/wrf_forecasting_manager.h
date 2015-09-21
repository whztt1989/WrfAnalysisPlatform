#ifndef WRF_FORECASTING_MANAGER_H_
#define WRF_FORECASTING_MANAGER_H_

#include "wrf_data_common.h"

class WrfForecastingManager{
public:
    static WrfForecastingManager* GetInstance();
    static bool DeleteInstance();

    void AddProbabilisticForecast(ProbabilisticPara& para);

    WrfGridValueMap* GetForecastingMap(WrfForecastingType forecasting_type, ProbabilisticPara& para);
    void GetForecastingMaps(WrfForecastingType forecasting_type, std::vector< ProbabilisticPara >& paras, std::vector< WrfGridValueMap* >& maps);

protected:
    WrfForecastingManager();
    ~WrfForecastingManager();

    static WrfForecastingManager* instance_;

private:
    std::vector< ProbabilisticPara > paras_;
    std::vector< std::vector< WrfGridValueMap* > > maps_;
};

#endif