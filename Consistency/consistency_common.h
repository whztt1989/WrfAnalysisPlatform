#ifndef CONSISTENCY_COMMON_H_
#define CONSISTENCY_COMMON_H_

#include <vector>
#include <QtCore/QObject>

enum ImageType{
    BASIC_IMAGE = 0x0,
    LEVEL_IMAGE,
    AGGREGATE_IMAGE,
    BLEND_IMAGE,
    CLUSTER_IMAGE
};

class BasicImage{
public:
    BasicImage(int w, int h, float* data);
    BasicImage(int w, int h, float* data, float* alpha);
	virtual ~BasicImage();

    enum RenderingMode{
        MAP_LEVLE = 0x0,
        MAP_LEVEL_ALPHA,
        MAP_COLOR
    };

    virtual void Render(float left, float right, float bottom, float top);

    int width() { return width_; }
    int height() { return height_; }
    float* data() { return data_.data(); }
    float* alpha() { return alpha_.data(); }
    ImageType image_type() { return image_type_; }

    void set_rendering_mode(RenderingMode mode);
    void set_alpha(float* alpha);

    bool Save(QString file_name);
    
protected:
    ImageType image_type_;
    std::vector< float > data_;
    std::vector< float > alpha_;
    std::vector< float > rgba_values_;
    RenderingMode rendering_mode_;

private:
    int width_, height_;
	bool is_alpha_set_;
};

class LevelImage : public BasicImage {
public:
    LevelImage(int w, int h, float* data);
    ~LevelImage(); 

    void Render(float left, float right, float bottom, float top);
};

class AlphaImage : public BasicImage {
public:
    AlphaImage(BasicImage* image);
    ~AlphaImage(); 

    void Render(float left, float right, float bottom, float top);
};

class AggregateImage : public BasicImage {
public:
    AggregateImage(int w, int h, float* data);
    ~AggregateImage(); 

    void Render(float left, float right, float bottom, float top);
};

class BlendImage : public BasicImage {
public:
    BlendImage(int w, int h, float* data);
    ~BlendImage(); 

    void Render(float left, float right, float bottom, float top);
};

class ClusterImage : public BasicImage {
public:
    ClusterImage(int w, int h, float* data);
    ~ClusterImage(); 

    void Render(float left, float right, float bottom, float top);
};

class Node{
public:
    Node(BasicImage* image);
    ~Node();

    std::vector< Node* > children;
    BasicImage* image;
    int id;
    int level;
};

#endif