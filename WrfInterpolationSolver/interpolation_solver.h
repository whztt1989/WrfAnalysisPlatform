#ifndef INTERPOLATION_SOLVER_H_
#define INTERPOLATION_SOLVER_H_

#include "../WrfDataLoader/wrf_data_common.h"

class ErrorMatrixDataModel;

class InterpolationSolver
{
public:
    InterpolationSolver();
    ~InterpolationSolver();

    /*
    * Normal inverse distance weighting interpolation for two-dimensional map
    * @param input_map
    * @param output_map
    * @param delta smoothing parameter
    * @param beta weighting power
    */
    static void Interpolate(WrfDiscreteValueMap* input_map, WrfDiscreteValueMap* output_map, float radius, float delta, float beta);

    static float MeanError(WrfDiscreteValueMap* input_map, float radius, float delta, float beta, ErrorMatrixDataModel* data_model, int para_index);
};

#endif