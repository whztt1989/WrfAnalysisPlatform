#include "wrf_event_comparison_view.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QSplitter>
#include <QtGui/QActionGroup>
#include <time.h>
#include <fstream>
#include "wrf_stamp_viewer.h"
#include "wrf_radar_coordinate_view.h"
#include "wrf_reforecast_manager.h"
#include "wrf_radar_item.h"
#include "wrf_statistic_solver.h"
#include "wrf_utility.h"
#include "wrf_data_manager.h"
#include "wrf_stamp.h"
#include "manual_group_table.h"
#include "rank_group_table.h"
#include "wrf_rank_para_dialog.h"
#include "color_mapping_generator.h"
#include "wrf_rendering_element_factory.h"
#include "wrf_probabilistic_forecast_dialog.h"

//#define DEBUG_TEST

WrfEventComparisonView::WrfEventComparisonView(){
    item_radius_[0] = 10;
    item_radius_[1] = 25;

    alpha_ = 2.0;
    iteration_num_ = 100;

    current_stamp_ = NULL;
    parallel_dataset_ = NULL;

    max_analog_number_ = 50;
    default_color_ = QColor(51, 160, 44);

    this->setFocusPolicy(Qt::StrongFocus);

    InitView();
}

WrfEventComparisonView::~WrfEventComparisonView(){

}

void WrfEventComparisonView::InitView(){
    current_stamp_view_ = new WrfStampViewer;
    current_stamp_view_->setFixedHeight(250);
    current_stamp_view_->SetTitle(QString("Current Weather"));

    historical_stamp_view_ = new WrfStampViewer;
    historical_stamp_view_->SetTitle(QString("Historical Analogs"));

    radar_coordinate_view_ = new WrfRadarCoordinateView;
    parallel_coordinate_ = new ParallelCoordinate;
    parallel_coordinate_->setFixedHeight(300);

    event_para_widget_ = new QWidget;
    event_para_ui_.setupUi(event_para_widget_);
    event_para_widget_->setFixedHeight(200);

    group_tab_widget_ = new QTabWidget;
    group_tab_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget* rank_main_widget = new QWidget;
    QToolBar* rank_tool_bar = new QToolBar;
    action_add_rank_group_ = new QAction(QIcon("./Resources/plus.png"), tr("Add new group"), this);
    rank_tool_bar->addAction(action_add_rank_group_);
    rank_group_widget_ = new RankGroupTable;
    rank_group_widget_->setFixedWidth(350);
    QVBoxLayout* rank_layout = new QVBoxLayout;
    rank_layout->setAlignment(Qt::AlignTop);
    rank_layout->addWidget(rank_tool_bar);
    rank_layout->addWidget(rank_group_widget_);
    rank_main_widget->setLayout(rank_layout);
    rank_main_widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    group_tab_widget_->addTab(rank_main_widget, QString("Rank Group"));
    group_tab_widget_->setTabPosition(QTabWidget::South);

    QWidget* manual_main_widget = new QWidget;
    QToolBar* manual_tool_bar = new QToolBar;
    action_add_manual_group_ = new QAction(QIcon("./Resources/plus.png"), tr("Add new group"), this);
    manual_tool_bar->addAction(action_add_manual_group_);
    manual_group_widget_ = new ManualGroupTable;
    manual_group_widget_->setFixedWidth(350);
    QVBoxLayout* manual_layout = new QVBoxLayout;
    manual_layout->setAlignment(Qt::AlignTop);
    manual_layout->addWidget(manual_tool_bar);
    manual_layout->addWidget(manual_group_widget_);
    manual_main_widget->setLayout(manual_layout);
    manual_main_widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    group_tab_widget_->addTab(manual_main_widget, QString("Manual Group"));

    QWidget* func_widget = new QWidget;
    QVBoxLayout* func_layout = new QVBoxLayout;
    func_layout->setContentsMargins(0, 0, 0, 0);
    func_layout->setAlignment(Qt::AlignTop);
    func_layout->addWidget(current_stamp_view_);
    func_layout->addWidget(event_para_widget_);
    func_layout->addWidget(group_tab_widget_);
    func_widget->setLayout(func_layout);
    func_widget->setContentsMargins(0, 0, 0, 0);
    func_widget->setFixedWidth(400);

    main_tool_bar_ = new QToolBar;
    main_tool_bar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    action_brush_ = new QAction(QIcon("./Resources/painter.jpg"), tr("Manual Selection"), this);
    action_brush_->setCheckable(true);
    action_brush_->setChecked(false);
    action_manual_group_mode_ = new QAction(QIcon("./Resources/hand_cursor.png"), tr("Manual Mode"), this);
    action_manual_group_mode_->setCheckable(true);
    action_manual_group_mode_->setChecked(true);
    action_rank_group_mode_ = new QAction(QIcon("./Resources/barchart.png"), tr("Rank Mode"), this);
    action_rank_group_mode_->setCheckable(true);
    action_rank_group_mode_->setChecked(false);
    action_show_average_result_ = new QAction(QIcon("./Resources/average_t.png"), tr("Show Average"), this);
    action_show_pqpf_ = new QAction(QIcon("./Resources/calibrated_result_t.png"), tr("Show PQPF"), this);
    action_show_precipitation_scatter_plot_ = new QAction(QIcon("./Resources/scatter_plot_t1.png"), tr("Show Precipitation Scatter Plot"), this);
    main_tool_bar_->addAction(action_brush_);
    main_tool_bar_->addSeparator();
    main_tool_bar_->addAction(action_manual_group_mode_);
    main_tool_bar_->addAction(action_rank_group_mode_);
    main_tool_bar_->addSeparator();
    main_tool_bar_->addAction(action_show_pqpf_);
    main_tool_bar_->addAction(action_show_average_result_);
    main_tool_bar_->addAction(action_show_precipitation_scatter_plot_);
    QActionGroup* color_mode_action_group = new QActionGroup(this);
    color_mode_action_group->addAction(action_manual_group_mode_);
    color_mode_action_group->addAction(action_rank_group_mode_);

    average_viewer_ = new WrfImageViewer;
    average_viewer_->setMinimumSize(300, 400);
    pqpf_viewer_ = new WrfImageViewer;
    pqpf_viewer_->setMinimumSize(300, 400);
    scatter_plot_ = new ScatterPlot;


    QSplitter* hor_splitter = new QSplitter(Qt::Horizontal);
    hor_splitter->addWidget(historical_stamp_view_);
    hor_splitter->addWidget(radar_coordinate_view_);
    QSplitter* ver_splitter = new QSplitter(Qt::Vertical);
    ver_splitter->addWidget(hor_splitter);
    ver_splitter->addWidget(parallel_coordinate_);
    ver_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* ver_layout = new QVBoxLayout;
    ver_layout->setContentsMargins(0, 0, 0, 0);
    ver_layout->addWidget(main_tool_bar_);
    ver_layout->addWidget(ver_splitter);

    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->setAlignment(Qt::AlignLeft);
    main_layout->addWidget(func_widget);
    main_layout->addLayout(ver_layout);

    this->setLayout(main_layout);

    connect(event_para_ui_.min_radius_slider, SIGNAL(sliderReleased()), this, SLOT(OnItemRadiusChanged()));
    connect(event_para_ui_.max_radius_slider, SIGNAL(sliderReleased()), this, SLOT(OnItemRadiusChanged()));
    connect(event_para_ui_.iteration_slider, SIGNAL(sliderReleased()), this, SLOT(OnIterationNumberChanged()));
    connect(event_para_ui_.alpha_slider, SIGNAL(sliderReleased()), this, SLOT(OnAlphaValueChanged()));
    connect(event_para_ui_.max_rank_slider, SIGNAL(sliderReleased()), this, SLOT(OnMaxRankChanged()));
    connect(event_para_ui_.min_radius_slider, SIGNAL(valueChanged(int)), this, SLOT(OnItemRadiusSliderChanged()));
    connect(event_para_ui_.max_radius_slider, SIGNAL(valueChanged(int)), this, SLOT(OnItemRadiusSliderChanged()));
    connect(event_para_ui_.iteration_slider, SIGNAL(valueChanged(int)), this, SLOT(OnIterationNumberSliderChanged()));
    connect(event_para_ui_.alpha_slider, SIGNAL(valueChanged(int)), this, SLOT(OnAlphaValueSliderChanged()));
    connect(event_para_ui_.max_rank_slider, SIGNAL(valueChanged(int)), this, SLOT(OnMaxRankSliderChanged()));

    connect(action_brush_, SIGNAL(triggered()), this, SLOT(OnBrushActionTriggered()));
    connect(action_add_manual_group_, SIGNAL(triggered()), this, SLOT(OnAddManualGroupTriggered()));
    connect(action_add_rank_group_, SIGNAL(triggered()), this, SLOT(OnAddRankGroupTriggered()));
    connect(color_mode_action_group, SIGNAL(triggered(QAction*)), this, SLOT(OnDateColorModeChanged()));

    connect(radar_coordinate_view_, SIGNAL(SelectionChanged()), this, SLOT(OnItemSelectionChanged()));

    connect(action_show_average_result_, SIGNAL(triggered()), this, SLOT(OnActionGenerateAverageTriggered()));
    connect(action_show_pqpf_, SIGNAL(triggered()), this, SLOT(OnActionGeneratePqpfTriggered()));
    connect(action_show_precipitation_scatter_plot_, SIGNAL(triggered()), this, SLOT(OnActionShowPrecipitationScatterPlotTriggered()));
}

