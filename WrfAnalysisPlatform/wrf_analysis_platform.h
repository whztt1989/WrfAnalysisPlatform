#ifndef WRF_ANALYSIS_PLATFORM_H
#define WRF_ANALYSIS_PLATFORM_H

#include <QtGui/QMainWindow>
#include "ui_wrf_analysis_platform.h"

class WrfCentralViewer;
class WrfControlPanel;
class WrfNumericalDataSet;
class WrfSiteDataSet;
class WrfParallelCoordinate;
class WrfDataManager;
class WrfMatrixViewer;
class WrfLineChart;
class WrfParameterWidget;
class WrfWeightHistogram;
class StoryLineWidget;
class WrfStampExplorer;

class WrfAnalysisPlatform : public QMainWindow
{
	Q_OBJECT

public:
	WrfAnalysisPlatform(QWidget *parent = 0, Qt::WFlags flags = 0);
	~WrfAnalysisPlatform();

private:
	Ui::WrfAnalysisPlatformClass ui;
	
	WrfCentralViewer* central_viewer_;
	WrfControlPanel* control_panel_;
	WrfParallelCoordinate* bias_parallel_viewer_;
	WrfParallelCoordinate* absolute_parallel_viewer_;
	WrfMatrixViewer* matrix_viewer_;
	WrfLineChart* bias_line_chart_viewer_;
	WrfLineChart* absolute_line_chart_viewer_;
	WrfParameterWidget* para_widget_;
	WrfWeightHistogram* histogram_viewer_;
	WrfStampExplorer* random_stamp_viewer_;
	WrfStampExplorer* positioned_stamp_viewer_;
	StoryLineWidget* story_line_viewer_;

	WrfDataManager* data_manager_;
	int current_year_, current_month_, current_day_, current_hour_;

	void InitWidget();
	void InitToolBar();
	void InitConnections();
	void UpdateSystem();

	private slots:
		void OnActionLoadNumericalDataTriggered();
		void OnActionLoadHistoricalDataTriggered();
		void OnActionViewVarationMapTriggered();
		void OnActionGenerateTestPcpTriggered();
		void OnActionGenerateTestLineChartTriggered();
		void OnActionGenerateTestMatrixChartTriggered();
		void OnActionGenerateTestHistogramTriggered();
		void OnActionOpenProjectTriggered();
		void OnActionSaveProjectTriggered();
		void OnActionGenerateBiasMapTriggered();
		void OnActionSelectingSiteTriggered();
		void OnActionBrushingSiteTriggered();
		void OnActionMagicWandTriggered();
		void OnActionErasingSiteTriggered();
		void OnActionStartEditingTriggered();
		void OnActionEndEditingTriggered();
		void OnActionParallelBiasViewTriggered();
		void OnActionParallelAbsoluteViewTriggered();
		void OnActionBiasLineViewTriggered();
		void OnActionAbsoluteLineViewTriggered();
		void OnActionStampMapViewTriggered();
		void OnActionAddReferenceMapTriggered();
		void OnActionSumTriggered();
		void OnActionMiniusTriggered();
		void OnActionVariationTriggered();
		void OnActionComplexStampMapViewTriggered();
		void OnActionAcceptRegionGrowingTriggered();
		void OnActionComparedModeTriggered();
		void OnActionNormalModeTriggered();
		void OnActionClusterBiasParallelRecordsTriggered();
		void OnActionClusterBiasLineChartRecordsTriggered();
		void OnActionClusterAbsoluteParallelRecordsTriggered();
		void OnActionClusterAbsoluteLineChartRecordsTriggered();

		void OnSelectedSiteChanged(int index);
		void OnSelectedAreaChanged();
		void OnInfoGainSelected(int);
		void OnBiasMapSelected(int);
		void OnStoryStampSelected(int);
		void OnBiasWeightChanged();
		void OnHistoryLengthChanged(int);
		void OnAttribWeightChanged();
};

#endif // WRF_ANALYSIS_PLATFORM_H
