/*
 * hierarchical_clusterer.cpp
 *
 *  Created on: Jun 15, 2015
 *      Author: nicolas
 */

#include <ClusterXX/clustering/clusterer_parameters.hpp>
#include <ClusterXX/clustering/hierarchical_clusterer.hpp>
#include <iostream>
#include <cassert>
#include <utility>

#include <ClusterXX/utils/utils.hpp>

ClusterXX::Hierarchical_Clusterer::Hierarchical_Clusterer(
		const Eigen::MatrixXd &_data,
		const std::shared_ptr<ClustererParameters> &_params,
		bool _dataIsDistanceMatrix) :
		originalData(_data), dataIsDistanceMatrix(_dataIsDistanceMatrix) {
	parameters = std::dynamic_pointer_cast<HierarchicalParameters>(_params);
	if (!parameters) {
		throw std::invalid_argument(
				"Could not convert ClustererParameters pointer into HierarchicalParameters pointer.");
	}
}

void ClusterXX::Hierarchical_Clusterer::initialize() {
	unsigned int N = originalData.cols();

	//initializing union find data structures
	unionFindDataStructure.resize(N);
	std::fill(unionFindDataStructure.begin(), unionFindDataStructure.end(), -1);
	for (unsigned int i = 0; i < N; ++i) {
		clusterRepresentatives.insert(i);
	}

	if (parameters->getLinkageMethod() == HierarchicalParameters::AVERAGE) {
		clusterSizes.resize(N);
		std::fill(clusterSizes.begin(), clusterSizes.end(), 1);
	}

	if (!dataIsDistanceMatrix) {
		if (parameters->getVerbose()) {
			std::cout << "Computing distance matrix, this can take a while... "
					<< std::flush;
		}
		distanceMatrix = parameters->getMetric()->computeMatrix(originalData);
		if (parameters->getVerbose()) {
			std::cout << "Done." << std::endl;
		}
	} else {
		//Simply copy the data
		distanceMatrix = originalData;
	}

	//Copy matrix into vector with half its size
	for (unsigned int i = 0; i < N; ++i) {
		for (unsigned int j = 0; j <= i; ++j) {
			double d = distanceMatrix(i, j);
			distanceMatrixCopy.push_back(d);
		}
	}

	assert(distanceMatrixCopy.size() == N * (N + 1) / 2);
}

double& ClusterXX::Hierarchical_Clusterer::getDistance(int i, int j) {
	if (j > i) {
		std::swap(i, j);
	}
	return distanceMatrixCopy[i * (i + 1) / 2 + j];
}

double ClusterXX::Hierarchical_Clusterer::worstPossibleDistance() const {
	if (parameters->getMetric()->isDistanceMetric()) {
		return std::numeric_limits<double>::infinity();
	} else {
		return -1.0 * std::numeric_limits<double>::infinity();
	}
}

bool ClusterXX::Hierarchical_Clusterer::isBetterDistance(double oldDistance,
		double newDistance) const {
	if (parameters->getMetric()->isDistanceMetric()) {
		return newDistance < oldDistance;
	} else {
		return newDistance > oldDistance;
	}
}

std::pair<int, int> ClusterXX::Hierarchical_Clusterer::findClustersToMerge() {
	std::pair<int, int> clustersToMerge { -1, -1 };
	double bestDistance = worstPossibleDistance();
	for (int i : clusterRepresentatives) {
		for (int j : clusterRepresentatives) {
			if (i != j) {
				double d = getDistance(i, j);
				if (isBetterDistance(bestDistance, d)) {
					bestDistance = d;
					clustersToMerge = std::make_pair(i, j);
				}
			}
		}
	}

	return clustersToMerge;
}

void ClusterXX::Hierarchical_Clusterer::updateDistances(int deletedCluster,
		int mergedCluster) {
	for (int i : clusterRepresentatives) {
		if (i != mergedCluster) {
			double dist1 = getDistance(i, deletedCluster);
			double dist2 = getDistance(i, mergedCluster);
			if (parameters->getLinkageMethod()
					== HierarchicalParameters::COMPLETE) {
				if (parameters->getMetric()->isDistanceMetric()) {
					getDistance(i, mergedCluster) = std::max(dist1, dist2);
				} else {
					getDistance(i, mergedCluster) = std::min(dist1, dist2);
				}
			} else if (parameters->getLinkageMethod()
					== HierarchicalParameters::SINGLE) {
				if (parameters->getMetric()->isDistanceMetric()) {
					getDistance(i, mergedCluster) = std::min(dist1, dist2);
				} else {
					getDistance(i, mergedCluster) = std::max(dist1, dist2);
				}
			} else if (parameters->getLinkageMethod()
					== HierarchicalParameters::AVERAGE) {
				double deletedClusterSize = clusterSizes[deletedCluster];
				double mergedClusterSize = clusterSizes[mergedCluster];
				getDistance(i, mergedCluster) = (deletedClusterSize * dist1
						+ mergedClusterSize * dist2)
						/ (deletedClusterSize + mergedClusterSize);
			} else {
				assert(false);
			}
		}
	}
}

void ClusterXX::Hierarchical_Clusterer::mergeClusters(int i, int j) {
	clusterRepresentatives.erase(i);
	unionFindDataStructure[i] = j;
	updateDistances(i, j);
	if (parameters->getLinkageMethod() == HierarchicalParameters::AVERAGE) {
		clusterSizes[j] += clusterSizes[i];
	}
}

int ClusterXX::Hierarchical_Clusterer::findClusterRepresentative(int i) const {
	int next = unionFindDataStructure[i];
	while (next != -1) {
		i = next;
		next = unionFindDataStructure[next];
	}
	return i;
}

void ClusterXX::Hierarchical_Clusterer::compute() {
	initialize();
	unsigned int K = parameters->getK();
	assert(K <= clusterRepresentatives.size() && K >= 2);
	int mergesToPerform = clusterRepresentatives.size() - K;
	int countIterations = 0;
	if (parameters->getVerbose()) {
		std::cout << "Advancement of hierarchical clustering : " << std::endl;
	}
	while (clusterRepresentatives.size() > K) {
		std::pair<int, int> clusterToMerge = findClustersToMerge();
		mergeClusters(clusterToMerge.first, clusterToMerge.second);
		++countIterations;
		if (parameters->getVerbose()) {
			Utilities::printAdvancement(countIterations, mergesToPerform);
		}
	}

	unsigned int N = originalData.cols();
	clusters.resize(N);
	std::map<int, unsigned int> map = Utilities::buildIndexMap<int>(
			clusterRepresentatives.cbegin(), clusterRepresentatives.cend());
	for (unsigned int i = 0; i < N; ++i) {
		int j = findClusterRepresentative(i);
		clusters[i] = map[j];
	}
}

const Eigen::MatrixXd& ClusterXX::Hierarchical_Clusterer::getDistanceMatrix() const {
	return distanceMatrix;
}
