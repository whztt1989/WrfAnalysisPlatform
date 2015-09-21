#ifndef IMAGE_GENERATOR_H_
#define IMAGE_GENERATOR_H_

#include <vector>
#include <QtCore/QString>
#include "consistency_common.h"
#include "data_type.h"
#include "segment_image.h"

struct Ellipse
{
	VectorF2D axis_length;
	VectorF2D center;
	float angle;
};

class ImageGenerator
{
public:
    ImageGenerator();
    ~ImageGenerator();

    static LevelImage* GenerateLevelImage(QString data_file_path, std::vector< float >& value_range_array);
    static bool GenerateLevelAlphaChannel(BasicImage* image_one, BasicImage* image_two, bool is_both);
    static AggregateImage* GenerateAggregateImage(std::vector< BasicImage* >& images, std::vector< float >& transfer_value);
    static BlendImage* GenerateBlendImage(BasicImage* image_one, BasicImage* image_two, float alpha_factor);
    static bool GenerateClusterImages(std::vector< BasicImage* >& images, std::vector< ClusterImage* >& cluster_images);

    static void ExtractEllipseInfo(BasicImage* image, std::vector< int >& region_count, std::vector< std::vector< int > >& region_index, std::vector< std::vector< struct Ellipse > >& ellipse_info);
    static float DiffSegementImage(SegmentImage* image_one, SegmentImage* image_two);

private:
    static void GaussianBlur(std::vector< float >& values, int w, int h, float radius);
	static void ExecuteErosion(int w, int h, std::vector< bool >& image_data);
	static void ExecuteDilation(int w, int h, std::vector< bool >& image_data);
	static int ExecuteRegionGrowing(int w, int h, std::vector< bool >& image_data, std::vector< int >& region_index);
	static void FitEllipse(int w, int h, int region_count, std::vector< int >& region_index, std::vector< struct Ellipse >& ellipse_info);
	static float DiffEllipse(struct Ellipse& ellipse_one, struct Ellipse& ellipse_two);
	static float DiffImage(BasicImage* image_one, BasicImage* image_two);
};

#endif