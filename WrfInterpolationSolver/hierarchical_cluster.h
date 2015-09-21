#ifndef HIERARCHICAL_CLUSTER_H_
#define HIERARCHICAL_CLUSTER_H_

#include <vector>

class WrfDiscreteValueMap;

class HierarchicalCluster
{
public:
    HierarchicalCluster();
    ~HierarchicalCluster();

    static void Cluster(WrfDiscreteValueMap* value_map, std::vector< std::vector< float > >& values, std::vector< std::vector< bool > >& linking_status, int cluster_number,
        std::vector< std::vector< float > >& cluster_center, std::vector< std::vector< int > >& cluster_nodes);
};

#endif