#include "wrf_image_viewer.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtCore/QDebug>
#include <iostream>
#include "color_mapping_generator.h"
#include "wrf_data_manager.h"
#include "wrf_rendering_element.h"
#include "wrf_rendering_element_factory.h"
#include "wrf_iso_line_plot_dialog.h"
#include <QtGui/QPainter>

WrfImageViewer::WrfImageViewer(int viewer_index)
    : status_(VIEWER_UNINITIALIZED), rendering_status_(RENDERING_OK),
    view_left_(0), view_right_(360), view_bottom_(-90), view_top_(90), 
    rendering_scale_(1), viewer_index_(viewer_index), title_height_(30),
    color_bar_width_(0), color_bar_element_(NULL), second_bar_element_(NULL) {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    map_range_.start_x = 0.0f;
    map_range_.end_x = 1.0f;
    map_range_.start_y = 0.0f;
    map_range_.end_y = 1.0f;
    map_range_.x_grid_space = 1.0f;
    map_range_.y_grid_space = 1.0f;
    map_range_.x_grid_number = 2;
    map_range_.y_grid_number = 2;

	context_menu_ = new QMenu;
	action_adding_iso_line_plot_ = new QAction(tr("Add Iso Line Plot"), this);
	context_menu_->addAction(action_adding_iso_line_plot_);
	context_menu_->addSeparator();

	connect(action_adding_iso_line_plot_, SIGNAL(triggered()), this, SLOT(OnActionAddIsoLinePlotTriggered()));

	setAutoFillBackground(false);
}

WrfImageViewer::~WrfImageViewer(){
    glDeleteLists(font_offset_, 128);
}

void WrfImageViewer::SetTitle(QString title){
    title_ = title;
    rendering_status_ |= TITLE_UPDATED;

    this->updateGL();
}

void WrfImageViewer::SetColorBarElement(WrfRenderingElement* color_bar){
    color_bar_element_ = color_bar;
    rendering_status_ |= COLOR_BAR_UPDATED;

    this->updateGL();
}

void WrfImageViewer::SetSecondColorBarElement(WrfRenderingElement* color_bar){
    second_bar_element_ = color_bar;
    rendering_status_ |= COLOR_BAR_UPDATED;
}

void WrfImageViewer::AddRenderingElement(WrfRenderingElement* element){
    element_vec_.push_back(element);
    rendering_status_ |= ELEMENT_UPDATED;

	QAction* action = new QAction(QString::fromLocal8Bit(element->name()), this);
	action->setCheckable(true);
	action->setChecked(true);
	action_element_visibility_.push_back(action);
	context_menu_->addAction(action);

	connect(action, SIGNAL(triggered()), this, SLOT(OnVisibilityActionTriggered()));

    this->updateGL();
}

void WrfImageViewer::AddIsoLinePlot(int mode, int datetime, WrfModelType model_type, WrfElementType element_type, int fhour, float iso_value, int ens){
	std::vector< WrfGridValueMap* > value_maps;
	WrfDataManager::GetInstance()->GetGridValueMap(datetime, model_type, element_type, fhour, value_maps);
	
	if ( ens < value_maps.size() ){
		WrfRenderingElement* element = WrfRenderingElementFactory::GenerateIsoLinePlot(mode, value_maps[ens], iso_value, font_offset_);
		this->AddRenderingElement(element);
	}

	for ( int i = 0; i < value_maps.size(); ++i ) delete value_maps[i];
	value_maps.clear();

	this->update();
}

void WrfImageViewer::SetMapRange(MapRange& range){
    map_range_ = range;
    rendering_status_ |= SIZE_UPDATED;

    this->updateGL();
}

void WrfImageViewer::SetSelectionPath(std::vector< QPointF >& path){
    brush_path_.assign(path.begin(), path.end());

    this->update();
}

void WrfImageViewer::GetSelectionPath(std::vector< QPointF >& path){
    path.assign(brush_path_.begin(), brush_path_.end());
}

void WrfImageViewer::SetTrackingPen(bool enabled){
    if ( status_ < VIEWER_NORMAL ) return;
    if ( enabled )
        status_ = VIEWER_TRACKING_PEN;
    else
        status_ = VIEWER_NORMAL;
}

void WrfImageViewer::initializeGL(){
    if ( glewInit() != GLEW_OK ) exit(0);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    status_ = VIEWER_NORMAL;

    font_offset_ = glGenLists(128);
    wglUseFontBitmaps(wglGetCurrentDC(), 0, 128, font_offset_);
}

void WrfImageViewer::resizeGL(int w, int h){
    rendering_status_ |= SIZE_UPDATED;

	this->update();
}

