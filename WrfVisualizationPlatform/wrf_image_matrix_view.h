#ifndef WRF_IAMGE_MATRIX_VIEW_H_
#define WRF_IAMGE_MATRIX_VIEW_H_

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtGui/QScrollArea>
#include <vector>
#include "wrf_data_common.h"

class WrfImage;
class WrfImageViewer;
class WrfElementSimilarityView;
class WrfGridRmsErrorElement;

class WrfImageMatrixView : public QWidget
{
    Q_OBJECT

public:
    WrfImageMatrixView();
    ~WrfImageMatrixView();

    void AddValueMap(WrfValueMap* value_map, MapRange range, QString title = QString(""));
    void AddRmsMapView(MapRange& range);
	void AddGeographicaMap(MapRange& range);
    void SetViewedImageNumber(int num);
    void SetTrackingPen(bool enabled);
    void SetTrackingScaling(bool enabled);

    void GetSelectedRegionContour(std::vector< QPointF >& contour);
	int GetMapSize() { return value_maps_.size(); }

    void Clear();
    void UpdateWidget();

private:
    int viewer_size_;
    int current_image_index_;
    int current_beginning_index_;
    std::vector< QPointF > region_contour_;

    std::vector< QString > titles_;
    std::vector< WrfValueMap* > value_maps_;

    std::vector< WrfImageViewer* > image_viewers_;
    QScrollArea* scroll_widget_;

    void UpdateWidgets(int size);
    void UpdateRmsElement();
    void InitWidget();

    private slots:
        void OnBrushingPathUpdated(int);
        void OnBrushingPathFinished(int);
        void OnImageSelected(int);
		void OnViewChanged(int);
};

#endif