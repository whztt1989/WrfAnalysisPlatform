#ifndef WRF_EVENT_SEARCHING_VIEW_H_
#define WRF_EVENT_SEARCHING_VIEW_H_

#include <QtGui/QWidget>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QStackedWidget>
#include <QtCore/QDateTime>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QTabWidget>
#include <QtGui/QDockWidget>
#include <vector>
#include <map>
#include "wrf_data_common.h"
#include "comparison_line_chart.h"
#include "table_lens.h"
#include "ui_subregion_rms_para_widget.h"
#include "ui_main_view_para_widget.h"
#include "ui_variable_rms_para_widget.h"

class WrfSimilarityTimeLineView;
class WrfScatterPlotMatrix;
class WrfDataManager;
class WrfReforecastManager;
class WrfMdsPlotView;
class PlotMatrixDataModel;
class WrfDetailInformationViewWidget;
class WrfElementDistributionWidget;
class WrfGridInformationWidget;
class WrfImageViewer;
class WrfSubregionRmsView;
class WrfGridRmsErrorElement;
class WrfGridMapELement;
class WrfImageElement;
class WrfSliderDialog;
class WrfSubregionSuggestionDialog;

typedef std::vector< std::vector< float* > > ElementSeriesVec;

class WrfEventSearchingView : public QWidget{
    Q_OBJECT
public:
    WrfEventSearchingView();
    ~WrfEventSearchingView();

	void SetRetrievalElements(std::vector< WrfElementType >& elements);
    void SetSelectedGridPointIndex(std::vector< int >& selected_index);
    void GetDateSimilarity(std::vector< float >& similarity);
	void UpdateDataAndView();
	void UpdateView();
   
signals:
    void FilteringApplied();
    void EventAnalyzingTriggered();

private:
    QToolBar* main_tool_bar_;

	QToolBar* geomap_tool_bar_;
	WrfImageViewer* geomap_view_;
	WrfGridRmsErrorElement* grid_rms_element_;
	std::vector< RmsUnit > grid_rms_units_;
	WrfGridMapELement* map_element_;
	WrfImageElement* background_element_;
	MapRange viewing_range_;

	ComparisonLineChart* region_rms_view_;
	ComparisonLineChartDataset* region_rms_dataset_;

	QToolBar* subregion_rms_tool_bar_;
	WrfSubregionRmsView* subregion_rms_view_;

	QToolBar* variable_tool_bar_;
    QTabWidget* table_lens_tab_widget_;
    std::vector< TableLens* > data_table_lens_;
    std::vector< TableLensDataset* > table_lens_dataset_;
	std::vector< float > normalized_values_;

	Ui::MainViewParaWidget main_view_para_widget_;
	QWidget* main_function_widget_;
	Ui::SubregionRmsParaWidget subregion_rms_para_widget_;
	QWidget* subregion_function_widget_;
	Ui::VariableRmsParaWidget variable_rms_para_widget_;
	QWidget* variable_function_widget_;

	// actions for main tool bar
    QAction* action_apply_selection_;
    QAction* action_show_event_analyzing_view_;
	QAction* action_show_selected_scatter_plot_;
	QAction* action_filtering_boosting_;
	QAction* action_voting_boosting_;

	// actions for geomap tool bar
	QAction* action_grid_brush_selection_;

	// actions for subregion rms filtering view
	QAction* action_subregion_rms_brush_selection_;
	QAction* action_subregion_rms_apply_selection_;

	// actions for the variable filtering view
	QAction* action_view_value_;
	QAction* action_view_scale_;
	QAction* action_setting_;

    WrfDataManager* data_manager_;
    WrfReforecastManager* reforecast_manager_;

	// overall parameters
	QDateTime forecasting_date_;
	std::vector< int > reference_date_time_;
	std::vector< WrfElementType > retrieval_elements_;
	MapRange retrieval_map_range_;
	
	int voting_mode_; // 0 for filtering and 1 for voting

	std::vector< int > selected_grid_point_index_;
	std::vector< int > current_selection_index_;

	std::vector< std::vector< bool > > is_date_selected_;	// [grid point][date]
	std::vector< std::vector< bool > > aggregated_is_date_selected_;

	// region RMS parameters
    int current_retrieval_grid_size_index_;
    int current_retrieval_grid_size_;
    std::vector< int > selected_retrieval_grid_sizes_;
    std::vector< int > region_selected_analog_nums_;

	// subregion RMS parameters
    std::vector< int > subregion_selected_analog_nums_;
    std::vector< std::vector< int > > selected_grid_n_paras_;
	std::vector< std::vector< float > > sorted_aggregated_rms_;	// [grid point][date] sorted similarity of the main element
	std::vector< std::vector< int > > sorted_date_index_;
	std::vector< float > aggregated_rms_average_;
	std::vector< int > subregion_sort_index_;
	float min_rms_value_, max_rms_value_;
	std::vector< float > climate_distribution_;
	std::vector< std::vector< int > > subregion_rms_suggestion_values_;

	// variable RMS parameters
    std::vector< std::vector< std::vector< float > > > date_grid_rms_;	// [date][grid point][element]
    

    // This is used to update the reforecast manager
    std::vector< float > date_similarity_;

	void UpdateSubregionRmsSuggestionValue(float cover_rate, std::vector< int >& suggestion_value);
	void GaussianSmooth(std::vector< int >& suggestion_value);

    void InitWidgets();
    void UpdateViewData();

	void UpdateAggregatedSelection();
    void UpdateGeomapView();
    void UpdateVariableRmsView();
    void UpdateSubregionRmsView();
    void UpdateRegionRmsView();

    void UpdateDateSimilarity();

	void UpdateSuggestionParaRange();
	void UpdateVariableSuggestionScale(TableLensDataset* data_set, int e);

    private slots:
		// main tool bar slots
        void OnApplySelection();
		void OnActionBoostingTriggered();
		void ShowSelectedScatterPlot();

        // geomap tool bar slots
        void OnGeomapSelectionTriggered();

		// main function widget slots
		void OnShowMultipleChanged();
		void OnRegionMaxViewingDateChanged();
		void OnShowRegionSuggestionChanged(int state);
		void OnRegionRmsSuggestionParaChanged();

		// subregion RMS tool bar slots
		void OnActionSettingTriggered();
		void OnActionViewValueTriggered(bool checked);
		void OnActionViewScaleTriggered(bool checked);
		void OnSubregionMaxViewingDateChanged();

		// subregion function widget slots
		void OnSortHorizontalChanged();
		void OnSortVerticalChanged();
		void OnShowSubregionSuggestionChanged(int state);
		void OnSubregionRmsSuggestionParaChanged();
		void OnSubregionRmsSuggestionLowerBoundChanged();
		void OnSubregionRmsSuggestionUpperBoundChanged();
		void OnSubregionViewModeChanged(int id);

		// variable function widget slots
		void OnShowVariableSuggstionChanged(int state);
		void OnVariableSuggestionparaChanged();

		// coordination slots
        void OnVariableRmsSelectionChanged(int);
        void OnGeomapSelectionChanged();
        void OnSubregionRmsSelectionChanged();
        void OnRegionRmsSelectionChanged(int);
        void OnRetrievalGridSizeChanged(int);

        void OnTableLensRecordIndexChanged(int, int);
        void OnSubregionRecordIndexChanged(int, int);

		void OnRmsColorMappingChanged();
};

#endif