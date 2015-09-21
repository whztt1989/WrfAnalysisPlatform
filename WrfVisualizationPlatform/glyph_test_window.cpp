#include "glyph_test_window.h"
#include <vtkSmartPointer.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkDataObject.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <string>
#include <time.h>
#include "wrf_image_viewer.h"
#include "wrf_data_manager.h"
#include "wrf_rendering_element_factory.h"
#include "wrf_uncertainty_glyph_element.h"
#include "./gco-v3.0/GCoptimization.h"

GlyphTestWindow::GlyphTestWindow(QWidget *parent /* = 0 */, Qt::WFlags flags /* = 0 */)
	: QMainWindow(parent, flags){
	ui_.setupUi(this);
	this->InitWidgets();

	data_manager_ = WrfDataManager::GetInstance();
	this->LoadData();

	current_level_ =3;
	available_level_number_ = 3;

#ifdef WEATHERVIS
	ConstructDataStructure();
	ExecPerceptionGraphCut();
	UpdateGlyphData();
#endif
}

GlyphTestWindow::~GlyphTestWindow(){
	data_manager_->DeleteInstance();
}

void GlyphTestWindow::InitWidgets(){
	phier_viewer_ = new WrfImageViewer;
	QHBoxLayout* phiertab_layout = new QHBoxLayout;
	phiertab_layout->addWidget(phier_viewer_);
	ui_.phier_view_widget->setLayout(phiertab_layout);

	//connect(ui_.sys_sampling_button, SIGNAL(clicked()), this, SLOT());
}

void GlyphTestWindow::LoadData(){
#ifdef WEATHERVIS
	//data_manager_->LoadDefaultData();
	data_manager_->LoadEnsembleData(WRF_ACCUMULATED_PRECIPITATION, std::string("E:/Data/ens_l/apcp_sfc_latlon_all_20150401_20150430_liaoLeeVC4.nc"));

	// initialize variables
	variable_vec_.push_back(WRF_ACCUMULATED_PRECIPITATION);
	//variable_vec_.push_back(WRF_PRECIPITABLE_WATER);
	value_maps_.resize(variable_vec_.size());

	// update the widget
	QDateTime temp_datetime = QDateTime(QDate(2015, 4, 5), QTime(0, 0));
	for (size_t i = 0; i < variable_vec_.size(); ++i) 
		data_manager_->GetGridValueMap(temp_datetime, WRF_NCEP_ENSEMBLES, variable_vec_[i], 24, value_maps_[i]);

	//std::vector< WrfGridValueMap* > temp_maps;
	//data_manager_->GetGridValueMap(temp_datetime, WRF_NCEP_ENSEMBLE_MEAN, WRF_ACCUMULATED_PRECIPITATION, 24, temp_maps);
	//background_map_ = temp_maps[0];
	background_map_ = value_maps_[0][0];

	phier_viewer_->ClearElement();
	phier_viewer_->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(background_map_));
	un_element_ = new WrfUncertaintyGlyphElement;
	phier_viewer_->AddRenderingElement(un_element_);
	overall_un_element_ = new WrfUncertaintyGlyphElement;
	phier_viewer_->AddRenderingElement(overall_un_element_);
	phier_viewer_->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(background_map_->element_type));
	data_manager_->GetModelMapRange(WRF_NCEP_ENSEMBLES, current_range_);
	phier_viewer_->SetMapRange(current_range_);
	phier_viewer_->SetTitle("NCEP ENSEMBLES");

	// add iso lines
	float iso_values[] = {10, 25, 50};
	qint64 time = temp_datetime.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
	for (int i = 0; i < 3; ++i)
		phier_viewer_->AddIsoLinePlot(0, time, WRF_NCEP_ENSEMBLES, WRF_ACCUMULATED_PRECIPITATION, 24, iso_values[i]);

	connect(ui_.exec_singopt_button, SIGNAL(clicked()), this, SLOT(OnExecuteSingleOptButtonClicked()));
