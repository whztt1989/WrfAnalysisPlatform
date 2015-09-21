#include "wrf_event_searching_view.h"
#include <QtGui/QGridLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QSplitter>
#include <QtGui/QLabel>
#include "wrf_similarity_time_line_view.h"
#include "wrf_data_manager.h"
#include "wrf_statistic_solver.h"
#include "wrf_image_viewer.h"
#include "wrf_utility.h"
#include "wrf_subregion_rms_view.h"
#include "wrf_rendering_element_factory.h"
#include "wrf_grid_rms_error_element.h"
#include "wrf_grid_map_element.h"
#include "wrf_image_element.h"
#include "wrf_reforecast_manager.h"
#include "wrf_color_bar_element.h"
#include "wrf_scatter_plot_dialog.h"
#include "wrf_ensemble_manager.h"
#include "scatter_plot.h"
#include "ui_variable_setting_dialog.h"
#include "wrf_slider_dialog.h"

#define USING_DISTANCE

WrfEventSearchingView::WrfEventSearchingView()
    : table_lens_dataset_(NULL), region_rms_dataset_(NULL), voting_mode_(1) {
    data_manager_ = WrfDataManager::GetInstance();
    reforecast_manager_ = WrfReforecastManager::GetInstance();

    this->setMinimumSize(200, 200);

    InitWidgets();
}

WrfEventSearchingView::~WrfEventSearchingView(){

}

void WrfEventSearchingView::InitWidgets(){
	// main tool bar
    main_tool_bar_ = new QToolBar;
	action_apply_selection_ = new QAction(QIcon("./Resources/calibrated_result_t.png"), tr("Apply Selection"), this);
	action_show_event_analyzing_view_ = new QAction(QIcon("./Resources/event_result_t.png"), tr("Event Analyzing"), this);

	action_filtering_boosting_ = new QAction(QIcon("./Resources/filtering.png"), tr("Filtering"), this);
	action_filtering_boosting_->setCheckable(true);
	action_filtering_boosting_->setChecked(false);
	action_voting_boosting_ = new QAction(QIcon("./Resources/boost.png"), tr("Voting"), this);
	action_voting_boosting_->setCheckable(true);
	action_voting_boosting_->setChecked(true);

	QActionGroup* boosting_group = new QActionGroup(this);
	boosting_group->setExclusive(true);
	boosting_group->addAction(action_filtering_boosting_);
	boosting_group->addAction(action_voting_boosting_);

	main_tool_bar_->addAction(action_apply_selection_);
	main_tool_bar_->addAction(action_show_event_analyzing_view_);
	main_tool_bar_->addSeparator();
	action_show_selected_scatter_plot_ = new QAction(QIcon("./Resources/scatter_plot_t.png"), tr("Scatter Plot"), this);
	main_tool_bar_->addAction(action_show_selected_scatter_plot_);
	main_tool_bar_->addSeparator();
	main_tool_bar_->addAction(action_filtering_boosting_);
	main_tool_bar_->addAction(action_voting_boosting_);
	main_tool_bar_->addSeparator();

	// geomap tool bar
	geomap_tool_bar_ = new QToolBar;
	geomap_tool_bar_->setContentsMargins(0, 0, 0, 0);
	geomap_tool_bar_->setOrientation(Qt::Vertical);
	action_grid_brush_selection_ = new QAction(QIcon("./Resources/painter.jpg"), tr("Brush Grid"), this);
	action_grid_brush_selection_->setCheckable(true);
	geomap_tool_bar_->addAction(action_grid_brush_selection_);

    // geomap view
    geomap_view_ = new WrfImageViewer;
    geomap_view_->SetTitle("RMS Difference Glyph Map");
	background_element_ = new WrfImageElement("./Resources/earth.png");
	background_element_->SetName(std::string("Geographical Background"));
    geomap_view_->AddRenderingElement(background_element_);
	map_element_ = new WrfGridMapELement();
	map_element_->SetName(std::string("Geographical Map"));
	geomap_view_->AddRenderingElement(map_element_);
	grid_rms_element_ = new WrfGridRmsErrorElement;
	geomap_view_->AddRenderingElement(grid_rms_element_);
    WrfColorBarElement* rms_bar = dynamic_cast< WrfColorBarElement* >(WrfRenderingElementFactory::GenerateColorBarElement(WRF_RMS));
    rms_bar->SetTitle("RMS");
    geomap_view_->SetColorBarElement(rms_bar);

    QWidget* geomap_widget = new QWidget;
    QHBoxLayout* geomap_layout = new QHBoxLayout;
    geomap_layout->setMargin(0);
    geomap_layout->addWidget(geomap_tool_bar_);
    geomap_layout->addWidget(geomap_view_);
    geomap_widget->setLayout(geomap_layout);

	// region rms view
	region_rms_view_ = new ComparisonLineChart;

	// main function widget
	main_function_widget_ = new QWidget;
	main_function_widget_->setFixedWidth(300);
	main_view_para_widget_.setupUi(main_function_widget_);

	QSplitter* first_row_splitter = new QSplitter(Qt::Horizontal);
	first_row_splitter->addWidget(geomap_widget);
	first_row_splitter->addWidget(region_rms_view_);
	first_row_splitter->addWidget(main_function_widget_);
	QList< int > first_row_widget_size;
	first_row_widget_size.push_back(100);
	first_row_widget_size.push_back(100);
	first_row_widget_size.push_back(100);
	first_row_splitter->setSizes(first_row_widget_size);
	first_row_splitter->setStretchFactor(0, 1);
	first_row_splitter->setStretchFactor(1, 1);
	first_row_splitter->setStretchFactor(2, 1);

	// subregion rms tool bar
	subregion_rms_tool_bar_ = new QToolBar();
	subregion_rms_tool_bar_->setContentsMargins(0, 0, 0, 0);
	subregion_rms_tool_bar_->setOrientation(Qt::Vertical);

	action_subregion_rms_brush_selection_ = new QAction(QIcon("./Resources/painter.jpg"), tr("Brush Subregion RMS"), this);
	action_subregion_rms_brush_selection_->setCheckable(true);
	action_subregion_rms_apply_selection_ = new QAction(QIcon("./Resources/select.png"), tr("Apply Selection"), this);
	subregion_rms_tool_bar_->addAction(action_subregion_rms_brush_selection_);
	subregion_rms_tool_bar_->addAction(action_subregion_rms_apply_selection_);

	// subregion rms view
	subregion_rms_view_ = new WrfSubregionRmsView;

	// subregion function widget
	subregion_function_widget_ = new QWidget;
	subregion_function_widget_->setFixedWidth(300);
	subregion_rms_para_widget_.setupUi(subregion_function_widget_);
	QButtonGroup* view_mode_group = new QButtonGroup;
	view_mode_group->setExclusive(true);
	view_mode_group->addButton(subregion_rms_para_widget_.subregion_rms_radiobutton, 0);
	view_mode_group->addButton(subregion_rms_para_widget_.final_result_radio_button, 1);
	
	QHBoxLayout* subregion_rms_layout = new QHBoxLayout;
	subregion_rms_layout->setMargin(0);
	subregion_rms_layout->addWidget(subregion_rms_tool_bar_);
	subregion_rms_layout->addWidget(subregion_rms_view_);
	subregion_rms_layout->addWidget(subregion_function_widget_);
	QWidget* subregion_rms_widget = new QWidget;
	subregion_rms_widget->setLayout(subregion_rms_layout);

	QSplitter* second_row_splitter = new QSplitter(Qt::Horizontal);
	second_row_splitter->addWidget(subregion_rms_widget);
	second_row_splitter->addWidget(subregion_function_widget_);

	// variable tool bar
	variable_tool_bar_ = new QToolBar;
	variable_tool_bar_->setContentsMargins(0, 0, 0, 0);
	variable_tool_bar_->setOrientation(Qt::Vertical);

	action_view_value_ = new QAction(QIcon("./Resources/barchart.png"), tr("View Value"), this);
	action_view_value_->setCheckable(true);
	action_view_value_->setChecked(false);
	action_view_scale_ = new QAction(QIcon("./Resources/line_chart.png"), tr("View Scale"), this);
	action_view_scale_->setCheckable(true);
	action_view_scale_->setChecked(true);
	QActionGroup* view_button_group = new QActionGroup(this);
	view_button_group->setExclusive(true);
	view_button_group->addAction(action_view_scale_);
	view_button_group->addAction(action_view_value_);
	action_setting_ = new QAction(QIcon("./Resources/setting.png"), tr("Setting"), this);

	variable_tool_bar_->addSeparator();
	variable_tool_bar_->addAction(action_view_value_);
	variable_tool_bar_->addAction(action_view_scale_);
	variable_tool_bar_->addSeparator();
	variable_tool_bar_->addAction(action_setting_);

	// variable widget
	QHBoxLayout* variable_lens_layout = new QHBoxLayout;
	variable_lens_layout->setMargin(0);
    table_lens_tab_widget_ = new QTabWidget;
	table_lens_tab_widget_->setTabPosition(QTabWidget::South);
	variable_lens_layout->addWidget(variable_tool_bar_);
	variable_lens_layout->addWidget(table_lens_tab_widget_);
	QWidget* variable_rms_widget = new QWidget;
	variable_rms_widget->setLayout(variable_lens_layout);

	// variable function widget
	variable_function_widget_ = new QWidget;
	variable_function_widget_->setFixedWidth(300);
	variable_rms_para_widget_.setupUi(variable_function_widget_);

	QSplitter* third_row_splitter = new QSplitter(Qt::Horizontal);
	third_row_splitter->addWidget(variable_rms_widget);
	third_row_splitter->addWidget(variable_function_widget_);

	// main splitter
    QSplitter* main_splitter = new QSplitter(Qt::Vertical);
    main_splitter->addWidget(first_row_splitter);
	main_splitter->addWidget(second_row_splitter);
    main_splitter->addWidget(third_row_splitter);
    QList< int > main_widget_size;
    main_widget_size.push_back(120);
    main_widget_size.push_back(100);
	main_widget_size.push_back(100);
    main_splitter->setSizes(main_widget_size);
    main_splitter->setStretchFactor(0, 1);
    main_splitter->setStretchFactor(1, 1);
	main_splitter->setStretchFactor(2, 1);

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(main_tool_bar_);
    main_layout->addWidget(main_splitter);
    main_layout->setMargin(0);

    this->setLayout(main_layout);

	// main actions
	connect(action_filtering_boosting_, SIGNAL(triggered()), this, SLOT(OnActionBoostingTriggered()));
	connect(action_voting_boosting_, SIGNAL(triggered()), this, SLOT(OnActionBoostingTriggered()));
	connect(action_setting_, SIGNAL(triggered()), this, SLOT(OnActionSettingTriggered()));

	// geomap actions 
	connect(main_view_para_widget_.show_multiple_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnShowMultipleChanged()));
	connect(main_view_para_widget_.max_viewing_slider, SIGNAL(valueChanged(int)), this, SLOT(OnRegionMaxViewingDateChanged()));
	connect(main_view_para_widget_.min_value_slider, SIGNAL(valueChanged(int)), this, SLOT(OnRmsColorMappingChanged()));
	connect(main_view_para_widget_.max_value_slider, SIGNAL(valueChanged(int)), this, SLOT(OnRmsColorMappingChanged()));
	connect(main_view_para_widget_.suggestion_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnShowRegionSuggestionChanged(int)));
	connect(main_view_para_widget_.lower_bound_slider, SIGNAL(valueChanged(int)), this, SLOT(OnRegionRmsSuggestionParaChanged()));

	// region RMS actions

	// subregion RMS actions
	connect(action_subregion_rms_brush_selection_, SIGNAL(triggered(bool)), subregion_rms_view_, SLOT(OnBrushActionTriggered(bool)));
	connect(action_subregion_rms_apply_selection_, SIGNAL(triggered()), this, SLOT(OnSubregionRmsSelectionChanged()));
	connect(subregion_rms_para_widget_.sort_horizontal_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnSortHorizontalChanged()));
	connect(subregion_rms_para_widget_.sort_vertical_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnSortVerticalChanged()));
	connect(subregion_rms_para_widget_.suggestion_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnShowSubregionSuggestionChanged(int)));
	connect(subregion_rms_para_widget_.lower_bound_slider, SIGNAL(valueChanged(int)), this, SLOT(OnSubregionRmsSuggestionLowerBoundChanged()));
	connect(subregion_rms_para_widget_.upper_bound_slider, SIGNAL(valueChanged(int)), this, SLOT(OnSubregionRmsSuggestionUpperBoundChanged()));
	connect(view_mode_group, SIGNAL(buttonClicked(int)), this, SLOT(OnSubregionViewModeChanged(int)));
	connect(subregion_rms_para_widget_.max_viewing_slider, SIGNAL(valueChanged(int)), this, SLOT(OnSubregionMaxViewingDateChanged()));

	// variable RMS actions
	connect(action_view_scale_, SIGNAL(triggered(bool)), this, SLOT(OnActionViewScaleTriggered(bool)));
	connect(action_view_value_, SIGNAL(triggered(bool)), this, SLOT(OnActionViewValueTriggered(bool)));
	connect(variable_rms_para_widget_.suggestion_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnShowVariableSuggstionChanged(int)));


    connect(action_apply_selection_, SIGNAL(triggered()), this, SLOT(OnApplySelection()));
    connect(action_grid_brush_selection_, SIGNAL(triggered()), this, SLOT(OnGeomapSelectionTriggered()));
    connect(action_show_event_analyzing_view_, SIGNAL(triggered()), this, SIGNAL(EventAnalyzingTriggered()));
	connect(action_show_selected_scatter_plot_, SIGNAL(triggered()), this, SLOT(ShowSelectedScatterPlot()));

    connect(geomap_view_, SIGNAL(SelectionFinished(int)), this, SLOT(OnGeomapSelectionChanged()));

    connect(region_rms_view_, SIGNAL(AnalogNumberChanged(int)), this, SLOT(OnRegionRmsSelectionChanged(int)));
    connect(region_rms_view_, SIGNAL(GridSizeChanged(int)), this, SLOT(OnRetrievalGridSizeChanged(int)));

    connect(subregion_rms_view_, SIGNAL(SelectionValueChanged()), this, SLOT(OnSubregionRmsSelectionChanged()));
    connect(subregion_rms_view_, SIGNAL(CurrentRecordChanged(int, int)), this, SLOT(OnSubregionRecordIndexChanged(int, int)));
}

