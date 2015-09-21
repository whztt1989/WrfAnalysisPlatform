#ifndef WRF_GRID_SIMILARITY_PACK_H_
#define WRF_GRID_SIMILARITY_PACK_H_

#include <vector>
#include <map>

class WrfGridRmsPack {
public:
    WrfGridRmsPack();
    ~WrfGridRmsPack();

    void SetRetrievalGridSize(int size);
    void SetSelectedGridPointIndex(std::vector< int >& index);
    void UpdateDateSelection(std::vector< int >& point_index, std::vector< std::vector< bool > >& selection);

    int FindGrid(int grid_index);

    // [date][grid point][element]
    std::vector< std::vector< std::vector< float > > > date_grid_point_rms;
    // [grid point][date]
    std::vector< std::vector< float > > aggregated_grid_point_rms;
    // [grid point][date]
    std::vector< std::vector< bool > > is_date_selected;

    std::vector< std::vector< float > > sorted_aggregated_rms;
    std::vector< std::vector< int > > sorted_index;

    std::map< int, int > index_map;

private:
    int grid_size_;
};

#endif