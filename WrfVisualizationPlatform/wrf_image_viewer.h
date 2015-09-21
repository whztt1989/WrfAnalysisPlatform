#ifndef WRF_IMAGE_VIEWER_H_
#define WRF_IMAGE_VIEWER_H_

#include "gl/glew.h"
#include <QtOpenGL/QGLWidget>
#include <QtCore/QPoint>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <vector>
#include "color_mapping_generator.h"
#include "wrf_data_common.h"

class WrfImage;
class WrfGridValueMap;
class WrfRenderingElement;

class WrfImageViewer : public QGLWidget
{
    Q_OBJECT

public:
    WrfImageViewer(int viewer_index = -1);
    ~WrfImageViewer();

    enum ViewerStatus {
        VIEWER_ERROR = 0x0,
        VIEWER_UNINITIALIZED,
        VIEWER_NORMAL,
        VIEWER_TRACKING_PEN,
    };

    enum RenderingStatus{
        RENDERING_OK        = 0x00000000,
        ELEMENT_UPDATED     = 0x00000001,
        SIZE_UPDATED        = 0x00000010,
        TITLE_UPDATED       = 0x00000100,
        COLOR_BAR_UPDATED   = 0x00001000,
    };

    void SetTrackingPen(bool enabled);
    void SetTitle(QString title);
    void SetColorBarElement(WrfRenderingElement* color_bar);
    void SetSecondColorBarElement(WrfRenderingElement* color_bar);
    void AddRenderingElement(WrfRenderingElement* element);
	void AddIsoLinePlot(int mode, int datetime, WrfModelType model_type, WrfElementType element_type, int fhour, float iso_value, int ens = 0);
    void SetMapRange(MapRange& range);

    void GetSelectionPath(std::vector< QPointF >& path);
    void SetSelectionPath(std::vector< QPointF >& path);

	void GetViewPara(float& rendering_scale, float& left, float& right, float& bottom, float& top);
	void SetViewPara(float rendering_scale, float left, float right, float bottom, float top);

    // clear and delete all rendering elements
    void ClearElement();

signals:
    void SelectionUpdated(int);
    void SelectionFinished(int);
    void ImageSelected(int);
	void ViewChanged(int);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintEvent(QPaintEvent*);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

	void contextMenuEvent(QContextMenuEvent *);
	void showEvent(QShowEvent *);

private:
    int viewer_index_;
    QString title_;

    ViewerStatus status_;
    unsigned int rendering_status_;
    
    float rendering_scale_;
    float view_left_, view_right_, view_bottom_, view_top_;
    int title_height_, color_bar_width_;
    GLuint font_offset_;

    MapRange map_range_;

    WrfRenderingElement* color_bar_element_;
    WrfRenderingElement* second_bar_element_;
    std::vector< WrfRenderingElement* > element_vec_;

    std::vector< QPointF > brush_path_;
    QPointF previous_pos_;

	QMenu* context_menu_;
	QAction* action_adding_iso_line_plot_;
	std::vector< QAction* > action_element_visibility_;

    void UpdateRenderingSize();

	private slots:
		void OnActionAddIsoLinePlotTriggered();
		void OnVisibilityActionTriggered();
};

#endif