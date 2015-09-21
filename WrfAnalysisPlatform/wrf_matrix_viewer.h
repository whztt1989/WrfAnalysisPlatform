#ifndef WRF_MATRIX_VIEWER_H_
#define WRF_MATRIX_VIEWER_H_

#include <QtOpenGL/QGLWidget>
#include "wrf_common.h"

class WrfMatrixViewer : public QGLWidget
{
public:
	WrfMatrixViewer();
	~WrfMatrixViewer();

	void set_data(WrfMatrixChartDataSet* data);
	void GetResidualIndex(std::vector< int >& residual_index);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	WrfMatrixChartDataSet* data_;
	std::vector< int > matrix_mapping_;
};

#endif