void WrfImageViewer::UpdateRenderingSize(){
    float scale, temp_width, temp_height;
    temp_width = map_range_.end_x - map_range_.start_x;
    temp_height = map_range_.end_y - map_range_.start_y;
    if ( temp_width / (this->width() - color_bar_width_) > temp_height / (this->height() - title_height_) ){
        scale = temp_width / (this->width() - color_bar_width_);
    } else {
        scale = temp_height / (this->height() - title_height_);
    }
    float center_x = (map_range_.start_x + map_range_.end_x) / 2;
    float center_y = (map_range_.start_y + map_range_.end_y) / 2;
    view_left_ = center_x - (this->width() - color_bar_width_) / 2 * scale;
    view_right_ = center_x + (this->width() - color_bar_width_) / 2 * scale;
    view_bottom_ = center_y - (this->height() - title_height_) / 2 * scale;
    view_top_ = center_y + (this->height() - title_height_) / 2 * scale;

    rendering_scale_ = 1;
}

void WrfImageViewer::paintEvent(QPaintEvent* event){
	makeCurrent();

    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, this->width(), 0, this->height(), 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -4);
    glClear(GL_COLOR_BUFFER_BIT);

	//if ( element_vec_.size() == 0 ) return;

    if ( rendering_status_ & TITLE_UPDATED ){
        if ( title_.length() != 0 ) 
            title_height_ = 40;
        else
            title_height_ = 0;
        rendering_status_ ^= TITLE_UPDATED;
    }

    if ( rendering_status_ & COLOR_BAR_UPDATED ){
        if ( color_bar_element_ != NULL )
            color_bar_width_ = 65;
        else
            color_bar_width_ = 0;
        if ( second_bar_element_ != NULL )
            color_bar_width_ += 60;
        rendering_status_ ^= COLOR_BAR_UPDATED;
    }

    if ( rendering_status_ & SIZE_UPDATED ) {
        UpdateRenderingSize();
        rendering_status_ ^= SIZE_UPDATED;
    }

    // rendering element
    glViewport(0, 0, this->width() - color_bar_width_, this->height() - title_height_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(view_left_, view_right_, view_bottom_, view_top_, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -4);

    for ( int i = 0; i < element_vec_.size(); ++i ) 
        if ( element_vec_[i] != NULL ) element_vec_[i]->Render(view_left_, view_right_, view_bottom_, view_top_);

    if ( status_ == VIEWER_TRACKING_PEN ){
        glColor3f(1.0, 0.6, 0.38);
		glLineWidth(2.0);
        glBegin(GL_LINE_LOOP);
        for ( int i = 0; i < brush_path_.size(); ++i )
            glVertex3f(brush_path_[i].rx(), brush_path_[i].ry(), 0);
        glEnd();
    }

    // rendering color bar
    glViewport(this->width() - color_bar_width_, 0, color_bar_width_, this->height() - title_height_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, color_bar_width_, 0, this->height() - title_height_, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -4);

    if ( color_bar_element_ != NULL ) color_bar_element_->Render(0, 60, 0, this->height() - title_height_);
    if ( second_bar_element_!= NULL ) second_bar_element_->Render(65, 125, 0, this->height() - title_height_);

	QFont title_font;
	title_font.setFamily("arial");
	title_font.setBold(true);
	title_font.setPixelSize(16);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::black);

	painter.setFont(title_font);
	painter.drawText(QRect(0, 0, this->width(), title_height_), Qt::AlignCenter, title_);

    if ( color_bar_element_ != NULL ) color_bar_element_->Render(painter, this->width() - color_bar_width_, this->width() - color_bar_width_ + 60, this->height(), title_height_);
    if ( second_bar_element_!= NULL ) second_bar_element_->Render(painter, this->width() - color_bar_width_ + 65, this->width() - color_bar_width_ + 125, this->height(), title_height_);

	painter.end();
}

void WrfImageViewer::mousePressEvent(QMouseEvent *event){
    if ( status_ == VIEWER_TRACKING_PEN && event->buttons() & Qt::LeftButton) {
        brush_path_.clear();
        QPoint p = event->pos();
        float longi = (float)p.x() / (this->width() - color_bar_width_) * (view_right_ - view_left_) + view_left_;
        float lati = (float)(this->height() - p.y()) / (this->height() - title_height_) * (view_top_ - view_bottom_) + view_bottom_;
        brush_path_.push_back(QPointF(longi, lati));
    }

    if ( event->buttons() & Qt::MidButton ){
        QPoint p = event->pos();
        float longi = (float)p.x() / (this->width() - color_bar_width_) * (view_right_ - view_left_) + view_left_;
        float lati = (float)(this->height() - p.y()) / (this->height() - title_height_) * (view_top_ - view_bottom_) + view_bottom_;
        previous_pos_ = QPointF(longi, lati);
    }
}

