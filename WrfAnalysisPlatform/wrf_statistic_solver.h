#ifndef WRF_STATISTIC_SOLVER_H_
#define WRF_STATISTIC_SOLVER_H_

#include <vector>
using std::vector;

#include "wrf_data_manager.h"

class WrfStatisticSolver
{
public:
	WrfStatisticSolver();
	~WrfStatisticSolver();
	bool GetSimilarityMap(int longitude_index, int latitude_index, int current_time, int time_length,
		std::vector< RecordSetMap* >& bias_set_vec, DataStampMap& historical_map, 
		std::vector< float >& attrib_weight, std::vector< float >& model_weight, std::vector< float >& attrib_max_values, std::vector< float >& attrib_min_values,
		WrfGridValueMap* similarity_map);
	bool GetBiasVarMap(std::vector< RecordSetMap* >& bias_set_vec, 
		std::vector< float >& attrib_weight, WeightMap& grid_model_weight, int grid_size,
		std::vector< float >& grid_alpha_map, int current_time, int his_time_length, float history_weight, float current_weight, 
		WrfGridValueMap* bias_var_map); 
	bool GetWindVarianceMap(std::vector< RecordSetMap* >& bias_set_vec, 
		WeightMap& grid_model_weight, std::vector< float >& grid_alpha_map, int grid_size,
		int current_time, 
		WrfGridValueMap* wind_var_map);
	
	float GetHittingRate(std::vector< RecordSetMap* >& bias_set_vec,
		std::vector< float >& attrib_weight, WeightMap& grid_model_weight, int grid_size,
		std::vector< float >& grid_alpha_map, int current_time, int his_time_length, float threshold, WrfGridValueMap* test_map);

	float GetMapDifference(WrfGridValueMap* src, WrfGridValueMap* dst);
	float GetMutalDistance(WrfGridValueMap* src, WrfGridValueMap* dst);

	void SortValues(std::vector< float >& values, float& max_value, float& min_value);
	void QuickSort(int head, int end, float* data);
	void TestSort();

protected:
	

private:
	float Ftime(int time_bias);
};

#endif