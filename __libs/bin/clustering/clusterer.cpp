#include <ClusterXX/clustering/clusterer.hpp>
#include <cassert>
#include <algorithm>
#include <iostream>

std::vector<int> ClusterXX::Clusterer::getClusters() {
	return clusters;
}

double ClusterXX::Clusterer::computeRandIndex(
		const std::vector<int> &clustering1,
		const std::vector<int> &clustering2) {
	assert(clustering1.size() == clustering2.size());
	unsigned int n = clustering1.size();
	int count = 0;
	for (unsigned int i = 0; i < n - 1; ++i) {
		for (unsigned int j = i + 1; j < n; ++j) {
			if ((clustering1[i] == clustering1[j])
					&& clustering2[i] == clustering2[j]) {
				count++;
			} else if ((clustering1[i] != clustering1[j])
					&& clustering2[i] != clustering2[j]) {
				count++;
			}
		}
	}

	auto numberOfPairs = [](int N) -> int {return N*(N-1)/2;};
	return (double) count / ((double) numberOfPairs(n));
}

double ClusterXX::Clusterer::computeRandIndex(
		const std::vector<int> &otherClustering) {
	return computeRandIndex(clusters, otherClustering);
}

double ClusterXX::Clusterer::computeAdjustedRandIndex(
		const std::vector<int> &clustering1,
		const std::vector<int> &clustering2) {
	assert(clustering1.size() == clustering2.size());
	unsigned int n = clustering1.size();

	int K1 = *max_element(clustering1.cbegin(), clustering1.cend()) + 1;
	int K2 = *max_element(clustering2.cbegin(), clustering2.cend()) + 1;

	std::vector<int> contingencyTable(K1 * K2, 0);
	std::vector<int> count1(K1, 0);
	std::vector<int> count2(K2, 0);

	for (unsigned int i = 0; i != n; ++i) {
		int c1 = clustering1[i];
		int c2 = clustering2[i];
		++contingencyTable[c1 * K2 + c2];
		++count1[c1];
		++count2[c2];
	}

	auto numberOfPairs = [](int n) -> int {return n*(n-1)/2;};

	std::transform(contingencyTable.begin(), contingencyTable.end(),
			contingencyTable.begin(), numberOfPairs);
	std::transform(count1.begin(), count1.end(), count1.begin(), numberOfPairs);
	transform(count2.begin(), count2.end(), count2.begin(), numberOfPairs);

	double index = (double) accumulate(contingencyTable.cbegin(),
			contingencyTable.cend(), 0.0);
	double temp1 = (double) accumulate(count1.cbegin(), count1.cend(), 0.0);
	double temp2 = (double) accumulate(count2.cbegin(), count2.cend(), 0.0);
	double maxIndex = 0.5 * (temp1 + temp2);
	double expectedIndex = temp1 * temp2 / (double) numberOfPairs(n);

	return (index - expectedIndex) / (maxIndex - expectedIndex);
}

double ClusterXX::Clusterer::computeAdjustedRandIndex(
		const std::vector<int> &otherClustering) {
	return computeAdjustedRandIndex(clusters, otherClustering);
}

void ClusterXX::Clusterer::printClusteringMatrix(
		const std::vector<std::string> &realLabels,
		const std::vector<int> &realClusters) {
	std::cout << std::endl << "****** Clustering results : ******" << std::endl;
	std::cout << "Classes : " << std::endl;
	for (unsigned int i = 0; i < realLabels.size(); ++i) {
		std::cout << i << " : " << realLabels[i] << std::endl;
	}

	unsigned int realClustersN = realLabels.size();
	unsigned int computedClustersN = *std::max_element(clusters.cbegin(),
			clusters.cend()) + 1;
	std::vector<int> clusteringGraph(realClustersN * computedClustersN, 0);

	assert(realClusters.size() == clusters.size());

	for (unsigned int i = 0; i < realClusters.size(); ++i) {
		int realCluster = realClusters[i];
		int computedCluster = clusters[i];
		clusteringGraph[realCluster * computedClustersN + computedCluster]++;
	}

	std::cout << "---\t";
	for (unsigned int j = 0; j < computedClustersN; ++j) {
		std::cout << "#" << j << "\t";
	}
	std::cout << "SUM\t" << std::endl;

	for (unsigned int i = 0; i < realClustersN; ++i) {
		std::cout << "#" << i << "\t";
		unsigned int sum = 0;
		for (unsigned int j = 0; j < computedClustersN; ++j) {
			sum += clusteringGraph[i * computedClustersN + j];
			std::cout << clusteringGraph[i * computedClustersN + j] << "\t";
		}
		std::cout << sum << std::endl;
	}

	unsigned int global_sum = 0;
	std::cout << "SUM" << "\t";
	for (unsigned int j = 0; j < computedClustersN; ++j) {
		unsigned int sum = 0;

		for (unsigned int i = 0; i < realClustersN; ++i) {
			sum += clusteringGraph[i * computedClustersN + j];
		}
		global_sum += sum;
		std::cout << sum << "\t";
	}

	std::cout << global_sum << std::endl;
}

void ClusterXX::Clusterer::printRawClustering(
		const std::vector<std::string> &labels) {
	assert(labels.size() == clusters.size());
	std::cout << "{" << std::endl;
	for (unsigned int i = 0; i < labels.size(); ++i) {
		std::cout << labels[i] << "\t" << clusters[i] << std::endl;
	}
	std::cout << "}" << std::endl;
}
