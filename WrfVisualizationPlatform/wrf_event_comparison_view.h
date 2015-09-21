#ifndef WRF_EVENT_COMPARISON_VIEW_H_
#define WRF_EVENT_COMPARISON_VIEW_H_

#include <QtGui/QWidget>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtCore/QDateTime>
#include <QtGui/QTabWidget>
#include <QtGui/QScrollArea>
#include <QtGui/QColorDialog>
#include "ui_event_comparison_para_widget.h"
#include "wrf_data_common.h"
#include "parallel_coordinate.h"
#include "wrf_image_viewer.h"
#include "scatter_plot.h"

class WrfStampViewer;
class WrfRadarCoordinateView;
class WrfRadarItem;
class WrfStamp;
class ManualGroupTable;
class RankGroupTable;

class DateGroup{
public:
    DateGroup(){}
    ~DateGroup(){}

    QString group_name;
    QColor group_color;
    std::vector< int > mem_index;
};

class WrfEventComparisonView : public QWidget
{
    Q_OBJECT

public:
    WrfEventComparisonView();
    ~WrfEventComparisonView();

    void SetSelectedGridIndex(std::vector< int >& grid_index);
    void SetViewingRange(MapRange& range) { viewing_range_ = range; }
    void SetBasicVariables(std::vector< WrfElementType >& variables);
    void LoadData();

    public slots:
        void OnRadarItemChanged();

protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

private:
    WrfStampViewer* current_stamp_view_;
    WrfStamp* current_stamp_;

    WrfStampViewer* historical_stamp_view_;

    WrfRadarCoordinateView* radar_coordinate_view_;

    ParallelCoordinate* parallel_coordinate_;
    ParallelDataset* parallel_dataset_;

    Ui::EventComparisonParaWidget event_para_ui_;
    QWidget* event_para_widget_;

    QTabWidget* group_tab_widget_;
    ManualGroupTable* manual_group_widget_;
    RankGroupTable* rank_group_widget_;

    QToolBar* main_tool_bar_;
    QAction* action_brush_;
    QAction* action_manual_group_mode_;
    QAction* action_rank_group_mode_;

    QAction* action_show_pqpf_;
    QAction* action_show_average_result_;
    QAction* action_show_precipitation_scatter_plot_;

    QAction* action_add_manual_group_;
    QAction* action_add_rank_group_;

    WrfImageViewer* average_viewer_;
    WrfImageViewer* pqpf_viewer_;
    ScatterPlot* scatter_plot_;

    MapRange viewing_range_;

    std::vector< int > selected_grid_index_;
    std::vector< WrfElementType > basic_variables_;

    std::vector< AxisData* > axis_data_vec_;
    std::vector< QString > axis_names_;
    std::vector< int > axis_order_;

    int max_analog_number_;
    std::vector< bool > is_date_selected_;
    int selected_date_number_;
    std::vector< WrfRadarItem* > date_items_;
    std::vector< WrfStamp* > date_stamps_;
    std::vector< QColor > date_color_;

    std::vector< int > current_selection_index_;

    std::vector< std::vector< float > > pearson_values_;

    QPointF center_point_;
    std::vector< QPointF > axis_direction_vec_;
    float item_radius_[2];

    // force directed layout parameters
    float alpha_;
    int iteration_num_;

    WrfElementType forecast_element_;
    QDateTime forecasting_date_;
    int fhour_;
    std::vector< int > retrieval_date_time_;

    std::vector< DateGroup* > manual_groups_;
    std::vector< AxisData* > rank_groups_;

    QColor default_color_;
    bool is_ctrl_pressed_;

    void InitView();
    void UpdateAxisData();
    void UpdateDateSelection();
    void UpdateAxisOrder();
    void BruteforceSearch(std::vector< bool >& is_searched, float current_value, float& max_value, std::vector< int >& current_index);

    void UpdateRadarItems();
    void UpdateItemPositions();

    void UpdateCurrentStampView();
    void UpdateHistoryStampView();
    void UpdateParallelCoordinate();
    void UpdateRadarCoordinateView();

    void UpdateCurrentSelection();

    WrfStamp* GenerateStamp(int date_time);

    void SaveData();

    private slots:
        void OnItemRadiusChanged();
        void OnIterationNumberChanged();
        void OnAlphaValueChanged();
        void OnMaxRankChanged();

        void OnItemRadiusSliderChanged();
        void OnIterationNumberSliderChanged();
        void OnAlphaValueSliderChanged();
        void OnMaxRankSliderChanged();

        void OnItemSelectionChanged();
        void OnBrushActionTriggered();
        void OnAddManualGroupTriggered();
        void OnAddRankGroupTriggered();
        void OnDateColorModeChanged();

        void OnActionGenerateAverageTriggered();
        void OnActionGeneratePqpfTriggered();
        void OnActionShowPrecipitationScatterPlotTriggered();
};

#endif