void WrfEventSearchingView::SetRetrievalElements(std::vector< WrfElementType >& elements){
	retrieval_elements_ = elements;
}

void WrfEventSearchingView::SetSelectedGridPointIndex(std::vector< int >& selected_index){
    selected_grid_point_index_ = selected_index;

    this->UpdateViewData();
	map_element_->UpdateMap();
	map_element_->SetViewingRange(retrieval_map_range_);
	background_element_->SetViewingRange(retrieval_map_range_);

    main_view_para_widget_.max_viewing_slider->setRange(200, is_date_selected_[0].size());
	main_view_para_widget_.max_viewing_slider->setValue(is_date_selected_[0].size());
	if ( is_date_selected_[0].size() > 200 ) main_view_para_widget_.max_viewing_slider->setValue(200);
	else main_view_para_widget_.max_viewing_slider->setValue(is_date_selected_[0].size());
	main_view_para_widget_.max_viewing_label->setText(QString("%0").arg(main_view_para_widget_.max_viewing_slider->value()));

	subregion_rms_para_widget_.max_viewing_slider->setRange(50, is_date_selected_[0].size());
	subregion_rms_para_widget_.max_viewing_slider->setValue(is_date_selected_[0].size());
	subregion_rms_para_widget_.max_viewing_label->setText(QString("%0").arg(is_date_selected_[0].size()));

    // update viewing range
    viewing_range_.start_x = 1e10;
    viewing_range_.start_y = 1e10;
    viewing_range_.end_x = -1e10;
    viewing_range_.end_y = -1e10;
    for ( int i = 0; i < selected_index.size(); ++i ){
        float lon = selected_index[i] % retrieval_map_range_.x_grid_number * retrieval_map_range_.x_grid_space + retrieval_map_range_.start_x;
        float lat = selected_index[i] / retrieval_map_range_.x_grid_number * retrieval_map_range_.y_grid_space + retrieval_map_range_.start_y;
        if ( lon < viewing_range_.start_x ) viewing_range_.start_x = lon;
        if ( lon > viewing_range_.end_x ) viewing_range_.end_x = lon;
        if ( lat < viewing_range_.start_y ) viewing_range_.start_y = lat;
        if ( lat > viewing_range_.end_y ) viewing_range_.end_y = lat;
    }
    float left = (viewing_range_.start_x + viewing_range_.end_x) / 2 - (viewing_range_.end_x - viewing_range_.start_x) * 0.75;
    float right = (viewing_range_.start_x + viewing_range_.end_x) / 2 + (viewing_range_.end_x - viewing_range_.start_x) * 0.75;
    float bottom = (viewing_range_.start_y + viewing_range_.end_y) / 2 - (viewing_range_.end_y - viewing_range_.start_y) * 0.75;
    float top = (viewing_range_.start_y + viewing_range_.end_y) / 2 + (viewing_range_.end_y - viewing_range_.start_y) * 0.75;

    viewing_range_.start_x = left;
    viewing_range_.end_x = right;
    viewing_range_.start_y = bottom;
    viewing_range_.end_y = top;

    geomap_view_->SetMapRange(viewing_range_);

    this->UpdateDataAndView();
}

void WrfEventSearchingView::GetDateSimilarity(std::vector< float >& similarity){
    UpdateDateSimilarity();

    similarity = date_similarity_;
}

