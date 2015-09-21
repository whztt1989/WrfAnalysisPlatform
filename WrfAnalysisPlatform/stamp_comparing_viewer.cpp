#include "stamp_comparing_viewer.h"

#include <QtGui/QMouseEvent>

#include "wrf_data_stamp.h"
#include "wrf_stamp_generator.h"
#include "qcolor_bar_controller.h"
#include "segm/msImageProcessor.h"

StampComparingViewer::StampComparingViewer(){
	stamp_pixmaps = NULL;
	value_maps = NULL;
	result_value_map_ = NULL;
	result_pixmap_ = NULL;
	mask_map_[0] = NULL;
	mask_map_[1] = NULL;
	selected_map_ = NULL;

	map_ids[0] = -1;
	map_ids[1] = -1;

	bin_size = 40;

	mouse_pos_  = QPointF(-1, -1);
	selection_radius_ = 3;

	mode_ = NORMAL;

	for ( int i = 0; i < bin_size; ++i )
		index_color.push_back(QColorBarController::GetInstance(METEO_COLOR_MAP)->GetColor(0, 1, (float)i / bin_size));

	/*QGraphicsScene *scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	scene->setSceneRect(0, 0, 400, 400);
	setScene(scene);
	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);*/

	this->setMouseTracking(false);
}

StampComparingViewer::~StampComparingViewer(){

}

void StampComparingViewer::SetComparingId(int id1, int id2){
	if ( value_maps == NULL ) return;

	map_ids[0] = id1;
	map_ids[1] = id2;

	if ( result_value_map_ == NULL ) result_value_map_ = new WrfGridValueMap;
	result_value_map_->start_latitude = value_maps->at(id1)->start_latitude;
	result_value_map_->end_latitude = value_maps->at(id1)->end_latitude;
	result_value_map_->start_longitude = value_maps->at(id1)->start_longitude;
	result_value_map_->end_longitude = value_maps->at(id1)->end_longitude;
	result_value_map_->latitude_grid_number = value_maps->at(id1)->latitude_grid_number;
	result_value_map_->longitude_grid_number = value_maps->at(id1)->longitude_grid_number;
	result_value_map_->latitude_grid_space = value_maps->at(id1)->latitude_grid_space;
	result_value_map_->longitude_grid_space = value_maps->at(id1)->longitude_grid_space;
	result_value_map_->max_value = value_maps->at(id1)->max_value;
	result_value_map_->min_value = value_maps->at(id1)->min_value;
	result_value_map_->values.resize(value_maps->at(id1)->longitude_grid_number * value_maps->at(id1)->latitude_grid_number);
	result_value_map_->level = value_maps->at(id1)->level + 1;
	result_value_map_->weight.resize(value_maps->at(id1)->weight.size());
	if ( value_maps->at(id2)->level + 1 > result_value_map_->level ) result_value_map_->level = value_maps->at(id2)->level + 1;
	
	selected_area_index_.resize(value_maps->at(id1)->longitude_grid_number * value_maps->at(id1)->latitude_grid_number);
	selected_area_index_.assign(selected_area_index_.size(), true);
	max_bin_count_ = 0;

	UpdateHistogramValues();
	UpdateMapHistogramValues();

	Minus(value_maps->at(id1), value_maps->at(id2), true, result_value_map_);

	if ( result_pixmap_ != NULL ) delete result_pixmap_;
	int w = (result_value_map_->end_longitude - result_value_map_->start_longitude) * 3;
	int h = (result_value_map_->end_latitude - result_value_map_->start_latitude) * 3;
	result_pixmap_ = WrfStampGenerator::GetInstance()->GenerateStamp(result_value_map_, w, h);
	UpdateResultHistogramValues();

	map_selected_index_.resize(w * h);
	map_selected_index_.assign(map_selected_index_.size(), false);

	if ( selected_map_ != NULL ) delete selected_map_;
	selected_map_ = new QImage(w, h, QImage::Format_ARGB32);
	selected_map_->fill(QColor(0, 0, 0, 0));

	this->update();
}

