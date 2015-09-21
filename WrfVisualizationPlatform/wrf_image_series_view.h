#ifndef WRF_IMAGE_SERIES_VIEW_H_
#define WRF_IMAGE_SERIES_VIEW_H_

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <vector>
#include "wrf_data_common.h"

class WrfImageViewer;

class WrfImageSeriesView : public QWidget
{
    Q_OBJECT

public:
    WrfImageSeriesView(int index = -1);
    ~WrfImageSeriesView();

    void SetTitle(QString title);
    void SetMaps(std::vector< WrfGridValueMap* >& maps, std::vector< QString >& titles);

signals:
    void SeriesSelected(int);

protected:
    void mouseDoubleClickEvent(QMouseEvent *);

private:
    int view_index_;
    QString title_string_;
    std::vector< WrfGridValueMap* > maps_;
    std::vector< QString > titles_;

    QLabel* title_label_;
    WrfImageViewer* image_viewer_;
    QButtonGroup* image_button_group_;

    void InitWidgets();
    void UpdateWidgets();

    private slots:
        void OnButtonClicked(int);
};

#endif