void WrfEventComparisonView::SetSelectedGridIndex(std::vector< int >& grid_index){
    selected_grid_index_ = grid_index;
}

void WrfEventComparisonView::SetBasicVariables(std::vector< WrfElementType >& variables){
    forecast_element_ = WRF_ACCUMULATED_PRECIPITATION;
    forecasting_date_ = WrfReforecastManager::GetInstance()->forecasting_date();
    fhour_ =  WrfReforecastManager::GetInstance()->fcst_hour();
    WrfReforecastManager::GetInstance()->GetReferenceDateTime(retrieval_date_time_);

    basic_variables_ = variables;
    srand((unsigned int)time(0));
    for ( int i = 0; i < basic_variables_.size(); ++i ){
        AxisData* axis_data = new AxisData;
        axis_data->variables.push_back(basic_variables_[i]);
        axis_data->weights.push_back(1.0);
        axis_data->normalize_values.push_back(1.0);
        axis_data->r = default_color_.red();
        axis_data->g = default_color_.green();
        axis_data->b = default_color_.blue();
        axis_data->name = enum_element_to_string(basic_variables_[i]);
        axis_data->is_axis = true;
        axis_data->analog_number = 0;

#ifndef DEBUG_TEST
        WrfReforecastManager::GetInstance()->PreProcessDataForEvent(axis_data, selected_grid_index_);
#else
        axis_data->rms_values.resize(20);
        axis_data->sort_date_index.resize(20);
        axis_data->date_sort_index.resize(20);

        
        for ( int j = 0; j < 20; ++j ){
            axis_data->rms_values[j] = rand();
            axis_data->sort_date_index[j] = j;
        }
        WrfStatisticSolver::Sort(axis_data->rms_values, axis_data->sort_date_index);
        for ( int j = 0; j < 20; ++j ) axis_data->date_sort_index[axis_data->sort_date_index[j]] = j;
#endif
        std::vector< QColor > color_vec;
        ColorMappingGenerator::GetInstance()->GetQualitativeColors(rank_groups_.size() + 1, color_vec);
        axis_data->r = color_vec[rank_groups_.size()].red();
        axis_data->g = color_vec[rank_groups_.size()].green();
        axis_data->b = color_vec[rank_groups_.size()].blue();

        rank_groups_.push_back(axis_data);
        rank_group_widget_->AddGroupInfo(rank_groups_.size() - 1, axis_data->is_axis, QString::fromLocal8Bit(axis_data->name.c_str()), QColor(axis_data->r, axis_data->g, axis_data->b), axis_data->analog_number);
    }

    UpdateAxisData();

    UpdateCurrentStampView();
}

void WrfEventComparisonView::UpdateAxisData(){
    axis_data_vec_.clear();
    axis_names_.clear();
    for ( int i = 0; i < rank_groups_.size(); ++i )
        if ( rank_groups_[i]->is_axis ){
            axis_data_vec_.push_back(rank_groups_[i]);
            axis_names_.push_back(QString::fromLocal8Bit(rank_groups_[i]->name.c_str()));
        }

        
    axis_order_.resize(axis_data_vec_.size());
    for ( int i = 0; i < axis_data_vec_.size(); ++i ) axis_order_[i] = i;

    UpdateDateSelection();

    if ( axis_order_.size() >= 4 ) UpdateAxisOrder();

    radar_coordinate_view_->SetData(axis_names_, max_analog_number_);

    current_selection_index_.clear();
    for ( int i = 0; i < is_date_selected_.size(); ++i )
        if ( is_date_selected_[i] ) current_selection_index_.push_back(i);
}

