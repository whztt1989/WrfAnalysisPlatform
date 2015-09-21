#ifndef WRF_FORECAST_COMPARISON_WIDGET_H_
#define WRF_FORECAST_COMPARISON_WIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QComboBox>
#include <QtGui/QAction>
#include <QtGui/QToolBar>
#include <string>
#include <vector>
#include "wrf_data_common.h"

class WrfImageMatrixView;
class WrfGridValueMap;

enum WrfComparisonType {
    ENSEMBLE_COMPARISON,
    REFORECAST_COMPARISON,
    ALL_COMPARISON
};

class WrfForecastComparisonWidget : public QWidget{
    Q_OBJECT
public:
    WrfForecastComparisonWidget(WrfComparisonType type);
    ~WrfForecastComparisonWidget();

    void UpdateWidget();

private:
    WrfImageMatrixView* image_matrix_view_;

	QAction* action_manual_region_selection_;
	QAction* action_add_map_;

	void InitWidget();

	private slots:
		void OnActionManualRegionSelectionTriggered();
		void OnActionAddMapTriggered();
};

#endif