#ifndef WRF_REFORECASTING_MANAGER_H_
#define WRF_REFORECASTING_MANAGER_H_

#include <map>
#include <QtCore/QDateTime>
#include "config.h"
#include "wrf_data_common.h"

class WrfGridRmsPack;

class WrfReforecastManager
{
public:
    static WrfReforecastManager* GetInstance();
    static bool DeleteInstance();

    /*
    * Set the current forecasting date
    */
    void SetForecastingDate(QDateTime& date);

    /*
    * Set the parameters related to analog based method 
    */
    void SetRetrievalParameters(int year_length, int day_length, int grid_size, int fcst_hour, int analog_num,
        std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements, 
        std::vector< float >& element_weights, std::vector< float >& normalize_values);

    QDateTime& forecasting_date() { return forecasting_date_; }
    int fcst_hour() { return fcst_hour_; }
    int retrieval_analog_num() { return retrieval_analog_num_; }
    int retrieval_grid_size() { return retrieval_grid_size_; }
    int retrieval_year_length() { return retrieval_year_length_; }
    int retrieval_day_length() { return retrieval_day_length_; }
    void GetRetrievalElements(std::vector< WrfElementType >& elements);
    void GetReferenceDateTime(std::vector< int >& date_time);

    void LoadData();
    /*
    * Get the RMS values based the initially selected retrieval grid size for global region
    */
    void PreProcessData();
    void PreProcessDataForEvent(AxisData* data, std::vector< int >& selected_grid_index);

    /*
    * Get the RMS values based on other retrieval grid sizes for the selected small region
    */
    void PreProcessFocusGrid(std::vector< int >& selected_index, std::vector< int >& selected_size);

    /*
    * Get the the RMS values of each element for selected grid point
    */
    void GetElementRmsValues(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< std::vector< float > > >& rms_values);

    /*
    * Get date selection for selected grid point
    */
    void GetDateSelection(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< bool > >& selection);

    /*
    * Get sorted aggregated RMS values for selected grid point
    */
    void GetSortedAggregatedRmsValues(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< float > >& sorted_rms_values, std::vector< std::vector< int > >& sorted_index);

    /*
    * Get sorted selected aggregated RMS values. This is used for glyph rendering in rms element.
    */
    void GetSortedSelectedAggregatedRmsValues(std::vector< std::vector< float > >& sorted_rms_values);

	/*
	*
	*/
	void GetSelectedClimatologicalDistribution(std::vector< float >& distribution);

    /* 
    * Update the date selection, this is what really affects the forecast result
    */
    bool UpdateDateSelection(int grid_size, std::vector< int >& selected_index, std::vector< std::vector< bool > >& is_date_selected);

    /*
    * Generate probabilistic forecasting map using the current date selection
    */
    WrfGridValueMap* GenerateForecastingMap(ProbabilisticPara& para);
	WrfGridValueMap* GenerateRawForecastingMap(ProbabilisticPara& para);
    
    float GetGridPointRmsValue(int grid_index, int date_index, int grid_size);

    void GetNormalizedValues(std::vector< float >& normalize_value);

    float GetCurrentGridAveragesValues(int grid_size, WrfElementType element, int grid_index);
    
    
protected:
    WrfReforecastManager();
    ~WrfReforecastManager();

    static WrfReforecastManager* instance_;

private:
    // forecasting
    QDateTime forecasting_date_;
    int fcst_hour_;

    // available retrieval data
    int retrieval_time_step_;
    int retrieval_year_length_, retrieval_day_length_;
    int retrieval_grid_size_;
    int retrieval_analog_num_;
    std::vector< WrfElementType > retrieval_ens_elements_;
    std::vector< WrfElementType > retrieval_ens_mean_elements_;
    std::vector< float > retrieval_element_weights_;
    std::vector< float > retrieval_normalize_values_;

    MapRange retrieval_map_range_;

    std::vector< int > retrieval_time_vec_;
    std::map< int, int > retrieval_date_map_;
    // [date][element][ens]
    std::vector< std::vector< std::vector< float* > > > retrieval_value_maps_;
    std::vector< bool > is_retrieval_data_completed_;

    // [element][ens]
    std::vector< std::vector< float* > > current_value_maps_;

    std::map< int, WrfGridRmsPack* > grid_rms_pack_map_; 

    std::vector< std::vector< bool > > is_date_selected_;

	// climatological distribution of selected grids
	std::vector< float > climate_distribution_;

    void LoadReferenceData();
    void GenerateRetrievalTimeVec();
    void UpdateDateSelection();

    WrfGridRmsPack* GenerateSimilarityPack(int grid_size, std::vector< int >& selected_grid_index);
    float GetRetrievalElementRms(int date_index, int element, int grid_index, int grid_size);
};

#endif