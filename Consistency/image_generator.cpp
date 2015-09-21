#include "image_generator.h"
#include <fstream>
#include <string>
#include <opencv/cv.h>
#include <opencv/highgui.h>

ImageGenerator::ImageGenerator(){

}

ImageGenerator::~ImageGenerator(){

}

LevelImage* ImageGenerator::GenerateLevelImage(QString data_file_path, std::vector< float >& value_range_array){
    LevelImage* result_image = NULL;

    float longitude_grid_space, latitude_grid_space;
    float start_longitude, end_longitude, start_latitude, end_latitude;
    int latitude_grid_number, longitude_grid_number;
    int temp_year, temp_month, temp_day, temp_hour, temp_level, temp_exposion_time;
    float iso_space, start_iso, end_iso, smooth_factor, bold_factor;
    std::string header_info;
    std::vector< float > values;

    std::ifstream input(data_file_path.toLocal8Bit().data());

    if ( input.good() ){
        getline(input, header_info);

        input >> temp_year >> temp_month >> temp_day >> temp_hour >> temp_exposion_time >> temp_level;
        input >> longitude_grid_space >> latitude_grid_space;
        input >> start_longitude >> end_longitude;
        input >> start_latitude >> end_latitude;
        input >> longitude_grid_number >> latitude_grid_number;
        input >> iso_space >> start_iso >> end_iso >> smooth_factor >> bold_factor;

        values.resize(longitude_grid_number * latitude_grid_number);
        for ( int i = 0; i < longitude_grid_number * latitude_grid_number; ++i ) input >> values[i];

        input.close();
       
        float* data = new float[longitude_grid_number * latitude_grid_number];

        for ( int i = 0; i < longitude_grid_number * latitude_grid_number; ++i ){
            int j;
            for ( j = value_range_array.size() - 1; j >= 0; --j )
                if ( values[i] >= value_range_array[j] ){
                    data[i] = (float)j / (value_range_array.size() - 1);
                    break;
                }
            if ( j == -1 ) data[i] = 0;
        }

        result_image = new LevelImage(longitude_grid_number, latitude_grid_number, data);

        delete []data;
    }

    return result_image;
}

