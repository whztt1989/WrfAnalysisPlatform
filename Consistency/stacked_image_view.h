#ifndef STACKED_IMAGE_VIEW_H_
#define STACKED_IMAGE_VIEW_H_

#include <QtOpenGL/QGLWidget>
#include <vector>
#include "consistency_common.h"

class StackedImageView : public QGLWidget{
    Q_OBJECT

public:
    StackedImageView(QWidget* parent = 0);
    ~StackedImageView();

	void SetData(std::vector< BasicImage* >& images);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	std::vector< BasicImage* > view_images_;
};

#endif