/*
 * spectral_clusterer.hpp
 *
 *  Created on: Jun 15, 2015
 *      Author: nicolas
 */

#ifndef SRC_ALGORITHMS_SPECTRAL_CLUSTERER_HPP_
#define SRC_ALGORITHMS_SPECTRAL_CLUSTERER_HPP_

#include <ClusterXX/clustering/clusterer.hpp>
#include <ClusterXX/clustering/clusterer_parameters.hpp>
#include <Eigen/Dense>
#include <memory>

namespace ClusterXX {

class Spectral_Clusterer: public Clusterer {
protected:
	//Data members
	const Eigen::MatrixXd &originalData;
	std::shared_ptr<SpectralParameters> parameters;
	Eigen::MatrixXd distanceMatrix;
	bool dataIsDistanceMatrix;
	Eigen::MatrixXd similarityMatrix;
	Eigen::MatrixXd laplacianMatrix;
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigenSolver;

	//Utility Functions
	void computeSimilarityMatrix();
	virtual void computeLaplacianMatrix() = 0;
	virtual void prepareKMeansData(Eigen::MatrixXd *kMeansData) {}

public:
	Spectral_Clusterer(const Eigen::MatrixXd &_data,
			const std::shared_ptr<ClustererParameters> &_params,
			bool dataIsDistanceMatrix);
	virtual ~Spectral_Clusterer() = default;

	void compute() override;
	void setDistanceMatrix(const Eigen::MatrixXd &_matrix);

	const Eigen::MatrixXd &getDistanceMatrix() {
		return distanceMatrix;
	}

	const Eigen::MatrixXd &getSimilarityMatrix() {
		return similarityMatrix;
	}

	const Eigen::MatrixXd &getLaplacianMatrix() {
		return laplacianMatrix;
	}

	const Eigen::MatrixXd &getEigenvectors() {
		return eigenSolver.eigenvectors();
	}

	const Eigen::VectorXd &getEigenvalues() {
		return eigenSolver.eigenvalues();
	}
};

class UnnormalizedSpectralClustering: public Spectral_Clusterer {
private:
	virtual void computeLaplacianMatrix() override;
public:
	UnnormalizedSpectralClustering(const Eigen::MatrixXd &_data,
			const std::shared_ptr<ClustererParameters> &_params,
			bool dataIsDistanceMatrix = false);
};

class NormalizedSpectralClustering_RandomWalk: public Spectral_Clusterer {
private:
	virtual void computeLaplacianMatrix() override;
public:
	NormalizedSpectralClustering_RandomWalk(const Eigen::MatrixXd &_data,
			const std::shared_ptr<ClustererParameters> &_params,
			bool dataIsDistanceMatrix = false);
};

class NormalizedSpectralClustering_Symmetric: public Spectral_Clusterer {
private:
	virtual void computeLaplacianMatrix() override;
public:
	NormalizedSpectralClustering_Symmetric(const Eigen::MatrixXd &_data,
			const std::shared_ptr<ClustererParameters> &_params,
			bool dataIsDistanceMatrix = false);
	virtual void prepareKMeansData(Eigen::MatrixXd *kMeansData) override;
};

class NormalizedSpectralClustering : public NormalizedSpectralClustering_Symmetric{
public :
	NormalizedSpectralClustering(const Eigen::MatrixXd &_data,
			const std::shared_ptr<ClustererParameters> &_params,
			bool dataIsDistanceMatrix = false);
};

} //End of namespace ClusterXX

#endif /* SRC_ALGORITHMS_SPECTRAL_CLUSTERER_HPP_ */