void WrfEventSearchingView::UpdateViewData(){
    current_retrieval_grid_size_ = reforecast_manager_->retrieval_grid_size();
    current_retrieval_grid_size_index_ = current_retrieval_grid_size_ / 2 - 1;

    selected_retrieval_grid_sizes_.clear();
    for ( int i = 2; i <= 6 || i <= current_retrieval_grid_size_; ++i ) if (i % 2 == 0 ) selected_retrieval_grid_sizes_.push_back(i);
    reforecast_manager_->PreProcessFocusGrid(selected_grid_point_index_, selected_retrieval_grid_sizes_);

    current_selection_index_.resize(selected_grid_point_index_.size());
    for ( int i = 0; i < selected_grid_point_index_.size(); ++i ) current_selection_index_[i] = i;

	reforecast_manager_->GetReferenceDateTime(reference_date_time_);
    region_selected_analog_nums_.resize(selected_retrieval_grid_sizes_.size());
    for ( int i = 0; i < selected_retrieval_grid_sizes_.size(); ++i )
            region_selected_analog_nums_[i] = reference_date_time_.size();

    subregion_selected_analog_nums_.resize(selected_grid_point_index_.size());
    subregion_selected_analog_nums_.assign(subregion_selected_analog_nums_.size(), reference_date_time_.size());
    
    selected_grid_n_paras_.resize(selected_retrieval_grid_sizes_.size());
    for ( int i = 0; i < selected_retrieval_grid_sizes_.size(); ++i ){
		selected_grid_n_paras_[i].resize(selected_grid_point_index_.size());
		selected_grid_n_paras_[i].assign(selected_grid_point_index_.size(), reference_date_time_.size());
	}

    //reforecast_manager_->GetDateSelection(current_retrieval_grid_size_, selected_grid_point_index_, is_date_selected_);
	is_date_selected_.resize(selected_grid_point_index_.size());
	for ( int i = 0; i < selected_grid_point_index_.size(); ++i ){
		is_date_selected_[i].resize(reference_date_time_.size());
		is_date_selected_[i].assign(is_date_selected_[i].size(), true);
	}

	aggregated_is_date_selected_ = is_date_selected_;

    //reforecast_manager_->GetRetrievalElements(retrieval_elements_);

    reforecast_manager_->GetElementRmsValues(current_retrieval_grid_size_, selected_grid_point_index_, date_grid_rms_);

    reforecast_manager_->GetSortedAggregatedRmsValues(current_retrieval_grid_size_, selected_grid_point_index_, sorted_aggregated_rms_, sorted_date_index_);

	reforecast_manager_->GetSelectedClimatologicalDistribution(climate_distribution_);

	aggregated_rms_average_.resize(sorted_aggregated_rms_.size());
	for ( int i = 0; i < aggregated_rms_average_.size(); ++i ){
		aggregated_rms_average_[i] = 0;
		for ( int j = 0; j < sorted_aggregated_rms_[i].size(); ++j )
			aggregated_rms_average_[i] += sorted_aggregated_rms_[i][j];
		aggregated_rms_average_[i] /= sorted_aggregated_rms_[i].size();
	}

    reforecast_manager_->GetNormalizedValues(normalized_values_);
	normalized_values_.assign(normalized_values_.size(), 1);

    WrfDataManager::GetInstance()->GetEnsembleMapRange(retrieval_map_range_);
}

void WrfEventSearchingView::OnRetrievalGridSizeChanged(int index){
	reforecast_manager_->UpdateDateSelection(current_retrieval_grid_size_, selected_grid_point_index_, is_date_selected_);

	// restore values
	selected_grid_n_paras_[current_retrieval_grid_size_index_] = subregion_selected_analog_nums_;

	current_retrieval_grid_size_index_ = index;

	// get the new values
	current_retrieval_grid_size_ = selected_retrieval_grid_sizes_[index];

	subregion_selected_analog_nums_ = selected_grid_n_paras_[index];

	reforecast_manager_->GetDateSelection(current_retrieval_grid_size_, selected_grid_point_index_, is_date_selected_);

	aggregated_is_date_selected_ = is_date_selected_;

	reforecast_manager_->GetElementRmsValues(current_retrieval_grid_size_, selected_grid_point_index_, date_grid_rms_);

	reforecast_manager_->GetSortedAggregatedRmsValues(current_retrieval_grid_size_, selected_grid_point_index_, sorted_aggregated_rms_, sorted_date_index_);

	aggregated_rms_average_.resize(sorted_aggregated_rms_.size());
	for ( int i = 0; i < aggregated_rms_average_.size(); ++i ){
		aggregated_rms_average_[i] = 0;
		for ( int j = 0; j < sorted_aggregated_rms_[i].size(); ++j )
			aggregated_rms_average_[i] += sorted_aggregated_rms_[i][j];
		aggregated_rms_average_[i] /= sorted_aggregated_rms_[i].size();
	}

	this->UpdateDataAndView();
}

void WrfEventSearchingView::UpdateDataAndView(){
	UpdateGeomapView();
	UpdateRegionRmsView();
	UpdateSubregionRmsView();
	UpdateVariableRmsView();
}

void WrfEventSearchingView::UpdateView(){
	geomap_view_->update();
	region_rms_view_->update();
	subregion_rms_view_->update();
	for ( int i = 0; i < data_table_lens_.size(); ++i ) data_table_lens_[i]->update();
}

void WrfEventSearchingView::UpdateGeomapView(){
    // TODO: update the RMS glyph
	std::vector< bool > temp_selection_indicators;
	temp_selection_indicators.resize(selected_grid_point_index_.size(), false);
	for ( int i = 0; i < current_selection_index_.size(); ++i ) temp_selection_indicators[current_selection_index_[i]] = true;

	grid_rms_units_.clear();
	for ( int i = 0; i < selected_grid_point_index_.size(); ++i ){
		RmsUnit unit;
		unit.lon = (selected_grid_point_index_[i] % retrieval_map_range_.x_grid_number + 0.5) * retrieval_map_range_.x_grid_space + retrieval_map_range_.start_x;
		unit.lat = (selected_grid_point_index_[i] / retrieval_map_range_.x_grid_number + 0.5) * retrieval_map_range_.y_grid_space + retrieval_map_range_.start_y;
		unit.grid_point_index = selected_grid_point_index_[i];
		for ( int j = 0; j < is_date_selected_[i].size(); ++j )
			if ( is_date_selected_[i][sorted_date_index_[i][j]] )
				unit.values.push_back(sorted_aggregated_rms_[i][j]);

		grid_rms_units_.push_back(unit);
	}
	grid_rms_element_->SetRmsUnits(grid_rms_units_);
	grid_rms_element_->SetRetrievalMapRange(retrieval_map_range_);
	grid_rms_element_->SetRetrievalSize(current_retrieval_grid_size_);
	grid_rms_element_->SetSelectedIndex(temp_selection_indicators);
	grid_rms_element_->SetMaxSize(retrieval_map_range_.x_grid_space < retrieval_map_range_.y_grid_space? retrieval_map_range_.x_grid_space : retrieval_map_range_.y_grid_space);

    geomap_view_->update();
}

void WrfEventSearchingView::OnGeomapSelectionTriggered(){
	if ( action_grid_brush_selection_->isChecked() )
		geomap_view_->SetTrackingPen(true);
	else {
		geomap_view_->SetTrackingPen(false);

		current_selection_index_.resize(selected_grid_point_index_.size());
		for ( int i = 0; i < selected_grid_point_index_.size(); ++i ) current_selection_index_[i] = i;

		this->UpdateDataAndView();
	}
}

void WrfEventSearchingView::OnGeomapSelectionChanged(){
	std::vector< QPointF > selected_contour;
	geomap_view_->GetSelectionPath(selected_contour);

	std::vector< int > temp_index;
	WrfStatisticSolver::GetSelectedGridIndex(selected_contour, retrieval_map_range_.start_x + 0.5 * retrieval_map_range_.x_grid_space, 
		retrieval_map_range_.start_y + 0.5 * retrieval_map_range_.y_grid_space, retrieval_map_range_.x_grid_space, retrieval_map_range_.y_grid_space, 
		retrieval_map_range_.x_grid_number, retrieval_map_range_.y_grid_number, temp_index);

	current_selection_index_.clear();
	for ( int i = 0; i < selected_grid_point_index_.size(); ++i )
		for ( int j = 0; j < temp_index.size(); ++j )
			if ( temp_index[j] == selected_grid_point_index_[i] ){
				current_selection_index_.push_back(i);
				break;
			}

	if ( current_selection_index_.size() == 0 ) {
		current_selection_index_.resize(selected_grid_point_index_.size());
		for ( int i = 0; i < selected_grid_point_index_.size(); ++i ) current_selection_index_[i] = i;
	}

	this->UpdateDataAndView();
}

