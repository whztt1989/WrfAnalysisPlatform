#ifndef WRF_REGION_DETECTION_VIEW_H_
#define WRF_REGION_DETECTION_VIEW_H_

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

class WrfRegionDetectionView : public QWidget {
    Q_OBJECT

public:
    WrfRegionDetectionView(WrfForecastingType type);
    ~WrfRegionDetectionView();

    void UpdateWidget();
	void UpdateView();
    void GetSelectedRegionContour(std::vector< QPointF >& contour);

signals:
    void RetrievalTriggered(int);
    void AddNewForecastTriggered();

private:
    QToolBar* region_detection_tool_bar_;
    WrfImageMatrixView* image_matrix_view_;

    // actions for the image matrix view
    QAction* action_manual_region_selection_;
    QAction* action_apply_region_retrieval_;
    QAction* action_add_new_forecast_;

    WrfForecastingType forecasting_type_;

    void InitWidget();

    private slots:
        void OnActionManualRegionSelectionTriggered();
        void OnRetrievalTriggered();
};

#endif