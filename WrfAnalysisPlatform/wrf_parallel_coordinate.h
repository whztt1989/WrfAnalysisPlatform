#ifndef WRF_PARALLEL_COORDINATE_H_
#define WRF_PARALLEL_COORDINATE_H_

#include <vector>
#include "gl/glew.h"
#include <QtOpenGL/QGLWidget>

#include "wrf_common.h"
#include "wrf_data_stamp.h"

class WrfParallelCoordinate : public QGLWidget
{
	Q_OBJECT
public:
	WrfParallelCoordinate();
	~WrfParallelCoordinate();

	void set_data_set(WrfParallelDataSet* data_set_t);
	void set_compared_data_set(WrfParallelDataSet* data_set_t);
	void set_highlight_index(const std::vector< bool >& highlight_index_t);
	void SetClusterOn();
	void SetClusterOff();
	bool is_cluster_on() { if (data_mode_ == CLUSTERED) return true; else return false; }
	void set_attrib_weight(std::vector< float >& weight);

	enum ParallelViewMode{
		BIAS_VIEW = 0x0,
		ABSOLUTE_VIEW
	};

	enum ParallelDataMode{
		NOMRAL = 0x0,
		COMPARED,
		CLUSTERED
	};

	enum PcpSystemStatus{
		SYSTEM_OK				= 0x000001,
		GL_INIT_ERROR			= 0x000010
	};

	void set_view_mode(ParallelViewMode mode);
	void set_data_mode(ParallelDataMode mode);
	void set_is_weight_on(bool is_on) { is_weight_on_ = is_on; }

	public slots:
		void OnSelectedAreaChanged();
		void OnHighLightChanged();
		void OnHighLightOff();

signals:
	void AttributeSelectedChanged(WrfGeneralDataStampType);
	void InfoGainSelected(int);
	void AttribWeightChanged();
	void HighLightChanged();
	void HighLightOff();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void mousePressEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	WrfParallelDataSet* data_set_;
	WrfParallelDataSet* compared_data_set_;
	ParallelDataMode data_mode_;

	std::vector< std::vector< WrfParallelRecord > > cluster_centers_;
	std::vector< std::vector< int > > cluster_flags_;
	std::vector< std::vector< int > > belonging_;

	std::vector< float > attrib_weight_;

	std::vector< bool > highlight_index_;
	std::vector< int > reranked_index_;
	std::vector< int > axis_pos_;
	float axis_bottom_y_value_, axis_top_y_value_;

	std::vector< int > adjust_plane_pos_;
	std::vector< std::vector< int > > current_identity_pos_;
	std::vector< std::vector< int > > historical_identity_pos_;
	std::vector< bool > is_current_model_shown_;
	std::vector< std::vector< bool > > is_record_highlight_;
	std::vector< int > model_painting_order_;
	std::vector< float > min_value_vec_, max_value_vec_;

	std::vector< std::vector< float > > info_gain_vec_;

	float x_border_width_scale_;
	float y_border_width_scale_;
	float attrib_text_height_scale_;
	float value_text_height_scale_;
	float axis_width_scale_;
	float model_identity_scale_;
	float attri_weight_circle_scale_;

	std::vector< float > point_pos_;
	std::vector< float > point_color_;
	std::vector< int > line_index_;

	int system_status_;
	bool is_data_updated_;
	ParallelViewMode view_mode_;
	bool is_high_light_on_;
	bool is_selection_on_;
	bool is_weight_on_;
	QPoint move_begin_pos_;
	int selection_axis_index_;

	bool compared_painting_order_;

	void PaintCoordinate();
	void PaintLines();
	void PaintIdentifyItems();
	void PaintCenterLine();

	void DrawCirclePlane(float radius, float angle, float r, float g, float b, float a);
	void DrawTheropyPlane(float inner_radius, float outer_radius, int index);
	void DrawBsplineCurve(std::vector< float >& point_pos, std::vector< float >& t_values, std::vector< float >& point_t_values, std::vector< int >& t_range_vec, int curve_level);
	void DrawLineRecords(std::vector< std::vector< WrfParallelRecord > >& records, std::vector< bool >& model_visibility, float r, float g, float b, float alpha_scale);

	void HierarchicalCluster(std::vector< std::vector< WrfParallelRecord > >& records, std::vector< float >& weight, std::vector< std::vector< WrfParallelRecord > >& centers);
	float Distance(WrfParallelRecord& record1, WrfParallelRecord& record2, std::vector< float >& weight);

	void UpdateView();
};

#endif