#include "plat_image_view.h"
#include "consistency_common.h"

PlatImageView::PlatImageView(QWidget* parent /* = 0 */){
    this->setMinimumSize(500, 200);
    view_left_ = -1;
    view_right_ = 1;
    view_bottom_ = -1;
    view_top_ = 1;
    max_width_ = 10;
    max_height_ = 10;
}

PlatImageView::~PlatImageView(){

}

void PlatImageView::SetData(Node* root){
    root_ = root;
    InitializeTreeIndex();

    this->updateGL();
}

void PlatImageView::initializeGL(){
    if ( glewInit() != GLEW_OK ) return;
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void PlatImageView::resizeGL(int w, int h){
    float w_scale = max_width_ / w;
    float h_scale = max_height_ / h;

    if ( w_scale < h_scale ){
        view_top_ = max_height_;
        view_bottom_ = 0;
        view_left_ = max_width_ / 2 - max_height_ * w / h / 2;
        view_right_ = max_width_ / 2 + max_height_ * w / h / 2;
    } else {
        view_top_ = max_height_ / 2 + max_width_ * h / w / 2;
        view_bottom_ = max_height_ / 2 - max_width_ * h / w / 2;
        view_left_ = 0;
        view_right_ = max_width_;
    }
}

void PlatImageView::paintGL(){
    glViewport(0, 0, this->width(), this->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(view_left_, view_right_, view_bottom_, view_top_, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT);
    // render the images
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for ( int i = 0; i < indexed_nodes_.size(); ++i )
        if ( indexed_nodes_[i]->image != NULL )
            indexed_nodes_[i]->image->Render(image_rects_[i].left(), image_rects_[i].right(), image_rects_[i].bottom(), image_rects_[i].top());
	glDisable(GL_BLEND);
}

void PlatImageView::InitializeTreeIndex(){
    level_node_count_.resize(10);
    level_node_count_.assign(10, 0);
    
    std::vector< Node* > reached_node_vec;
    reached_node_vec.push_back(root_);
    image_rects_.clear();
    indexed_nodes_.clear();
    edge_links_.clear();
    root_->level = 0;
    max_level_ = -1;
    int node_index = 0;
    Node* node;
    while ( reached_node_vec.size() != 0 ){
        node = reached_node_vec.at(reached_node_vec.size() - 1);
        level_node_count_[node->level]++;
        image_rects_.push_back(QRectF(level_node_count_[node->level], node->level, 0, 0));
        indexed_nodes_.push_back(node);
        if ( node->level > max_level_ ) max_level_ = node->level;
        reached_node_vec.pop_back();
        for ( int i = 0; i < node->children.size(); ++i ){
            if ( node->children[i]->id == -1 ) {
                reached_node_vec.push_back(node->children[i]);
                node->children[i]->level = node->level + 1;
                node_index++;
                node->children[i]->id = node_index;
            }
            EdgeLink link;
            link.id_one = node->id;
            link.id_two = node->children[i]->id;
            edge_links_.push_back(link);
        }
    }
    max_level_node_count_ = -1;
    for ( int i = 0; i < max_level_; ++i ) 
        if ( level_node_count_[i] > max_level_node_count_ ) max_level_node_count_ = level_node_count_[i];

    float max_width = 100.0f;
    float average_width = max_width / max_level_node_count_;
    float border_scale = 0.1 * average_width;
    float image_height = average_width;
    for ( int i = 0; i < indexed_nodes_.size(); ++i ){
        image_rects_[i].setTop((max_level_ - image_rects_[i].top() + 1) * image_height - border_scale);
        image_rects_[i].setBottom(image_rects_[i].top() - image_height + 2 * border_scale);
        image_rects_[i].setLeft(((max_level_node_count_ - level_node_count_[indexed_nodes_[i]->level]) / 2.0 + image_rects_[i].left() - 1) * average_width + border_scale);
        image_rects_[i].setRight(image_rects_[i].left() + average_width - 2 * border_scale);
    }

    max_width_ = max_width;
    max_height_ = image_height * (max_level_ + 1);
    resizeGL(this->width(), this->height());
}