void WrfEventComparisonView::UpdateDateSelection(){
    // update date selection
    is_date_selected_.resize(axis_data_vec_[0]->rms_values.size());
    is_date_selected_.assign(is_date_selected_.size(), false);
    for ( int i = 0; i < rank_groups_.size(); ++i ){
        for ( int j = 0; j < rank_groups_[i]->analog_number; ++j )
            is_date_selected_[rank_groups_[i]->sort_date_index[j]] = true;
    }

    selected_date_number_ = 0;
    for ( int i = 0; i < is_date_selected_.size(); ++i ) 
        if ( is_date_selected_[i] ) selected_date_number_++;
}

void WrfEventComparisonView::UpdateAxisOrder(){
    pearson_values_.resize(axis_data_vec_.size());
    for ( int i = 0; i < axis_data_vec_.size(); ++i ) {
        pearson_values_[i].resize(axis_data_vec_.size());
        pearson_values_[i].assign(pearson_values_[i].size(), 0);
    }

    for ( int i = 0; i < axis_data_vec_.size() - 1; ++i )
        for ( int j = i + 1; j < axis_data_vec_.size(); ++j ){
            float mean_x = 0, mean_y = 0;
            int accu_count = 0;
            for ( int k = 0; k < is_date_selected_.size(); ++k )
                if ( is_date_selected_[k] ){
                    mean_x += axis_data_vec_[i]->date_sort_index[k];
                    mean_y += axis_data_vec_[j]->date_sort_index[k];
                    accu_count++;
                }
            mean_x /= accu_count;
            mean_y /= accu_count;

            float exy = 0, ex2 = 0, ey2 = 0;
            for ( int k = 0; k < is_date_selected_.size(); ++k )
                if ( is_date_selected_[k] ){
                    exy += (axis_data_vec_[i]->date_sort_index[k] - mean_x) * (axis_data_vec_[j]->date_sort_index[k] - mean_y);
                    ex2 += pow(axis_data_vec_[i]->date_sort_index[k] - mean_x, 2);
                    ey2 += pow(axis_data_vec_[j]->date_sort_index[k] - mean_y, 2);
                }
            pearson_values_[i][j] = abs(exy / (sqrt(ex2) * sqrt(ey2)));
            pearson_values_[j][i] = pearson_values_[i][j];
        }

    // find the maximum path
    std::vector< bool > is_searched;
    float current_value = 0, max_value = 0;
    std::vector< int > current_order;
    is_searched.resize(axis_data_vec_.size(), false);
    is_searched[0] = true;
    current_order.push_back(0);

    BruteforceSearch(is_searched, current_value, max_value, current_order);

    axis_names_.clear();
    for ( int i = 0; i < axis_order_.size(); ++i )
        axis_names_.push_back(QString::fromLocal8Bit(axis_data_vec_[axis_order_[i]]->name.c_str()));
}

void WrfEventComparisonView::BruteforceSearch(std::vector< bool >& is_searched, float current_value, float& max_value, std::vector< int >& current_order){
    if ( current_order.size() == axis_data_vec_.size() ){
        current_value += pearson_values_[current_order[current_order.size() - 1]][current_order[0]];
        if ( current_value > max_value ){
            axis_order_ = current_order;
            max_value = current_value;
            return;
        }
    }
    for ( int i = 0; i < is_searched.size(); ++i )
        if ( !is_searched[i] ){
            is_searched[i] = true;
            current_value += pearson_values_[current_order[current_order.size() - 1]][i];
            current_order.push_back(i);

            BruteforceSearch(is_searched, current_value, max_value, current_order);

            current_order.pop_back();
            current_value -= pearson_values_[current_order[current_order.size() - 1]][i];
            is_searched[i] = false;
        }
}

void WrfEventComparisonView::UpdateRadarItems(){
    if ( date_items_.size() == 0 ){
        date_items_.resize(is_date_selected_.size(), NULL);
    }

    if ( date_stamps_.size() == 0 ){
        date_stamps_.resize(is_date_selected_.size(), NULL);
        UpdateHistoryStampView();
    }

    if ( date_color_.size() == 0 ){
        date_color_.resize(is_date_selected_.size(), default_color_);
    }

    // generate items and add to the radar coordinate view
    for ( int i = 0; i < is_date_selected_.size(); ++i ){
        if ( is_date_selected_[i] && date_items_[i] == NULL ){
            date_items_[i] = new WrfRadarItem;
            date_items_[i]->SetDateIndex(i);
            radar_coordinate_view_->scene()->addItem(date_items_[i]);
        } else if ( !is_date_selected_[i] && date_items_[i] != NULL ){
            radar_coordinate_view_->scene()->removeItem(date_items_[i]);
            delete date_items_[i];
            date_items_[i] = NULL;
        }

        if ( is_date_selected_[i] && date_stamps_[i] == NULL ){
            date_stamps_[i] = GenerateStamp(retrieval_date_time_[i]);

            if ( date_stamps_[i] != NULL ) historical_stamp_view_->scene()->addItem(date_stamps_[i]);
        }

        if ( is_date_selected_[i] ){
            date_items_[i]->axis_rank.resize(axis_names_.size());
            date_items_[i]->axis_pos.resize(axis_names_.size());
            int min_rank = max_analog_number_;
            for ( int j = 0; j < axis_names_.size(); ++j ){
                date_items_[i]->axis_rank[j] = axis_data_vec_[axis_order_[j]]->date_sort_index[i];
                if ( date_items_[i]->axis_rank[j] > max_analog_number_ ){
                    date_items_[i]->axis_pos[j] = radar_coordinate_view_->center_point + radar_coordinate_view_->axis_length * radar_coordinate_view_->axis_direction_vec[j] * (0.1 + 0.2 * exp(((float)max_analog_number_ - date_items_[i]->axis_rank[j])) / 100);
                } else {
                    date_items_[i]->axis_pos[j] = radar_coordinate_view_->center_point + radar_coordinate_view_->axis_length * radar_coordinate_view_->axis_direction_vec[j] * (0.3 + 0.7 * (1.0 - (float)date_items_[i]->axis_rank[j] / max_analog_number_));
                }

                if ( date_items_[i]->axis_rank[j] < min_rank ) min_rank = date_items_[i]->axis_rank[j];
            }

            date_items_[i]->SetData(axis_names_, max_analog_number_, date_color_[i]);
            date_items_[i]->SetRadius(item_radius_[0] + (item_radius_[1] - item_radius_[0]) * (max_analog_number_ - min_rank) / max_analog_number_);
            date_items_[i]->UpdateItem();

            date_stamps_[i]->SetData(axis_names_, max_analog_number_, date_color_[i]);
            date_stamps_[i]->axis_rank = date_items_[i]->axis_rank;
            date_stamps_[i]->UpdateItem();
        }
    }

    std::vector< bool > temp_is_selected;
    temp_is_selected.resize(is_date_selected_.size(), false);
    for ( int i = 0; i < current_selection_index_.size(); ++i ) temp_is_selected[current_selection_index_[i]] = true;
    for ( int i = 0; i < temp_is_selected.size(); ++i )
        if ( date_items_[i] != NULL ) {
            if ( temp_is_selected[i] ) date_items_[i]->setOpacity(1.0);
            else date_items_[i]->setOpacity(0.3);
        }
}

