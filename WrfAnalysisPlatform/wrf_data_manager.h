#ifndef WRF_DATA_MANAGER_H_
#define WRF_DATA_MANAGER_H_

#include <QtCore/QString>
#include <map>
#include <vector>
#include <string>

#include "wrf_data_stamp.h"

#include "wrf_data_converter.h"

class WrfStatisticSolver;

class WrfDataManager
{
public:
	static WrfDataManager* GetInstance();
	static bool DeleteInstance();

	WrfDataManager();
	~WrfDataManager();

	int current_time() { return current_time_; }

	WrfGridValueMap* GetBiasMap();
	WrfGridValueMap* GetWindVarMap();

	WrfParallelDataSet* GetParallelDataSet(bool is_absolute);
	WrfParallelDataSet* GetComparedParallelDataSet(bool is_absolute);

	WrfLineChartDataSet* GetLineChartDataSet(WrfGeneralDataStampType type, bool is_absolute);
	WrfLineChartDataSet* GetComparedLineChartDataSet(WrfGeneralDataStampType type, bool is_absolute);

	WrfHistogramDataSet* GetHistogramDataSet();
	void GetSiteAlpha(std::vector< float >& alpha);

	void SetHighLightIndex(std::vector< bool >& index);
	void GetHighLightIndex(std::vector< bool >& index);

	void SetHistoryWeight(float weight) { history_weight_ = weight; }
	void SetCurrentWeight(float weight) { current_weight_ = weight; }
	void SetHistoryLength(int lenght) { history_time_length_ = lenght; }

	void SetRootDir(const std::string dir) { root_dir_ = dir + "/"; }
	void LoadDefaultData(int current, int day_length);
	void GenerateBiasMap(int day_length);
	void GenerateBaseMaps();
	void GenerateInitialModelWeight();
	void UpdateHistoricalData();
	void UpdateGridModelWeight(int time);
	void UpdateSimpleGridModelWeight(int time);
	void UpdateSiteAlphaMap();
	void GetAverageInfoGainMaps(std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index);
	void GetSingleAttributeInfoGainMaps(int attrib_index, std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index);
	WrfGridValueMap* GetInfoGainMap(int index) { return info_gain_maps.at(index); }
	void GetInfoGainWeight(int index, std::vector< float >& weights);
	int AddStoryStamp(WrfGridValueMap* value_stamp, std::vector< int >& related_ids);
	WrfGridValueMap* GetStoryStamp(int index);

	WrfDataRecordSet* GetNumericalDataRecordSet(const WrfTime& time);
	WrfDataRecordSet* GetHistoricalDataRecordSet(const WrfTime& time);
	std::vector< RecordSetMap* >* GetBiasVec() { return &bias_set_vec_; }

	void GetSelectedGridIndex(std::vector< int >& seleted_index);
	void set_seleted_grid_index(std::vector< int >& index);
	void set_compared_grid_index(std::vector< int >& index);
	void GetAttribWeight(std::vector< float >& weight);
	void SetAttribWeight(std::vector< float >& weight);
	void GetModelWeight(std::vector< float >& weight);
	void GetAttribWeightTheropy(int step_num, std::vector< std::vector< float > >& theropy);

	void set_longitude_grid_space(float space) { longitude_grid_space_ = space; }
	void set_latitude_grid_space(float space) { latitude_grid_space_ = space; }
	void set_start_longitude(float value) { start_longitude_ = value; }
	void set_end_longitude(float value) { end_longitude_ = value; }
	void set_start_latitude(float value) { start_latitude_ = value; }
	void set_end_latitude(float value) { end_latitude_ = value; }
	void set_longtitude_grid_number(int value) { longitude_grid_number_ = value; }
	void set_latitude_grid_number(int value) { latitude_grid_number_ = value; }
	float longitude_grid_space() { return longitude_grid_space_; }
	float latitude_grid_space() { return latitude_grid_space_; }
	float start_longitude() { return start_longitude_; }
	float end_longitude() { return end_longitude_; }
	float start_latitude() { return start_latitude_; }
	float end_latitude() { return end_latitude_; }
	int longitude_grid_number() { return longitude_grid_number_; }
	int latitude_grid_number() { return latitude_grid_number_; }
	void set_grid_size(int size) { grid_size_ = size; }
	int grid_size() { return grid_size_; }
	void set_wind_var_threshold(float value) { wind_var_threshold_ = value; }
	float wind_var_threshold() { return wind_var_threshold_; }

