/*
 * hierarchical_clusterer.hpp
 *
 *  Created on: Jun 15, 2015
 *      Author: nicolas
 */

#ifndef SRC_ALGORITHMS_HIERARCHICAL_CLUSTERER_HPP_
#define SRC_ALGORITHMS_HIERARCHICAL_CLUSTERER_HPP_

#include <ClusterXX/clustering/clusterer.hpp>
#include <ClusterXX/clustering/clusterer_parameters.hpp>
#include <Eigen/Dense>
#include <memory>
#include <set>

namespace ClusterXX{

class Hierarchical_Clusterer : public Clusterer {
private:
	const Eigen::MatrixXd &originalData;
	std::shared_ptr<HierarchicalParameters> parameters;
	Eigen::MatrixXd distanceMatrix;
	bool dataIsDistanceMatrix;
	std::vector<int> unionFindDataStructure;
	std::set<int> clusterRepresentatives;
	std::vector<int> clusterSizes;
	std::vector<double> distanceMatrixCopy;
	// Utility functions
	void initialize();
	double& getDistance(int i, int j);
	double worstPossibleDistance() const;
	bool isBetterDistance(double oldDistance, double newDistance) const;
	void updateDistances(int deletedCluster, int newCluster);
	void mergeClusters(int i, int j);
	std::pair<int, int> findClustersToMerge();
	int findClusterRepresentative(int i) const;
public:
	Hierarchical_Clusterer(const Eigen::MatrixXd &_data,
			const std::shared_ptr<ClustererParameters> &_params, bool _dataIsDistanceMatrix = false);
	void setDistanceMatrix(const Eigen::MatrixXd &_matrix);
	void compute() override;
	const Eigen::MatrixXd& getDistanceMatrix() const;
};

} //End of namespace ClusterXX

#endif /* SRC_ALGORITHMS_HIERARCHICAL_CLUSTERER_HPP_ */