void WrfImageViewer::mouseMoveEvent(QMouseEvent *event){
    if ( status_ == VIEWER_TRACKING_PEN && event->buttons() & Qt::LeftButton ) {
        QPoint p = event->pos();
        float longi = (float)p.x() / (this->width() - color_bar_width_) * (view_right_ - view_left_) + view_left_;
        float lati = (float)(this->height() - p.y()) / (this->height() - title_height_) * (view_top_ - view_bottom_) + view_bottom_;
        brush_path_.push_back(QPointF(longi, lati));

        emit SelectionUpdated(viewer_index_);

        this->update();
    }

    if ( event->buttons() & Qt::MidButton ){
        QPoint p = event->pos();
        float longi = (float)p.x() / (this->width() - color_bar_width_) * (view_right_ - view_left_) + view_left_;
        float lati = (float)(this->height() - p.y()) / (this->height() - title_height_) * (view_top_ - view_bottom_) + view_bottom_;

        view_left_ += previous_pos_.rx() - longi;
        view_right_ += previous_pos_.rx() - longi;
        view_top_ += previous_pos_.ry() - lati;
        view_bottom_ += previous_pos_.ry() - lati;

        longi = (float)p.x() / (this->width() - color_bar_width_) * (view_right_ - view_left_) + view_left_;
        lati = (float)(this->height() - p.y()) / (this->height() - title_height_) * (view_top_ - view_bottom_) + view_bottom_;

        previous_pos_ = QPointF(longi, lati);

		emit ViewChanged(viewer_index_);

        this->update();
    }
}

void WrfImageViewer::mouseReleaseEvent(QMouseEvent *event){
    if ( status_ == VIEWER_TRACKING_PEN ) {
        emit SelectionFinished(viewer_index_);
        this->update();
    }
}

void WrfImageViewer::wheelEvent(QWheelEvent *event){
    QPoint p = event->pos();

    float longi = (float)p.x() / (this->width() - color_bar_width_) * (view_right_ - view_left_) + view_left_;
    float lati = (float)(this->height() - p.y()) / (this->height() - title_height_) * (view_top_ - view_bottom_) + view_bottom_;

    float temp_scale = event->delta() / 1000.0;
    float size_scale = rendering_scale_ / (rendering_scale_ + temp_scale);

    if ( rendering_scale_ + temp_scale < 0.1 ) return;

    view_left_ = longi + (view_left_ - longi) * size_scale;
    view_right_ = longi + (view_right_ - longi) * size_scale;
    view_top_ = lati + (view_top_ - lati) * size_scale;
    view_bottom_ = lati + (view_bottom_ - lati) * size_scale;

    rendering_scale_ += temp_scale;

	emit ViewChanged(viewer_index_);

    this->update();
}

void WrfImageViewer::mouseDoubleClickEvent(QMouseEvent *event){
    emit ImageSelected(viewer_index_);
}

void WrfImageViewer::ClearElement(){
    for ( int i = 0; i < element_vec_.size(); ++i ) delete element_vec_[i];
    element_vec_.clear();
	for ( int i = 0; i < action_element_visibility_.size(); ++i ){
		context_menu_->removeAction(action_element_visibility_[i]);
		delete action_element_visibility_[i];
	}
	action_element_visibility_.clear();
    if ( color_bar_element_ != NULL ) delete color_bar_element_;
    color_bar_element_ = NULL;
    title_ = "";

    this->update();
}

void WrfImageViewer::contextMenuEvent(QContextMenuEvent *event){
	QCursor cur = this->cursor();

	context_menu_->exec(cur.pos());
}

void WrfImageViewer::OnActionAddIsoLinePlotTriggered(){
	WrfIsoLinePlotDialog para_dialog;
	int datetime;
	WrfModelType model_type;
	WrfElementType element_type;
	int fhour;
	float iso_value;
	int ens;
	int mode;
	if ( para_dialog.exec() == QDialog::Accepted ){
		para_dialog.GetSelectedParameters(mode, datetime, model_type, element_type, fhour, iso_value, ens);
		this->AddIsoLinePlot(mode, datetime, model_type, element_type, fhour, iso_value, ens);
	}
}

void WrfImageViewer::OnVisibilityActionTriggered(){
	for ( int i = 0; i < element_vec_.size(); ++i ){
		element_vec_[i]->SetVisible(action_element_visibility_[i]->isChecked());
	}
	this->update();
}

void WrfImageViewer::showEvent(QShowEvent *event){
	this->update();
}

void WrfImageViewer::GetViewPara(float& rendering_scale, float& left, float& right, float& bottom, float& top){
	rendering_scale = rendering_scale_;
	left = view_left_;
	right = view_right_;
	bottom = view_bottom_;
	top = view_top_;
}

void WrfImageViewer::SetViewPara(float rendering_scale, float left, float right, float bottom, float top){
	rendering_scale_ = rendering_scale;
	view_left_ = left;
	view_right_ = right;
	view_bottom_ = bottom;
	view_top_ = top;

	this->update();
}