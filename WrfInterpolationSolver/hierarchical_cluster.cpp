#include "hierarchical_cluster.h"
#include "../WrfDataLoader/wrf_data_common.h"

HierarchicalCluster::HierarchicalCluster(){

}

HierarchicalCluster::~HierarchicalCluster(){

}

void HierarchicalCluster::Cluster(WrfDiscreteValueMap* value_map, std::vector< std::vector< float > >& values, std::vector< std::vector< bool > >& linking_status, int cluster_number,
    std::vector< std::vector< float > >& cluster_center, std::vector< std::vector< int > >& cluster_nodes){
    
    std::vector< int > cluster_index;
    std::vector< int > center_index;
    cluster_index.resize(values.size());
    for ( int i = 0; i < cluster_index.size(); ++i ) cluster_index[i] = i;
    cluster_center.assign(values.begin(), values.end());
    center_index.resize(cluster_center.size());
    for ( int i = 0; i < center_index.size(); ++i ) center_index[i] = i;

    int nearest_index[2];
    while ( cluster_center.size() > cluster_number ){
        int min_dis_pair[2];
        min_dis_pair[0] = -1;
        min_dis_pair[1] = -1;
        float min_distance = 1e10;
        for ( int i = 0; i < cluster_center.size() - 1; ++i )
            for ( int j = i + 1; j < cluster_center.size(); ++j ){
                // check linking status
                bool is_linking = false;
                for ( int k = 0; k < cluster_index.size(); ++k )
                    if ( cluster_index[k] == center_index[i] ){
                        for ( int t = 0; t < cluster_index.size(); ++t )
                            if ( t != k && cluster_index[t] == center_index[j] && linking_status[k][t] ){
                                is_linking = true;
                                break;
                            }
                        if ( is_linking ) break;
                    }
                if ( !is_linking ) continue;

                float temp_dis = 0;
                for ( int k = 0; k < cluster_center[i].size(); ++k )
                    temp_dis += abs(cluster_center[i][k] - cluster_center[j][k]);
                if ( temp_dis < min_distance ){
                    min_distance = temp_dis;
                    min_dis_pair[0] = i;
                    min_dis_pair[1] = j;
                }
            }
        if ( min_dis_pair[0] == -1 || min_dis_pair[1] == -1 ) break;
        memset(cluster_center[min_dis_pair[0]].data(), 0, cluster_center[0].size() * sizeof(float));
        int cluster_node_count = 0;
        for ( int i = 0; i < values.size(); ++i )
            if ( cluster_index[i] == center_index[min_dis_pair[0]] || cluster_index[i] == center_index[min_dis_pair[1]] ){
                cluster_index[i] = center_index[min_dis_pair[0]];
                for ( int j = 0; j < cluster_center[0].size(); ++j )
                    cluster_center[min_dis_pair[0]][j] += values[i][j];
                cluster_node_count++;
            }
        for ( int j = 0; j < cluster_center[0].size(); ++j )
            cluster_center[min_dis_pair[0]][j] /= cluster_node_count;

        for ( int i = min_dis_pair[1]; i < cluster_center.size() - 1; ++i ){
            for ( int j = 0; j < cluster_center[0].size(); ++j )
                cluster_center[i][j] = cluster_center[i + 1][j];
            center_index[i] = center_index[i + 1];
        }
        cluster_center.resize(cluster_center.size() - 1);
        center_index.resize(center_index.size() - 1);
    }

    cluster_nodes.resize(cluster_center.size());
    for ( int i = 0; i < cluster_center.size(); ++i )
        for ( int j = 0; j < values.size(); ++j )
            if ( cluster_index[j] == center_index[i] ){
                cluster_nodes[i].push_back(j);
            }
}