void WrfEventComparisonView::UpdateItemPositions(){
    // force directed method
    if ( selected_date_number_ == 0 ) return;

    std::vector< WrfRadarItem* > radar_items;
    for ( int i = 0; i < date_items_.size(); ++i )
        if ( date_items_[i] != NULL ) radar_items.push_back(date_items_[i]);

    float norm_value = this->width() * this->height() / (selected_date_number_ * 10);

    std::vector< QPointF > translations;
    translations.resize(radar_items.size());

    for ( int i = 0; i < iteration_num_; ++i ){
        translations.assign(translations.size(), QPointF(0, 0));
        for ( int j = 0; j < radar_items.size(); ++j ){
            for ( int k = 0; k < radar_items[j]->axis_pos.size(); ++k ){
                QPointF bias = radar_items[j]->axis_pos[k] - radar_items[j]->pos();
                //float temp_length = bias.manhattanLength();
                //translations[j] += alpha_ * bias * pow(temp_length, gamma_ - 1) / norm_value; 
                translations[j] += bias * exp(-1 * alpha_ * radar_items[j]->axis_rank[k] / max_analog_number_);
            }

            /*for ( int k = 0; k < radar_items.size(); ++k )
            if ( k != j ){
            QPointF bias = radar_items[j]->pos() - radar_items[k]->pos();
            float temp_length = bias.manhattanLength();
            if ( temp_length < 1e-5 ) temp_length = 1;
            if ( temp_length < radar_items[j]->item_radius + radar_items[k]->item_radius ){
            QPointF temp_translation = beta_ * bias * 8000 / pow(temp_length, theta_ + 1);
            float scale = radar_items[k]->item_radius / (radar_items[k]->item_radius + radar_items[j]->item_radius);
            translations[j] += temp_translation * scale;
            translations[k] += -1 * temp_translation * (1.0 - scale);
            }
            }*/
        }
        
        float cool_t = exp(-1.0 * i / 100);
        for ( int j = 0; j < radar_items.size(); ++j ) {
            QPointF new_pos = radar_items[j]->pos() + translations[j] * cool_t;
            if ( new_pos.rx() > this->radar_coordinate_view_->scene()->width() ) new_pos.setX(this->radar_coordinate_view_->scene()->width());
            if ( new_pos.rx() < 0 ) new_pos.setX(0);
            if ( new_pos.ry() > this->radar_coordinate_view_->scene()->height() ) new_pos.setY(this->radar_coordinate_view_->scene()->height());
            if ( new_pos.ry() < 0 ) new_pos.setY(0);
            radar_items[j]->setPos(new_pos);
        }

        radar_coordinate_view_->scene()->update();
    }
    radar_coordinate_view_->scene()->update();
}

void WrfEventComparisonView::OnRadarItemChanged(){
    UpdateRadarItems();
    UpdateItemPositions();
}

WrfStamp* WrfEventComparisonView::GenerateStamp(int date_time){
    std::vector< WrfGridValueMap* > history_fcst_maps;
    WrfDataManager::GetInstance()->GetGridValueMap(date_time, WRF_NCEP_ENSEMBLES, forecast_element_, fhour_, history_fcst_maps);
    if ( history_fcst_maps.size() == 0 ) 
        WrfDataManager::GetInstance()->GetGridValueMap(date_time, WRF_NCEP_ENSEMBLE_MEAN, forecast_element_, fhour_, history_fcst_maps);
    if ( history_fcst_maps.size() == 0 ) return NULL;

    if ( history_fcst_maps.size() > 1 ){
        for ( int i = 1; i < history_fcst_maps.size(); ++i ){
            for ( int j = 0; j < history_fcst_maps[0]->map_range.x_grid_number * history_fcst_maps[0]->map_range.y_grid_number; ++j )
                history_fcst_maps[0]->values[j] += history_fcst_maps[i]->values[j];
        }
        for ( int i = 0; i < history_fcst_maps[0]->map_range.x_grid_number * history_fcst_maps[0]->map_range.y_grid_number; ++i )
            history_fcst_maps[0]->values[i] /= history_fcst_maps.size();

        for ( int i = 1; i < history_fcst_maps.size(); ++i ) delete history_fcst_maps[i];
        history_fcst_maps.resize(1);
    }

    std::vector< WrfGridValueMap* > reanalysis_map;
    WrfDataManager::GetInstance()->GetGridValueMap(date_time, WRF_REANALYSIS, forecast_element_, fhour_, reanalysis_map);

    QDateTime base_date = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0));
    qint64 temp = date_time;
    temp *= 3600000;
    QDateTime history_date = base_date.addMSecs(temp);

    WrfStamp* stamp = new WrfStamp;
    stamp->SetValueMap(history_fcst_maps[0], reanalysis_map[0], viewing_range_);
    stamp->SetScatterData(selected_grid_index_);
    stamp->SetDateString(QLocale(QLocale::C).toString(history_date, QString("yyyy MMM dd")));
    stamp->SetDateTime(date_time);
    stamp->SetIsScatterOn(true);

    return stamp;
}