void WrfEventSearchingView::UpdateRegionRmsView(){
	if ( region_rms_dataset_ == NULL ) region_rms_dataset_ = new ComparisonLineChartDataset;
	region_rms_dataset_->data_name = QString("Large Region RMS Difference");
	region_rms_dataset_->label_names.clear();
	region_rms_dataset_->label_colors.clear();

	if ( main_view_para_widget_.show_multiple_checkbox->isChecked() ){
		for ( int i = 0; i < selected_retrieval_grid_sizes_.size(); ++i ){
			region_rms_dataset_->label_names.push_back(QString("Size = %0*%0").arg(selected_retrieval_grid_sizes_[i]));
		}
		ColorMappingGenerator::GetInstance()->GetQualitativeColors(selected_retrieval_grid_sizes_.size(), region_rms_dataset_->label_colors);

		region_rms_dataset_->values.clear();
		region_rms_dataset_->values.resize(selected_retrieval_grid_sizes_.size());
		for ( int i = 0; i < selected_retrieval_grid_sizes_.size(); ++i ){
			region_rms_dataset_->values[i].resize(reference_date_time_.size());
			for ( int j = 0; j < reference_date_time_.size(); ++j ){
				float temp_average = 0;

				for ( int k = 0; k < current_selection_index_.size(); ++k ){
					//int temp_date_index = sorted_date_index_[current_selection_index_[k]][j];
					int temp_date_index = j;
					temp_average += reforecast_manager_->GetGridPointRmsValue(selected_grid_point_index_[current_selection_index_[k]], temp_date_index, selected_retrieval_grid_sizes_[i]);
				}
				temp_average /= current_selection_index_.size();

				region_rms_dataset_->values[i][j] = temp_average;
			}
		}

		// update stopping values
		region_rms_dataset_->stopping_values.resize(selected_retrieval_grid_sizes_.size());
		for ( int i = 0; i < selected_retrieval_grid_sizes_.size(); ++i )
			region_rms_dataset_->stopping_values[i] = region_selected_analog_nums_[i];

		region_rms_view_->SetDataset(region_rms_dataset_);
		region_rms_view_->SetSortingIndex(current_retrieval_grid_size_index_);
		region_rms_view_->SetMaximumViewingNum(main_view_para_widget_.max_viewing_slider->value());
	} else {
		region_rms_dataset_->label_names.push_back(QString("Size = %0*%0").arg(selected_retrieval_grid_sizes_[current_retrieval_grid_size_index_]));
		std::vector< QColor > temp_colors;
		ColorMappingGenerator::GetInstance()->GetQualitativeColors(selected_retrieval_grid_sizes_.size(), temp_colors);
		region_rms_dataset_->label_colors.push_back(temp_colors[current_retrieval_grid_size_index_]);

		region_rms_dataset_->values.clear();
		region_rms_dataset_->values.resize(1);
		region_rms_dataset_->values[0].resize(reference_date_time_.size());
		for ( int i = 0; i < reference_date_time_.size(); ++i ){
			float temp_average = 0;

			for ( int k = 0; k < current_selection_index_.size(); ++k ){
				int temp_date_index = i;
				temp_average += reforecast_manager_->GetGridPointRmsValue(selected_grid_point_index_[current_selection_index_[k]], temp_date_index, selected_retrieval_grid_sizes_[current_retrieval_grid_size_index_]);
			}
			temp_average /= current_selection_index_.size();

			region_rms_dataset_->values[0][i] = temp_average;
		}

		// update stopping values
		region_rms_dataset_->stopping_values.clear();
		region_rms_dataset_->stopping_values.push_back(region_selected_analog_nums_[current_retrieval_grid_size_index_]);

		region_rms_view_->SetDataset(region_rms_dataset_);
		region_rms_view_->SetSortingIndex(0);
		region_rms_view_->SetMaximumViewingNum(main_view_para_widget_.max_viewing_slider->value());

		float min_rate = main_view_para_widget_.lower_bound_slider->value() / 100.0;
		region_rms_view_->SetSuggestionRate(min_rate);
	}
}

void WrfEventSearchingView::OnRegionRmsSelectionChanged(int num){
	// recorded the selected analog number
	region_selected_analog_nums_[current_retrieval_grid_size_index_] = num;
	
	UpdateAggregatedSelection();

	if ( subregion_rms_para_widget_.suggestion_checkbox->isChecked() ) OnSubregionRmsSuggestionParaChanged();

	this->UpdateDataAndView();
}

void WrfEventSearchingView::OnShowMultipleChanged(){
	this->UpdateRegionRmsView();
}

void WrfEventSearchingView::OnRegionMaxViewingDateChanged(){
	main_view_para_widget_.max_viewing_label->setText(QString("%0").arg(main_view_para_widget_.max_viewing_slider->value()));

	this->UpdateRegionRmsView();
}

void WrfEventSearchingView::OnShowRegionSuggestionChanged(int state){
	if ( state == Qt::Checked ){
		float min_rate = main_view_para_widget_.lower_bound_slider->value() / 100.0;

		region_rms_view_->SetSuggestionRate(min_rate);
		region_rms_view_->SetSuggestionOn();
	} else {
		region_rms_view_->SetSuggestionOff();
	}
}

void WrfEventSearchingView::OnRegionRmsSuggestionParaChanged(){
	float min_rate = main_view_para_widget_.lower_bound_slider->value() / 100.0;

	main_view_para_widget_.low_cover_label->setText(QString("%0").arg(min_rate));

	if ( main_view_para_widget_.suggestion_checkbox->isChecked() ){
		region_rms_view_->SetSuggestionRate(min_rate);
	}
}

void WrfEventSearchingView::OnSubregionMaxViewingDateChanged(){
	subregion_rms_para_widget_.max_viewing_label->setText(QString("%0").arg(subregion_rms_para_widget_.max_viewing_slider->value()));

	this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::UpdateSubregionRmsView(){
    std::vector< std::vector< float > > temp_sorted_rms;
    std::vector< std::vector< bool > > temp_is_date_selected;
    std::vector< int > temp_selected_n_para;
	std::vector< std::vector< int > > temp_suggestion_values;
	
	subregion_sort_index_.resize(current_selection_index_.size());
	for ( int i = 0; i < current_selection_index_.size(); ++i ) subregion_sort_index_[i] = current_selection_index_[i];

	if ( subregion_rms_para_widget_.sort_horizontal_checkbox->isChecked() ){
		std::vector< float > temp_average;
		temp_average.resize(current_selection_index_.size());
		for ( int i = 0; i < current_selection_index_.size(); ++i ) temp_average[i] = aggregated_rms_average_[current_selection_index_[i]];

		WrfStatisticSolver::Sort(temp_average, subregion_sort_index_);
	}

    temp_is_date_selected.resize(subregion_sort_index_.size());
	temp_sorted_rms.resize(subregion_sort_index_.size());
	temp_selected_n_para.resize(subregion_sort_index_.size());

	if ( subregion_rms_para_widget_.sort_vertical_checkbox->isChecked() ){
		for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
			temp_sorted_rms[i] = sorted_aggregated_rms_[subregion_sort_index_[i]];    
			temp_selected_n_para[i] = subregion_selected_analog_nums_[subregion_sort_index_[i]];

			temp_is_date_selected[i].resize(sorted_aggregated_rms_[subregion_sort_index_[i]].size());
			for ( int j = 0; j < sorted_aggregated_rms_[subregion_sort_index_[i]].size(); ++j ){
				int temp_date_index = sorted_date_index_[subregion_sort_index_[i]][j];
				temp_is_date_selected[i][j] = is_date_selected_[subregion_sort_index_[i]][temp_date_index];
			}
		}
	} else {
		for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
			temp_selected_n_para[i] = subregion_selected_analog_nums_[subregion_sort_index_[i]];
			temp_is_date_selected[i] = is_date_selected_[subregion_sort_index_[i]];

			temp_sorted_rms[i].resize(sorted_aggregated_rms_[subregion_sort_index_[i]].size());    
			for ( int j = 0; j < sorted_aggregated_rms_[subregion_sort_index_[i]].size(); ++j ){
				int temp_date_index = sorted_date_index_[subregion_sort_index_[i]][j];
				temp_sorted_rms[i][j] = sorted_aggregated_rms_[subregion_sort_index_[i]][temp_date_index];
			}
		}
	}
    
	/// TODO: Update the suggestion values
	if ( subregion_rms_para_widget_.suggestion_checkbox->isChecked() ){
		temp_suggestion_values.resize(2);

		temp_suggestion_values[0].resize(subregion_sort_index_.size());
		temp_suggestion_values[1].resize(subregion_sort_index_.size());
		for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
			temp_suggestion_values[0][i] = subregion_rms_suggestion_values_[0][subregion_sort_index_[i]];
			temp_suggestion_values[1][i] = subregion_rms_suggestion_values_[1][subregion_sort_index_[i]];
		}
	}

    subregion_rms_view_->SetData(temp_sorted_rms, temp_is_date_selected, temp_suggestion_values, temp_selected_n_para);
    subregion_rms_view_->SetMaximumViewingNum(subregion_rms_para_widget_.max_viewing_slider->value());

	std::vector< RmsUnit > subregion_rms_units;
	subregion_rms_units.resize(subregion_sort_index_.size());
	for ( int i = 0; i < subregion_sort_index_.size() && subregion_sort_index_[i] < grid_rms_units_.size(); ++i ){
		subregion_rms_units[i] = grid_rms_units_[subregion_sort_index_[i]];
	}
	subregion_rms_view_->SetRmsUnits(subregion_rms_units);
}

