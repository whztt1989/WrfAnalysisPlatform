#include "segment_image.h"
#include "image_generator.h"

SegmentImage::SegmentImage(BasicImage* t_image)
    : image(t_image){
    ProcessImage();
}

SegmentImage::~SegmentImage(){

}

void SegmentImage::ProcessImage(){
    ImageGenerator::ExtractEllipseInfo(image, region_count, region_index, ellipse_info);
}