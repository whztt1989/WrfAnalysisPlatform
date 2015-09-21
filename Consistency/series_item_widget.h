#ifndef SERIES_ITEM_WIDGET_H_
#define SERIES_ITEM_WIDGET_H_

#include <QtGui/QWidget>
#include <QtCore/QString>
#include <vector>

class PlatImageView;
class StackedImageView;
class QSlider;
class CompareTree;
class Node;

class SeriesItemWidget : public QWidget{
    Q_OBJECT

public:
    SeriesItemWidget(QString xml_file_path);
    ~SeriesItemWidget();

private:
    QString xml_file_path_;
    std::vector< QString > image_series_;

    PlatImageView* plat_image_view_;
    StackedImageView* stacked_image_view_;
    QSlider* mode_selection_slider_;

    Node* root_;

    void InitWidget();
    void LoadData();
    void ProcessData();
};

#endif