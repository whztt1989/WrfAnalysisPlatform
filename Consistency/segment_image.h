#ifndef SEGMENT_IMAGE_H_
#define SEGMENT_IMAGE_H_

#include <QtCore/QObject>
#include "consistency_common.h"

class SegmentImage : public QObject
{
	Q_OBJECT

public:
	SegmentImage(BasicImage* t_image);
	~SegmentImage();

    int width() { return image->width(); }
    int height() { return image->height(); }

    void Render(float left, float right, float bottom, float top);
    void RenderSegment(float left, float right, float bottom, float top, std::vector< int >& segment_index);
    
    BasicImage* image;
    std::vector< int > region_count;
    std::vector< std::vector< int > > region_index;
    std::vector< std::vector< struct Ellipse > > ellipse_info;

    void ProcessImage();
};

#endif