	WrfParallelDataSet* GetTestPcpData() { return bias_parallel_dataset_; }
	WrfHistogramDataSet* GetTestHistogram() { return histogram_dataset_; }

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & root_dir_;
		ar & latitude_grid_space_;
		ar & longitude_grid_space_;
		ar & longitude_grid_number_;
		ar & latitude_grid_number_;
		ar & start_longitude_;
		ar & end_longitude_;
		ar & start_latitude_;
		ar & end_latitude_;
		ar & story_line_maps_;
		ar & map_related_elements_;
		//ar & bias_set_vec_;
		//ar & numerical_data_;
		//ar & historical_data_;
		//ar & base_map_data_;
		//ar & data_converter_;
		//ar & model_weight_;
		//ar & selected_grid_index_;
		//ar & current_time_;
		//ar & histogram_dataset_;
	}

	static WrfDataManager* instance_;

protected:

private:
	void LoadT639();
	void LoadEcFine();
	void LoadNcep();
	void LoadJapanGsm();
	void LoadScene();
	void NormalizeData();

	std::vector< float > attribute_max_value_;
	std::vector< float > attribute_min_value_;

	float history_weight_;
	float current_weight_;

	int current_time_;
	int history_time_length_;
	int grid_size_;

	std::string root_dir_;
	float longitude_grid_space_, latitude_grid_space_;
	float start_longitude_, end_longitude_, start_latitude_, end_latitude_;
	int latitude_grid_number_, longitude_grid_number_;

	std::vector< float > site_alpha_;

	std::vector< RecordSetMap* > bias_set_vec_;
	std::vector< DataStampMap* > numerical_data_;
	DataStampMap historical_data_;
	DataStampMap base_map_data_;
	WeightMap grid_model_weight_map_;

	WrfGridValueMap* bias_map_;
	WrfGridValueMap* wind_var_map_;

	WrfParallelDataSet* bias_parallel_dataset_;
	WrfParallelDataSet* absolute_parallel_dataset_;
	WrfLineChartDataSet* bias_line_chart_dataset_;
	WrfLineChartDataSet* absolute_line_chart_dataset_;

	WrfParallelDataSet* compared_bias_parallel_dataset_;
	WrfParallelDataSet* compared_absolute_parallel_dataset_;
	WrfLineChartDataSet* compared_bias_line_chart_dataset_;
	WrfLineChartDataSet* compared_absolute_line_chart_dataset_;

	WrfHistogramDataSet* histogram_dataset_;

	std::vector< float > attribute_weight_;
	std::vector< float > model_weight_;
	float similarity_threshold_;
	float wind_var_threshold_;

	std::vector< bool > selected_grid_index_;
	std::vector< bool > compared_grid_index_;
	std::vector< bool > selected_patch_index_;
	std::vector< bool > high_light_index_;

	std::vector< std::vector< float > > attrib_weight_theropy_;

	WrfDataConverter* data_converter_;
	WrfStatisticSolver* statistic_solver_;

	std::vector< WrfGridValueMap* > info_gain_maps;
	std::vector< std::vector< float > > info_gain_weight_vec;

	std::vector< WrfGridValueMap* > story_line_maps_;
	std::vector< std::vector< int > > map_related_elements_;

	void LoadGridData(QString file_path, WrfDataStampType stamp_type, WrfGeneralDataStampType data_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter);
	void LoadGridDataFormatThree(QString file_path, WrfDataStampType stamp_type, WrfGeneralDataStampType data_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter);
	void LoadDiscreteData(QString file_path, WrfDataStampType stamp_type, WrfGeneralDataStampType data_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter);
	void LoadHighAltitudeData(QString file_path, WrfDataStampType stamp_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter);
	void MatrixMul(float* matrix1, int m1, int n1, float* matrix2, int m2, int n2, float* result);
	void MatrixT(float* matrix, int m, int n, float* result);
	void MatrixInv(float* matrix, int m, int n, float* result);
	float Ftime(int time_bias);

	void GetParallelDataSet(std::vector< bool >& selected_index, bool is_absolute, WrfParallelDataSet* parallel_dataset);
	void GetLineChartDataSet(WrfGeneralDataStampType data_type, std::vector< bool >& selected_index, bool is_absolute, WrfLineChartDataSet* line_chart_dataset);
};

#endif