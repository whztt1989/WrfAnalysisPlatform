#ifndef WRF_STAMP_VIEWER_H_
#define WRF_STAMP_VIEWER_H_

#include <vector>
#include <QtGui/QGraphicsView>
#include <QtGui/QPixmap>

class WrfGridValueMap;
class WrfStampItem;

class WrfStampViewer : public QGraphicsView
{
	Q_OBJECT

public:
	WrfStampViewer();
	~WrfStampViewer();

	void SetItemVarMaps(std::vector< WrfGridValueMap* >& maps);

private:
	std::vector< WrfGridValueMap* > grid_value_maps_;
	std::vector< WrfStampItem* > stamp_items_;
	std::vector< std::vector< float > > item_forces_;
	std::vector< QPixmap* > stamp_pixmaps_;

	QGraphicsScene* view_scene_;

	void UpdateItemForces();
};

#endif