void WrfEventSearchingView::OnSortVerticalChanged(){
	this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::OnSortHorizontalChanged(){
	this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::OnShowSubregionSuggestionChanged(int state){
	if ( !subregion_rms_para_widget_.sort_horizontal_checkbox->isChecked() || !subregion_rms_para_widget_.sort_vertical_checkbox->isChecked() ){
		subregion_rms_para_widget_.suggestion_checkbox->setChecked(false);
	}
	
	subregion_rms_suggestion_values_.resize(2);
	this->UpdateSubregionRmsSuggestionValue(subregion_rms_para_widget_.lower_bound_slider->value() / 100.0, subregion_rms_suggestion_values_[0]);

	for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i ){
		float temp_distribution = climate_distribution_[i];
		if ( temp_distribution < 0.75 ) temp_distribution = 0.75;

		int temp_value = 20 + 30 * exp(-0.03 / (abs(1.0 - temp_distribution) + 1e-5));
		if ( subregion_rms_suggestion_values_[0][i] < temp_value ) subregion_rms_suggestion_values_[0][i] = (int)temp_value;
	}

	GaussianSmooth(subregion_rms_suggestion_values_[0]);

	this->UpdateSubregionRmsSuggestionValue(subregion_rms_para_widget_.upper_bound_slider->value() / 100.0, subregion_rms_suggestion_values_[1]);
	float region_threshold = region_rms_view_->GetSelectedRMSValue();
	for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i ){
		int temp_index = 0;
		while ( temp_index < sorted_aggregated_rms_[i].size() && sorted_aggregated_rms_[i][temp_index] < region_threshold ) temp_index++;
		if ( subregion_rms_suggestion_values_[1][i] < temp_index ) subregion_rms_suggestion_values_[1][i] = temp_index;
	}
	GaussianSmooth(subregion_rms_suggestion_values_[1]);

	this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::OnSubregionRmsSuggestionParaChanged(){
	OnSubregionRmsSuggestionLowerBoundChanged();
	OnSubregionRmsSuggestionUpperBoundChanged();

	//this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::OnSubregionRmsSuggestionLowerBoundChanged(){
	subregion_rms_para_widget_.low_cover_label->setText(QString::number(subregion_rms_para_widget_.lower_bound_slider->value() / 100.0, 'g', 3));

	this->UpdateSubregionRmsSuggestionValue(subregion_rms_para_widget_.lower_bound_slider->value() / 100.0, subregion_rms_suggestion_values_[0]);

	for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i ){
		float temp_distribution = climate_distribution_[i];
		if ( temp_distribution < 0.75 ) temp_distribution = 0.75;

		//int temp_value = 20 + 30 * exp(-0.03 / (abs(1.0 - temp_distribution) + 1e-5));
		//if ( subregion_rms_suggestion_values_[0][i] < temp_value ) subregion_rms_suggestion_values_[0][i] = (int)temp_value;
		if ( subregion_rms_suggestion_values_[0][i] < 20 ) subregion_rms_suggestion_values_[0][i] = 20;
	}
	
	GaussianSmooth(subregion_rms_suggestion_values_[0]);

	this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::OnSubregionRmsSuggestionUpperBoundChanged(){
	subregion_rms_para_widget_.high_cover_label->setText(QString::number(subregion_rms_para_widget_.upper_bound_slider->value() / 100.0, 'g', 3));

	this->UpdateSubregionRmsSuggestionValue(subregion_rms_para_widget_.upper_bound_slider->value() / 100.0, subregion_rms_suggestion_values_[1]);
	float region_threshold = region_rms_view_->GetSelectedRMSValue();
	for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i ){
		int temp_index = 0;
		while ( temp_index < sorted_aggregated_rms_[i].size() && sorted_aggregated_rms_[i][temp_index] < region_threshold ) temp_index++;
		if ( subregion_rms_suggestion_values_[1][i] < temp_index ) subregion_rms_suggestion_values_[1][i] = temp_index;
		if ( subregion_rms_suggestion_values_[1][i] < subregion_rms_suggestion_values_[0][i] ) subregion_rms_suggestion_values_[1][i] = subregion_rms_suggestion_values_[0][i];
	}
	GaussianSmooth(subregion_rms_suggestion_values_[1]);

	this->UpdateSubregionRmsView();
}

void WrfEventSearchingView::UpdateSuggestionParaRange(){
	min_rms_value_ = 1e10;
	max_rms_value_ = -1e10;

	for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i ){
		for ( int j = 0; j < sorted_aggregated_rms_[i].size(); ++j ){
			if ( sorted_aggregated_rms_[i][j] > max_rms_value_ ) max_rms_value_ = sorted_aggregated_rms_[i][j];
			if ( sorted_aggregated_rms_[i][j] < min_rms_value_ ) min_rms_value_ = sorted_aggregated_rms_[i][j];
		}
	}
}

void WrfEventSearchingView::UpdateSubregionRmsSuggestionValue(float cover_rate, std::vector< int >& suggestion_value){
	// temp usage T = selected threshold for the region aggregated RMS
	// energy = ( - cover + alpha) * exp(-1 * |f(N) - T||N - NS|);
	// theta = NS * N;
	float epsilon = 1e-5;
	int total_analog_num = region_selected_analog_nums_[current_retrieval_grid_size_index_];
	int cover_num = -1;

	suggestion_value.resize(sorted_aggregated_rms_.size());

	std::vector< int > region_sort_index;
	region_rms_view_->GetSortedIndex(region_sort_index);

	std::vector< float > energy_values;
	energy_values.resize(sorted_aggregated_rms_[0].size(), 0);
	for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i ){
		energy_values.assign(energy_values.size(), 0);

		std::vector< bool > temp_is_selected;
		temp_is_selected.resize(sorted_aggregated_rms_[i].size(), false);
		for ( int j = 0; j < total_analog_num; ++j )
			temp_is_selected[region_sort_index[j]] = true;

		float coverage = 0.0;
		/*float max_mutal_info = -1e10;
		int max_index = -1;*/
		for ( int j = 0; j < sorted_aggregated_rms_[i].size(); ++j ){
			if ( temp_is_selected[sorted_date_index_[i][j]] ) coverage += 1;

			if ( coverage >= cover_rate * total_analog_num ) {
				cover_num = j;
				break;
			}

			/*float p[2][2];
			p[0][0] = 0;
			p[0][1] = 0;
			p[1][0] = 0;
			p[1][1] = 0;
			float sp[2], sq[2];
			sp[0] = 0;
			sp[1] = 0;
			sq[0] = 0;
			sq[1] = 0;

			for ( int k = 0; k <= j; ++k)
				if ( temp_is_selected[sorted_date_index_[i][k]] ) p[1][1] += 1;
				else p[1][0] += 1;
			for ( int k = j + 1; k < sorted_aggregated_rms_[i].size(); ++k )
				if ( temp_is_selected[sorted_date_index_[i][k]] ) p[0][1] += 1;
				else p[0][0] += 1;
			sp[1] = j + 1;
			sp[0] = sorted_aggregated_rms_[i].size() - sp[1];
			sq[1] = total_analog_num;
			sq[0] = sorted_aggregated_rms_[i].size() - total_analog_num;

			for ( int ii = 0; ii < 2; ++ii ){
				sp[ii] /= sorted_aggregated_rms_[ii].size();
				sq[ii] /= sorted_aggregated_rms_[ii].size();
				for ( int jj = 0; jj < 2; ++jj )
					p[ii][jj] /= sorted_aggregated_rms_[ii].size();
			}

			float temp_info = 0;
			for ( int ii = 0; ii < 2; ++ii )
				for ( int jj = 0; jj < 2; ++jj )
					if ( p[ii][jj] != 0 )
						temp_info += p[ii][jj] * log(p[ii][jj] / (sp[ii] * sq[jj]));
			if ( temp_info > max_mutal_info ){
				max_mutal_info = temp_info;
				max_index = j;
			}*/
		}

		float temp_distribution = climate_distribution_[i];
		if ( temp_distribution < 0.75 ) temp_distribution = 0.75;

		suggestion_value[i] = cover_num;
		//suggestion_value[i] = beta * cover_num + (1.0 - beta) * (20 + 30 * exp(-0.03 / (abs(1.0 - temp_distribution) + epsilon)));
		//suggestion_value[i] = max_index + 1;
	}
}

void WrfEventSearchingView::GaussianSmooth(std::vector< int >& suggestion_value){
	std::vector< float > temp_value;
	temp_value.resize(suggestion_value.size());

	std::vector< float > gaussian_kernel;
	int kernel_size = (int)suggestion_value.size() * 0.1;
	gaussian_kernel.resize(2 * kernel_size + 1);
	for ( int i = 0; i <= kernel_size; ++i ){
		float temp_value = 1.0 / (sqrt(2 * 3.14159) * kernel_size * 0.5) * exp(-1 * i * i / (2.0 * kernel_size * kernel_size * 0.25));
		gaussian_kernel[kernel_size + i] = temp_value;
		gaussian_kernel[kernel_size - i] = temp_value;
	}

	for ( int i = 0; i < gaussian_kernel.size(); ++i ) std::cout << gaussian_kernel[i] << "   ";
	std::cout << std::endl;

	for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
		float gaussian_sum = 0.0;
		gaussian_sum += gaussian_kernel[kernel_size];
		temp_value[i] = 0;
		temp_value[i] += gaussian_kernel[kernel_size] * suggestion_value[subregion_sort_index_[i]];
		for ( int j = 1; j <= kernel_size; ++j ){
			if ( i - j >= 0 ) {
				gaussian_sum += gaussian_kernel[kernel_size - j];
				temp_value[i] += gaussian_kernel[kernel_size - j] * suggestion_value[subregion_sort_index_[i - j]];
			}
			if ( i + j < suggestion_value.size() ){
				gaussian_sum += gaussian_kernel[kernel_size + j];
				temp_value[i] += gaussian_kernel[kernel_size + j] * suggestion_value[subregion_sort_index_[i + j]];
			}
		}
		temp_value[i] /= gaussian_sum;
	}

	for ( int i = 0; i < suggestion_value.size(); ++i )
		suggestion_value[subregion_sort_index_[i]] = (int)temp_value[i];
}

