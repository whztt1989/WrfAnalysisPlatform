#ifndef WRF_STAMP_ITEM_H_
#define WRF_STAMP_ITEM_H_

#include <QtGui/QGraphicsPixmapItem>
#include <QtCore/QObject>

class WrfStampItem : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT

public:
	WrfStampItem();
	~WrfStampItem();

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

#endif