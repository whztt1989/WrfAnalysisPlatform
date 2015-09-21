#ifndef PLAT_IMAGE_VIEW_H_
#define PLAT_IMAGE_VIEW_H_

#include "gl/glew.h"
#include <QtOpenGL/QGLWidget>

class Node;

struct EdgeLink{
    int id_one, id_two;
};

class PlatImageView : public QGLWidget{
    Q_OBJECT

public:
    PlatImageView(QWidget* parent = 0);
    ~PlatImageView();

    void SetData(Node* root);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    Node* root_;
    std::vector< QRectF > image_rects_; 
    std::vector< Node* > indexed_nodes_;
    std::vector< EdgeLink > edge_links_;
    int max_level_;
    int max_level_node_count_;
    float max_width_, max_height_;
    std::vector< int > level_node_count_;

    float view_left_, view_right_, view_bottom_, view_top_;

    void InitializeTreeIndex();
};

#endif