void WrfEventComparisonView::UpdateCurrentSelection(){
    const double PI = 3.14159265358979323846;

    std::vector< QPointF > selection_path = radar_coordinate_view_->brush_path;

    std::vector< int > pre_index;
    if ( is_ctrl_pressed_ ) pre_index = current_selection_index_;

    current_selection_index_.clear();

    if ( selection_path.size() > 3 ) {
        for ( int i = 0; i < is_date_selected_.size(); ++i )
            if ( is_date_selected_[i] && date_items_[i] != NULL ) {
                double angle = 0;
                float x = date_items_[i]->pos().x();
                float y = date_items_[i]->pos().y();
                for ( int j = 0, k = selection_path.size() - 1; j < selection_path.size(); k = j++ ){
                    double x1,y1,x2,y2;
                    x1 = selection_path[j].rx() - x;
                    y1 = selection_path[j].ry() - y;
                    x2 = selection_path[k].rx() - x;
                    y2 = selection_path[k].ry() - y;

                    double radian = atan2(y1, x1) - atan2(y2, x2);
                    radian = abs(radian);
                    if (radian > PI) 
                        radian = 2 * PI - radian;
                    angle += radian;            
                }

                if ( fabs(6.28318530717958647692 - abs(angle)) < 1 ) 
                    current_selection_index_.push_back(i);
            }
    }

    if ( is_ctrl_pressed_ ){
        for ( int i = 0; i < pre_index.size(); ++i ){
            bool is_exit = false;
            for ( int j = 0; j < current_selection_index_.size(); ++j )
                if ( current_selection_index_[j] == pre_index[i] ){
                    is_exit = true;
                    break;
                }
            if ( !is_exit ) current_selection_index_.push_back(pre_index[i]);
        }
    }

    if ( current_selection_index_.size() == 0 ){
        for ( int i = 0; i < is_date_selected_.size(); ++i )
            if ( is_date_selected_[i] ) current_selection_index_.push_back(i);

        radar_coordinate_view_->SetIsLineOn(false);
    } else
        radar_coordinate_view_->SetIsLineOn(true);
}

void WrfEventComparisonView::OnItemSelectionChanged(){
    UpdateCurrentSelection();
    
    UpdateHistoryStampView();
    
    UpdateRadarCoordinateView();

    UpdateParallelCoordinate();
}

void WrfEventComparisonView::resizeEvent(QResizeEvent *event){
    UpdateRadarItems();
    UpdateItemPositions();
}

void WrfEventComparisonView::keyPressEvent(QKeyEvent *event){
    if ( event->key() == Qt::Key_Control ){
        is_ctrl_pressed_ = true;
    }
}

void WrfEventComparisonView::keyReleaseEvent(QKeyEvent *event){
    if ( event->key() == Qt::Key_Control ){
        is_ctrl_pressed_ = false;
    }
}

void WrfEventComparisonView::UpdateCurrentStampView(){
    std::vector< WrfGridValueMap* > current_fcst_maps;
    WrfDataManager::GetInstance()->GetGridValueMap(forecasting_date_, WRF_NCEP_ENSEMBLES, forecast_element_, fhour_, current_fcst_maps);
    if ( current_fcst_maps.size() == 0 ) 
        WrfDataManager::GetInstance()->GetGridValueMap(forecasting_date_, WRF_NCEP_ENSEMBLE_MEAN, forecast_element_, fhour_, current_fcst_maps);
    if ( current_fcst_maps.size() == 0 ) return;

    if ( current_fcst_maps.size() > 1 ){
        for ( int i = 1; i < current_fcst_maps.size(); ++i ){
            for ( int j = 0; j < current_fcst_maps[0]->map_range.x_grid_number * current_fcst_maps[0]->map_range.y_grid_number; ++j )
                current_fcst_maps[0]->values[j] += current_fcst_maps[i]->values[j];
        }
        for ( int i = 0; i < current_fcst_maps[0]->map_range.x_grid_number * current_fcst_maps[0]->map_range.y_grid_number; ++i )
            current_fcst_maps[0]->values[i] /= current_fcst_maps.size();

        for ( int i = 1; i < current_fcst_maps.size(); ++i ) delete current_fcst_maps[i];
        current_fcst_maps.resize(1);
    }

    std::vector< WrfGridValueMap* > reanalysis_map;
    WrfDataManager::GetInstance()->GetGridValueMap(forecasting_date_, WRF_REANALYSIS, forecast_element_, fhour_, reanalysis_map);
    if ( current_stamp_ == NULL ) current_stamp_ = new WrfStamp;
    current_stamp_->SetValueMap(current_fcst_maps[0], NULL, viewing_range_);
    current_stamp_->SetDateString(QLocale(QLocale::C).toString(forecasting_date_, QString("yyyy MMM dd")));
    qint64 time = forecasting_date_.msecsTo(QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0))) / 3600000 * -1;
    current_stamp_->SetDateTime(time);

    current_stamp_view_->scene()->addItem(current_stamp_);

    std::vector< WrfStamp* > temp_stamps;
    temp_stamps.push_back(current_stamp_);
    current_stamp_view_->SetStamps(temp_stamps);

    std::vector< int > current_selected_index;
    for ( int i = 0; i < temp_stamps.size(); ++i ) current_selected_index.push_back(i);
    current_stamp_view_->SetSelectedIndex(current_selected_index);
}

void WrfEventComparisonView::UpdateHistoryStampView(){
    historical_stamp_view_->SetStamps(date_stamps_);
    historical_stamp_view_->SetSelectedIndex(current_selection_index_);
}

void WrfEventComparisonView::UpdateParallelCoordinate(){
    if ( parallel_dataset_ == NULL ) parallel_dataset_ = new ParallelDataset;
    parallel_dataset_->ClearData();
    parallel_dataset_->subset_records.resize(1);
    parallel_dataset_->record_color.resize(1);
    parallel_dataset_->is_record_selected.resize(1);
    for ( int i = 0; i < axis_names_.size(); ++i )
        parallel_dataset_->axis_names.push_back(axis_names_[i]);

    for ( int i = 0; i < current_selection_index_.size(); ++i ){
        // parallel coordinate 
        ParallelRecord* record = new ParallelRecord;
        for ( int j = 0; j < axis_names_.size(); ++j )
            record->values.push_back(axis_data_vec_[axis_order_[j]]->rms_values[axis_data_vec_[axis_order_[j]]->date_sort_index[current_selection_index_[i]]]);
        parallel_dataset_->subset_records[0].push_back(record);
        parallel_dataset_->record_color[0].push_back(date_color_[current_selection_index_[i]]);
        parallel_dataset_->is_record_selected[0].push_back(true);
    }

    for ( int i = 0; i < axis_names_.size(); ++i ){
        int order = axis_order_[i];
        float min_value = 1e10, max_value = -1e10;
        for ( int j = 0; j < is_date_selected_.size(); ++j )
            if ( is_date_selected_[j] ) {
                float temp_value = axis_data_vec_[order]->rms_values[axis_data_vec_[order]->date_sort_index[j]];
                if ( temp_value > max_value ) max_value = temp_value;
                if ( temp_value < min_value ) min_value = temp_value;
            }
        if ( max_value - min_value < 0.1 ) {
            max_value += 0.1;
        }

        std::vector< QString > axis_anchor;
        axis_anchor.push_back(QString::number(min_value));
        axis_anchor.push_back(QString::number(max_value));
        parallel_dataset_->axis_anchors.push_back(axis_anchor);

        for ( int j = 0; j < parallel_dataset_->subset_records[0].size(); ++j )
            parallel_dataset_->subset_records[0][j]->values[i] = (parallel_dataset_->subset_records[0][j]->values[i] - min_value) / (max_value - min_value);
    }

    parallel_dataset_->CompleteInput();
    parallel_coordinate_->SetDataset(parallel_dataset_);
}