#else
	vtkSmartPointer<vtkGenericDataObjectReader> reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
	reader->SetFileName("E:/Data/japan/Grid/timestep0_0003600.vtk");
	reader->SetReadAllScalars(1);
	reader->SetReadAllVectors(1);
	reader->Update();
	vtkDataObject* data = reader->GetOutput();
	vtkUnstructuredGrid* unstructuredGrid = vtkUnstructuredGrid::SafeDownCast(data);
	vtkPointData* pData = unstructuredGrid->GetPointData();
	int numArray = pData->GetNumberOfArrays();
	vtkIntArray* xarray = vtkIntArray::SafeDownCast(pData->GetArray(0));
	vtkFloatArray* yarray = vtkFloatArray::SafeDownCast(pData->GetArray(1));
	vtkFloatArray* xparray = vtkFloatArray::SafeDownCast(pData->GetArray(2));
	vtkFloatArray* yparray = vtkFloatArray::SafeDownCast(pData->GetArray(3));
	vtkPoints* p = unstructuredGrid->GetPoints();
	std::vector< WrfUncertaintyGlyphElement::UnGlyphUnit > point_glyph;
	point_glyph.resize(p->GetNumberOfPoints());
	for (int i = 0; i < p->GetNumberOfPoints(); ++i){
		point_glyph[i].lat = p->GetPoint(i)[1];
		point_glyph[i].lon = p->GetPoint(i)[0];
		point_glyph[i].radius = 10;
	}

	double bounds[6];
	p->GetBounds(bounds);
	current_range_.start_x = bounds[0];
	current_range_.end_x = bounds[1];
	current_range_.start_y = bounds[2];
	current_range_.end_y = bounds[3];
	un_element_ = new WrfUncertaintyGlyphElement;
	un_element_->SetUncertaintyData(point_glyph);
	phier_viewer_->AddRenderingElement(un_element_);
	phier_viewer_->SetMapRange(current_range_);
#endif
}

void GlyphTestWindow::OnExecuteSingleOptButtonClicked(){
	this->ExecPerceptionGraphCut();
	this->UpdateGlyphData();
	phier_viewer_->update();
}

void GlyphTestWindow::SystemSampling(){

}

void GlyphTestWindow::RandomSampling(){

}

void GlyphTestWindow::ConstructDataStructure(){
	// construct candidate sites and test nodes
	// tips: for small regular grid data, candidate sites and test nodes are all the grid points
	for (int i = 0; i < current_range_.y_grid_number; ++i)
		for (int j = 0; j < current_range_.x_grid_number; ++j){
			int temp_index = i * current_range_.x_grid_number + j;
			// get the mean and variance values
			float lat = current_range_.start_y + i * current_range_.y_grid_space;
			float lon = current_range_.start_x + j * current_range_.x_grid_space;

			SiteUnit temp_site;
			temp_site.pos[0] = lat;
			temp_site.pos[1] = lon;
			this->cand_sites_.push_back(temp_site);

			NodeUnit temp_node;
			temp_node.pos[0] = lat;
			temp_node.pos[1] = lon;
			for (int v = 0; v < variable_vec_.size(); ++v){
				std::vector< float > temp_values;
				temp_values.resize(2, 0);
				for (int k = 0; k < value_maps_.size(); ++k){
					temp_values[0] += value_maps_[v][k]->values[temp_index];
				}
				temp_values[0] /= value_maps_[v].size();
				for (int k = 0; k < value_maps_.size(); ++k){
					temp_values[1] += pow(value_maps_[v][k]->values[temp_index] - temp_values[0], 2);
				}
				temp_values[1] = sqrt(temp_values[1] / value_maps_[v].size());
				temp_node.values.push_back(temp_values);
			}
			test_nodes_.push_back(temp_node);
		}
}

void GlyphTestWindow::ExecOneStepHier(){

}

