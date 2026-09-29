/*
 * kmeans_clusterer.hpp
 *
 *  Created on: Jun 15, 2015
 *      Author: nicolas
 */

#ifndef SRC_ALGORITHMS_KMEANS_CLUSTERER_HPP_
#define SRC_ALGORITHMS_KMEANS_CLUSTERER_HPP_

#include <ClusterXX/clustering/clusterer.hpp>
#include <ClusterXX/clustering/clusterer_parameters.hpp>
#include <Eigen/Dense>
#include <memory>

namespace ClusterXX {

class Single_KMeans_Clusterer: public Clusterer {
private:
	const Eigen::MatrixXd &data;
	std::shared_ptr<KMeansParameters> parameters;

	Eigen::MatrixXd medoids;
	std::vector<bool> dataToCluster;

	double currentDistortion;

	//Utilities
	void initializeClustersRandomly();
	void recalculateMeans();
	double kMeansIteration();

public:
	Single_KMeans_Clusterer(const Eigen::MatrixXd &_data,
			std::shared_ptr<ClustererParameters> _params);
	Single_KMeans_Clusterer(const Eigen::MatrixXd &_data,
			std::shared_ptr<ClustererParameters> _params,
			std::vector<int> *initialClusters);

	void compute() override;
	Eigen::MatrixXd getMedoids();
	double getDistortion();
};

class Multiple_KMeans_Clusterer: public Clusterer {
private:
	const Eigen::MatrixXd &data;
	std::shared_ptr<KMeansParameters> parameters;
	std::vector<Single_KMeans_Clusterer> kmeansClusterers;
	Eigen::MatrixXd medoids;
	double currentDistortion;
public:
	Multiple_KMeans_Clusterer(const Eigen::MatrixXd &_data,
			std::shared_ptr<ClustererParameters> _params);
	void compute() override;
	Eigen::MatrixXd getMedoids();
	double getDistortion();
};

class KMeans_Clusterer : public Multiple_KMeans_Clusterer{
public:
	KMeans_Clusterer(const Eigen::MatrixXd &_data,
			std::shared_ptr<ClustererParameters> _params) : Multiple_KMeans_Clusterer(_data, _params){
	}
};

} //End of namespace ClusterXX

#endif /* SRC_ALGORITHMS_KMEANS_CLUSTERER_HPP_ */
