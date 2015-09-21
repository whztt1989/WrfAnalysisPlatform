#include "interpolation_solver.h"
#include "mean_error_matrix.h"
#include <assert.h>

InterpolationSolver::InterpolationSolver(){

}

InterpolationSolver::~InterpolationSolver(){

}

void InterpolationSolver::Interpolate(WrfDiscreteValueMap* input_map, WrfDiscreteValueMap* output_map, float radius, float delta, float beta){
    for ( int i = 0; i < output_map->values.size(); ++i ){
        float x = output_map->values[i].longitude;
        float y = output_map->values[i].latitude;
        
        float accu_weight = 0;
        float accu_value = 0;
        for ( int j = 0; j < input_map->values.size(); ++j ){
            float temp_dis = sqrt(pow(input_map->values[j].longitude - x, 2) + pow(input_map->values[j].latitude - y, 2));
            if ( temp_dis < radius ){
                float temp_weight = 1.0 / pow(temp_dis + delta, beta);
                accu_value += input_map->values[j].value1 * temp_weight;
                accu_weight += temp_weight;
            }
        }

        if ( accu_weight > 1e-10 ){
            output_map->values[i].value1 = accu_value / accu_weight;
        } else {
            output_map->values[i].value1 = accu_value;
        }
    }
}

float InterpolationSolver::MeanError(WrfDiscreteValueMap* input_map, float radius, float delta, float beta, ErrorMatrixDataModel* data_model, int para_index){
    float mean_error = 0;

    for ( int i = 0; i < input_map->values.size(); ++i ){
        float x = input_map->values[i].longitude;
        float y = input_map->values[i].latitude;

        float accu_weight = 0;
        float accu_value = 0;
        for ( int j = 0; j < input_map->values.size(); ++j ){
            if ( j == i ) continue;
            float temp_dis = sqrt(pow(input_map->values[j].longitude - x, 2) + pow(input_map->values[j].latitude - y, 2));
            if ( temp_dis < radius ){
                float temp_weight = 1.0 / pow(temp_dis + delta, beta);
                accu_value += input_map->values[j].value2 * temp_weight;
                accu_weight += temp_weight;
            }
        }

        if ( accu_weight > 1e-10 ){
            accu_value /= accu_weight;
        } 

        data_model->volume_values[i][para_index] = abs(accu_value - input_map->values[i].value2);
        mean_error += abs(accu_value - input_map->values[i].value2);

        assert(mean_error < 100000 && mean_error >= 0);
    }

    assert(mean_error < 100000 && mean_error >= 0);

    mean_error = sqrt(mean_error / input_map->values.size());

    assert(mean_error < 100000 && mean_error >= 0);

    return mean_error;
}