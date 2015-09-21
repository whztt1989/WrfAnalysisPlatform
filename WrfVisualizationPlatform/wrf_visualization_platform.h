#ifndef WRF_VISUALIZATION_PLATFORM_H
#define WRF_VISUALIZATION_PLATFORM_H

#include "config.h"
#include <QtGui/QMainWindow>
#include <QtGui/QScrollArea>
#include <QtCore/QDateTime>
#include <QtGui/QStackedWidget>
#include <QtGui/QCalendarWidget>
#include "ui_wrf_visualization_platform.h"
#include "wrf_data_common.h"
#include "step_item.h"

class WrfDataManager;
class WrfEnsembleDataView;
class WrfEventSearchingView;
class WrfRegionDetectionView;
class WrfForecastComparisonWidget;
class WrfEventAnalyzingView;
class FlowChart;
class WrfEventComparisonView;

class WrfVisualizationPlatform : public QMainWindow
{
    Q_OBJECT

public:
    WrfVisualizationPlatform(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~WrfVisualizationPlatform();

private:
    Ui::WrfVisualizationPlatformClass ui;

    WrfDataManager* data_manager_;

    WrfEnsembleDataView* ensemble_data_view_;

    WrfRegionDetectionView* ensemble_region_detection_view_;

    WrfRegionDetectionView* reforecast_region_detection_view_;
    
    WrfEventSearchingView* event_searching_view_;

    WrfEventAnalyzingView* event_analyzing_view_;

    WrfEventComparisonView* event_comparison_view_;

    WrfForecastComparisonWidget* comparison_view_;

    QStackedWidget* main_stacked_widget_;

    FlowChart* flow_chart_;

	QCalendarWidget* calendar_widget_;

    // the selected date to be calibrated
    QDateTime current_datetime_;
    int fcst_hour_;

    // the length of days to be evaluated
    int searching_day_length_, searching_year_length_;
    int searching_grid_size_;
    int searching_analog_num_;

    std::vector< WrfElementType > ensemble_elements_;
    std::vector< WrfElementType > ensemble_mean_elements_;
    std::vector< float > element_weight_;
    std::vector< float > element_normalize_values_;

	std::vector< QPointF > region_contour;
	std::vector< int > selected_grid_index;

    MapRange retrieval_map_range_;
    
    std::vector< ProbabilisticPara > prob_forecast_para_vec_;

    std::vector< bool > is_procedure_executed_;

    void InitWidgets();
    void UpdateRetrievalMapRange(std::vector< QPointF >& region_contour);

    private slots:
        void OnActionLoadEnsembleDataTriggered();
        void OnActionLoadNarrDataTriggered();
        void OnActionLoadChinaReanalysisDataTriggered();
        void OnActionLoadCombinedDataTriggered();

        void OnActionViewEnsembleTriggered();
        void OnActionPreProcessTriggered();
        void OnActionProbabilisticForecastTriggered();
        void OnActionVisualAssistedFilteringTriggered(int);
        void OnActionShowTimeEventTriggered();
        void OnActionApplyCalibrationTriggered();

        void OnStepItemClicked(StepItemType type);
        void OnStepItemDoubleClicked(StepItemType type);

		void OnRmsMappingChanged();

        void OnActionRawDataRetrievalTriggered();
};

#endif // WRF_VISUALIZATION_PLATFORM_H
