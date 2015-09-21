#include "consistency_vis.h"
#include <QtGui/QFileDialog>
#include <QtGui/QVBoxLayout>
#include "image_generator.h"
#include "series_item_widget.h"

ConsistencyVis::ConsistencyVis(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
    ui.setupUi(this);
    InitWidget();

    connect(ui.actionLoad_Images, SIGNAL(triggered()), this, SLOT(OnActionLoadImagesTriggered()));
}

ConsistencyVis::~ConsistencyVis()
{

}

void ConsistencyVis::InitWidget(){
    QVBoxLayout* central_widget_layout = new QVBoxLayout;
    if ( ui.centralWidget->layout() != NULL ){
        delete ui.centralWidget->layout();
    }
    ui.centralWidget->setLayout(central_widget_layout);
}

void ConsistencyVis::OnActionLoadImagesTriggered(){
    QString xml_file_str = QFileDialog::getOpenFileName(this, tr("Open Xml File"));

    if ( xml_file_str.length() == 0 ) return;

    SeriesItemWidget* series_widget = new SeriesItemWidget(xml_file_str);
    ui.centralWidget->layout()->addWidget(series_widget);
}

void ConsistencyVis::TestSaveImages(){
    std::vector< float > range;
    range.push_back(0);
    range.push_back(20);

    LevelImage* level_image1 = ImageGenerator::GenerateLevelImage(image_series_[0][0], range);
    LevelImage* level_image2 = ImageGenerator::GenerateLevelImage(image_series_[0][1], range);

    for ( int i = 0; i < image_series_.size(); ++i )
        for ( int j = 0; j < image_series_[i].size(); ++j ){
            
            
            //ExecuteErosion(level_image, 2);
            //ExecuteDilation(level_image, 2);
            /*LevelImage* dilation_image = new LevelImage;
            dilation_image->height = level_image->height;
            dilation_image->width = level_image->width;
            dilation_image->data = new int[level_image->width * level_image->height];
            memcpy(dilation_image->data, level_image->data, sizeof(int) * level_image->width * level_image->height);
            ExecuteDilation(dilation_image, 4);
            ExecuteErosion(dilation_image, 4);

            LevelImage* erosion_image = new LevelImage;
            erosion_image->height = level_image->height;
            erosion_image->width = level_image->width;
            erosion_image->data = new int[level_image->width * level_image->height];
            memcpy(erosion_image->data, level_image->data, sizeof(int) * level_image->width * level_image->height);
            ExecuteErosion(erosion_image, 4);
            ExecuteDilation(erosion_image, 4);

            for ( int k = 0; k < level_image->width * level_image->height; ++k )
                if ( erosion_image->data[k] || dilation_image->data[k] ) 
                    level_image->data[k] = 1;
                else
                    level_image->data[k] = 0;

            delete dilation_image;
            delete erosion_image;*/

            /*QImage image(level_image->width, level_image->height, QImage::Format_ARGB32);
            for ( int y = 0; y < level_image->height; ++y )
            for ( int x = 0; x < level_image->width; ++x ){
            QColor color;
            color.setHsv(125, 255 * level_image->data[y * level_image->width + x] / 3, 255 - 155 * level_image->data[y * level_image->width + x] / 3);
            image.setPixel(x, level_image->height - y - 1, color.rgba());
            }
            image.save(QString("./testimages/origin%0%1.png").arg(i).arg(j));*/
        }
}