void StampComparingViewer::UpdateViewer(){
	this->update();
}

void StampComparingViewer::UpdateHistogramValues(){
	WrfGridValueMap* map_one = value_maps->at(map_ids[0]);
	WrfGridValueMap* map_two = value_maps->at(map_ids[1]);
	bias_histogram_values_.resize(2 * bin_size);
	bias_histogram_values_.assign(bias_histogram_values_.size(), 0);
	for ( int i = 0; i < map_one->values.size(); ++i )
		if ( selected_area_index_[i] ){
			int bin_index = (int)((map_one->values[i] - map_two->values[i]) / 1.0 * (bin_size - 0.01));
			bin_index += bin_size;
			if ( bin_index >= bias_histogram_values_.size() ) bin_index = bias_histogram_values_.size() - 1;
			bias_histogram_values_[bin_index] += 1;
		}
	float temp_max_count = 0;
	for ( int i = 0; i < bias_histogram_values_.size(); ++i ) if ( bias_histogram_values_[i] > temp_max_count ) temp_max_count = bias_histogram_values_[i];
	for ( int i = 0; i < bias_histogram_values_.size(); ++i ) bias_histogram_values_[i] /= temp_max_count;
}

void StampComparingViewer::UpdateMapHistogramValues(){
	for ( int k = 0; k < 2; ++k ){
		WrfGridValueMap* map = value_maps->at(map_ids[k]);
		map_histogram_values_[k].resize(bin_size);
		map_histogram_values_[k].assign(map_histogram_values_[k].size(), 0);
		for ( int i = 0; i < map->values.size(); ++i )
			if ( selected_area_index_[i] ){
				int bin_index = (int)(map->values[i] / 1.0 * (bin_size - 0.01));
				if ( bin_index >= map_histogram_values_[k].size() ) bin_index = map_histogram_values_[k].size() - 1;
				map_histogram_values_[k][bin_index] += 1;
			}
		for ( int i = 0; i < map_histogram_values_[k].size(); ++i ) if ( map_histogram_values_[k][i] > max_bin_count_ ) max_bin_count_ = map_histogram_values_[k][i];
	}
}

void StampComparingViewer::UpdateResultHistogramValues(){
	result_histogram_values_.resize(bin_size);
	result_histogram_values_.assign(result_histogram_values_.size(), 0);
	for ( int i = 0; i < result_value_map_->values.size(); ++i )
		if ( selected_area_index_[i] ){
			int bin_index = (int)(result_value_map_->values[i] / 1.0 * (bin_size - 0.01));
			if ( bin_index >= result_histogram_values_.size() ) bin_index = result_histogram_values_.size() - 1;
			result_histogram_values_[bin_index] += 1;
		}
	for ( int i = 0; i < result_histogram_values_.size(); ++i ) if ( result_histogram_values_[i] > max_bin_count_ ) max_bin_count_ = result_histogram_values_[i];
}

