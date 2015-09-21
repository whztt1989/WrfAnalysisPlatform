#ifndef IMAGE_THEME_VIEW_H_
#define IMAGE_THEME_VIEW_H_

#include "gl/glew.h"
#include <QtOpenGL/QGLWidget>

class SegmentImage;

class ImageThemeView : public QGLWidget
{
	Q_OBJECT

public:
	ImageThemeView();
	~ImageThemeView();

    void SetData(std::vector< SegmentImage* > images);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

    void PaintOverall();
    void PaintSelection();
    
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

private:
    QRectF view_rect_;

    int selection_index_;

    std::vector< SegmentImage* > forecasting_images_;
    std::vector< std::vector< float > > change_uncertainty_values_;
    std::vector< std::vector< float > > accuracy_uncertainty_values_;

    std::vector< std::vector< int > > corr_segment_index_;
};

#endif