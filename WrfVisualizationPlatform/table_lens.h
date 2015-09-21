#ifndef TABLE_LENS_H_
#define TABLE_LENS_H_

#include "gl/glew.h"
#include <QtGui/QColor>
#include <QtGui/QMenu>
#include <QtOpenGL/QGLWidget>
#include <vector>
#include <iostream>
#include "wrf_data_common.h"

class TableLensDataset {
public:
    TableLensDataset() {}
    ~TableLensDataset() {}

    QString data_name;

	// [node][record]
    std::vector< std::vector< float > > record_values;
    std::vector< float > value_ranges;

	// [node]
    std::vector< float > record_absolute_values; 
    std::vector< float > absolute_value_ranges;

	// [node][record]
    std::vector< std::vector< float > > scale_values;
    std::vector< float > scale_value_ranges;
	
	// [node][record]
    std::vector< std::vector< bool > > is_record_selected;
	std::vector< std::vector< int > > record_index;

	std::vector< float > energy_value;
};

class TableLens : public QGLWidget{
    Q_OBJECT
public:
    TableLens(int index, QWidget *parent = 0);
    ~TableLens();

    void SetDataset(TableLensDataset* data);
	void SetHighlightRecordIndex(int node_index, int record_index);
	void SetRmsUnits(std::vector< RmsUnit >& units);

	void SetViewMode(int);
	
	void SetSuggestionRate(float rate);
	void SetSuggestionOn();
	void SetSuggestionOff();

	enum ViewStatus{
		NORMAL_STATUS	= 0x00000000,
		SCALE_MODE		= 0x00000001,
		RMS_MODE		= 0x00000010,
		ABS_VALUE_ON	= 0x00000100,
		ADJUSTING_ON	= 0x00100000,
		VIEW_DIRTY		= 0x01000000,
		SUGGESTION_ON	= 0x10000000
	};

signals:
    void SelectionChanged(int);
    void CurrentRecordChanged(int node_index, int record_index);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    int lens_index_;
    TableLensDataset* dataset_;

	unsigned int view_status_;

	// rendering parameters
	float scale_threshold_;
	float max_scale_;
	int mutual_suggestion_index_;
	float current_covering_rate_;

	int focus_node_index_;
	int focus_record_index_;

	std::vector< RmsUnit > rms_units_;
	std::vector< float > sorted_values_;

    // layout parameters
    int title_height_, node_height_, x_coor_text_height_;
    int border_size_, energy_coor_width_, coor_indicator_width_, y_value_text_width_;
    float left_, x_step_, bottom_;
	float coor_center_, coor_height_;
	int indicator_radius_, node_margin_;

    void UpdateSelection();
	void UpdateSuggestion();

	void RenderGridInfoPie(int index);
	void RenderArc(QColor color, float radius, float begin_theta, float end_theta);
};

#endif