void StampComparingViewer::paintEvent(QPaintEvent *event){
	QPainter* painter = new QPainter(this);
	if ( map_ids[0] != -1 ){
		int x = 10;
		int y = 10;
		QPixmap* map = stamp_pixmaps->at(map_ids[0]);
		painter->drawPixmap(x, y, map->width(), map->height(), *map);
		if ( mode_ == ROI_MODE ){
			painter->drawImage(x, y, *selected_map_);
		}

		int begin_x = 20 + result_pixmap_->width();
		int end_x = begin_x + result_pixmap_->width();
		int base_y = y + result_pixmap_->height();
		int histogram_height = result_pixmap_->height() - 10;
		int step = (end_x - begin_x) / map_histogram_values_[0].size();
		int temp_x = begin_x;
		for ( int i = 0; i < map_histogram_values_[0].size(); ++i ){
			int temp_y = map_histogram_values_[0][i] / max_bin_count_ * histogram_height;
			painter->fillRect(temp_x, base_y - temp_y, step, temp_y, index_color[i]);
			temp_x += step;
		}
		painter->setPen(Qt::black);
		painter->drawLine(begin_x, base_y, end_x, base_y);
	}
	if ( map_ids[1] != -1 ){
		QPixmap* map = stamp_pixmaps->at(map_ids[1]);
		int x = 10;
		int y = 10 + map->height() + 10;
		painter->drawPixmap(x, y, map->width(), map->height(), *map);
		if ( mode_ == ROI_MODE ){
			painter->drawImage(x, y, *selected_map_);
		}

		int begin_x = 20 + result_pixmap_->width();
		int end_x = begin_x + result_pixmap_->width();
		int base_y = y + result_pixmap_->height();
		int histogram_height = result_pixmap_->height() - 10;
		int step = (end_x - begin_x) / map_histogram_values_[1].size();
		int temp_x = begin_x;
		for ( int i = 0; i < map_histogram_values_[1].size(); ++i ){
			int temp_y = map_histogram_values_[1][i] / max_bin_count_ * histogram_height;
			painter->fillRect(temp_x, base_y - temp_y, step, temp_y, index_color[i]);
			temp_x += step;
		}
		painter->setPen(Qt::black);
		painter->drawLine(begin_x, base_y, end_x, base_y);
	}

	if ( result_pixmap_ != NULL ){
		int begin_x = 10;
		int end_x = 20 + 2 * result_pixmap_->width();
		int center_x = 20 + result_pixmap_->width();
		int base_y = 2 * (10 + result_pixmap_->height()) + 10 + result_pixmap_->height() / 2;
		int histogram_height = result_pixmap_->height() - 10;
		int step = result_pixmap_->width() / (bias_histogram_values_.size() / 2);
		for ( int i = 0; i < bias_histogram_values_.size() / 2; ++i ){
			int temp_y = bias_histogram_values_[i] * histogram_height / 2;
			painter->fillRect(center_x - step * (bin_size - i), base_y, step, temp_y, index_color[index_color.size() - i - 1]);
		}
		for ( int i = bias_histogram_values_.size() / 2; i < bias_histogram_values_.size(); ++i ){
			int temp_y = bias_histogram_values_[i] * histogram_height / 2;
			painter->fillRect(center_x + step * (i - bin_size), base_y - temp_y, step, temp_y, index_color[i - index_color.size()]);
		}
		painter->setPen(Qt::black);
		painter->drawLine(begin_x, base_y, end_x, base_y);
	}

	if ( result_pixmap_!= NULL ){
		int x = 10;
		int y = 10 + 3 * (10 + result_pixmap_->height());
		painter->drawPixmap(x, y, result_pixmap_->width(), result_pixmap_->height(), *result_pixmap_);

		// draw bias histogram
		int begin_x = 20 + result_pixmap_->width();
		int end_x = begin_x + result_pixmap_->width();
		int base_y = y + result_pixmap_->height();
		int histogram_height = result_pixmap_->height() - 10;
		int step = (end_x - begin_x) / result_histogram_values_.size();
		int temp_x = begin_x;
		painter->setPen(Qt::black);
		for ( int i = 0; i < result_histogram_values_.size(); ++i ){
			int temp_y = result_histogram_values_[i] / max_bin_count_ * histogram_height;
			painter->fillRect(temp_x, base_y - temp_y, step, temp_y, index_color[i]);
			temp_x += step;
		}
		painter->drawLine(begin_x, base_y, end_x, base_y);
	}

	delete painter;
}

void StampComparingViewer::OnAddRoiActionTriggered(){
	selected_area_index_.assign(selected_area_index_.size(), false);
	mode_ = ROI_MODE;
}

void StampComparingViewer::OnClearRoiActionTriggered(){
	ClearSelection();
	this->update();
}

