#ifndef WRF_PARA_N_VIEW_H_
#define WRF_PARA_N_VIEW_H_

#include "gl/glew.h"
#include <QtOpenGL/QGLWidget>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <vector>
#include "wrf_data_common.h"

class WrfSubregionRmsView : public QGLWidget{
    Q_OBJECT
public:
    WrfSubregionRmsView();
    ~WrfSubregionRmsView();

	void SetData(std::vector< std::vector< float > >& data, std::vector< std::vector< bool > >& is_data_selected, 
		std::vector< std::vector< int > >& suggestion_values, std::vector< int >& selected_values);

	void GetSelectionValues(std::vector< int >& n_value);

    void SetHighlightRecordIndex(int node_index, int record_index);

	void SetMaximumViewingNum(int max_num);

	void SetRmsUnits(std::vector< RmsUnit >& units);

	void ApplySelection();

	void SetViewMode(int mode);

	enum ViewStatus{
		NORMAL_STATUS	= 0x0000,
		FINAL_VIEW		= 0x0001,
		BRUSHING_ON		= 0x0010,
		FLOATING_ON		= 0x0100,
		VIEW_DIRTY		= 0x1000
	};

	public slots:
		void OnBrushActionTriggered(bool b);
    
signals:
    void SelectionValueChanged();
    void CurrentRecordChanged(int node_index, int record_index);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

private:
    std::vector< std::vector< float > > data_values_;
    std::vector< std::vector< bool > > is_data_selected_;
	std::vector< std::vector< int > > suggestion_values_;
    std::vector< int > selected_value_;
	std::vector< RmsUnit > rms_units_;

	// rendering parameters
	int max_viewing_num_;
    int focus_node_index_;
    int focus_record_index_;
	std::vector< QPoint > selecting_path_;

	unsigned int view_status_;

	// layout parameters
    int border_size_, suggestion_text_width_, coor_indicator_width_, y_value_text_width_;
    int title_height_, node_height_, x_coor_text_height_;
	float left_, x_step_, bottom_, y_step_;
	float node_margin_, indicator_radius_;

	void RenderGridInfoPie(int index);
	void RenderArc(QColor color, float radius, float begin_theta, float end_theta);
};

#endif