/*
* The alpha channel is affected by
* 1. grid value, which could be generated using hierarchical images
* 2. region 1) appearance and disappearance of regions 2) shape of regions, described with eclipse
*/
bool ImageGenerator::GenerateLevelAlphaChannel(BasicImage* image_one, BasicImage* image_two, bool is_both){
    // construct the hierarchical images and generate the alpha channels
    std::vector< float > alpha_channel;
    alpha_channel.resize(image_one->width() * image_one->height(), 0);

    //std::vector< std::vector< float > > pixel_values_one;
    //std::vector< std::vector< float > > pixel_values_two;

    //int image_level = 0;
    //int temp_size = 1;
    //while ( temp_size < image_one->width() && temp_size < image_one->height() ){
    //    temp_size *= 2;
    //    image_level += 1;
    //}
    //image_level -= 3;

    //int sample_step = 1;
    //pixel_values_one.resize(image_level);
    //pixel_values_two.resize(image_level);
    //for ( int i = 0; i < image_level; ++i ){
    //    int temp_w, temp_h;
    //    temp_w = image_one->width() / sample_step;
    //    temp_h = image_one->height() / sample_step;
    //    pixel_values_one[i].resize(temp_w * temp_h);
    //    for ( int h = 0; h < temp_h; ++h )
    //        for ( int w = 0; w < temp_w; ++w ){
				//float pixel_value = 0;
				//int pixel_count = 0;
				//for ( int th = h * sample_step; th < (h + 1) * sample_step && th < image_one->height(); ++th )
				//	for ( int tw = w * sample_step; tw < (w + 1) * sample_step && tw < image_one->width(); ++tw ){
				//		pixel_value += image_one->data()[th * image_one->width() + tw];
				//		pixel_count++;
				//	}
    //            pixel_values_one[i][h * temp_w + w] = pixel_value / pixel_count;
    //        }
    //    GaussianBlur(pixel_values_one[i], temp_w, temp_h, 2);

    //    temp_w = image_two->width() / sample_step;
    //    temp_h = image_two->height() / sample_step;
    //    pixel_values_two[i].resize(temp_w * temp_h);
    //    for ( int h = 0; h < temp_h; ++h )
    //        for ( int w = 0; w < temp_w; ++w ){
				//float pixel_value = 0;
				//int pixel_count = 0;
				//for ( int th = h * sample_step; th < (h + 1) * sample_step && th < image_two->height(); ++th )
				//	for ( int tw = w * sample_step; tw < (w + 1) * sample_step && tw < image_two->width(); ++tw ){
				//		pixel_value += image_two->data()[th * image_two->width() + tw];
				//		pixel_count++;
				//	}
				//	pixel_values_two[i][h * temp_w + w] = pixel_value / pixel_count;
    //        }
    //    GaussianBlur(pixel_values_two[i], temp_w, temp_h, 2);

    //    std::vector< float > sample_alphas;
    //    sample_alphas.resize(temp_w * temp_h);
    //    for ( int k = 0; k < temp_h * temp_w; ++k ) sample_alphas[k] = abs(pixel_values_one[i][k] - pixel_values_two[i][k]);

    //    // define the mapping of level to alpha
    //    float scale_factor = 0.1;
    //    for ( int k = 0; k < temp_w * temp_h; ++k ) sample_alphas[k] *= scale_factor;

    //    for ( int h = 0; h < image_one->height(); ++h )
    //        for ( int w = 0; w < image_one->width(); ++w ){
    //            int wi = w / sample_step;
    //            int he = h / sample_step;
    //            if ( wi >= temp_w || he >= temp_h ) continue;

    //            alpha_channel[h * image_one->width() + w] += sample_alphas[he * temp_w + wi];
    //        }

    //    sample_step *= 2;
    //}

	// region
	std::vector< float > region_alpha;
	region_alpha.resize(image_one->width() * image_one->height(), 0);

	std::vector< std::vector< int > > region_index_one, region_index_two;
	std::vector< int > region_count_one, region_count_two;
	std::vector< std::vector< Ellipse > > ellipse_one, ellipse_two;
	ExtractEllipseInfo(image_one, region_count_one, region_index_one, ellipse_one);
	ExtractEllipseInfo(image_two, region_count_two, region_index_two, ellipse_two);

	float counting_thresh = 0.333; 
	for ( int i = 0; i < region_index_one.size(); ++i ){
		// region correlation checking
		std::vector< std::vector< float > > correlation_map;
		correlation_map.resize(region_count_one[i] + 1);
		for ( int j = 0; j < correlation_map.size(); ++j ) correlation_map[j].resize(region_count_two[i] + 1, 0);
		for ( int j = 0; j < image_one->width() * image_one->height(); ++j ) 
			correlation_map[region_index_one[i][j] + 1][region_index_two[i][j] + 1] += 1;
		for ( int j = 0; j < correlation_map.size(); ++j ){
			float temp_acc = 0;
			for ( int k = 0; k < correlation_map[j].size(); ++k ) temp_acc += correlation_map[j][k];
			if ( temp_acc > 1e-5 )
				for ( int k = 0; k < correlation_map[j].size(); ++k ) correlation_map[j][k] /= temp_acc;
		}

		// calculate the difference value
		std::vector< float > index_alpha_vec;
		for ( int j = 1; j < correlation_map.size(); ++j ){
			float accu_weight = 0;
			float temp_alpha = 0;
			for ( int k = 0; k < correlation_map[j].size(); ++k )
				if ( correlation_map[j][k] > counting_thresh ){
					accu_weight += correlation_map[j][k];
					if ( k != 0 )
						temp_alpha += DiffEllipse(ellipse_one[i][j - 1], ellipse_two[i][k - 1]) * correlation_map[j][k];
					else 
						temp_alpha += correlation_map[j][k];
				}
			if ( accu_weight > 1e-5 ) temp_alpha /= accu_weight;
			index_alpha_vec.push_back(temp_alpha);
		}
		for ( int j = 0; j < image_one->width() * image_one->height(); ++j )
			if ( region_index_one[i][j] != -1 && index_alpha_vec[region_index_one[i][j]] > region_alpha[j] ) region_alpha[j] = index_alpha_vec[region_index_one[i][j]];

		index_alpha_vec.clear();
		for ( int j = 1; j < correlation_map[0].size(); ++j ){
			float accu_weight = 0;
			float temp_alpha = 0;
			for ( int k = 0; k < correlation_map.size(); ++k )
				if ( correlation_map[k][j] > counting_thresh ){
					accu_weight += correlation_map[k][j];
					if ( k != 0 )
						temp_alpha += DiffEllipse(ellipse_one[i][k - 1], ellipse_two[i][j - 1]) * correlation_map[k][j];
					else 
						temp_alpha += correlation_map[k][j];
				}
				if ( accu_weight > 1e-5 ) temp_alpha /= accu_weight;
				index_alpha_vec.push_back(temp_alpha);
		}
		for ( int j = 0; j < image_one->width() * image_one->height(); ++j )
			if ( region_index_two[i][j] != -1 && index_alpha_vec[region_index_two[i][j]] > region_alpha[j] ) region_alpha[j] = index_alpha_vec[region_index_two[i][j]];
	}

	float region_factor = 1.0f;
	for ( int i = 0; i < image_one->width() * image_one->height(); ++i) 
		alpha_channel[i] = alpha_channel[i] * (1.0 - region_factor) + region_alpha[i] * region_factor; 
    
    // if is_both then the alpha channel will be added to both of the images, otherwise only image_two is added
    image_one->set_alpha(alpha_channel.data());
    if ( is_both ) image_two->set_alpha(alpha_channel.data());

    return true;
}