void StampComparingViewer::OnSegmentActionTriggered(){
	for ( int k = 0; k < 2; ++k ){
		if ( map_ids[k] != -1 ){
			QImage ori_image = stamp_pixmaps->at(map_ids[k])->toImage();
			Cluster(ori_image, center_pos_[k], belonging_[k]);
			std::vector< unsigned char > pixel_values;
			int w = ori_image.width() * 1.2;
			int h = ori_image.height() * 1.2;
			pixel_values.resize(w * h * 4, 0);
			pixel_values.assign(w * h * 4, 255);
			center_bias_[k].resize(center_pos_[k].size());
			for ( int i = 0; i < center_pos_[k].size() / 2; ++i )
				center_bias_[k][2 * i] = (int)(center_pos_[k][2 * i] * 0.2);
			for ( int i = 0; i < center_pos_[k].size() / 2; ++i )
				center_bias_[k][2 * i + 1] = (int)(center_pos_[k][2 * i + 1] * 0.2);
			if ( mask_map_[k] != NULL ) delete mask_map_[k];
			QImage temp_image(pixel_values.data(), w, h, 4 * w, QImage::Format_ARGB32);
			for ( int i = 0; i < ori_image.width() * ori_image.height(); ++i ){
				int longi_index = i % ori_image.width();
				int lati_index = i / ori_image.width();

				int cur_longi_index = longi_index + center_bias_[k][belonging_[k][i] * 2];
				int cur_lati_index = lati_index + center_bias_[k][belonging_[k][i] * 2 + 1];
				temp_image.setPixel(cur_longi_index, cur_lati_index, ori_image.pixel(longi_index, lati_index));
			}

			bool b = temp_image.save("test2.bmp");
			mask_map_[k] = new QPixmap();
			mask_map_[k]->convertFromImage(temp_image);
		}
	}

	mode_ = SEGMENT_MODE;
}

void StampComparingViewer::ClearSelection(){
	selected_area_index_.assign(selected_area_index_.size(), false);
	map_selected_index_.assign(map_selected_index_.size(), false);
	selected_map_->fill(QColor(0, 0, 0, 0));
}

void StampComparingViewer::Cluster(WrfGridValueMap* map, std::vector< int >& center_pos, std::vector< int >& belonging_index){
	std::vector< byte > pixel_values;
	pixel_values.resize(map->longitude_grid_number * map->latitude_grid_number);
	for (int i = 0; i < pixel_values.size(); ++i ) pixel_values[i] = (byte)(map->values[i] * 255);
	msImageProcessor* processor = new msImageProcessor;
	processor->DefineImage(pixel_values.data(), GRAYSCALE, map->longitude_grid_number, map->latitude_grid_number);
	processor->Segment(1, 5, 300, MED_SPEEDUP);
	int* labels;
	float* modes;
	int* modePointCounts;
	int region_count = processor->GetRegions(&labels, &modes, &modePointCounts);
	belonging_index.resize(map->longitude_grid_number * map->latitude_grid_number);
	for ( int i = 0; i < belonging_index.size(); ++i ) belonging_index[i] = labels[i];
	center_pos.resize(region_count * 2, 0);
	center_pos.assign(center_pos.size(), 0);
	for ( int i = 0; i < map->values.size(); ++i ){
		center_pos[belonging_index[i] * 2] += i % map->longitude_grid_number;
		center_pos[belonging_index[i] * 2 + 1] += i / map->longitude_grid_number;
	}
	for ( int i = 0; i < center_pos.size(); ++i ) center_pos[i] /= modePointCounts[i / 2];

	delete labels;
	delete modes;
	delete modePointCounts;
}

