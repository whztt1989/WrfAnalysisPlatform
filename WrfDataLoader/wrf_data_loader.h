#ifndef WRF_DATA_LOADER_H_
#define WRF_DATA_LOADER_H_

#include "wrf_data_loader_global.h"

#include <vector>
#include <string>

#include "wrf_data_common.h"

class WRF_DATA_EXPORT WrfDataLoader
{
public:
    static WrfDataLoader* GetInstance();
    static bool DeleteInstance();

    void SetRootPath(std::string& root_path);
    WrfValueMap* LoadData(WrfTime& time, WrfModelType model_type, WrfElementType element_type, int exposion_time = 0);
    void LoadData(WrfTime& begin_time, WrfTime& end_time, WrfModelType model_type, WrfElementType element_type, std::vector< WrfValueMap* >& value_maps, int exposion_time = 0);

protected:
    WrfDataLoader();
    ~WrfDataLoader();

    static WrfDataLoader* instance_;

private:
    std::string data_root_path_;

    WrfDiscreteValueMap* LoadFormatThreeData(std::string& file_name);

    /**
    * The structured grid data
    **/
    WrfGridValueMap* LoadFormatFourData(std::string& file_name);
};

#endif