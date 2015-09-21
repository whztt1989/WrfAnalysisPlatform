#include "transfer_solver.h"
#include "consistency_common.h"
#include "transfer_object.h"

TransferSolver::TransferSolver(){

}

TransferSolver::~TransferSolver(){

}

bool TransferSolver::Solve(LevelImage* image_one, LevelImage* image_two, std::vector< TransferObject* >& transfer_object_vec){
    LevelImage* segment_image_one = new LevelImage();
    segment_image_one->width = image_one->width;
    segment_image_one->height = image_one->height;
    segment_image_one->data = new int[image_one->width * image_one->height];
    int segment_count_one = EvaluateRegionsByGrowing(image_one, segment_image_one, 20);

    LevelImage* segment_image_two = new LevelImage();
    segment_image_two->width = image_two->width;
    segment_image_two->height = image_two->height;
    segment_image_two->data = new int[image_two->width * image_two->height];
    int segment_count_two = EvaluateRegionsByGrowing(image_two, segment_image_two, 20);

    std::vector< std::vector< int > > region_map;
    MatchRegions(segment_image_one, segment_count_one, segment_image_two, segment_count_two, region_map);

    // Generate transfer objects

	return false;
}

bool TransferSolver::IsRegionFit(LevelImage* image_one, int region_index_one, LevelImage* image_two, int region_index_two){
    int pixel_count_one = 0;
    int pixel_count_two = 0;

    return false;
}

void TransferSolver::MatchRegions(LevelImage* image_one, int region_count_one, LevelImage* image_two, int region_count_two, std::vector< std::vector< int > >& region_map){
    region_map.clear();
    region_map.resize(region_count_one);

    for ( int i = 0; i < region_count_one; ++i )
        for ( int j = 0; j < region_count_two; ++j )
            if ( IsRegionFit(image_one, i, image_two, j) ) region_map[i].push_back(j);
}

int TransferSolver::EvaluateRegionsByGrowing(LevelImage* input_image, LevelImage* segment_image, int region_thresh){
    if ( input_image == NULL || segment_image == NULL ) return -1;

    int current_segment_index = 0;
    int segement_count = 0;
    for ( int i = 0; i < input_image->width * input_image->height; ++i ) input_image->data[i] = -1;

    std::vector< QPoint > point_vec;
    point_vec.resize(input_image->width * input_image->height);
    int growing_index = 0;
    std::vector< QPoint > region_index;
    region_index.resize(input_image->width * input_image->height);
    int region_size_index = 0;

    for ( int i = 0; i < input_image->height; ++i )
        for ( int j = 0; j < input_image->width; ++j )
            if ( segment_image->data[i * input_image->width + j] == -1 ){
                point_vec[growing_index] = QPoint(j, i);
                growing_index++;
                region_index[region_size_index] = QPoint(j, i);
                region_size_index++;

                while ( growing_index > 0 ){
                    int x = point_vec[growing_index - 1].x();
                    int y = point_vec[growing_index - 1].y();
                    int point_index = y * input_image->width + x;
                    segment_image->data[y * input_image->width + x] = segement_count;
                    growing_index--;

                    if ( y + 1 < input_image->height ){
                        int temp_index = (y + 1) * input_image->width + x;
                        if ( segment_image->data[temp_index] == -1 && input_image->data[temp_index] == input_image->data[point_index] ){
                            point_vec[growing_index] = QPoint(x, y + 1);
                            growing_index++;
                            region_index[region_size_index] = QPoint(x, y + 1);
                            region_size_index++;
                        }
                    }

                    if ( y - 1 >= 0 ){
                        int temp_index = (y - 1) * input_image->width + x;
                        if ( segment_image->data[temp_index] == -1 && input_image->data[temp_index] == input_image->data[point_index] ){
                            point_vec[growing_index] = QPoint(x, y - 1);
                            growing_index++;
                            region_index[region_size_index] = QPoint(x, y - 1);
                            region_size_index++;
                        }
                    }

                    if ( x + 1 < input_image->width ){
                        int temp_index = y * input_image->width + x + 1;
                        if ( segment_image->data[temp_index] == -1 && input_image->data[temp_index] == input_image->data[point_index] ){
                            point_vec[growing_index] = QPoint(x + 1, y);
                            growing_index++;
                            region_index[region_size_index] = QPoint(x + 1, y);
                            region_size_index++;
                        }
                    }

                    if ( x - 1 >= 0  ){
                        int temp_index = y * input_image->width + x - 1;
                        if ( segment_image->data[temp_index] == -1 && input_image->data[temp_index] == input_image->data[point_index] ){
                            point_vec[growing_index] = QPoint(x - 1, y);
                            growing_index++;
                            region_index[region_size_index] = QPoint(x - 1, y);
                            region_size_index++;
                        }
                    }
                }
                if ( region_size_index < region_thresh ){
                    for ( int k = 0; k < region_size_index; ++k ){
                        int x = region_index[k].x();
                        int y = region_index[k].y();
                        int temp_index = y * input_image->width + x;
                        segment_image->data[temp_index] = 9999;
                    }
                } else {
                     segement_count++;
                }
                region_size_index = 0;
            }

    return segement_count;
}