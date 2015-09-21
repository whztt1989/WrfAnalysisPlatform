#ifndef STAMP_COMPARING_VIEWER_H_
#define STAMP_COMPARING_VIEWER_H_

#include <QtGui/QGraphicsView>
#include <QtGui/QWidget>

class WrfGridValueMap;

class StampComparingViewer : public QWidget
{
	Q_OBJECT

public:
	StampComparingViewer();
	~StampComparingViewer();

	std::vector< QPixmap* >* stamp_pixmaps;
	std::vector< WrfGridValueMap* >* value_maps;

	enum InteractionMode{
		NORMAL = 0x0,
		ROI_MODE,
		SEGMENT_MODE
	};

	void SetComparingId(int id1, int id2);
	void UpdateViewer();
	WrfGridValueMap* GetOperationMap() { return result_value_map_; }

	public slots:
		void OnClearRoiActionTriggered();
		void OnAddRoiActionTriggered();
		void OnSegmentActionTriggered();
		void OnPlusActionTriggered();
		void OnMinusActionTriggered();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

private:
	InteractionMode mode_;
	WrfGridValueMap* result_value_map_;
	QPixmap* result_pixmap_;
	QPixmap* mask_map_[2];
	QImage* selected_map_;
	int map_ids[2];
	int bin_size;
	int max_bin_count_;
	std::vector< bool > selected_area_index_;
	std::vector< bool > map_selected_index_;
	std::vector< int > belonging_index1_, belonging_index2_;
	std::vector< int > cluster_center_index1_, cluster_center_index2_;
	std::vector< float > bias_histogram_values_, map_histogram_values_[2], result_histogram_values_;
	std::vector< QColor > index_color;
	std::vector< int > belonging_[2], center_pos_[2];
	std::vector< int > center_bias_[2];
	QPointF mouse_pos_;
	int selection_radius_;

	void Cluster(WrfGridValueMap* map, std::vector< int >& center_pos, std::vector< int >& belonging_index);
	void Cluster(QImage& image, std::vector< int >& center_pos, std::vector< int >& belonging_index);
	void ClearSelection();
	void UpdateHistogramValues();
	void UpdateMapHistogramValues();
	void UpdateResultHistogramValues();
	void AddSelection(QPointF point);
	void UpdateSelection();

	void Minus(WrfGridValueMap* map1, WrfGridValueMap* map2, bool is_enhanced, WrfGridValueMap* result_map);
	void Plus(WrfGridValueMap* map1, WrfGridValueMap* map2, bool is_enhanced, WrfGridValueMap* result_map);
};

#endif