void WrfEventComparisonView::UpdateRadarCoordinateView(){
    UpdateRadarItems();
    UpdateItemPositions();

    std::vector< std::vector< int > > record_rank;
    std::vector< QColor > record_color;
    for ( int i = 0; i < current_selection_index_.size(); ++i ){
        // radar view
        std::vector< int > temp_rank;
        for ( int j = 0; j < axis_names_.size(); ++j )
            temp_rank.push_back(axis_data_vec_[axis_order_[j]]->date_sort_index[current_selection_index_[i]]);
        record_rank.push_back(temp_rank);
        record_color.push_back(date_color_[current_selection_index_[i]]);
    }
    radar_coordinate_view_->SetViewData(record_rank, record_color);
}

void WrfEventComparisonView::OnAlphaValueChanged(){
    alpha_ = event_para_ui_.alpha_slider->value();
    event_para_ui_.alpha_label->setText(QString::number(alpha_));

    UpdateRadarItems();
    UpdateItemPositions();
}

void WrfEventComparisonView::OnIterationNumberChanged(){
    iteration_num_ = event_para_ui_.iteration_slider->value();
    event_para_ui_.iteration_label->setText(QString::number(iteration_num_));

    UpdateRadarItems();
    UpdateItemPositions();
}

void WrfEventComparisonView::OnItemRadiusChanged(){
    item_radius_[0] = event_para_ui_.min_radius_slider->value();
    event_para_ui_.min_radius_label->setText(QString::number(item_radius_[0]));
    item_radius_[1] = event_para_ui_.max_radius_slider->value();
    event_para_ui_.max_radius_label->setText(QString::number(item_radius_[1]));

    for ( int i = 0; i < is_date_selected_.size(); ++i )
        if ( is_date_selected_[i] ){
            int min_rank = max_analog_number_;
            for ( int j = 0; j < axis_names_.size(); ++j ){
                if ( date_items_[i]->axis_rank[j] < min_rank ) min_rank = date_items_[i]->axis_rank[j];
            }
            date_items_[i]->SetRadius(item_radius_[0] + (item_radius_[1] - item_radius_[0]) * (max_analog_number_ - min_rank) / max_analog_number_);
            date_items_[i]->UpdateItem();
        }
    radar_coordinate_view_->scene()->update();
}

void WrfEventComparisonView::OnMaxRankChanged(){
    max_analog_number_ = event_para_ui_.max_rank_slider->value();
    event_para_ui_.max_rank_label->setText(QString::number(max_analog_number_));

    radar_coordinate_view_->SetData(axis_names_, max_analog_number_);
    radar_coordinate_view_->update();

    UpdateRadarItems();
    UpdateItemPositions();
}

void WrfEventComparisonView::OnAlphaValueSliderChanged(){
    alpha_ = event_para_ui_.alpha_slider->value();
    event_para_ui_.alpha_label->setText(QString::number(alpha_));
}

void WrfEventComparisonView::OnIterationNumberSliderChanged(){
    iteration_num_ = event_para_ui_.iteration_slider->value();
    event_para_ui_.iteration_label->setText(QString::number(iteration_num_));
}

void WrfEventComparisonView::OnItemRadiusSliderChanged(){
    item_radius_[0] = event_para_ui_.min_radius_slider->value();
    event_para_ui_.min_radius_label->setText(QString::number(item_radius_[0]));
    item_radius_[1] = event_para_ui_.max_radius_slider->value();
    event_para_ui_.max_radius_label->setText(QString::number(item_radius_[1]));
}

void WrfEventComparisonView::OnMaxRankSliderChanged(){
    max_analog_number_ = event_para_ui_.max_rank_slider->value();
    event_para_ui_.max_rank_label->setText(QString::number(max_analog_number_));
}

void WrfEventComparisonView::OnBrushActionTriggered(){
    if ( action_brush_->isChecked() )
        radar_coordinate_view_->SetBrushOn();
    else
        radar_coordinate_view_->SetBrushOff();
}

void WrfEventComparisonView::OnAddManualGroupTriggered(){
    if ( current_selection_index_.size() == 0 ) return;

    QColorDialog dialog;
    if ( dialog.exec() != QDialog::Accepted ) return;

    DateGroup* group = new DateGroup;
    group->group_color = dialog.selectedColor();
    group->group_name = QString("Group %0").arg(manual_groups_.size() + 1);
    group->mem_index = current_selection_index_;

    manual_groups_.push_back(group);
    // update widget
    manual_group_widget_->AddGroupInfo(manual_groups_.size() - 1, group->group_name, group->group_color);

    OnDateColorModeChanged();
}