void WrfEventSearchingView::OnSubregionRmsSelectionChanged(){
	if ( !subregion_rms_para_widget_.sort_horizontal_checkbox->isChecked() || !subregion_rms_para_widget_.sort_vertical_checkbox->isChecked() ) return;

	subregion_rms_view_->ApplySelection();

    std::vector< int > temp_n_value;
    subregion_rms_view_->GetSelectionValues(temp_n_value);

    for ( int i = 0; i < subregion_sort_index_.size(); ++i )
        subregion_selected_analog_nums_[subregion_sort_index_[i]] = temp_n_value[i];

	UpdateAggregatedSelection();

	if ( region_rms_dataset_ != NULL ){
		std::vector< int > region_sort_index;
		region_rms_view_->GetSortedIndex(region_sort_index);

		int max_view_num = main_view_para_widget_.max_viewing_slider->value();
		int num_per_bin = max_view_num / 40;
		int bin_num = reference_date_time_.size() / num_per_bin;
		
		region_rms_dataset_->distributions.resize(bin_num);
		region_rms_dataset_->distributions.assign(bin_num, 0);

		for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
			for ( int j = 0; j < subregion_selected_analog_nums_[subregion_sort_index_[i]]; ++j ){
				for ( int k = 0; k < region_sort_index.size(); ++k )
					if ( region_sort_index[k] == sorted_date_index_[subregion_sort_index_[i]][j] ){
						int bin_index = k / num_per_bin;
						if ( bin_index >= bin_num ) bin_index = bin_num - 1;
						region_rms_dataset_->distributions[bin_index] += 1.0 / subregion_selected_analog_nums_[subregion_sort_index_[i]];
					}
			}
		}

		float max_bin_value = 0;
		for ( int i = 0; i < region_rms_dataset_->distributions.size(); ++i )
			if ( region_rms_dataset_->distributions[i] > max_bin_value )
				max_bin_value = region_rms_dataset_->distributions[i];
		if ( max_bin_value != 0 ){
			for ( int i = 0; i < region_rms_dataset_->distributions.size(); ++i )
				region_rms_dataset_->distributions[i] /= max_bin_value;
		}
	}
	

    this->UpdateDataAndView();
}

void WrfEventSearchingView::OnSubregionViewModeChanged(int id){
	subregion_rms_view_->SetViewMode(id);
}

void WrfEventSearchingView::UpdateAggregatedSelection(){
	std::vector< int > temp_sorted_index;
	region_rms_view_->GetSortedIndex(temp_sorted_index);

	for ( int i = 0; i < current_selection_index_.size(); ++i ){
		is_date_selected_[current_selection_index_[i]].resize(reference_date_time_.size());
		is_date_selected_[current_selection_index_[i]].assign(is_date_selected_[current_selection_index_[i]].size(), false);
	}

	if ( voting_mode_ == 0 ){
		// update the average part
		for ( int i = 0; i < region_selected_analog_nums_[current_retrieval_grid_size_index_]; ++i )
			for ( int j = 0; j < current_selection_index_.size(); ++j )
				is_date_selected_[current_selection_index_[j]][temp_sorted_index[i]] = true;

		// update the region part
		for ( int i = 0; i < current_selection_index_.size(); ++i )
			for ( int j = subregion_selected_analog_nums_[current_selection_index_[i]]; j < reference_date_time_.size(); ++j ){
				is_date_selected_[current_selection_index_[i]][sorted_date_index_[current_selection_index_[i]][j]] = is_date_selected_[current_selection_index_[i]][sorted_date_index_[current_selection_index_[i]][j]] && false;
			}
	} else {
		// update the average part
		for ( int i = 0; i < region_selected_analog_nums_[current_retrieval_grid_size_index_]; ++i )
			for ( int j = 0; j < current_selection_index_.size(); ++j )
				is_date_selected_[current_selection_index_[j]][temp_sorted_index[i]] = true;

		// update the region part
		for ( int i = 0; i < current_selection_index_.size(); ++i )
			for ( int j = 0; j < subregion_selected_analog_nums_[current_selection_index_[i]]; ++j ){
				is_date_selected_[current_selection_index_[i]][sorted_date_index_[current_selection_index_[i]][j]] = true;
			}
	}

	aggregated_is_date_selected_ = is_date_selected_;
}

void WrfEventSearchingView::UpdateVariableRmsView(){
    if ( data_table_lens_.size() < retrieval_elements_.size() ){
        int temp_size = data_table_lens_.size();
        data_table_lens_.resize(retrieval_elements_.size());
        table_lens_dataset_.resize(retrieval_elements_.size());

        for ( int i = temp_size; i < retrieval_elements_.size(); ++i ){
            data_table_lens_[i] = new TableLens(i);
            table_lens_dataset_[i] = new TableLensDataset;
            table_lens_tab_widget_->addTab(data_table_lens_[i], QString::fromLocal8Bit(enum_element_to_string(retrieval_elements_[i])));

            connect(data_table_lens_[i], SIGNAL(SelectionChanged(int)), this, SLOT(OnVariableRmsSelectionChanged(int)));
            connect(data_table_lens_[i], SIGNAL(CurrentRecordChanged(int, int)), this, SLOT(OnTableLensRecordIndexChanged(int, int)));
        }
        table_lens_tab_widget_->updateGeometry();
    } else if ( data_table_lens_.size() > retrieval_elements_.size() ){
        int temp_size = data_table_lens_.size();
        for ( int i = temp_size - 1; i >= retrieval_elements_.size(); --i ){
            table_lens_tab_widget_->removeTab(i);
            delete data_table_lens_[i];
            data_table_lens_.pop_back();
            delete table_lens_dataset_[i];
            table_lens_dataset_.pop_back();
        }
        table_lens_tab_widget_->updateGeometry();
    }

    for ( int e = 0; e < retrieval_elements_.size(); ++e ){
        // update attribute names
        table_lens_dataset_[e]->data_name = QString("RMS Difference of A Separate Variables in Small Regions");

        // update record data
        table_lens_dataset_[e]->record_values.resize(subregion_sort_index_.size());
        table_lens_dataset_[e]->record_absolute_values.resize(subregion_sort_index_.size());
        table_lens_dataset_[e]->is_record_selected.resize(subregion_sort_index_.size());
		table_lens_dataset_[e]->record_index.resize(subregion_sort_index_.size());
        table_lens_dataset_[e]->scale_values.resize(subregion_sort_index_.size());

        for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
            int grid_point_index = subregion_sort_index_[i];

			table_lens_dataset_[e]->record_values[i].clear();
			table_lens_dataset_[e]->record_index[i].clear();
            for ( int j = 0; j < reference_date_time_.size(); ++j ){
                int date_index = sorted_date_index_[grid_point_index][j];

				if ( !aggregated_is_date_selected_[grid_point_index][date_index] ) continue;

                table_lens_dataset_[e]->record_values[i].push_back(date_grid_rms_[date_index][grid_point_index][e]);
				table_lens_dataset_[e]->record_index[i].push_back(date_index);
            }

			float absolute_element_value = reforecast_manager_->GetCurrentGridAveragesValues(current_retrieval_grid_size_, retrieval_elements_[e], selected_grid_point_index_[grid_point_index]);
			table_lens_dataset_[e]->record_absolute_values[i] = absolute_element_value;
			
			table_lens_dataset_[e]->scale_values[i].resize(table_lens_dataset_[e]->record_values[i].size());
			for ( int j = 0; j < table_lens_dataset_[e]->record_values[i].size(); ++j ){
				table_lens_dataset_[e]->scale_values[i][j] = abs(table_lens_dataset_[e]->record_values[i][j]) / (abs(table_lens_dataset_[e]->record_absolute_values[i]) + normalized_values_[e]);
			}

			table_lens_dataset_[e]->is_record_selected[i].resize(table_lens_dataset_[e]->record_values[i].size());
			table_lens_dataset_[e]->is_record_selected[i].assign(table_lens_dataset_[e]->is_record_selected[i].size(), true);
        }

        // update value ranges
		table_lens_dataset_[e]->value_ranges.resize(2);
		table_lens_dataset_[e]->value_ranges[0] = 0;
		table_lens_dataset_[e]->value_ranges[1] = -1e10;

        // update absolute value ranges
        table_lens_dataset_[e]->absolute_value_ranges.resize(2);
        table_lens_dataset_[e]->absolute_value_ranges[0] = 1e10;
        table_lens_dataset_[e]->absolute_value_ranges[1] = -1e10;

        table_lens_dataset_[e]->scale_value_ranges.resize(2);
        table_lens_dataset_[e]->scale_value_ranges[0] = 0;
        table_lens_dataset_[e]->scale_value_ranges[1] = -1e10;

        for ( int i = 0; i < table_lens_dataset_[e]->record_values.size(); ++i )
            for ( int j = 0; j < table_lens_dataset_[e]->record_values[i].size(); ++j ){
				if ( table_lens_dataset_[e]->record_values[i][j] > table_lens_dataset_[e]->value_ranges[1] )
					table_lens_dataset_[e]->value_ranges[1] = table_lens_dataset_[e]->record_values[i][j];

                if ( table_lens_dataset_[e]->scale_values[i][j] > table_lens_dataset_[e]->scale_value_ranges[1] )
                    table_lens_dataset_[e]->scale_value_ranges[1] = table_lens_dataset_[e]->scale_values[i][j];
            }
		if ( table_lens_dataset_[e]->scale_value_ranges[1] > 2 ) table_lens_dataset_[e]->scale_value_ranges[1] = 2;

		for ( int i = 0; i < table_lens_dataset_[e]->record_absolute_values.size(); ++i ){
			if ( table_lens_dataset_[e]->record_absolute_values[i] < table_lens_dataset_[e]->absolute_value_ranges[0] )
				table_lens_dataset_[e]->absolute_value_ranges[0] = table_lens_dataset_[e]->record_absolute_values[i];
			if ( table_lens_dataset_[e]->record_absolute_values[i] > table_lens_dataset_[e]->absolute_value_ranges[1] )
				table_lens_dataset_[e]->absolute_value_ranges[1] = table_lens_dataset_[e]->record_absolute_values[i];
		}

		if ( retrieval_elements_[e] == WRF_ACCUMULATED_PRECIPITATION || retrieval_elements_[e] == WRF_PRECIPITABLE_WATER ){
			table_lens_dataset_[e]->absolute_value_ranges[0] = 0;
			if ( table_lens_dataset_[e]->absolute_value_ranges[1] > table_lens_dataset_[e]->value_ranges[1] ){
				table_lens_dataset_[e]->value_ranges[1] = table_lens_dataset_[e]->absolute_value_ranges[1];
			} else {
				table_lens_dataset_[e]->absolute_value_ranges[1] = table_lens_dataset_[e]->value_ranges[1];
			}
		}

		UpdateVariableSuggestionScale(table_lens_dataset_[e], e);

        data_table_lens_[e]->SetDataset(table_lens_dataset_[e]);
    }

	std::vector< RmsUnit > table_lens_units;
	table_lens_units.resize(subregion_sort_index_.size());

	for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
		table_lens_units[i] = grid_rms_units_[subregion_sort_index_[i]];
	}
	for ( int i = 0; i < retrieval_elements_.size(); ++i )
		data_table_lens_[i]->SetRmsUnits(table_lens_units);
}

