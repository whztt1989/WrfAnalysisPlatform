#include "wrf_stamp_viewer.h"
#include "wrf_stamp_item.h"
#include "wrf_stamp_generator.h"
#include "wrf_data_stamp.h"

WrfStampViewer::WrfStampViewer(){
	view_scene_ = new QGraphicsScene();
	view_scene_->setSceneRect(0, 0, 800, 600);

	this->setScene(view_scene_);

	this->setFixedSize(810, 610);
}

WrfStampViewer::~WrfStampViewer(){

}

void WrfStampViewer::SetItemVarMaps(std::vector< WrfGridValueMap* >& maps){
	grid_value_maps_.resize(maps.size());
	grid_value_maps_.assign(maps.begin(), maps.end());

	if ( stamp_items_.size() != 0 ){
		for ( int i = 0; i < stamp_items_.size(); ++i ) delete stamp_items_[i];
	}
	stamp_items_.clear();
	if ( stamp_pixmaps_.size() != 0 ){
		for ( int i = 0; i < stamp_pixmaps_.size(); ++i ) delete stamp_pixmaps_[i];
	}
	stamp_pixmaps_.clear();

	for ( int i = 0; i < grid_value_maps_.size(); ++i ){
		WrfGridValueMap* value_map = grid_value_maps_[i];
		int w = (value_map->end_longitude - value_map->start_longitude) * 3;
		int h = (value_map->end_latitude - value_map->start_latitude) * 3;
		QPixmap* temp_map = WrfStampGenerator::GetInstance()->GenerateStamp(value_map, w, h);

		WrfStampItem* temp_item = new WrfStampItem;
		temp_item->setPixmap(*temp_map);

		stamp_items_.push_back(temp_item);
		stamp_pixmaps_.push_back(temp_map);

		view_scene_->addItem(temp_item);
		temp_item->setPos(400, 300);
	}

	UpdateItemForces();
}

void WrfStampViewer::UpdateItemForces(){

}