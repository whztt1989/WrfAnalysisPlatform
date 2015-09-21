#ifndef COMPARISON_LINE_CHART_H_
#define COMPARISON_LINE_CHART_H_

#include "gl/glew.h"
#include <QtGui/QColor>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <vector>
#include <iostream>

class ComparisonLineChartDataset {
public:
    ComparisonLineChartDataset() {}
    ~ComparisonLineChartDataset() {}

    QString data_name;
    std::vector< QString > label_names;
    std::vector< QColor > label_colors;
    std::vector< std::vector< float > > values;
    std::vector< int > stopping_values;
	std::vector< float > distributions;
};

class ComparisonLineChart : public QGLWidget{
    Q_OBJECT
public:
    ComparisonLineChart(QWidget *parent = 0);
    ~ComparisonLineChart();

    void SetDataset(ComparisonLineChartDataset* data);
    void SetSelectionValue(int value);
    void SetSortingIndex(int index);
    void SetMaximumViewingNum(int max_num);
	void GetSortedIndex(std::vector< int >& sorted_index) { sorted_index = sorted_index_; }
	float GetSelectedRMSValue() { return dataset_->values[current_sort_index_][sorted_index_[selected_analog_num_ - 1]]; }
	void SetSuggestionRate(float min_rate);
	void SetSuggestionOn();
	void SetSuggestionOff();

signals:
    void AnalogNumberChanged(int);
	void GridSizeChanged(int);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent* event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *);

private:
    ComparisonLineChartDataset* dataset_;

    int label_width_, label_height_, label_text_width_, border_size_, margin_size_;
    int coor_text_width_, coor_text_height_;
    int title_height_, distribution_bar_height_;

    float coor_bottom_, coor_right_, coor_top_, coor_left_;
    float value_y_step_, value_x_step_;

    int current_analog_num_;
    int selected_analog_num_;
    int current_sort_index_;

    float min_value_, max_value_;
    int base_index_;

	float min_rate_;
	int min_distribution_index_;

    std::vector< int > sorted_index_;

    QPoint current_mouse_pos_;

    int max_viewing_num_;

    bool is_data_updated_;
    bool is_floating_;
	bool is_suggestion_on_;

    QMenu* context_menu_;
    QMenu* apply_grid_size_menu_;
    QActionGroup* apply_grid_size_action_group_;

    void UpdateRenderingBase(int base);
    void Sort(int attrib_index, int begin, int end);
	void UpdateSuggestion();

    private slots:
        void OnApplyGridSizeTriggered();
        void OnApplyAnalogNumberTriggered();
};

#endif