void WrfEventSearchingView::OnVariableRmsSelectionChanged(int lens_index){
	if ( voting_mode_ == 0 ){
		for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
			int grid_point_index = subregion_sort_index_[i];

			for ( int j = 0; j < table_lens_dataset_[0]->record_values[i].size(); ++j ){
				int date_index = table_lens_dataset_[0]->record_index[i][j];
				if ( !aggregated_is_date_selected_[grid_point_index][date_index] ) continue;

				int temp_count = 0;
				for ( int k = 0; k < table_lens_dataset_.size(); ++k ) 
					if ( table_lens_dataset_[k]->is_record_selected[i][j] ) temp_count++;

				if ( temp_count >= table_lens_dataset_.size() / 2 ) 
					is_date_selected_[grid_point_index][date_index] = is_date_selected_[grid_point_index][date_index] && true;
				else
					is_date_selected_[grid_point_index][date_index] = is_date_selected_[grid_point_index][date_index] && false;
			}
		}
	} else {
		std::vector< int > temp_sorted_index;
		region_rms_view_->GetSortedIndex(temp_sorted_index);

		std::vector< std::vector< unsigned char > > voting_result;
		voting_result.resize(is_date_selected_.size());
		for ( int i = 0; i < is_date_selected_.size(); ++i ) voting_result[i].resize(is_date_selected_[i].size(), 0);

		// update the average part
		for ( int i = 0; i < region_selected_analog_nums_[current_retrieval_grid_size_index_]; ++i )
			for ( int j = 0; j < current_selection_index_.size(); ++j )
				voting_result[current_selection_index_[j]][temp_sorted_index[i]] += 1;

		// update the region part
		for ( int i = 0; i < current_selection_index_.size(); ++i )
			for ( int j = 0; j < subregion_selected_analog_nums_[current_selection_index_[i]]; ++j ){
				voting_result[current_selection_index_[i]][sorted_date_index_[current_selection_index_[i]][j]] += 1;
			}

		for ( int i = 0; i < subregion_sort_index_.size(); ++i ){
			int grid_point_index = subregion_sort_index_[i];

			for ( int j = 0; j < table_lens_dataset_[0]->record_values[i].size(); ++j ){
				int date_index = table_lens_dataset_[0]->record_index[i][j];
				if ( !aggregated_is_date_selected_[grid_point_index][date_index] ) continue;

				int temp_count = 0;
				for ( int k = 0; k < table_lens_dataset_.size(); ++k ) 
					if ( table_lens_dataset_[k]->is_record_selected[i][j] ) temp_count++;

				//if ( temp_count >= table_lens_dataset_.size() / 2 ) voting_result[grid_point_index][date_index] += 1;
				if ( temp_count == table_lens_dataset_.size() ) voting_result[grid_point_index][date_index] += 1;
			}
		}

		for ( int i = 0; i < current_selection_index_.size(); ++i ){
			is_date_selected_[current_selection_index_[i]].resize(reference_date_time_.size());
			for ( int j = 0; j < is_date_selected_[current_selection_index_[i]].size(); ++j )
				if ( voting_result[current_selection_index_[i]][j] >= 2 )
					is_date_selected_[current_selection_index_[i]][j] = true;
				else 
					is_date_selected_[current_selection_index_[i]][j] = false;
		}
	}

	this->UpdateSubregionRmsView();
	this->UpdateGeomapView();
}

void WrfEventSearchingView::OnShowVariableSuggstionChanged(int state){
	if ( state == Qt::Checked ){
		for ( int i = 0; i < data_table_lens_.size(); ++i ){
			data_table_lens_[i]->SetSuggestionOn();
		}
	} else {
		for ( int i = 0; i < data_table_lens_.size(); ++i ){
			data_table_lens_[i]->SetSuggestionOff();
		}
	}
}

void WrfEventSearchingView::OnVariableSuggestionparaChanged(){
}

void WrfEventSearchingView::UpdateVariableSuggestionScale(TableLensDataset* data_set, int e){
	std::vector< int > temp_sorted_index;
	region_rms_view_->GetSortedIndex(temp_sorted_index);

	std::vector< std::vector< bool > > region_is_selected, subregion_is_selected;
	region_is_selected = is_date_selected_;
	subregion_is_selected = is_date_selected_;
	for ( int i = 0; i < current_selection_index_.size(); ++i ){
		region_is_selected[current_selection_index_[i]].resize(reference_date_time_.size());
		region_is_selected[current_selection_index_[i]].assign(is_date_selected_[current_selection_index_[i]].size(), false);

		subregion_is_selected[current_selection_index_[i]].resize(reference_date_time_.size());
		subregion_is_selected[current_selection_index_[i]].assign(is_date_selected_[current_selection_index_[i]].size(), false);
	}

	for ( int i = 0; i < region_selected_analog_nums_[current_retrieval_grid_size_index_]; ++i )
		for ( int j = 0; j < current_selection_index_.size(); ++j )
			region_is_selected[current_selection_index_[j]][temp_sorted_index[i]] = true;

	for ( int i = 0; i < current_selection_index_.size(); ++i )
		for ( int j = 0; j < subregion_selected_analog_nums_[current_selection_index_[i]]; ++j ){
			subregion_is_selected[current_selection_index_[i]][sorted_date_index_[current_selection_index_[i]][j]] = true;
		}

	data_set->energy_value.resize(21, 0);
	data_set->energy_value.assign(21, 0);

	for ( int t = 0; t < 21; ++t ){
		float temp_scale = t * data_set->scale_value_ranges[1] / 20;

		float p[2][2];
		p[0][0] = 0;
		p[0][1] = 0;
		p[1][0] = 0;
		p[1][1] = 0;
		float sp[2], sq[2];
		sp[0] = 0;
		sp[1] = 0;
		sq[0] = 0;
		sq[1] = 0;

		for ( int i = 0; i < current_selection_index_.size(); ++i ){
			int grid_point_index = current_selection_index_[i];
			float absolute_element_value = reforecast_manager_->GetCurrentGridAveragesValues(current_retrieval_grid_size_, retrieval_elements_[e], selected_grid_point_index_[grid_point_index]);

			for ( int j = 0; j < reference_date_time_.size(); ++j ){
				int date_index = sorted_date_index_[grid_point_index][j];

				float element_rms_value = date_grid_rms_[date_index][grid_point_index][e];
				float scale_value = abs(element_rms_value) / (abs(absolute_element_value) + normalized_values_[e]);

#ifdef USING_DISTANCE
				if ( (subregion_is_selected[grid_point_index][date_index] && scale_value >= temp_scale )
					|| (!subregion_is_selected[grid_point_index][date_index] && scale_value < temp_scale) )
					sp[0] += 1;

				if ( (region_is_selected[grid_point_index][date_index] && scale_value >= temp_scale )
					|| (!region_is_selected[grid_point_index][date_index] && scale_value < temp_scale) )
					sp[0] += 1;
#else
				if ( subregion_is_selected[grid_point_index][date_index] && scale_value >= temp_scale )
					p[1][0] += 1;
				else if ( subregion_is_selected[grid_point_index][date_index] && scale_value < temp_scale )
					p[1][1] += 1;
				else if (!subregion_is_selected[grid_point_index][date_index] && scale_value >= temp_scale )
					p[0][0] += 1;
				else
					p[0][1] += 1;

				if ( subregion_is_selected[grid_point_index][date_index] )
					sp[1] += 1;
				else
					sp[0] += 1;
				if ( scale_value >= temp_scale )
					sq[0] += 1;
				else
					sq[1] += 1;
#endif
			}
		}
#ifdef USING_DISTANCE
		data_set->energy_value[t] = sp[0];
#else
		int total_count = reference_date_time_.size() * current_selection_index_.size();
		for ( int i = 0; i < 2; ++i ){
			sp[i] /= total_count;
			sq[i] /= total_count;
			for ( int j = 0; j < 2; ++j )
				p[i][j] /= total_count;
		}

		data_set->energy_value[t] = 0;
		for ( int i = 0; i < 2; ++i )
			for ( int j = 0; j < 2; ++j )
				if ( p[i][j] != 0 ) data_set->energy_value[t] += p[i][j] * log(p[i][j] / (sp[i] * sq[j]));
#endif
	}

	for ( int i = 0; i < data_set->energy_value.size(); ++i ) data_set->energy_value[i] = abs(data_set->energy_value[i]);
	float min_value = 1e10, max_value = -1e10;
	for ( int i = 0; i < data_set->energy_value.size(); ++i ){
		if ( data_set->energy_value[i] > max_value ) max_value = data_set->energy_value[i];
		if ( data_set->energy_value[i] < min_value ) min_value = data_set->energy_value[i];
	}
	for ( int i = 0; i < data_set->energy_value.size(); ++i ){
		data_set->energy_value[i] = data_set->energy_value[i] - min_value;
	}
	for ( int i = 0; i < data_set->energy_value.size(); ++i ) 
		data_set->energy_value[i] = log(data_set->energy_value[i] + 2.0);

	min_value = 1e10;
	max_value = -1e10;
	for ( int i = 0; i < data_set->energy_value.size(); ++i ){
		if ( data_set->energy_value[i] > max_value ) max_value = data_set->energy_value[i];
		if ( data_set->energy_value[i] < min_value ) min_value = data_set->energy_value[i];
	}
	for ( int i = 0; i < data_set->energy_value.size(); ++i ){
		data_set->energy_value[i] = (data_set->energy_value[i] - min_value) / (max_value - min_value) * 0.9 + 0.1;
	}

	for ( int i = 0; i < data_set->energy_value.size(); ++i )
		std::cout << data_set->energy_value[i] << "  ";
	std::cout << std::endl;
}