void StampComparingViewer::Cluster(QImage& image, std::vector< int >& center_pos, std::vector< int >& belonging_index){
	std::vector< byte > pixel_values;
	pixel_values.resize(image.width() * image.height() * 3);
	for (int i = 0; i < image.width() * image.height(); ++i ) {
		pixel_values[3 * i] = image.bits()[i * 4 + 1];
		pixel_values[3 * i + 1] = image.bits()[i * 4 + 2];
		pixel_values[3 * i + 2] = image.bits()[i * 4 + 3];
	}
	msImageProcessor* processor = new msImageProcessor;
	processor->DefineImage(pixel_values.data(), COLOR, image.width(), image.height());
	processor->Segment(1, 5, 300, MED_SPEEDUP);
	int* labels;
	float* modes;
	int* modePointCounts;
	int region_count = processor->GetRegions(&labels, &modes, &modePointCounts);
	belonging_index.resize(image.width() * image.height());
	for ( int i = 0; i < belonging_index.size(); ++i ) belonging_index[i] = labels[i];
	center_pos.resize(region_count * 2, 0);
	center_pos.assign(center_pos.size(), 0);
	for ( int i = 0; i < image.width() * image.height(); ++i ){
		center_pos[belonging_index[i] * 2] += i % image.width();
		center_pos[belonging_index[i] * 2 + 1] += i / image.width();
	}
	for ( int i = 0; i < center_pos.size(); ++i ) center_pos[i] /= modePointCounts[i / 2];

	delete labels;
	delete modes;
	delete modePointCounts;
}

void StampComparingViewer::Minus(WrfGridValueMap* map1, WrfGridValueMap* map2, bool is_enhanced, WrfGridValueMap* result_map){
	float min_value = 1e10;
	float max_value = -1e10;
	for ( int i = 0; i < map1->values.size(); ++i ){
		result_map->values[i] = abs(map1->values[i] - map2->values[i]);
		if ( result_map->values[i] > max_value ) max_value = result_map->values[i];
		if ( result_map->values[i] < min_value ) min_value = result_map->values[i];
	}

	if ( is_enhanced ){
		for ( int i = 0; i < result_map->values.size(); ++i )
			result_map->values[i] = (result_map->values[i] - min_value) / (max_value - min_value);
	}

	for ( int i = 0; i < result_map->weight.size(); ++i ) result_map->weight[i] = (map1->weight[i] + map2->weight[i]) / 2;
}

void StampComparingViewer::Plus(WrfGridValueMap* map1, WrfGridValueMap* map2, bool is_enhanced, WrfGridValueMap* result_map){
	float min_value = 1e10;
	float max_value = -1e10;
	for ( int i = 0; i < map1->values.size(); ++i ){
		result_map->values[i] = abs(map1->values[i] + map2->values[i]);
		if ( result_map->values[i] > max_value ) max_value = result_map->values[i];
		if ( result_map->values[i] < min_value ) min_value = result_map->values[i];
	}

	if ( is_enhanced ){
		for ( int i = 0; i < result_map->values.size(); ++i )
			result_map->values[i] = (result_map->values[i] - min_value) / (max_value - min_value);
	}

	for ( int i = 0; i < result_map->weight.size(); ++i ) result_map->weight[i] = (map1->weight[i] + map2->weight[i]) / 2;
}

void StampComparingViewer::mousePressEvent(QMouseEvent *event){
	QPointF mouse_pos_ = event->pos();

	if ( mode_ == ROI_MODE ){
		AddSelection(mouse_pos_);
		this->update();
	}
}

void StampComparingViewer::mouseMoveEvent(QMouseEvent *event){
	QPointF mouse_pos_ = event->pos();

	if ( mode_ == ROI_MODE ){
		AddSelection(mouse_pos_);
		this->update();
	}
}

void StampComparingViewer::mouseReleaseEvent(QMouseEvent *event){
	if ( mode_ == ROI_MODE ){
		UpdateSelection();
		UpdateHistogramValues();
		this->update();
	}
}

