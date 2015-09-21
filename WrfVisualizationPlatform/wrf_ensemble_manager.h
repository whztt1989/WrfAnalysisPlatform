#ifndef WRF_ENSEMBLE_MANAGER_H_
#define WRF_ENSEMBLE_MANAGER_H_

#include <QtCore/QDateTime>
#include <map>
#include <vector>
#include "wrf_data_common.h"

class WrfEnsembleManager {
public:
    static WrfEnsembleManager* GetInstance();
    static bool DeleteInstance();

    void SetCurrentDate(QDateTime& date);

    void SetForecastingParameters(int fcst_hour, std::vector< WrfElementType >& ensemble_elements, std::vector< float >& normalize_values);

	QDateTime CurrrentDate() { return current_date_; }
	int FcstHour() { return fhour_; }

    WrfGridValueMap* GetUncertaintyMap(WrfElementType element);

    WrfGridValueMap* GenerateEnsembleForecastingMap(ProbabilisticPara& para);

	WrfGridValueMap* GenerateAverageMap(WrfElementType element);

protected:
    WrfEnsembleManager();
    ~WrfEnsembleManager();

    static WrfEnsembleManager* instance_;

private:
    QDateTime current_date_;
    int fhour_;

    std::vector< WrfElementType > ens_elements_;
    std::vector< float > ens_element_normalize_values_;

    std::map< WrfElementType, WrfGridValueMap* > uncertainty_maps_;

    WrfGridValueMap* GenerateUncertaintyMaps(WrfElementType element);
};

#endif