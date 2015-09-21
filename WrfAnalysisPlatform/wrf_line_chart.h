#ifndef WRF_LINE_CHART_H_
#define WRF_LINE_CHART_H_

#include "GL/glew.h"
#include <QtOpenGL/QGLWidget>

#include "wrf_data_stamp.h"

class WrfLineChart : public QGLWidget
{
	Q_OBJECT

public:
	enum ViewMode{
		BIAS_VIEW = 0x0,
		ABSOLUTE_VIEW
	};
	enum DataMode{
		NORMAL_MODE = 0x0,
		COMPARED_MODE,
		CLUSTER_MODE
	};

	WrfLineChart(ViewMode mode);
	~WrfLineChart();

	void set_view_mode(ViewMode mode);
	void set_data_mode(DataMode mode);
	void set_data(WrfLineChartDataSet* data);
	void set_compared_data(WrfLineChartDataSet* data);
	void SetClusterOn();
	void SetClusterOff();
	bool is_cluster_on() { if (data_mode_ == CLUSTER_MODE) return true; else return false; }

	public slots:
		void OnSelectedAreaChanged();
		void OnSelectedAttributeChanged(WrfGeneralDataStampType);
		void ApplyChanges();
		void OnHighLightChanged();
		void OnHighLightOff();

signals:
	void HighLightChanged();
	void HighLightOff();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);

private:
	WrfLineChartDataSet* data_;
	WrfLineChartDataSet* compared_data_;

	std::vector< std::vector< LineChartRecord* > > cluster_centers_;
	std::vector< std::vector< int > > cluster_flags_;
	std::vector< std::vector< int > > belonging_;

	std::vector< int > axis_pos_;
	QPoint move_begin_pos_;

	std::vector< bool > highlight_index_;
	std::vector< std::vector< bool > > is_record_highlight_;
	bool is_high_light_on_;
	bool is_selection_on_;
	int selection_axis_index_;

	std::vector< float > scene_values_;
	ViewMode view_mode_;
	DataMode data_mode_;
	DataMode previous_mode_;
	std::vector< int > painting_order_vec_;
	std::vector< bool > model_visibility_;

	float y_border_height, x_border_width, y_text_height, x_text_width, identity_height_;
	float line_min_value_, line_max_value_;
	float axis_bottom_y_value_, axis_top_y_value_;
	WrfGeneralDataStampType data_type_;
	std::vector< std::vector< int > > model_indentity_pos_;

	float mean_value_, origin_mean_value_;
	float value_radius_, origin_value_radius_;

	float mean_height_, radius_length_, mean_x_axis_;

	bool compared_data_order_;
	bool is_mean_selected_;
	bool is_radius_selected_;

	void PaintIdentities();
	void PaintAbsoluteAxis();
	void PaintBiasAxis();
	void PaintLines(std::vector< std::vector< LineChartRecord* > >& data_set, float r, float g, float b, float alpha_scale);
	void DrawCirclePlane(float radius, float angle, float r, float g, float b, float a);
	void DrawBsplineCurve(std::vector< float >& point_pos, std::vector< float >& t_values, std::vector< float >& point_t_values, std::vector< int >& t_range_vec, int curve_level);

	void PaintAdjustController();

	void HierarchicalCluster(std::vector< std::vector< LineChartRecord* > >& records, std::vector< std::vector< LineChartRecord* > >& centers);
	float Distance(LineChartRecord* record1, LineChartRecord* record2);
};

#endif