void GlyphTestWindow::ExecPerceptionGraphCut(){
	float data_proximity_para = ui_.data_proximity_edit->text().toFloat();
	float data_similarity_para = ui_.data_similarity_edit->text().toFloat();
	float data_weight = ui_.data_weight_edit->text().toFloat();
	float smoothness_weight = ui_.smoothness_weight_edit->text().toFloat();
	float label_proximity_para = ui_.label_proximity_edit->text().toFloat();
	float label_similarity_para = ui_.label_similarity_edit->text().toFloat();
	float label_weight = ui_.label_weight_edit->text().toFloat();

	// Step 1: construct class
	int nodeNumber = test_nodes_.size();
	int labelNumber = nodeNumber * this->available_level_number_;
	GCoptimizationGridGraph* gc = new GCoptimizationGridGraph(current_range_.x_grid_number, current_range_.y_grid_number, labelNumber);

	// Step 2: construct data cost, smooth cost and label cost for each label
	std::vector< std::vector< float > > mean_dif;	// data similarity
	std::vector< std::vector< float > > dis_dif;	// data proximity

	for (int i = 0; i < labelNumber; ++i){
		mean_dif.resize(nodeNumber);
		dis_dif.resize(nodeNumber);
		for (int j = 0; j < nodeNumber; ++j){
			mean_dif[j].resize(labelNumber, 0);
			dis_dif[j].resize(labelNumber, 0);
		}
	}

	std::vector< float > labelVar;	// label cost
	labelVar.resize(labelNumber, 0);
	std::vector< float > labelMean;
	labelMean.resize(labelNumber, 0);

	std::vector< std::vector< float > > smooth_cost;
	smooth_cost.resize(labelNumber);
	for (int i = 0; i < labelNumber; ++i) smooth_cost[i].resize(labelNumber);

	for (int i = 0; i < labelNumber; ++i){
		int level = i / nodeNumber + current_level_;
		float radius = pow(2.0, level) / 2 + 0.1;
		int y = i % nodeNumber / current_range_.x_grid_number;
		int x = i % nodeNumber % current_range_.x_grid_number;
		int start_x = x - (int)radius;
		if (start_x < 0) start_x = 0;
		int end_x = x + (int)radius;
		if (end_x >= current_range_.x_grid_number) end_x = current_range_.x_grid_number - 1;
		int start_y = y - (int)radius;
		if (start_y < 0) start_y = 0;
		int end_y = y + (int)radius;
		if (end_y >= current_range_.y_grid_number) end_y = current_range_.y_grid_number - 1;
		double mean = 0, variance = 0;
		int node_count = 0;
		for (int ty = start_y; ty <= end_y; ++ty)
			for (int tx = start_x; tx <= end_x; ++tx)
				if (pow((double)ty - y, 2) + pow((double)tx - x, 2) < radius * radius) {
					mean += test_nodes_[ty * current_range_.x_grid_number + tx].values[0][0];
					node_count++;
				}
		mean /= node_count;

		for (int ty = start_y; ty <= end_y; ++ty)
			for (int tx = start_x; tx <= end_x; ++tx)
				if (pow((double)ty - y, 2) + pow((double)tx - x, 2) < radius * radius) {
					variance += abs(test_nodes_[ty * current_range_.x_grid_number + tx].values[0][0] - mean);
				}
		for (int j = 0; j < nodeNumber; ++j){
			float temp_dis = sqrt(pow(test_nodes_[j].pos[0] - this->cand_sites_[i % nodeNumber].pos[0], 2) + pow(test_nodes_[j].pos[1] - this->cand_sites_[i % nodeNumber].pos[1], 2));
			if (temp_dis < radius) 
				dis_dif[j][i] = temp_dis / radius;
			else {
				dis_dif[j][i] = exp(4 * (temp_dis + 1) / radius);
				if (dis_dif[j][i] > 100) dis_dif[j][i] = 100;
			}
			mean_dif[j][i] = abs(test_nodes_[j].values[0][0] - mean);
		}

		labelMean[i] = mean;
		labelVar[i] = variance / node_count;
	}

	for (int i = 0; i < labelNumber; ++i ){
		for (int j = i; j < labelNumber; ++j){
			if (i == j){
				smooth_cost[i][j] = 0;
				continue;
			}
			int y = i % nodeNumber / current_range_.x_grid_number;
			int x = i % nodeNumber % current_range_.x_grid_number;
			int ty = j % nodeNumber / current_range_.x_grid_number;
			int tx = j % nodeNumber % current_range_.x_grid_number; 
			if (sqrt(pow((double)x - tx, 2) + pow((double)y - ty, 2)) < pow(2.0, current_level_) / 2 + 0.1)
				smooth_cost[i][j] = 1.0;
			else
				smooth_cost[i][j] = 0.5;

			smooth_cost[j][i] = smooth_cost[i][j];
		}
	}

	// normalize label cost and data cost
	NormalizeVector(dis_dif);
	NormalizeVector(mean_dif);
	NormalizeVector(smooth_cost);
	NormalizeVector(labelVar);

	// Step 3: multi-label graph cut
	for (int i = 0; i < nodeNumber; ++i)
		for (int j = 0; j < labelNumber; ++j)
			gc->setDataCost(i, j, (int)((mean_dif[i][j] * data_similarity_para + dis_dif[i][j] * data_proximity_para) * data_weight));
	for (int i = 0; i < labelNumber; ++i)
		for (int j = 0; j < labelNumber; ++j)
			gc->setSmoothCost(i, j, smooth_cost[i][j] * smoothness_weight);
	std::vector< int > intLabelVar;
	intLabelVar.resize(labelNumber);
	for (int i = 0; i < labelNumber; ++i) intLabelVar[i] = (int)((labelVar[i] * label_similarity_para + (i / nodeNumber + 1) * label_proximity_para) * label_weight);
	for (int i = 0; i < labelNumber; ++i)
		gc->setLabelCost(intLabelVar.data());
	// Step 4:
	printf("\nBefore optimization energy is %d",gc->compute_energy());
	gc->expansion(2);
	printf("\nAfter optimization energy is %d",gc->compute_energy());

	result_labels_.resize(nodeNumber);
	for (int i = 0; i < nodeNumber; ++i)
		result_labels_[i] = gc->whatLabel(i);
}