void WrfEventSearchingView::OnTableLensRecordIndexChanged(int grid_index, int record_index){
	if ( record_index != -1 && grid_index != -1 ){
		int grid_point_index = subregion_sort_index_[grid_index];

		int date_index = table_lens_dataset_[0]->record_index[grid_index][record_index];
		int temp_index = -1;
		for ( int i = 0; i < sorted_date_index_[grid_point_index].size(); ++i )
			if ( sorted_date_index_[grid_point_index][i] == date_index ){
				temp_index = i;
				break;
			}
		if ( temp_index != - 1 )
			subregion_rms_view_->SetHighlightRecordIndex(grid_index, temp_index);

		grid_rms_element_->SetIndicatorIndex(subregion_sort_index_[grid_index]);
	} else if ( grid_index != -1 && record_index == -1 ){
		for ( int i = 0; i < data_table_lens_.size(); ++i )
			data_table_lens_[i]->SetHighlightRecordIndex(grid_index, -1);

		grid_rms_element_->SetIndicatorIndex(subregion_sort_index_[grid_index]);
	}

	geomap_view_->update();
}

void WrfEventSearchingView::OnSubregionRecordIndexChanged(int grid_index, int record_index){
	if ( record_index != -1 && grid_index != -1 ){
		int grid_point_index = subregion_sort_index_[grid_index];

		int date_index = sorted_date_index_[grid_point_index][record_index];

		if ( !aggregated_is_date_selected_[grid_point_index][date_index] ) {
			for ( int i = 0; i < data_table_lens_.size(); ++i )
				data_table_lens_[i]->SetHighlightRecordIndex(grid_index, -1);
		} else {
			int temp_index = -1;
			for ( int i = 0; i < table_lens_dataset_[0]->record_index[grid_index].size(); ++i )
				if ( table_lens_dataset_[0]->record_index[grid_index][i] == date_index ){
					temp_index = i;
					break;
				}

			if ( temp_index != -1 ){
				for ( int i = 0; i < data_table_lens_.size(); ++i )
					data_table_lens_[i]->SetHighlightRecordIndex(grid_index, temp_index);
			}
		}

		grid_rms_element_->SetIndicatorIndex(subregion_sort_index_[grid_index]);
	} else if ( grid_index != -1 && record_index == -1 ){
		for ( int i = 0; i < data_table_lens_.size(); ++i )
			data_table_lens_[i]->SetHighlightRecordIndex(grid_index, -1);

		grid_rms_element_->SetIndicatorIndex(subregion_sort_index_[grid_index]);
	}

	geomap_view_->update();
}

void WrfEventSearchingView::UpdateDateSimilarity(){
    date_similarity_.resize(reference_date_time_.size());
    date_similarity_.assign(reference_date_time_.size(), 0);

    float max_value = 0;
    for ( int i = 0; i < sorted_aggregated_rms_.size(); ++i )
        for ( int j = 0; j < sorted_aggregated_rms_[i].size(); ++j ){
			int date_index = sorted_date_index_[i][j];
            if ( is_date_selected_[i][date_index] ) {
                date_similarity_[date_index] += 1;
                if ( date_similarity_[date_index] > max_value )
                    max_value = date_similarity_[date_index];
            }
		}
    if ( max_value > 0 ){
        for ( int i = 0; i < date_similarity_.size(); ++i ) date_similarity_[i] /= max_value;
    }
}
void WrfEventSearchingView::OnApplySelection(){
    reforecast_manager_->UpdateDateSelection(current_retrieval_grid_size_, selected_grid_point_index_, is_date_selected_);

    emit FilteringApplied();
}

void WrfEventSearchingView::ShowSelectedScatterPlot(){
	WrfScatterPlotDialog scatter_dialog;
	if ( scatter_dialog.exec() == QDialog::Accepted ){
		WrfModelType x_model, y_model;
		WrfElementType x_element, y_element;
		int x_ens, y_ens;
		float x_min = 1e10, x_max = -1e10, y_min = 1e10, y_max = -1e10;

		scatter_dialog.GetSelectedParameters(x_model, x_element, x_ens, x_min, x_max, y_model, y_element, y_ens, y_min, y_max);

		int fhour = WrfEnsembleManager::GetInstance()->FcstHour();
		QDateTime temp_date = WrfEnsembleManager::GetInstance()->CurrrentDate();
		int datetime = temp_date.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;

		std::vector< float* > x_data, y_data;
		WrfDataManager::GetInstance()->GetGridData(datetime, x_model, x_element, fhour, x_data);
		WrfDataManager::GetInstance()->GetGridData(datetime, y_model, y_element, fhour, y_data);

		ScatterPlot* plot = new ScatterPlot;
		plot->SetAxisNames(std::string(enum_element_to_string(x_element)), std::string(enum_element_to_string(y_element)));

		std::vector< float > plot_values;

		for ( int i = 0; i < current_selection_index_.size(); ++i ){
			int grid_index = selected_grid_point_index_[current_selection_index_[i]];
			float temp_x = 0;
			if ( x_ens == -1 ){
				for ( int j = 0; j < x_data.size(); ++j )
					temp_x += *(x_data[j] + grid_index);
				temp_x /= x_data.size();
			} else {
				temp_x = *(x_data[x_ens] + grid_index);
			}
			plot_values.push_back(temp_x);
			float temp_y = 0;
			if ( y_ens == -1 ){
				for ( int j = 0; j < y_data.size(); ++j )
					temp_y += *(y_data[j] + grid_index);
				temp_y /= y_data.size();
			} else {
				temp_y = *(y_data[y_ens] + grid_index);
			}
			plot_values.push_back(temp_y);
		}
		plot->SetData(plot_values);
		
		plot->SetAxisValueRange(x_min, x_max, y_min, y_max);
		plot->show();
	}
}


void WrfEventSearchingView::OnActionViewValueTriggered(bool checked){
	if ( checked ){
		for ( int i = 0; i < data_table_lens_.size(); ++i )
			data_table_lens_[i]->SetViewMode(0);
	}
}

void WrfEventSearchingView::OnActionViewScaleTriggered(bool checked){
	if ( checked ){
		for ( int i = 0; i < data_table_lens_.size(); ++i )
			data_table_lens_[i]->SetViewMode(1);
	}
}

void WrfEventSearchingView::OnActionSettingTriggered(){
	Ui::VariableDialog dialog_ui;
	QDialog* dialog = new QDialog;
	dialog_ui.setupUi(dialog);

	QGridLayout* dialog_layout = new QGridLayout;
	std::vector< QDoubleSpinBox* > spin_boxes;
	for ( int i = 0; i < retrieval_elements_.size(); ++i ){
		QLabel* label = new QLabel(tr(enum_element_to_string(retrieval_elements_[i])));
		QDoubleSpinBox* spin_box = new QDoubleSpinBox;
		dialog_layout->addWidget(label, i, 0, 1, 1);
		dialog_layout->addWidget(spin_box, i, 1, 1, 1);

		spin_box->setValue(normalized_values_[i]);
		spin_boxes.push_back(spin_box);
	}
	dialog_ui.main_widget->setLayout(dialog_layout);

	if ( dialog->exec() == QDialog::Accepted ){
		for ( int i = 0; i < retrieval_elements_.size(); ++i )
			normalized_values_[i] = spin_boxes[i]->value();
		this->UpdateVariableRmsView();
	}

	delete dialog;
}

void WrfEventSearchingView::OnActionBoostingTriggered(){
	/*if ( action_filtering_boosting_->isChecked() ){
		if ( voting_mode_ != 0 ){
			voting_mode_ = 0;

			UpdateSelection();

			UpdateSubregoinRmsView();
			UpdateGeomapView();
			UpdateVariableRmsView();
		}
	} else {
		if ( boosting_mode_ != 1){
			boosting_mode_ = 1;

			UpdateSelection();

			UpdateSubregoinRmsView();
			UpdateGeomapView();
			UpdateVariableRmsView();
		}
	}*/
}

void WrfEventSearchingView::OnRmsColorMappingChanged(){
	float min_thresh = main_view_para_widget_.min_value_slider->value();
	main_view_para_widget_.min_value_label->setText(QString("%0").arg(min_thresh));
	float max_thresh = main_view_para_widget_.max_value_slider->value();
	main_view_para_widget_.max_value_label->setText(QString("%0").arg(max_thresh));

	ColorMappingGenerator::GetInstance()->SetRmsMapping(min_thresh, max_thresh);
	this->UpdateView();
}