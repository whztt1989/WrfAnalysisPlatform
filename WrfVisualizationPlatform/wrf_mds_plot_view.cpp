#include "wrf_mds_plot_view.h"
//#include <opencv2/core/core.hpp> 
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <math.h>

WrfMdsPlotView::WrfMdsPlotView()
    : is_tracking_pen_(false){
   setAutoFillBackground(false);
}

WrfMdsPlotView::~WrfMdsPlotView(){

}

void WrfMdsPlotView::SetData(std::vector< std::vector< float > >& data){
    data_values_.assign(data.begin(), data.end());
    is_selected_.resize(data_values_.size());
    is_selected_.assign(is_selected_.size(), true);

    UpdateViewData();

    this->update();
}

void WrfMdsPlotView::GetSelectionIndex(std::vector< bool >& selection_index){
    selection_index = is_selected_;
}

void WrfMdsPlotView::SetSelectionIndex(std::vector< bool >& selection_index){
    is_selected_ = selection_index;

    this->update();
}

void WrfMdsPlotView::UpdateViewData(){
    // project the data using pca
    /*project_values_.resize(data_values_.size() * 2);

    if ( project_values_.size() == 0 ) return;
    
    cv::Mat data_map(data_values_.size(), data_values_[0].size(), CV_32F);
    for ( int i = 0; i < data_values_.size(); ++i )
        for ( int j = 0; j < data_values_[i].size(); ++j )
            data_map.at< float >(i, j) = data_values_[i][j];
    cv::PCA pca(data_map, cv::Mat(), 0);
    std::vector< std::vector< float > > scaled_vec;
    scaled_vec.resize(2);
    for ( int i = 0; i < scaled_vec.size(); ++i ) scaled_vec[i].resize(data_values_[0].size());
    for ( int i = 0; i < 2; ++i )
        for ( int j = 0; j < data_values_[0].size(); ++j )
            scaled_vec[i][j] = pca.eigenvectors.at<float>(i, j);
    for ( int i = 0; i < data_values_.size(); ++i ){
        float x = 0, y = 0;
        for ( int j = 0; j < data_values_[0].size(); ++j ){
            x += scaled_vec[0][j] * data_values_[i][j];
            y += scaled_vec[1][j] * data_values_[i][j];
        }
        project_values_[2 * i] = x;
        project_values_[2 * i + 1] = y;
    }
    float min_x = 1e10, min_y = 1e10;
    float max_x = -1e10, max_y = -1e10;
    for ( int i = 0; i < project_values_.size() / 2; ++i ){
        if ( project_values_[i * 2] > max_x ) max_x = project_values_[i * 2];
        if ( project_values_[i * 2] < min_x ) min_x = project_values_[i * 2];
        if ( project_values_[i * 2 + 1] > max_y ) max_y = project_values_[i * 2 + 1];
        if ( project_values_[i * 2 + 1] < min_y ) min_y = project_values_[i * 2 + 1];
    }
    float x_range = abs(max_x - min_x) + 1e-2;
    float y_range = abs(max_y - min_y) + 1e-2;
    for ( int i = 0; i < project_values_.size() / 2; ++i ){
        project_values_[i * 2] = (project_values_[i * 2] - min_x) / x_range;
        project_values_[i * 2 + 1] = (project_values_[i * 2 + 1] - min_y) / y_range * 0.9;
    }*/
}

