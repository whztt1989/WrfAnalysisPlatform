#ifndef WRF_ENSEMBLE_DATA_VIEW_H_
#define WRF_ENSEMBLE_DATA_VIEW_H_

#include <QtGui/QWidget>
#include <QtGui/QScrollArea>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QScrollArea>
#include <QtCore/QDateTime>
#include <vector>
#include "wrf_data_common.h"

class WrfDataManager;
class WrfImageMatrixView;
class WrfImageSeriesView;

class WrfEnsembleDataView : public QWidget {
    Q_OBJECT

public:
    WrfEnsembleDataView();
    ~WrfEnsembleDataView();

    void SetPara(QDateTime date, int forecast_hour, std::vector< WrfElementType >& ensemble_elements, std::vector< WrfElementType >& ensemble_mean_elements);
    void UpdateWidget();
    void GetSelectedRegionContour(std::vector< QPointF >& contour);

signals:
    void RetrievalTriggered();

private:
    QToolBar* main_tool_bar_;

    // actions for the image matrix view
    QAction* action_manual_region_selection_;
    QAction* action_apply_region_retrieval_;


    WrfImageMatrixView* image_matrix_view_;
    QScrollArea* image_series_scroll_area_;
    QWidget* image_series_scroll_widget_;

    WrfDataManager* data_manager_;

    QDateTime viewing_date_;
    int fhour_;
    std::vector< WrfElementType > ens_elements_;
    std::vector< WrfElementType > ens_mean_elements_;

    // value maps of elements from selected models
    // ensemble element in the front
    // [element]
    std::vector< std::vector< WrfGridValueMap* > > viewing_maps_;

    // the first series is for the uncertainty map, while following series are
    // for the element. The first image of all series will be the uncertainty
    // map
    // names of the series, uncertainty, rain, etc.
    std::vector< QString > series_titles_;

    // name for each value map
    std::vector< std::vector< QString > > value_map_titles_;

    std::vector< WrfImageSeriesView* > image_series_;

    void InitWidget();
    void LoadViewingData();

    private slots:
        void OnImageSeriesSelected(int);
        void OnActionManualRegionSelectionTriggered();
};

#endif