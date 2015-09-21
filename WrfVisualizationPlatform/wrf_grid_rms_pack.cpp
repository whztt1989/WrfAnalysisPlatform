#include "wrf_grid_rms_pack.h"

WrfGridRmsPack::WrfGridRmsPack(){

}

WrfGridRmsPack::~WrfGridRmsPack(){

}

void WrfGridRmsPack::SetRetrievalGridSize(int size){
    grid_size_ = size;
}

void WrfGridRmsPack::SetSelectedGridPointIndex(std::vector< int >& index){
    for ( int i = 0; i < index.size(); ++i )
        index_map.insert(std::map< int, int >::value_type(index[i], i));
}

int WrfGridRmsPack::FindGrid(int grid_index){
    std::map< int, int >::iterator iter = index_map.find(grid_index);
    if ( iter != index_map.end() )
        return iter->second;
    else
        return -1;
}

void WrfGridRmsPack::UpdateDateSelection(std::vector< int >& point_index, std::vector< std::vector< bool > >& selection){
    std::map< int, int >::iterator iter;
    for ( int i = 0; i < point_index.size(); ++i ){
        iter = index_map.find(point_index[i]);
        if ( iter == index_map.end() ) continue;
        int temp_index = iter->second;

        if ( selection[i].size() != is_date_selected[temp_index].size() ) continue;
        is_date_selected[temp_index] = selection[i];
    }
}