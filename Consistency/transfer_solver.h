#ifndef TRANSFER_SOLVER_H_
#define TRANSFER_SOLVER_H_

#include <vector>

class LevelImage;
class TransferObject;

class TransferSolver
{
public:
	TransferSolver();
	~TransferSolver();

	bool Solve(LevelImage* image_one, LevelImage* image_two, std::vector< TransferObject* >& transfer_object_vec);

private:
	LevelImage* image_one;
	LevelImage* image_two;

    int EvaluateRegionsByGrowing(LevelImage* input_image, LevelImage* segment_image, int region_thresh);
    void MatchRegions(LevelImage* image_one, int region_count_one, LevelImage* image_two, int region_count_two, std::vector< std::vector< int > >& region_map);
    bool IsRegionFit(LevelImage* image_one, int region_index_one, LevelImage* image_two, int region_index_two);
};

#endif