void WrfEventComparisonView::OnAddRankGroupTriggered(){
    // update widget
    WrfRankParaDialog rank_dialog;
    if ( rank_dialog.exec() != QDialog::Accepted ) return;

    std::vector< WrfElementType > ens_elements;
    std::vector< WrfElementType > ens_mean_elements;
    int analog_number;
    std::vector< float > weight;
    std::vector< float > normalize_value;
    rank_dialog.GetParameters(analog_number, ens_elements, ens_mean_elements, weight, normalize_value);

    AxisData* axis_data = new AxisData;
    axis_data->is_axis = false;
    axis_data->name = QString("Group %0").arg(rank_groups_.size()).toLocal8Bit().data();
    axis_data->analog_number = analog_number;
    axis_data->variables = ens_elements;
    for ( int i = 0; i < ens_mean_elements.size(); ++i ) axis_data->variables.push_back(ens_mean_elements[i]);
    axis_data->weights = weight;
    axis_data->normalize_values = normalize_value;
 
    std::vector< QColor > color_vec;
    ColorMappingGenerator::GetInstance()->GetQualitativeColors(rank_groups_.size() + 1, color_vec);
    axis_data->r = color_vec[rank_groups_.size()].red();
    axis_data->g = color_vec[rank_groups_.size()].green();
    axis_data->b = color_vec[rank_groups_.size()].blue();

    WrfReforecastManager::GetInstance()->PreProcessDataForEvent(axis_data, selected_grid_index_);

    rank_groups_.push_back(axis_data);
    rank_group_widget_->AddGroupInfo(rank_groups_.size() - 1, axis_data->is_axis, QString::fromLocal8Bit(axis_data->name.c_str()), QColor(axis_data->r, axis_data->g, axis_data->b), axis_data->analog_number);

    UpdateAxisData();

    UpdateRadarCoordinateView();
    UpdateParallelCoordinate();
    UpdateHistoryStampView();
}

void WrfEventComparisonView::OnDateColorModeChanged(){
    if ( action_manual_group_mode_->isChecked() ){
        date_color_.assign(date_color_.size(), default_color_);
        for ( int i = 0; i < manual_groups_.size(); ++i ){
            DateGroup* group = manual_groups_[i];
            for ( int j = 0; j < group->mem_index.size(); ++j ) date_color_[group->mem_index[j]] = group->group_color;
        }
    } else {
        date_color_.assign(date_color_.size(), default_color_);
        for ( int i = 0; i < rank_groups_.size(); ++i ){
            AxisData* group = rank_groups_[i];
            for ( int j = 0; j < group->analog_number; ++j ) date_color_[group->sort_date_index[j]] = QColor(group->r, group->g, group->b);
        }
    }

    this->UpdateRadarCoordinateView();
    this->UpdateParallelCoordinate();
}

void WrfEventComparisonView::SaveData(){
    std::ofstream output("savedata.txt");
    if ( !output.good() ) return;

    output << selected_grid_index_.size() << " ";
    for ( int i = 0; i < selected_grid_index_.size(); ++i ) output << selected_grid_index_[i] << " ";
    output << basic_variables_.size() << " ";
    for ( int i = 0; i < basic_variables_.size(); ++i ) output << (int)(basic_variables_[i]) << " ";
    output << axis_data_vec_.size() << " ";
    for ( int i = 0; i < axis_data_vec_.size(); ++i ){
        AxisData* data = axis_data_vec_[i];
        output << data->variables.size() << " ";
        for ( int j = 0; j < data->variables.size(); ++j ) output << (int)data->variables[j] << " ";
        output << data->weights.size() << " ";
        for ( int j = 0; j < data->weights.size(); ++j ) output << data->weights[j] << " ";
        output << data->rms_values.size() << " ";
        for ( int j = 0; j < data->rms_values.size(); ++j ) output << data->rms_values[j] << " ";
        output << data->sort_date_index.size() << " ";
        for ( int j = 0; j < data->sort_date_index.size(); ++j ) output << data->sort_date_index[j] << " ";
        output << data->date_sort_index.size() << " ";
        for ( int j = 0; j < data->date_sort_index.size(); ++j ) output << data->date_sort_index[j] << " ";
    }
    output << axis_names_.size() << " ";
    for (int i = 0; i < axis_names_.size(); ++i ) output << axis_names_[i].toLocal8Bit().data() << " ";
    output << axis_order_.size() << " ";
    for (int i = 0; i < axis_order_.size(); ++i ) output << axis_order_[i] << " ";

    output << max_analog_number_ << " ";
    output << is_date_selected_.size() << " ";
    for (int i = 0; i < is_date_selected_.size(); ++i ) output << (char)(is_date_selected_[i]) << " ";
    output << selected_date_number_ << " ";
    output << pearson_values_.size() << " ";
    for ( int i = 0; i < pearson_values_.size(); ++i ){
        output << pearson_values_[i].size() << " ";
        for ( int j = 0; j < pearson_values_[i].size(); ++j ) output << pearson_values_[i][j] << " ";
    }
    output << center_point_.rx()  << " " << center_point_.ry() << " ";
    output << axis_direction_vec_.size() << " ";
    for ( int i = 0; i < axis_direction_vec_.size(); ++i ) output << axis_direction_vec_[i].rx()  << " " << axis_direction_vec_[i].ry() << " ";
    output << item_radius_[0]  << " " << item_radius_[1] << " ";
    output << alpha_ << " " << iteration_num_ << " ";

    output.close();
}

void WrfEventComparisonView::LoadData(){
    std::ifstream input("savedata.txt");
    if ( !input.good() ) return;

    size_t temp_size;
    input >> temp_size;
    selected_grid_index_.resize(temp_size);
    for ( int i = 0; i < selected_grid_index_.size(); ++i ) input >> selected_grid_index_[i];
    input >> temp_size;
    basic_variables_.resize(temp_size);
    for ( int i = 0; i < basic_variables_.size(); ++i ) {
        int temp_int;
        input >> temp_int;
        basic_variables_[i] = WrfElementType(temp_int);
    }
    input >> temp_size;
    axis_data_vec_.resize(temp_size);
    for ( int i = 0; i < axis_data_vec_.size(); ++i ){
        axis_data_vec_[i] = new AxisData;
        AxisData* data = axis_data_vec_[i];
        input >> temp_size;
        data->variables.resize(temp_size);
        for ( int j = 0; j < data->variables.size(); ++j ) {
            int temp_int;
            input >> temp_int;
            data->variables[j] = WrfElementType(temp_int);
        }
        input >> temp_size;
        data->weights.resize(temp_size);
        for ( int j = 0; j < data->weights.size(); ++j ) input >> data->weights[j];
        input >> temp_size;
        data->rms_values.resize(temp_size);
        for ( int j = 0; j < data->rms_values.size(); ++j ) input >> data->rms_values[j];
        input >> temp_size;
        data->sort_date_index.resize(temp_size);
        for ( int j = 0; j < data->sort_date_index.size(); ++j ) input >> data->sort_date_index[j];
        input >> temp_size;
        data->date_sort_index.resize(temp_size);
        for ( int j = 0; j < data->date_sort_index.size(); ++j ) input >> data->date_sort_index[j];
    }
    input >> temp_size;
    axis_names_.resize(temp_size);
    for (int i = 0; i < axis_names_.size(); ++i ) {
        std::string temp_str;
        input >> temp_str;
        axis_names_[i] = QString::fromLocal8Bit(temp_str.c_str());
    }
    input >> temp_size;
    axis_order_.resize(temp_size);
    for (int i = 0; i < axis_order_.size(); ++i ) input >> axis_order_[i];

    input >> max_analog_number_;
    input >> temp_size;
    is_date_selected_.resize(temp_size);
    for (int i = 0; i < is_date_selected_.size(); ++i ) {
        char temp_char;
        input >> temp_char;
        is_date_selected_[i] = temp_char;
    }
    input >> selected_date_number_;
    input >> temp_size;
    pearson_values_.resize(temp_size);
    for ( int i = 0; i < pearson_values_.size(); ++i ){
        input >> temp_size;
        pearson_values_[i].resize(temp_size);
        for ( int j = 0; j < pearson_values_[i].size(); ++j ) input >> pearson_values_[i][j];
    }
    input >> center_point_.rx() >> center_point_.ry();
    input >> temp_size;
    axis_direction_vec_.resize(temp_size);
    for ( int i = 0; i < axis_direction_vec_.size(); ++i ) input >> axis_direction_vec_[i].rx() >> axis_direction_vec_[i].ry();
    input >> item_radius_[0] >> item_radius_[1];
    input >> alpha_ /*>> iteration_num_*/;

    input.close();

    radar_coordinate_view_->SetData(axis_names_, max_analog_number_);
    //UpdateRadarItems();
}

