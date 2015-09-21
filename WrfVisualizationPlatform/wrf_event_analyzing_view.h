#ifndef WRF_EVENT_ANALYZING_VIEW_H_
#define WRF_EVENT_ANALYZING_VIEW_H_

#include <QtGui/QWidget>
#include <QtGui/QToolBar>
#include <QtCore/QDateTime>
#include <QtGui/QAction>
#include <QtGui/QSlider>
#include <vector>
#include "wrf_data_common.h"

class WrfSimilarityTimeLineView;
class WrfStampViewer;
class WrfMdsPlotView;
class WrfStamp;
class WrfImageViewer;
class ScatterPlot;
class WrfEnsembleDataView;

class WrfEventAnalyzingView : public QWidget{
    Q_OBJECT
public:
    WrfEventAnalyzingView();
    ~WrfEventAnalyzingView();

    void SetEventInfo(WrfElementType element, std::vector< float >& similarity);

    void SetViewingRange(MapRange& range);
	void SetSelectedGridIndex(std::vector< int >& selected_grid_index);
	void SetSelectionPath(std::vector< QPointF >& contour);

    void UpdateWidget();

private:
    WrfSimilarityTimeLineView* time_line_view_;
    WrfStampViewer* stamp_viewer_;
    WrfStampViewer* current_stamp_viewer_;
    WrfMdsPlotView* mds_plot_;

	WrfEnsembleDataView* ens_data_view_;

    WrfImageViewer* average_viewer_;
    WrfImageViewer* pqpf_viewer_;
    ScatterPlot* scatter_plot_;

    QToolBar* main_tool_bar_;
    QAction* action_show_pqpf_;
    QAction* action_show_average_result_;
    QAction* action_show_precipitation_scatter_plot_;
	QAction* action_show_scatter_plot_;
    QSlider* analog_num_slider_;

    WrfElementType forecast_element_;
    QDateTime forecasting_date_;
    int fhour_;

	int mds_mode_;

    std::vector< int > retrieval_date_time_;
    std::vector< float > date_similarity_;
	std::vector< int > selected_grid_index_;
	std::vector< QPointF > contour_path_;

    MapRange viewing_range_;

    int year_length_, day_length_;
    int selected_analog_number_, previous_selected_analog_number_;

    std::vector< int > max_similarity_index_;

    std::vector< WrfStamp* > event_stamps_;
    std::vector< WrfStamp* > current_event_stamps_;

    void InitWidget();
    void UpdateTimeLineView();
    void UpdateCurrentStampView();
    void UpdateHistoryStampView(int mode);
    void UpdateMdsPlotView(int mode);

    private slots:
        void OnMdsSelectedChanged();
        void OnAnalogNumberChanged();
        void OnActionGenerateAverageTriggered();
        void OnActionGeneratePqpfTriggered();
        void OnActionShowPrecipitationScatterPlotTriggered();
		void OnActionShowVariableScatterPlotTriggered();

		void OnFcstMdsTriggered();
		void OnReanalysisMdsTriggered();

		void OnEventTimeTriggered(int time);
};

#endif