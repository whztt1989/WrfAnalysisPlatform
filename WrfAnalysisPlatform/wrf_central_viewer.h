#ifndef WRF_CENTRAL_VIEWER_H_
#define WRF_CENTRAL_VIEWER_H_

#include "GL/glew.h"
#include <QtOpenGL/QGLWidget>
#include <QtGui/QImage>
#include "data_type.h"
#include "isolinecreator.h"

class WrfGridValueMap;
class WrfGridSimilarityMap;

class WrfCentralViewer : public QGLWidget
{
	Q_OBJECT

public:
	WrfCentralViewer();
	~WrfCentralViewer();

	enum ViewMode {
		VIEWING = 0x0,
		SELECTING_SITE,
		ADDING_AREA,
		MAGIC_WAND,
		ERASING_AREA,
		HIGHLIGHT_MODE
	};
	enum SelectionMode{
		NORMAL_MODE = 0x0,
		COMPARED_MODE,
	};

	void SetViewMode(ViewMode mode);
	void SetSelectionMode(SelectionMode mode);

	void ClearSelection();
	void set_bias_map(WrfGridValueMap* bias_map);
	void update_bias_map(WrfGridValueMap* bias_map);
	void set_wind_var_map(WrfGridValueMap* var_map);
	void set_grid_value_map(WrfGridValueMap* value_map);

	void set_selected_area(std::vector< int >& area_grid_index);
	void GetSelectedArea(std::vector< int >& area_grid_index);

	void AcceptCandidateArea();

	WrfGridValueMap* bias_map() { return bias_map_; }

	//generate isolines @gcdofree
	void StartEditIsolines();
	void EndEditIsolines();

	public slots:
		void OnWindVarThresholdChanged();
		void OnHighLightChanged();
		void OnHighLightOff();

signals:
	void SiteSelected(int grid_index);
	void SelectedAreaChanged();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	// Paint the map as the background
	void PaintBackgroundMap();

	// Paint the confidence heat map
	void PaintBiasMap();

	void PaintColorBar();

	void PaintGridPoints();

	void PaintSelectedArea();

	void PaintCandidateSelectedArea();

	void PaintSites();

	void PaintWindVariance();

	//paint isolines @gcdofree
	void PaintIsoLines();
	void PaintSelectIsoLine();
	void PaintEditIsoLine();

	// Paint the tip icons
	void PaintTipIcons();

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);

	void DrawCirclePlane(float radius, float r, float g, float b, float a);

private:
	ViewMode view_mode_;
	ViewMode previous_mode_;
	SelectionMode selection_mode_;

	int selection_radius_;

	QImage china_map_image_;
	float map_left_, map_right_, map_bottom_, map_top_;

	WrfGridValueMap* grid_value_map_;
	WrfGridValueMap* bias_map_;
	WrfGridSimilarityMap* similarity_map_;
	WrfGridValueMap* wind_var_map_;
	std::vector< int > selected_area_;

	std::vector< std::vector< float > > map_data_;

	float start_longitude_, end_longitude_, start_latitude_, end_latitude_;
	int longitude_grid_number_, latitude_grid_number_;
	float left_, right_, bottom_, top_;
	float x_bias_, y_bias_;
	float scale_;

	GLuint selected_tex_, candidate_tex_, bias_tex_;
	std::vector< float > selected_area_pixel_value_, candidate_area_pixel_value_, bias_pixel_value_, compared_area_pixel_values_;
	std::vector< float > site_alpha_;
	
	std::vector< bool > is_grid_selected_;
	std::vector< char > is_candidate_selected_;
	std::vector< bool > is_compared_selected_;
	std::vector< bool > is_grid_high_light_;
	QPoint previous_press_pos_;

	std::vector< float > bias_vertex_pos_;
	std::vector< float > bias_vertex_color_;
	std::vector< int > bias_tri_index_;

	std::vector< QPoint > selection_lines_;

	//@gcdofree
	IsolineCreator isolineCreator;
	bool is_edit_isoline;
	bool is_draw_edit_line;
	vector<float> isolineValueVector;
	vector< vector<isotools::Isoline> > isolineList;
	vector<isotools::Point2D> editList;
	int edit_isoline_index, edit_isoline_detail, edit_start_pos, edit_end_pos, edit_aid_pos;

	void UpdateBackgroundTexture();
	void LoadMapData();
	void UpdateSelectedArea();
	void AddSelection(QPoint point);

	//generate isolines @gcdofree
	void GenerateIsoLines();
	float GetEulerDistance(float x1, float y1, float x2, float y2);
	void getInterpolationResult(vector<float> originData, vector<float> &resultData, vector<float> originIndex, vector<float> resultIndex);
	void UpdateCandidateSelectedArea();
	void GetGridIndex(float longitude, float latitude, int& grid_x, int& grid_y);
	void GetGridIndexByWindowPos(int pos_x, int pos_y, int& grid_x, int& grid_y);
	void CandidateRegionGrowing(float threshold, float bias_alpha, float wind_alpha, std::vector< char >& is_selected);

	void UpdateSelectedBuffer();
	void UpdateCandidateSelectedBuffer();
	void UpdateBiasMapBuffer();

	void UpdateTextures();
};

#endif
