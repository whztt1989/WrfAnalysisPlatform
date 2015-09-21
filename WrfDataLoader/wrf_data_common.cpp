#include "wrf_data_common.h"

WrfGridValueMap* WrfGridValueMap::ConvertDiscrete2Grid(WrfDiscreteValueMap* discrete_map, WrfGridValueMap* template_map){
    std::cout << "Processing Converting: " << template_map->header_info << std::endl;

    WrfGridValueMap* new_grid_map = new WrfGridValueMap(*template_map);

    std::vector< float > temp_discrete_map_values;
    std::vector< float > increment_map_values;

    temp_discrete_map_values.resize(discrete_map->values.size(), 0);
    increment_map_values.resize(template_map->latitude_grid_number * template_map->longitude_grid_number, 0);

    const float interplation_radius = 1.5;

    // Step Five: Decrease the radius to run step one to step four, until the increment is smaller than stop value or radius is smaller than 1

    // Step One: Interpolate on the base stamp to get the site value
    for ( int i = 0; i < discrete_map->values.size(); ++i )
        temp_discrete_map_values[i] = Interpolate(template_map, discrete_map->values[i].longitude, discrete_map->values[i].latitude, interplation_radius);

    // Step Two: Calculate the bias value of the site value and interpolated site value
    for ( int i = 0; i < discrete_map->values.size(); ++i )
        temp_discrete_map_values[i] -= discrete_map->values[i].value2;

    // Step Three: Adjust the grid increment based on the accumulation average of the nearest sites in a specified radius
    for (int i = 0; i < template_map->latitude_grid_number; ++i )
        for ( int j = 0; j < template_map->longitude_grid_number; ++j ){
            float longitude = template_map->start_longitude + template_map->longitude_grid_space * j;
            float latitude = template_map->start_latitude + template_map->latitude_grid_space * i;

            float value = 0;
            float weight = 0;
            for ( int k = 0; k < discrete_map->values.size(); ++k ){
                float temp_dis = Distance(longitude, latitude, discrete_map->values[k].longitude, discrete_map->values[k].latitude);
                if ( temp_dis < interplation_radius){
                    float temp_weight = (pow(interplation_radius, 2) - pow(temp_dis, 2)) / (pow(interplation_radius, 2) + pow(temp_dis, 2));
                    value += temp_discrete_map_values[k] * temp_weight;
                    weight += temp_weight;
                }
            }
            if ( weight != 0 ) value /= weight;
            increment_map_values[i * template_map->longitude_grid_number + j] = value;
        }

    // Step Four: Adjust the base map
    for ( int i = 0; i < template_map->values.size(); ++i )
        new_grid_map->values[i] -= increment_map_values[i];

    return new_grid_map;
}

WrfGridValueMap* WrfGridValueMap::ConvertDiscrete2Grid(WrfDiscreteValueMap* discrete_map){
    return WrfGridValueMap::ConvertDiscrete2Grid(discrete_map, this);
}

float WrfGridValueMap::Interpolate(WrfGridValueMap* value_map, float longitude, float latitude, float radius){
    int negative_indicator = value_map->start_latitude - value_map->end_latitude < 0 ? 1 : -1;
    int long_index = (int)((longitude - radius - value_map->start_longitude) / value_map->longitude_grid_space);
    int lati_index = (int)((latitude - radius * negative_indicator - value_map->start_latitude) / value_map->latitude_grid_space);

    int long_next_index = (int)((longitude + radius - value_map->start_longitude) / value_map->longitude_grid_space);
    int lati_next_index = (int)((latitude + radius * negative_indicator - value_map->start_latitude) / value_map->latitude_grid_space);
    if ( long_index < 0 ) {
        long_index = 0;
        if ( lati_index < 0 ) lati_index = 0;
        if ( lati_index > value_map->latitude_grid_number - 1 ) lati_index = value_map->latitude_grid_number - 1;
        return value_map->values[lati_index * value_map->longitude_grid_number + long_index];
    }
    if ( long_index >= value_map->longitude_grid_number - 1) {
        long_index = value_map->longitude_grid_number - 1;
        if ( lati_index < 0 ) lati_index = 0;
        if ( lati_index > value_map->latitude_grid_number - 1 ) lati_index = value_map->latitude_grid_number - 1;
        return value_map->values[lati_index * value_map->longitude_grid_number + long_index];
    }
    if ( long_next_index >= value_map->longitude_grid_number - 1 ) long_next_index = value_map->longitude_grid_number - 1;
    if ( lati_index < 0 ) {
        lati_index = 0;
        return value_map->values[lati_index * value_map->longitude_grid_number + long_index];
    }
    if ( lati_index >= value_map->latitude_grid_number - 1) {
        lati_index = value_map->latitude_grid_number - 1;
        return value_map->values[lati_index * value_map->longitude_grid_number + long_index];
    }
    if ( lati_next_index >= value_map->latitude_grid_number - 1) lati_next_index = value_map->latitude_grid_number - 1;

    float weight = 0;
    float value = 0;
    for ( int i = lati_index; i < lati_next_index; ++i )
        for ( int j = long_index; j < long_next_index; ++j ){
            float temp_dis = Distance(j * value_map->longitude_grid_space + value_map->start_longitude,
                i * value_map->latitude_grid_space + value_map->start_latitude,
                longitude, latitude);
            if ( temp_dis > radius ) continue;
            float temp_weight = (pow(radius, 2) - pow(temp_dis, 2)) / (pow(radius, 2) + pow(temp_dis, 2));
            value += value_map->values[i * value_map->longitude_grid_number + j] * temp_weight;
            weight += temp_weight;
        }
        if ( weight != 0 ) value /= weight;

        return value;
}

float WrfGridValueMap::Distance(float x1, float y1, float x2, float y2){
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}