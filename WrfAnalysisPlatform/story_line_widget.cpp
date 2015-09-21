#include "story_line_widget.h"
#include "wrf_data_manager.h"
#include "wrf_data_stamp.h"
#include "wrf_stamp_generator.h"
#include "node.h"

StoryLineWidget::StoryLineWidget(){
	QGraphicsScene *scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	scene->setSceneRect(QRectF(0, 0, 3200, 300));
	setScene(scene);
	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);

	this->setAlignment(Qt::AlignLeft);

	story_stamp_vec_.clear();
	result_value_map_ = NULL;
}

StoryLineWidget::~StoryLineWidget(){

}

void StoryLineWidget::AddStoryStamp(int index){
	story_stamp_vec_.push_back(index);
	WrfGridValueMap* value_map = WrfDataManager::GetInstance()->GetStoryStamp(index);

	QString tool_tip_string;

	if ( value_map->level == 0 ){
		tool_tip_string = QString("Rain: %0\t\nPressure: %1  \t\nTemperature: %2  \t\nHeight: %3  \t\nWind_longi: %4  \t\nWind_lati: %5  \t\nRelative Humidity: %6\t\n").arg(value_map->weight[0]).arg(value_map->weight[1]).arg(value_map->weight[2]).arg(value_map->weight[3]).arg(value_map->weight[4]).arg(value_map->weight[5]).arg(value_map->weight[6]);
	} else {
		tool_tip_string = QString("Operation Map");
	}
	
	int w = (value_map->end_longitude - value_map->start_longitude) * 3;
	int h = (value_map->end_latitude - value_map->start_latitude) * 3;
	QPixmap* temp_map = WrfStampGenerator::GetInstance()->GenerateStamp(value_map, w, h);

	stamp_pixmaps_.push_back(temp_map);

	Node *node;
	if ( value_map->level != 0 )
		node = new Node(NULL, temp_map, index, value_map->level);
	else
		node = new Node(NULL, temp_map, value_map->weight, value_map->info, index, value_map->level);

	node->SetFixedPosOn();
	node->setPos(20 + (temp_map->width() + 20) * (story_stamp_vec_.size() - 1) + temp_map->width() / 2, 20 + temp_map->height() / 2);

	stamp_items_.push_back(node);

	scene()->addItem(node);

	connect(node, SIGNAL(ItemSelected(int)), this, SIGNAL(BiasMapSelected(int)));

	node->setToolTip(tool_tip_string);
}

WrfGridValueMap* StoryLineWidget::GetOperationResult(OperationType op_type){
	last_operation_elements_.clear();

	std::vector< WrfGridValueMap* > elements;
	for ( int i = 0; i < stamp_items_.size(); ++i )
		if ( stamp_items_[i]->is_selected() ){
			elements.push_back(WrfDataManager::GetInstance()->GetStoryStamp(stamp_items_[i]->id()));
			last_operation_elements_.push_back(stamp_items_[i]->id());
		}
	switch (op_type){
	case SUM_OP:
	case MINUS_OP:
		if ( elements.size() != 2) return NULL;
		break;
	case VAR_OP:
		if ( elements.size() < 2 ) return NULL;
		break;
	default:
		return NULL;
		break;
	}

	if ( result_value_map_ == NULL ){
		result_value_map_ = new WrfGridValueMap;
		result_value_map_->start_latitude = elements[0]->start_latitude;
		result_value_map_->end_latitude = elements[0]->end_latitude;
		result_value_map_->start_longitude = elements[0]->start_longitude;
		result_value_map_->end_longitude = elements[0]->end_longitude;
		result_value_map_->latitude_grid_number = elements[0]->latitude_grid_number;
		result_value_map_->longitude_grid_number = elements[0]->longitude_grid_number;
		result_value_map_->latitude_grid_space = elements[0]->latitude_grid_space;
		result_value_map_->longitude_grid_space = elements[0]->longitude_grid_space;
		result_value_map_->max_value = elements[0]->max_value;
		result_value_map_->min_value = elements[0]->min_value;
		result_value_map_->values.resize(elements[0]->longitude_grid_number * elements[0]->latitude_grid_number);
	}

	switch (op_type){
	case SUM_OP:
		SumMap(elements);
		break;
	case MINUS_OP:
		Minius(elements);
		break;
	case VAR_OP:
		Variation(elements);
		break;
	default:
		return NULL;
		break;
	}
	
	int temp_level = -1;
	for ( int i = 0; i < elements.size(); ++i ) if ( elements[i]->level > temp_level ) temp_level = elements[i]->level;
	result_value_map_->level = temp_level + 1;

	return result_value_map_;
}

void StoryLineWidget::GetOperationElementIds(std::vector< int >& ids){
	ids.resize(last_operation_elements_.size());
	ids.assign(last_operation_elements_.begin(), last_operation_elements_.end());
}

void StoryLineWidget::Minius(std::vector< WrfGridValueMap* >& maps){
	WrfGridValueMap* map_one = maps[0];
	WrfGridValueMap* map_two = maps[1];

	float min_value = 1e10;
	float max_value = -1e10;
	for ( int i = 0; i < map_one->values.size(); ++i ){
		result_value_map_->values[i] = abs(map_one->values[i] - map_two->values[i]);
		if ( result_value_map_->values[i] > max_value ) max_value = result_value_map_->values[i];
		if ( result_value_map_->values[i] < min_value ) min_value = result_value_map_->values[i];
	}
	for ( int i = 0; i < result_value_map_->values.size(); ++i )
		result_value_map_->values[i] = (result_value_map_->values[i] - min_value) / (max_value - min_value);
}

void StoryLineWidget::SumMap(std::vector< WrfGridValueMap* >& maps){

}

void StoryLineWidget::Variation(std::vector< WrfGridValueMap* >& maps){

}