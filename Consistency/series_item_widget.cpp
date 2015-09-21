#include "series_item_widget.h"
#include <QtGui/QSlider>
#include <QtGui/QHBoxLayout>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include "plat_image_view.h"
#include "stacked_image_view.h"
#include "consistency_common.h"
#include "image_generator.h"

SeriesItemWidget::SeriesItemWidget(QString xml_file_path)
    : xml_file_path_(xml_file_path), root_(NULL){
    LoadData();
    InitWidget();
    ProcessData();

    this->setMinimumHeight(200);
}

SeriesItemWidget::~SeriesItemWidget(){

}

void SeriesItemWidget::InitWidget(){
    plat_image_view_ = new PlatImageView;
    stacked_image_view_ = new StackedImageView;
    mode_selection_slider_ = new QSlider(Qt::Vertical);
    mode_selection_slider_->setFixedHeight(200);
    mode_selection_slider_->setRange(0, 100);
    mode_selection_slider_->setValue(50);

    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->addWidget(plat_image_view_);
    main_layout->addWidget(stacked_image_view_);
    main_layout->addWidget(mode_selection_slider_);

    this->setLayout(main_layout);
}

void SeriesItemWidget::LoadData(){
    QFile file(xml_file_path_);

    QDomDocument dom_document;

    if ( file.open(QFile::ReadOnly | QFile::Text ) ){
        QString error_str;
        int error_line;
        int error_column;

        if (!dom_document.setContent(&file, false, &error_str, &error_line, &error_column)){
            qDebug() << "Error: Parse error at line " << error_line << ","
                << "colume " << error_column << ": "
                << qPrintable(error_str);
            return;
        }

        QDomElement root = dom_document.documentElement();
        if(root.tagName() != "Images"){
            qDebug() << "Error: Not an image template file";
            return;
        }

        QDomElement image_series_element = root.firstChildElement("ImageSeries");
        QDomNode image_node = image_series_element.firstChild();
        std::vector< QString > image_str_vec;
        while ( !image_node.isNull() ){
            image_series_.push_back(image_node.toElement().attribute("path"));
            image_node = image_node.nextSibling();
        }

        file.close();
    }
}