void StampComparingViewer::AddSelection(QPointF point){
	int x1 = 10, y1 = 10, x2 = 10 + selected_map_->width(), y2 = 10 + selected_map_->height();

	if ( (point.rx() - x1) * (point.rx() - x2) < 0 && (point.ry() - y1) * (point.ry() - y2) < 0 ){
		int begin_x = point.rx() - selection_radius_ - x1;
		int end_x = point.rx() + selection_radius_ - x1;
		int begin_y = point.ry() - selection_radius_ - y1;
		int end_y = point.ry() + selection_radius_ - y1;

		if ( begin_x < 0 ) begin_x = 0;
		if ( end_x >= selected_map_->width() ) end_x = selected_map_->width() - 1;
		if ( begin_y < 0 ) begin_y = 0;
		if ( end_y >= selected_map_->height() ) end_y = selected_map_->height() - 1;

		for ( int y = begin_y; y <= end_y; ++y )
			for ( int x = begin_x; x <= end_x; ++x ){
				selected_map_->setPixel(x, y, qRgba(255, 0, 0, 255));
				map_selected_index_[y * selected_map_->width() + x] = true;
			}
	}

	x1 = 10;
	y1 = 20 + selected_map_->height();
	x2 = 10 + selected_map_->width();
	y2 = 20 + selected_map_->height() * 2;

	if ( (point.rx() - x1) * (point.rx() - x2) < 0 && (point.ry() - y1) * (point.ry() - y2) < 0 ){
		int begin_x = point.rx() - selection_radius_ - x1;
		int end_x = point.rx() + selection_radius_ - x1;
		int begin_y = point.ry() - selection_radius_ - y1;
		int end_y = point.ry() + selection_radius_ - y1;

		if ( begin_x < 0 ) begin_x = 0;
		if ( end_x >= selected_map_->width() ) end_x = selected_map_->width() - 1;
		if ( begin_y < 0 ) begin_y = 0;
		if ( end_y >= selected_map_->height() ) end_y = selected_map_->height() - 1;

		for ( int y = begin_y; y <= end_y; ++y )
			for ( int x = begin_x; x <= end_x; ++x ){
				selected_map_->setPixel(x, y, qRgba(255, 0, 0, 255));
				map_selected_index_[y * selected_map_->width() + x] = true;
			}
	}
}

void StampComparingViewer::UpdateSelection(){
	for ( int i = 0; i < result_value_map_->latitude_grid_number; ++i )
		for ( int j = 0; j < result_value_map_->longitude_grid_number; ++j ){
			int x = (float)j / result_value_map_->longitude_grid_number * selected_map_->width();
			int y = (float)i / result_value_map_->latitude_grid_number * selected_map_->height();

			if ( map_selected_index_[y * selected_map_->width() + x] ) selected_area_index_[i * result_value_map_->longitude_grid_number + j] = true;
		}
}

void StampComparingViewer::OnMinusActionTriggered(){
	Minus(value_maps->at(map_ids[0]), value_maps->at(map_ids[1]), true, result_value_map_);
	if ( result_pixmap_ != NULL ) delete result_pixmap_;
	int w = (result_value_map_->end_longitude - result_value_map_->start_longitude) * 3;
	int h = (result_value_map_->end_latitude - result_value_map_->start_latitude) * 3;
	result_pixmap_ = WrfStampGenerator::GetInstance()->GenerateStamp(result_value_map_, w, h);
	UpdateResultHistogramValues();

	this->update();
}

void StampComparingViewer::OnPlusActionTriggered(){
	Plus(value_maps->at(map_ids[0]), value_maps->at(map_ids[1]), true, result_value_map_);
	if ( result_pixmap_ != NULL ) delete result_pixmap_;
	int w = (result_value_map_->end_longitude - result_value_map_->start_longitude) * 3;
	int h = (result_value_map_->end_latitude - result_value_map_->start_latitude) * 3;
	result_pixmap_ = WrfStampGenerator::GetInstance()->GenerateStamp(result_value_map_, w, h);
	UpdateResultHistogramValues();

	this->update();
}