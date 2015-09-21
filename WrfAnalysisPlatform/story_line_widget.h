#ifndef STORY_LINE_WIDGET_H_
#define STORY_LINE_WIDGET_H_

#include <vector>
#include <QtGui/QGraphicsView>
#include <QtGui/QPixmap>

class WrfGridValueMap;
class WrfStampItem;
class Node;

class StoryLineWidget : public QGraphicsView
{
	Q_OBJECT

public:
	StoryLineWidget();
	~StoryLineWidget();

	enum OperationType{
		SUM_OP = 0x0,
		MINUS_OP,
		VAR_OP
	};

	WrfGridValueMap* GetOperationResult(OperationType op_type);
	void AddStoryStamp(int index);
	void GetOperationElementIds(std::vector< int >& ids);

signals:
	void BiasMapSelected(int);

private:
	WrfGridValueMap* result_value_map_;

	std::vector< int > story_stamp_vec_;
	std::vector< Node* > stamp_items_;
	std::vector< QPixmap* > stamp_pixmaps_;
	std::vector< int > last_operation_elements_;

	void SumMap(std::vector< WrfGridValueMap* >& maps);
	void Minius(std::vector< WrfGridValueMap* >& maps);
	void Variation(std::vector< WrfGridValueMap* >& maps);
};

#endif