void SeriesItemWidget::ProcessData(){
    // construct the compare tree here
    root_ = NULL;

    // step1: construct the level images
    // step1.1: construct the basic level images
    std::vector< Node* > basic_nodes;

    std::vector< float > range;
    range.push_back(0);
	range.push_back(10);
    range.push_back(20);
	range.push_back(35);
    for ( int i = 0; i < image_series_.size(); ++i ){
        LevelImage* level_image = ImageGenerator::GenerateLevelImage(image_series_[i], range);
		level_image->Save(QString("./testresult/level%0.png").arg(i));
        Node* node = new Node(level_image);
        basic_nodes.push_back(node);
    }
    // step1.2: construct the alpha channels for the basic level images
    std::vector< Node* > alpha_nodes;

    // Generate the change alpha nodes
    for ( int i = 0; i < basic_nodes.size(); ++i ){
        AlphaImage* image = new AlphaImage(basic_nodes[i]->image);
        Node* node = new Node(image);
        alpha_nodes.push_back(node);
    }
    for ( int i = 0; i < alpha_nodes.size(); ++i ){
        Node* node = alpha_nodes[i];
        if ( i != 0 ) node->children.push_back(basic_nodes[i - 1]);
        node->children.push_back(basic_nodes[i]);
        if ( i != basic_nodes.size() - 1 ) {
			node->children.push_back(basic_nodes[i + 1]);
			ImageGenerator::GenerateLevelAlphaChannel(alpha_nodes[i]->image, alpha_nodes[i + 1]->image, true);
		}
    }
	for ( int i = 0; i < alpha_nodes.size(); ++i ){
		LevelImage* temp_image = new LevelImage(alpha_nodes[i]->image->width(), alpha_nodes[i]->image->height(), alpha_nodes[i]->image->alpha());
		temp_image->Save(QString("./testresult/alphachange%0.png").arg(i));
		delete temp_image;
	}
    // Generate the accuracy alpha nodes
    for ( int i = 0; i < basic_nodes.size(); ++i ){
        AlphaImage* image = new AlphaImage(basic_nodes[i]->image);
        Node* node = new Node(image);
        alpha_nodes.push_back(node);
    }
    for ( int i = alpha_nodes.size() / 2; i < alpha_nodes.size(); ++i ){
        Node* node = alpha_nodes[i];
        node->children.push_back(basic_nodes[0]);
        if ( i != alpha_nodes.size() / 2 ) {
            node->children.push_back(basic_nodes[i - alpha_nodes.size() / 2]);
            ImageGenerator::GenerateLevelAlphaChannel(alpha_nodes[i]->image, alpha_nodes[alpha_nodes.size() / 2]->image, false);

			LevelImage* temp_image = new LevelImage(alpha_nodes[i]->image->width(), alpha_nodes[i]->image->height(), alpha_nodes[i]->image->alpha());
			temp_image->Save(QString("./testresult/alphaaccuracy%0.png").arg(i - alpha_nodes.size() / 2));
			delete temp_image;
        }
    }

    /*LevelImage* level_image1 = ImageGenerator::GenerateLevelImage(image_series_[3], range);
    LevelImage* level_image2 = ImageGenerator::GenerateLevelImage(image_series_[4], range);
    level_image1->Save(QString("./testimages/test1.png"));
    level_image2->Save(QString("./testimages/test2.png"));
    ImageGenerator::GenerateLevelAlphaChannel(level_image1, level_image2, true);
    level_image1->Save(QString("./testimages/compare1.png"));
    level_image2->Save(QString("./testimages/compare2.png"));*/

    // step2: construct the aggregating images
    std::vector< Node* > aggregate_nodes;
    
    std::vector< float > transfer_values;
    // TODO: initialize the transfer values
    float accu_weight = 0;
    transfer_values.resize(10);
    for ( int i = 0; i < transfer_values.size(); ++i ){
        transfer_values[i] = pow(10.0 - i, 2);
        accu_weight += transfer_values[i];
    }
    for ( int i = 0; i < transfer_values.size(); ++i ) transfer_values[i] /= accu_weight;
    
    // Add the change alpha image
    std::vector< BasicImage* > change_alpha_images;
    for ( int i = 0; i < alpha_nodes.size() / 2; ++i ) change_alpha_images.push_back(alpha_nodes[i]->image);
    AggregateImage* change_image = ImageGenerator::GenerateAggregateImage(change_alpha_images, transfer_values);
    Node* change_node = new Node(change_image);
    for ( int i = 0; i < alpha_nodes.size() / 2; ++i ) change_node->children.push_back(alpha_nodes[i]);
    aggregate_nodes.push_back(change_node);
	change_image->Save(QString("./testresult/aggregatechange.png"));
    // Add the accuracy alpha image
    std::vector< BasicImage* > accuracy_alpha_image;
    for ( int i = 1; i < alpha_nodes.size() / 2; ++i ) accuracy_alpha_image.push_back(alpha_nodes[i + alpha_nodes.size() / 2]->image);
    AggregateImage* accuracy_image = ImageGenerator::GenerateAggregateImage(accuracy_alpha_image, transfer_values);
    Node* accuracy_node = new Node(accuracy_image);
    for ( int i = 1; i < alpha_nodes.size() / 2; ++i ) accuracy_node->children.push_back(alpha_nodes[i + alpha_nodes.size() / 2]);
    aggregate_nodes.push_back(accuracy_node);
	accuracy_image->Save(QString("./testresult/aggregateaccuracy.png"));

    // step3: construct the blending images
    std::vector< Node* > blending_nodes;
    int series_size = 5;
    for ( int i = 0; i <= series_size; ++i ){
        BlendImage* image = ImageGenerator::GenerateBlendImage(aggregate_nodes[0]->image, aggregate_nodes[1]->image, (float)i / series_size);
        Node* node = new Node(image);
        node->children.push_back(aggregate_nodes[0]);
        node->children.push_back(aggregate_nodes[1]);
        blending_nodes.push_back(node);
		image->Save(QString("./testresult/blend%0.png").arg(i));
    }

    // step4: construct the cluster images
    std::vector< Node* > cluster_nodes;
    std::vector< ClusterImage* > cluster_images;
    std::vector< BasicImage* > input_images;
    for ( int i = 0; i < blending_nodes.size(); ++i ) input_images.push_back(blending_nodes[i]->image);
    if ( ImageGenerator::GenerateClusterImages(input_images, cluster_images) ){
        for ( int i = 0; i < cluster_images.size(); ++i ){
            Node* node = new Node(cluster_images[i]);
            for ( int j = 0; j < blending_nodes.size(); ++j ) node->children.push_back(blending_nodes[j]);
            cluster_nodes.push_back(node);
        }
    }

    // step4: construct the root node
    root_ = new Node(NULL);
    for ( int i = 0; i < cluster_nodes.size(); ++i ) root_->children.push_back(cluster_nodes[i]);

    plat_image_view_->SetData(root_);
}