AggregateImage* ImageGenerator::GenerateAggregateImage(std::vector< BasicImage* >& images, std::vector< float >& transfer_value){
    if ( images.size() > transfer_value.size() || images.size() == 0 ) return NULL;

    int w = images[0]->width();
    int h = images[0]->height();
    float* data = new float[w * h];
    memset(data, 0, sizeof(float) * w * h);
    for ( int i = 0; i < images.size(); ++i ){
        for ( int j = 0; j < w * h; ++j ) data[j] += (1 - data[j]) * images[i]->alpha()[j] * transfer_value[i];
    }

    AggregateImage* image = new AggregateImage(images[0]->width(), images[0]->height(), data);

    return image;
}

BlendImage* ImageGenerator::GenerateBlendImage(BasicImage* image_one, BasicImage* image_two, float alpha_factor){
    if ( image_one == NULL || image_two == NULL ) return NULL;

    int w = image_one->width();
    int h = image_one->height();
    float* data = new float[w * h];
    for ( int i = 0; i < w * h; ++i ) data[i] = image_one->data()[i] * alpha_factor + image_two->data()[i] * (1 - alpha_factor);

    BlendImage* image = new BlendImage(w, h, data);

    return image;
}

bool ImageGenerator::GenerateClusterImages(std::vector< BasicImage* >& images, std::vector< ClusterImage* >& cluster_images){
	std::vector< std::vector< float > > distance_matrix;
	distance_matrix.resize(images.size());
	for ( int i = 0; i < distance_matrix.size(); ++i ) distance_matrix[i].resize(images.size(), 0);

	for ( int i = 0; i < distance_matrix.size() - 1; ++i )
		for ( int j = i + 1; j < distance_matrix.size(); ++j ){
			distance_matrix[i][j] = DiffImage(images[i], images[j]);
			distance_matrix[j][i] = distance_matrix[i][j];
		}

	std::vector< int > cluster_index;
	cluster_index.resize(images.size());
	for ( int i = 0; i < cluster_index.size(); ++i ) cluster_index[i] = i;
	float cutting_thresh = 10;
	do{
		float mean_dis_value = 100000;
		int mean_index = -1;
		for ( int i = 0; i < images.size() - 1; ++i )
			if ( cluster_index[i] != cluster_index[i + 1] && distance_matrix[i][i + 1] < mean_dis_value ){
				mean_index = i;
				mean_dis_value = distance_matrix[i][i + 1];
			}
		if ( mean_index != -1 && mean_dis_value < cutting_thresh ){
			for ( int i = mean_index + 1; i < images.size() && cluster_index[i] == cluster_index[mean_index + 1]; ++i )
				cluster_index[i] = cluster_index[mean_index];
		} else {
			break;
		}
	} while (true);

	int cluster_begin = 0;
	int cluster_end = 0;
	while ( cluster_begin < images.size() ){
		cluster_end = cluster_begin;
		while ( cluster_end < images.size()  && cluster_index[cluster_end] == cluster_index[cluster_begin] ) cluster_end++;
		int represent_index = (cluster_end - 1 + cluster_begin) / 2;
		ClusterImage* image = new ClusterImage(images[represent_index]->width(), images[represent_index]->height(), images[represent_index]->data());
		cluster_images.push_back(image);
		cluster_begin = cluster_end;
	}

    return true;
}