void GlyphTestWindow::UpdateGlyphData(){
	int nodeNumber = test_nodes_.size();
	int labelNumber = nodeNumber * this->available_level_number_;

	srand((unsigned int)time(0));
	std::vector< bool > is_label_exist;
	is_label_exist.resize(labelNumber, false);
	std::vector< WrfUncertaintyGlyphElement::UnGlyphUnit > glyph_units;
	glyph_units.resize(labelNumber);
	for (int i = 0; i < nodeNumber; ++i){
		int label = result_labels_[i];
		if (!is_label_exist[label]){
			glyph_units[label].mean = test_nodes_[i].values[0][0];
			glyph_units[label].node_num = 1;
			glyph_units[label].lon = i % current_range_.x_grid_number * current_range_.x_grid_space + current_range_.start_x;
			glyph_units[label].lat = i / current_range_.x_grid_number * current_range_.y_grid_space + current_range_.start_y;
			glyph_units[label].radius = 0;
			glyph_units[label].var = 0;
			is_label_exist[label] = true;
			glyph_units[label].r = (int)((float)rand() / RAND_MAX * 255);
			glyph_units[label].g = (int)((float)rand() / RAND_MAX * 255);
			glyph_units[label].b = (int)((float)rand() / RAND_MAX * 255);
		} else {
			glyph_units[label].mean += test_nodes_[i].values[0][0];
			glyph_units[label].node_num += 1;
			glyph_units[label].lon += i % current_range_.x_grid_number * current_range_.x_grid_space + current_range_.start_x;
			glyph_units[label].lat += i / current_range_.x_grid_number * current_range_.y_grid_space + current_range_.start_y;
		}
	}
	for (int i = 0; i < labelNumber; ++i)
		if (is_label_exist[i]){
			glyph_units[i].mean /= glyph_units[i].node_num;
			glyph_units[i].lat /= glyph_units[i].node_num;
			glyph_units[i].lon /= glyph_units[i].node_num;
		}
	for (int i = 0; i < nodeNumber; ++i){
		int label = result_labels_[i];
		glyph_units[label].var += pow(test_nodes_[i].values[0][0] - glyph_units[label].mean, 2);
		glyph_units[label].radius += sqrt(pow(test_nodes_[i].pos[0] - glyph_units[label].lat, 2) + pow(test_nodes_[i].pos[1] - glyph_units[label].lon, 2));
	}
	for (int i = 0; i < labelNumber; ++i)
		if (is_label_exist[i]){
			glyph_units[i].var = sqrt((double)(glyph_units[i].var / glyph_units[i].node_num));
			//glyph_units[i].radius /= glyph_units[i].node_num;
			glyph_units[i].radius = sqrt((float)glyph_units[i].node_num) * 0.3;
		}
	std::vector< WrfUncertaintyGlyphElement::UnGlyphUnit > node_glyph;
	for (int i = 0; i < nodeNumber; ++i){
		int label = result_labels_[i];
		WrfUncertaintyGlyphElement::UnGlyphUnit unit;
		unit.lat = test_nodes_[i].pos[0];
		unit.lon = test_nodes_[i].pos[1];
		unit.r = glyph_units[label].r;
		unit.g = glyph_units[label].g;
		unit.b = glyph_units[label].b;
		unit.radius = 0.2;
		node_glyph.push_back(unit);
	}
	un_element_->SetUncertaintyData(node_glyph);

	std::vector< WrfUncertaintyGlyphElement::UnGlyphUnit > cluster_glyph;
	for (int i = 0; i < labelNumber; ++i)
		if (is_label_exist[i]) cluster_glyph.push_back(glyph_units[i]);
	overall_un_element_->SetUncertaintyData(cluster_glyph);
}

void GlyphTestWindow::NormalizeVector(std::vector< std::vector< float > >& vec){
	float minValue = 1e10;
	float maxValue = -1e10;
	for (int i = 0; i < vec.size(); ++i)
		for (int j = 0; j < vec[i].size(); ++j){
			if (vec[i][j] < 0) continue;
			if (minValue > vec[i][j]) minValue = vec[i][j];
			if (maxValue < vec[i][j]) maxValue = vec[i][j];
		}
	for (int i = 0; i < vec.size(); ++i)
		for (int j = 0; j < vec[i].size(); ++j)
			if (vec[i][j] < 0)
				vec[i][j] = 99;
			else
				vec[i][j] = vec[i][j] / maxValue;
}

void GlyphTestWindow::NormalizeVector(std::vector< float >& vec){
	float minValue = 1e10;
	float maxValue = -1e10;
	for (int i = 0; i < vec.size(); ++i){
		if (vec[i] < 0) continue;
		if (minValue > vec[i]) minValue = vec[i];
		if (maxValue < vec[i]) maxValue = vec[i];
	}
	for (int i = 0; i < vec.size(); ++i)
		if (vec[i] < 0)
			vec[i] = 9999;
		else
			vec[i] = vec[i] / maxValue;
}