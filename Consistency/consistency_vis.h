#ifndef CONSISTENCY_VIS_H
#define CONSISTENCY_VIS_H

#include <QtGui/QMainWindow>
#include "ui_consistency_vis.h"

class LevelImage;
class QListWidget;

class ConsistencyVis : public QMainWindow
{
	Q_OBJECT

public:
	ConsistencyVis(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ConsistencyVis();

private:
	Ui::ConsistencyVisClass ui;
    QListWidget* central_list_view_;

    std::vector< std::vector< QString > > image_series_;

    void InitWidget();
    void TestSaveImages();

    private slots:
        void OnActionLoadImagesTriggered();
};

#endif // CONSISTENCY_VIS_H