void ImageGenerator::GaussianBlur(std::vector< float >& values, int w, int h, float radius){
    std::vector< float > temp_values;
    temp_values.assign(values.begin(), values.end());

    for ( int i = 0; i < h; ++i )
        for ( int j = 0; j < w; ++j ){
            float scale_factor = 0;
            float value = 0;
            for ( int hr = -1 * radius; hr < radius; ++hr )
                for ( int wr = -1 * radius; wr <= radius; ++wr){
                    int temph = i + hr;
                    int tempw = j + wr;
                    if ( temph < 0 || temph >= h || tempw < 0 || tempw >= w ) continue;

                    float temp_factor = exp(-1 * (pow((double)(i - temph), 2) + pow((double)(j - tempw), 2)));
                    scale_factor += temp_factor;
                    value += temp_values[temph * w + tempw] * temp_factor;
                }
            if ( scale_factor > 1e-10 ) value /= scale_factor;
            values[i * w + j] = value;
        }
}

void ImageGenerator::ExtractEllipseInfo(BasicImage* image, std::vector< int >& region_count, std::vector< std::vector< int > >& region_index, std::vector< std::vector< struct Ellipse > >& ellipse_info){
	ellipse_info.clear();
	region_count.clear();

	std::vector< float > cutting_threshold;
	for ( int i = 0; i < 4; ++i ) cutting_threshold.push_back(0.01 + i / 4.0);

	region_index.resize(cutting_threshold.size());

	//image->Save("./testresult/ellipse0.png");

	int w = image->width();
	int h = image->height();
	int erosion_run = 3;
	std::vector< bool > image_data;
	image_data.resize(image->width() * image->height());
	for ( int i = 0; i < cutting_threshold.size(); ++i ){
		std::vector< Ellipse > temp_ellipse_info;

		for ( int j = 0; j < w * h; ++j )
			if ( image->data()[j] > cutting_threshold[i] ) image_data[j] = 1;
			else image_data[j] = 0;

		// step 1: erosion
		for ( int j = 0; j < erosion_run; ++j ) ExecuteErosion(w, h, image_data);
		for ( int j = 0; j < erosion_run; ++j ) ExecuteDilation(w, h, image_data);
		
		// step 2: region growing to detect regions
		int temp_region_count = ExecuteRegionGrowing(w, h, image_data, region_index[i]);
		region_count.push_back(temp_region_count);

		// step 3: shape using eclipse
		FitEllipse(w, h, temp_region_count, region_index[i], temp_ellipse_info);

		// for check temporary result
		/*std::vector< float > temp_data;
		temp_data.resize(image->width() * image->height());
		for ( int k = 0; k < image->width() * image->height(); ++k ) 
			if ( image_data[k] ) temp_data[k] = 0.5;
			else temp_data[k] = 0;
		LevelImage* test_image1 = new LevelImage(image->width(), image->height(), temp_data.data());
		test_image1->Save("./testresult/ellipse1.png");

		for ( int k = 0; k < temp_ellipse_info.size(); ++k ){
			float test = cos(90.0 / 180.0 * 3.1415);
			float cos_beta = cos(temp_ellipse_info[k].angle / 180.0 * 3.1415);
			float sin_beta = sin(temp_ellipse_info[k].angle / 180.0 * 3.1415);
			for ( int theta = 0; theta < 360; ++ theta ){
				float cos_theta = cos((float)theta / 180.0 * 3.1415);
				float sin_theta = sin((float)theta / 180.0 * 3.1415);
				int x = (int)(temp_ellipse_info[k].axis_length.x() * cos_beta * cos_theta - temp_ellipse_info[k].axis_length.y() * sin_beta * sin_theta + temp_ellipse_info[k].center.x());
				int y = (int)(temp_ellipse_info[k].axis_length.x() * sin_beta * cos_theta + temp_ellipse_info[k].axis_length.y() * cos_beta * sin_theta + temp_ellipse_info[k].center.y());
				if ( x < 0 || y < 0 || x >= image->width() || y >= image->height() ) continue;
				temp_data[y * image->width() + x] = 1.0;
			}
		}

		LevelImage* test_image = new LevelImage(image->width(), image->height(), temp_data.data());
		test_image->Save("./testresult/ellipse.png");*/


		ellipse_info.push_back(temp_ellipse_info);
	}
}

