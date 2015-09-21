#ifndef WRF_DETAIL_OVERVIEW_WIDGET_H_
#define WRF_DETAIL_OVERVIEW_WIDGET_H_

#include <QtGui/QWidget>
#include "wrf_data_common.h"

class WrfImageViewer;
class WrfDetailInformationViewWidget;

class WrfDetailOverviewWidget : public QWidget {
    Q_OBJECT

public:
    WrfDetailOverviewWidget(WrfDetailInformationViewWidget* parent);
    ~WrfDetailOverviewWidget();

    void UpdateWidget();

private:
    WrfDetailInformationViewWidget* parent_;

    WrfGridValueMap* forecasting_result_map_;
    WrfImageViewer* forecasting_viewer_;

    void InitWidget();
};

#endif