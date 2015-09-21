#ifndef GLYPH_TEST_WINDOW_H_
#define GLYPH_TEST_WINDOW_H_

#include <QtGui/QMainWindow>
#include <vector>
#include <map>
#include "wrf_data_common.h"
#include "ui_glyph_test_window.h"

#define WEATHERVIS

class WrfDataManager;
class WrfImageViewer;
class WrfUncertaintyGlyphElement;

struct SiteUnit{
	// longitude and latitude
	float pos[2];
};

struct NodeUnit{
	// longitude and latitude
	float pos[2];
	// mean and variance
	std::vector< std::vector< float > > values;
};

class GlyphTestWindow  : public QMainWindow {
	Q_OBJECT

public:
	GlyphTestWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~GlyphTestWindow();

private:
	Ui::GlyphTestWindow ui_;

	WrfDataManager* data_manager_;
	WrfImageViewer* phier_viewer_;

	std::vector< WrfElementType > variable_vec_;
	std::vector< std::vector< WrfGridValueMap* > > value_maps_;
	MapRange current_range_;
	WrfGridValueMap* background_map_;
	WrfUncertaintyGlyphElement* un_element_;
	WrfUncertaintyGlyphElement* overall_un_element_;

	// candidate sites
	std::vector< SiteUnit > cand_sites_;
	// test nodes
	std::vector< NodeUnit > test_nodes_;
	std::vector< int > result_labels_;

	int current_level_;
	int available_level_number_;
	int expected_level_;

	void InitWidgets();
	void LoadData();
	
	// sample the regular grid if the resolution is too high
	void SystemSampling();
	// sample the scatter data if the number of points is too high
	void RandomSampling();
	// 
	void ConstructDataStructure();
	// execute multi label optimization based on perception
	void ExecPerceptionGraphCut();
	// execute one step normal hierarchical clustering
	void ExecOneStepHier();
	// 
	void UpdateGlyphData();

	void NormalizeVector(std::vector< std::vector< float > >& vec);
	void NormalizeVector(std::vector< float >& vec);

	private slots:
		void OnExecuteSingleOptButtonClicked();
};

#endif