void WrfMdsPlotView::initializeGL(){
    if ( glewInit() != GLEW_OK ){
        exit(0);
    }
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void WrfMdsPlotView::resizeGL(int w, int h){
    view_size_ = this->width() * 0.8;
    if ( view_size_ > this->height() * 0.8 ) view_size_ = this->height() * 0.8;

    view_left_ = 0.5 - this->width() / (2 * view_size_);
	view_right_ = 0.5 + this->width() / (2 * view_size_);
    view_top_ = 0.5 + this->height() / (2 * view_size_);
	view_bottom_ = 0.5 - this->height() / (2 * view_size_);
}

void WrfMdsPlotView::paintEvent(QPaintEvent* event){
	makeCurrent();

    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(view_left_, view_right_, view_bottom_, view_top_, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2);

    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(5.0);

    glBegin(GL_POINTS);
    for ( int i = 0; i < project_values_.size() / 2; ++i ){
        if ( is_selected_[i] )
            glColor3f(1.0, 0.0, 0.0);
        else
            glColor3f(0.0, 0.0, 0.0);
        glVertex3f(project_values_[2 * i], project_values_[2 * i + 1], 0);
    }
    glEnd();


	if ( is_tracking_pen_ ){
		glColor3f(1.0, 0.6, 0.38);
		glLineWidth(2.0);
        glBegin(GL_LINE_LOOP);
        for ( int i = 0; i < selection_path_.size(); ++i )
            glVertex3f(selection_path_[i].rx(), selection_path_[i].ry(), 0);
        glEnd();
    }

	QFont title_font;
	title_font.setFamily("arial");
	title_font.setBold(true);
	title_font.setPixelSize(16);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::black);

	painter.setFont(title_font);
	QString title = QString("Parameter N Overview");
	painter.drawText(QRectF(0, 0, this->width(), 40), Qt::AlignCenter, QString("MDS Classification"));

	painter.end();
}

void WrfMdsPlotView::SetTrackingPen(bool enabled){
    is_tracking_pen_ = enabled;

    if ( !is_tracking_pen_ )
        is_selected_.assign(is_selected_.size(), true);

    this->updateGL();
}

void WrfMdsPlotView::mousePressEvent(QMouseEvent *event){
    if ( is_tracking_pen_ ){
        selection_path_.clear();
        QPoint p = event->pos();
        float x = (float)(p.x() - this->width() / 2) / view_size_ + 0.5;
        float y = (float)(this->height() / 2 - p.y()) / view_size_ + 0.5;
        selection_path_.push_back(QPointF(x, y));
    }
}

void WrfMdsPlotView::mouseMoveEvent(QMouseEvent *event){
    if ( is_tracking_pen_ ){
        QPoint p = event->pos();
        float x = (float)(p.x() - this->width() / 2) / view_size_ + 0.5;
        float y = (float)(this->height() / 2 - p.y()) / view_size_ + 0.5;
        selection_path_.push_back(QPointF(x, y));
        this->update();
    }
}

void WrfMdsPlotView::mouseReleaseEvent(QMouseEvent *event){
    if ( is_tracking_pen_ ){
        if ( selection_path_.size() > 5 ){
            UpdateSelection();
        } else {
            UpdatePointSelection(event->pos());
        }

        this->update();

        emit SelectionUpdated();
    }
}

void WrfMdsPlotView::UpdatePointSelection(QPoint p){
    int min_index = -1;
    float min_dis = 1e10;

    float x = (float)p.x() / this->width() * (view_right_ - view_left_) + view_left_;
    float y = (float)(this->height() - p.y()) / this->height() * (view_top_ - view_bottom_) + view_bottom_;
    float scale = this->width() / (view_right_ - view_left_);
    for ( int i = 0; i < project_values_.size() / 2; ++i ){
        float temp_dis = abs(project_values_[2 * i] - x) + abs(project_values_[2 * i + 1] - y);
        if ( temp_dis < min_dis && temp_dis * scale < 15 ){
            min_dis = temp_dis;
            min_index = i;
        }
    }

    if ( min_index != -1 ){
        is_selected_[min_index] = !is_selected_[min_index];
        this->update();
    }
}

void WrfMdsPlotView::UpdateSelection(){
    const double PI = 3.14159265358979323846;

    for ( int i = 0; i < project_values_.size() / 2; ++i ){
        double angle = 0;
        float x = project_values_[2 * i];
        float y = project_values_[2 * i + 1];
        for ( int j = 0, k = selection_path_.size() - 1; j < selection_path_.size(); k = j++ ){
            double x1,y1,x2,y2;
            x1 = selection_path_[j].rx() - x;
            y1 = selection_path_[j].ry() - y;
            x2 = selection_path_[k].rx() - x;
            y2 = selection_path_[k].ry() - y;

            double radian = atan2(y1, x1) - atan2(y2, x2);
            radian = abs(radian);
            if (radian > PI) 
                radian = 2 * PI - radian;
            angle += radian;            
        }

        if ( fabs(6.28318530717958647692 - abs(angle)) < 1 ) 
            is_selected_[i] = true;
        else
            is_selected_[i] = false;
    }
}