int ImageGenerator::ExecuteRegionGrowing(int w, int h, std::vector< bool >& image_data, std::vector< int >& region_index){
	int region_count = 0;

	region_index.resize(w * h);
	region_index.assign(region_index.size(), -1);

	for ( int i = 0; i < h; ++i )
		for ( int j = 0; j < w; ++j )
			if ( image_data[i * w + j] && region_index[i * w + j] == -1 ){
				std::vector< Point2D > pixel_vec;
				pixel_vec.push_back(Point2D(j, i));

				int pixel_count = 0;
				while ( pixel_vec.size() != 0 ){
					Point2D p = pixel_vec.at(pixel_vec.size() - 1);
					region_index[p.y() * w + p.x()] = region_count;
					pixel_vec.pop_back();
					pixel_count++;

					if ( (p.y() + 1) < h && image_data[(p.y() + 1) * w + p.x()] 
						&& region_index[(p.y() + 1) * w + p.x()] == -1 ) {
							pixel_vec.push_back(Point2D(p.x(), p.y() + 1));
							region_index[(p.y() + 1) * w + p.x()] = region_count;
						}
					if ( (p.y() - 1) >= 0 && image_data[(p.y() - 1) * w + p.x()] 
						&& region_index[(p.y() - 1) * w + p.x()] == -1 ) {
						pixel_vec.push_back(Point2D(p.x(), p.y() - 1));
						region_index[(p.y() - 1) * w + p.x()] = region_count;
					}
					if ( (p.x() + 1) < w && image_data[p.y() * w + p.x() + 1]
						&& region_index[p.y() * w + p.x() + 1] == -1 ){
							pixel_vec.push_back(Point2D(p.x() + 1, p.y()));
							region_index[p.y() * w + p.x() + 1] = region_count;
						}
					if ( (p.x() - 1) >= 0 && image_data[p.y() * w + p.x() - 1]
						&& region_index[p.y() * w + p.x() - 1] == -1 ){
							pixel_vec.push_back(Point2D(p.x() - 1, p.y()));
							region_index[p.y() * w + p.x() - 1] = region_count;
						}
				}
				if ( pixel_count < 10 ){
					for ( int k = 0; k < w * h; ++k ) 
						if ( region_index[k] == region_count ) region_index[k] = 99999;
				} else {
					region_count++;
				}
			}

	return region_count;
}

void ImageGenerator::ExecuteDilation(int w, int h, std::vector< bool >& image_data){
	std::vector< bool > temp_data;
	temp_data.assign(image_data.begin(), image_data.end());

	for ( int i = 0; i < h; ++i )
		for ( int j = 0; j < w; ++j )
			if ( !temp_data[i * w + j] ){
				if ( (i - 1 >= 0 ) && temp_data[(i - 1) * w + j] ) {
					image_data[i * w + j] = true;
					continue;
				}
				if ( (i + 1 < h) && temp_data[(i + 1) * w + j] ) {
					image_data[i * w + j] = true;
					continue;
				}
				if ( (j - 1) >= 0 && temp_data[i * w + j - 1] ){
					image_data[i * w + j] = true;
					continue;
				}
				if ( (j + 1) < w && temp_data[i * w + j + 1] ){
					image_data[i * w + j] = true;
					continue;
				}
			}
}

void ImageGenerator::ExecuteErosion(int w, int h, std::vector< bool >& image_data){
	std::vector< bool > temp_data;
	temp_data.assign(image_data.begin(), image_data.end());

	for ( int i = 0; i < h; ++i )
		for ( int j = 0; j < w; ++j )
			if ( temp_data[i * w + j] ){
				if ( (i - 1 >= 0 ) && !temp_data[(i - 1) * w + j] ) {
					image_data[i * w + j] = false;
					continue;
				}
				if ( (i + 1 < h) && !temp_data[(i + 1) * w + j] ) {
					image_data[i * w + j] = false;
					continue;
				}
				if ( (j - 1) >= 0 && !temp_data[i * w + j - 1] ){
					image_data[i * w + j] = false;
					continue;
				}
				if ( (j + 1) < w && !temp_data[i * w + j + 1] ){
					image_data[i * w + j] = false;
					continue;
				}
			}
}

