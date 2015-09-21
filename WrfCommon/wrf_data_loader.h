#ifndef WRF_DATA_LOADER_H_
#define WRF_DATA_LOADER_H_

#include "wrf_common_global.h"

#include <vector>
#include <string>

#include "wrf_data_common.h"

class QDomElement;

class WRFCOMMON_EXPORT WrfDataLoader
{
public:
    static WrfDataLoader* GetInstance();
    static bool DeleteInstance();

    WrfValueMap* LoadData(WrfTime& time, WrfModelType model_type, WrfElementType element_type, int exposion_time = 0);
    void LoadData(WrfTime& begin_time, WrfTime& end_time, WrfModelType model_type, WrfElementType element_type, std::vector< WrfValueMap* >& value_maps, int exposion_time = 0);

protected:
    WrfDataLoader();
    ~WrfDataLoader();

    static WrfDataLoader* instance_;

private:
    std::string data_root_path_;

    std::vector< std::vector< std::string > > path_infos_;
    std::vector< std::vector< int > > format_infos_;

    WrfDiscreteValueMap* LoadFormatThreeData(std::string& file_name);

    /**
    * The structured grid data
    **/
    WrfGridValueMap* LoadFormatFourData(std::string& file_name);

    void LoadFormatNcData(std::string& file_name);

    void InitLoaderInfo();
    void LoadModelPath(QDomElement& dom_element, std::string& model_root, std::vector< std::string >& info, std::vector< int >& format_info);
};

#endif