void WrfEventComparisonView::OnActionGenerateAverageTriggered(){
    if ( current_selection_index_.size() == 0 ) return;

    std::vector< int > selected_time_vec;
    for ( int i = 0; i < current_selection_index_.size(); ++i )
        selected_time_vec.push_back(retrieval_date_time_[current_selection_index_[i]]);


    WrfGridValueMap* value_map = WrfStatisticSolver::GenerateAverageResult(selected_time_vec, forecast_element_, fhour_);

    average_viewer_->ClearElement();
    average_viewer_->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(value_map));
    average_viewer_->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(value_map->element_type));
    average_viewer_->SetMapRange(viewing_range_);
    average_viewer_->SetTitle("Average Reanalysis");

    average_viewer_->show();
}

void WrfEventComparisonView::OnActionGeneratePqpfTriggered(){
    if ( current_selection_index_.size() == 0 ) return;

    std::vector< int > selected_time_vec;
    for ( int i = 0; i < current_selection_index_.size(); ++i )
        selected_time_vec.push_back(retrieval_date_time_[current_selection_index_[i]]);

    WrfProbabilisticForecastDialog prob_dialog;

    if ( prob_dialog.exec() ){
        ProbabilisticPara para;
        prob_dialog.GetSelectionParas(para);

        WrfGridValueMap* value_map = WrfStatisticSolver::GeneratePqpfResult(selected_time_vec, para, fhour_);
        pqpf_viewer_->ClearElement();
        pqpf_viewer_->AddRenderingElement(WrfRenderingElementFactory::GenerateRenderingElement(value_map));
        pqpf_viewer_->SetColorBarElement(WrfRenderingElementFactory::GenerateColorBarElement(value_map->element_type));
        pqpf_viewer_->SetMapRange(viewing_range_);
        pqpf_viewer_->SetTitle(QString("Probability of Precipitation > %0 mm").arg(para.thresh));

        pqpf_viewer_->show();
    }
}

void WrfEventComparisonView::OnActionShowPrecipitationScatterPlotTriggered(){
    if ( current_selection_index_.size() == 0 ) return;

    std::vector< int > selected_time_vec;
    for ( int i = 0; i < current_selection_index_.size(); ++i )
        selected_time_vec.push_back(retrieval_date_time_[current_selection_index_[i]]);

    scatter_plot_->SetAxisNames(std::string("Forecast"), std::string("Observed"));

    std::vector< float > plot_values;

    MapRange reanalysis_map_range;
    WrfDataManager::GetInstance()->GetModelMapRange(WRF_REANALYSIS, reanalysis_map_range);
        
    for ( int t = 0; t < selected_time_vec.size(); ++t ){
        std::vector< WrfGridValueMap* > fcst_maps;
        WrfDataManager::GetInstance()->GetGridValueMap(selected_time_vec[t], WRF_NCEP_ENSEMBLES, forecast_element_, fhour_, fcst_maps);
        if ( fcst_maps.size() == 0 ) 
            WrfDataManager::GetInstance()->GetGridValueMap(selected_time_vec[t], WRF_NCEP_ENSEMBLE_MEAN, forecast_element_, fhour_, fcst_maps);
        if ( fcst_maps.size() == 0 ) return;

        if ( fcst_maps.size() > 1 ){
            for ( int i = 1; i < fcst_maps.size(); ++i ){
                for ( int j = 0; j < fcst_maps[0]->map_range.x_grid_number * fcst_maps[0]->map_range.y_grid_number; ++j )
                    fcst_maps[0]->values[j] += fcst_maps[i]->values[j];
            }
            for ( int i = 0; i < fcst_maps[0]->map_range.x_grid_number * fcst_maps[0]->map_range.y_grid_number; ++i )
                fcst_maps[0]->values[i] /= fcst_maps.size();

            for ( int i = 1; i < fcst_maps.size(); ++i ) delete fcst_maps[i];
            fcst_maps.resize(1);
        }

        std::vector< WrfGridValueMap* > reanalysis_map;
        WrfDataManager::GetInstance()->GetGridValueMap(selected_time_vec[t], WRF_REANALYSIS, forecast_element_, fhour_, reanalysis_map);

        WrfGridValueMap* converted_map = WrfStatisticSolver::Convert2Map(reanalysis_map[0], fcst_maps[0]->map_range);

        for ( int i = 0; i < selected_grid_index_.size(); ++i ){
            float num_value = fcst_maps[0]->values[selected_grid_index_[i]];
            float reana_value = converted_map->values[selected_grid_index_[i]];
            if ( num_value > 3000 || reana_value > 3000 || num_value < 0 || reana_value < 0) {
                num_value = 0;
                reana_value = 0;
            }
            plot_values.push_back(num_value);
            plot_values.push_back(reana_value);
        }
    }

    scatter_plot_->SetData(plot_values);
    scatter_plot_->SetAxisValueRange(0, 50, 0, 50);
    scatter_plot_->show();
}