void ImageGenerator::FitEllipse(int w, int h, int region_count, std::vector< int >& region_index, std::vector< struct Ellipse >& ellipse_info){
	std::vector< std::vector< CvPoint2D32f > > edge_points;
	edge_points.resize(region_count);

	for ( int i = 0; i < h; ++i )
		for ( int j = 0; j < w; ++j )
			if ( region_index[i * w + j] !=- 1 ){
				int region_value = region_index[i * w + j];
				if ( ((i - 1) >= 0 && region_index[(i - 1) * w + j] != region_value)
					|| ((i + 1) < h && region_index[(i + 1) * w + j] != region_value)
					|| ((j - 1) >= 0 && region_index[i * w + j - 1] != region_value)
					|| ((j + 1) < w && region_index[i * w + j + 1] != region_value ) ){
					CvPoint2D32f p;
					p.x = j;
					p.y = i;
					edge_points[region_value].push_back(p);
					continue;
				}
			}

	 CvBox2D32f *box;
	 box = (CvBox2D32f *)malloc(sizeof(CvBox2D32f));

	 for ( int i = 0; i < region_count; ++i ){
		 float center_x = 0, center_y = 0;
		 float ma_x, ma_y, mi_x, mi_y;
		 ma_x = -1000000;
		 ma_y = -1000000;
		 mi_x = 10000000;
		 mi_y = 10000000;
		 for ( int k = 0; k < edge_points[i].size(); ++k ){
			 if ( edge_points[i][k].x > ma_x ) ma_x = edge_points[i][k].x;
			 if ( edge_points[i][k].x < mi_x ) mi_x = edge_points[i][k].x;
			 if ( edge_points[i][k].y > ma_y ) ma_y = edge_points[i][k].y;
			 if ( edge_points[i][k].y < mi_y ) mi_y = edge_points[i][k].y;
		 }
		 center_x = (ma_x + mi_x) / 2;
		 center_y = (ma_y + mi_y) / 2;
		 float max_x, max_y;
		 float max_dis = 0;
		 for ( int k = 0; k < edge_points[i].size(); ++k ){
			 float temp_dis = sqrt(pow(edge_points[i][k].x - center_x, 2) + pow(edge_points[i][k].y - center_y, 2));
			 if ( temp_dis > max_dis ){
				 max_dis = temp_dis;
				 max_x = edge_points[i][k].x - center_x;
				 max_y = edge_points[i][k].y - center_y;
			 }
		 }
		 if ( max_y < 0 ){
			 max_x *= -1;
			 max_y *= -1;
		 }
		 max_x /= max_dis;
		 max_y /= max_dis;
		 float min_x = -1 * max_y;
		 float min_y = max_x;
		 float min_dis = 0;
		 for ( int k = 0; k < edge_points[i].size(); ++k ){
			 float temp_dis = abs((edge_points[i][k].x - center_x) * min_x + (edge_points[i][k].y - center_y) * min_y);
			 if ( temp_dis > min_dis ){
				 min_dis = temp_dis;
			 }
		 }
		 Ellipse temp_ellipse;
		 temp_ellipse.axis_length.SetX(max_dis);
		 temp_ellipse.axis_length.SetY(min_dis);
		 temp_ellipse.center.SetX(center_x);
		 temp_ellipse.center.SetY(center_y);
		 temp_ellipse.angle = acos(max_x) / 3.1415 * 180;
		 ellipse_info.push_back(temp_ellipse);

		 /*cvFitEllipse(edge_points[i].data(), edge_points[i].size(), box);
		 Ellipse temp_ellipse;
		 temp_ellipse.center.SetX(box->center.x);
		 temp_ellipse.center.SetY(box->center.y);
		 temp_ellipse.axis_length.SetX(box->size.width * 0.5);
		 temp_ellipse.axis_length.SetY(box->size.height * 0.5);
		 temp_ellipse.angle = box->angle;
		 ellipse_info.push_back(temp_ellipse);*/
	 }
}


float ImageGenerator::DiffEllipse(struct Ellipse& ellipse_one, struct Ellipse& ellipse_two){
	float max_width = 0;
	if ( ellipse_one.axis_length.x() > max_width ) max_width = ellipse_one.axis_length.x();
	if ( ellipse_two.axis_length.x() > max_width ) max_width = ellipse_two.axis_length.x();
	float max_height = 0;
	if ( ellipse_one.axis_length.y() > max_height ) max_height = ellipse_one.axis_length.y();
	if ( ellipse_two.axis_length.y() > max_height ) max_height = ellipse_two.axis_length.y();

	double dis = abs(ellipse_one.angle - ellipse_two.angle) / 180.0 + abs(ellipse_one.axis_length.x() - ellipse_two.axis_length.x()) / max_width
				+ abs(ellipse_one.axis_length.y() - ellipse_two.axis_length.y()) / max_height + abs(ellipse_one.center.x() - ellipse_two.center.x()) / 20.0
				+ abs(ellipse_one.center.y() - ellipse_two.center.y()) / 20.0;
	dis = dis / 5.0;

	return exp(-1 / (dis + 0.01));
}

float ImageGenerator::DiffImage(BasicImage* image_one, BasicImage* image_two){
	float dis_value = 0;
	for ( int i = 0; i < image_one->width() * image_one->height(); ++i )
		dis_value += abs(image_one->data()[i] - image_two->data()[i]);

	return dis_value;
}

float ImageGenerator::DiffSegementImage(SegmentImage* image_one, SegmentImage* image_two){
    std::vector< float > alpha_channel;
    alpha_channel.resize(image_one->width() * image_one->height(), 0);

    std::vector< float > region_alpha;
    region_alpha.resize(image_one->width() * image_one->height(), 0);

    float counting_thresh = 0.333; 
    for ( int i = 0; i < image_one->region_count.size(); ++i ){
        // region correlation checking
        std::vector< std::vector< float > > correlation_map;
        correlation_map.resize(image_one->region_count[i] + 1);
        for ( int j = 0; j < correlation_map.size(); ++j ) correlation_map[j].resize(image_two->region_count[i] + 1, 0);
        for ( int j = 0; j < image_one->width() * image_one->height(); ++j ) 
            correlation_map[image_one->region_index[i][j] + 1][image_two->region_index[i][j] + 1] += 1;
        for ( int j = 0; j < correlation_map.size(); ++j ){
            float temp_acc = 0;
            for ( int k = 0; k < correlation_map[j].size(); ++k ) temp_acc += correlation_map[j][k];
            if ( temp_acc > 1e-5 )
                for ( int k = 0; k < correlation_map[j].size(); ++k ) correlation_map[j][k] /= temp_acc;
        }

        // calculate the difference value
        std::vector< float > index_alpha_vec;
        for ( int j = 1; j < correlation_map.size(); ++j ){
            float accu_weight = 0;
            float temp_alpha = 0;
            for ( int k = 0; k < correlation_map[j].size(); ++k )
                if ( correlation_map[j][k] > counting_thresh ){
                    accu_weight += correlation_map[j][k];
                    if ( k != 0 )
                        temp_alpha += DiffEllipse(image_one->ellipse_info[i][j - 1], image_two->ellipse_info[i][k - 1]) * correlation_map[j][k];
                    else 
                        temp_alpha += correlation_map[j][k];
                }
                if ( accu_weight > 1e-5 ) temp_alpha /= accu_weight;
                index_alpha_vec.push_back(temp_alpha);
        }
        for ( int j = 0; j < image_one->width() * image_one->height(); ++j )
            if ( image_one->region_index[i][j] != -1 && index_alpha_vec[image_one->region_index[i][j]] > region_alpha[j] ) 
                region_alpha[j] = index_alpha_vec[image_one->region_index[i][j]];

        index_alpha_vec.clear();
        for ( int j = 1; j < correlation_map[0].size(); ++j ){
            float accu_weight = 0;
            float temp_alpha = 0;
            for ( int k = 0; k < correlation_map.size(); ++k )
                if ( correlation_map[k][j] > counting_thresh ){
                    accu_weight += correlation_map[k][j];
                    if ( k != 0 )
                        temp_alpha += DiffEllipse(image_one->ellipse_info[i][k - 1], image_two->ellipse_info[i][j - 1]) * correlation_map[k][j];
                    else 
                        temp_alpha += correlation_map[k][j];
                }
                if ( accu_weight > 1e-5 ) temp_alpha /= accu_weight;
                index_alpha_vec.push_back(temp_alpha);
        }
        for ( int j = 0; j < image_one->width() * image_one->height(); ++j )
            if ( image_two->region_index[i][j] != -1 && index_alpha_vec[image_two->region_index[i][j]] > region_alpha[j] ) 
                region_alpha[j] = index_alpha_vec[image_two->region_index[i][j]];
    }

    float region_factor = 1.0f;
    for ( int i = 0; i < image_one->width() * image_one->height(); ++i) 
        alpha_channel[i] = alpha_channel[i] * (1.0 - region_factor) + region_alpha[i] * region_factor; 

    return (float)rand() / RAND_MAX;
}