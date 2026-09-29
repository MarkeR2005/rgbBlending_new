/*
 * spectral_clusterer.cpp
 *
 *  Created on: Jun 15, 2015
 *      Author: nicolas
 */

#include <ClusterXX/clustering/clusterer_parameters.hpp>
#include <ClusterXX/clustering/kmeans_clusterer.hpp>
#include <ClusterXX/clustering/spectral_clusterer.hpp>
#include <ClusterXX/utils/utils.hpp>
#include <ClusterXX/utils/math.hpp>
#include <iostream>
#include <algorithm>
#include <Eigen/Eigenvalues>

ClusterXX::Spectral_Clusterer::Spectral_Clusterer(const Eigen::MatrixXd &_data,
		const std::shared_ptr<ClustererParameters> &_params,
		bool _dataIsDistanceMatrix) :
		originalData(_data), dataIsDistanceMatrix(_dataIsDistanceMatrix) {
	parameters = std::dynamic_pointer_cast<SpectralParameters>(_params);
	if (!parameters) {
		throw std::invalid_argument(
				"Could not convert ClustererParameters pointer into HierarchicalParameters pointer.");
	}
}

void ClusterXX::Spectral_Clusterer::computeSimilarityMatrix() {

	if (!dataIsDistanceMatrix) {
		if (parameters->getVerbose()) {
			std::cout
					<< "Computing the Distance Matrix, this can take a while... "
					<< std::flush;
		}

		distanceMatrix = parameters->getMetric()->computeMatrix(originalData);

		if (parameters->getVerbose()) {
			std::cout << "Done." << std::endl;
		}
	}

	else {
		distanceMatrix = originalData;
	}

	//Assert fails otherwise because of double precision
	for (unsigned int i = 0; i < distanceMatrix.cols(); ++i) {
		distanceMatrix(i, i) = 0;
	}

	//Check that all coefficients are positive
	if (!std::all_of(distanceMatrix.data(),
			distanceMatrix.data() + distanceMatrix.size(),
			Utilities::Math::greaterThanZero)) {
		std::cout << "Not all the matrix coefficients are positive."
				<< std::endl;
		assert(false);
	}

	//Assign correct values in the diagonal
	unsigned int N = originalData.cols();
	for (unsigned int i = 0; i < N; ++i) {
		//Distance matrix, set to infinity
		if (parameters->getMetric()->isDistanceMetric()) {
			distanceMatrix(i, i) = std::numeric_limits<double>::infinity();
		}
		//Similarity matrix, set to 0
		else {
			distanceMatrix(i, i) = 0;
		}
	}

	//Initialize similarity Matrix
	if (parameters->getMetric()->isDistanceMetric()
			&& parameters->getTransformationMethod().getMethodName()
					== SpectralParameters::GraphTransformationMethod::NO_TRANSFORMATION) {
		std::cout
				<< "Cannot compute the similarity graph for a distance matrix with no transformation."
				<< std::endl;
		assert(false);
	}

	else if (!parameters->getMetric()->isDistanceMetric()
			&& parameters->getTransformationMethod().getMethodName()
					== SpectralParameters::GraphTransformationMethod::NO_TRANSFORMATION) {
		std::cout << "Using raw Similarity Matrix." << std::endl;
		similarityMatrix = distanceMatrix;
	}

	else if (parameters->getTransformationMethod().getMethodName()
			== SpectralParameters::GraphTransformationMethod::K_NEAREST_NEIGHBORS) {

		if (parameters->getVerbose()) {
			std::cout << "Computing the K Nearest Neighbor Similarity Matrix."
					<< std::endl;
		}

		similarityMatrix = Eigen::MatrixXd::Zero(N, N);

		for (unsigned int i = 0; i < N; ++i) {
			std::vector<size_t> sortedIndexes;
			if (parameters->getMetric()->isDistanceMetric()) {
				sortedIndexes = Utilities::sort_indexes_increasing(
						Utilities::eigen2Stl(distanceMatrix.col(i)));
			} else {
				sortedIndexes = Utilities::sort_indexes_decreasing(
						Utilities::eigen2Stl(distanceMatrix.col(i)));
			}
			for (unsigned int k = 0;
					k
							< parameters->getTransformationMethod().getKNearestNeighbors();
					++k) {
				unsigned int indexNeighbor = sortedIndexes[k];
				similarityMatrix(i, indexNeighbor) = 1.0;
				similarityMatrix(indexNeighbor, i) = 1.0;
			}
		}
	}

	else if (parameters->getMetric()->isDistanceMetric()
			&& parameters->getTransformationMethod().getMethodName()
					== SpectralParameters::GraphTransformationMethod::GAUSSIAN_MIXTURE) {
		double stdDev =
				parameters->getTransformationMethod().getGaussianModelStdDev();
		assert(stdDev > 0);
		auto gaussianMixtureFunction = [stdDev](double dist) -> double {
			return std::exp(-1 * dist/(2 * std::pow(stdDev, 2.0)));
		};
		//Apply unary expression
		similarityMatrix = distanceMatrix.unaryExpr(gaussianMixtureFunction);
	}

	else {
		//Should not happen
		assert(false);
	}
}

void ClusterXX::Spectral_Clusterer::compute() {
	unsigned int N = originalData.cols();

	computeSimilarityMatrix();

	if (parameters->getVerbose()) {
		std::cout << "Computing the Laplacian Matrix." << std::endl;
	}
	computeLaplacianMatrix();

	if (parameters->getVerbose()) {
		std::cout << "Solving the Eigen Problem." << std::endl;
	}

	eigenSolver.compute(laplacianMatrix);
	Eigen::MatrixXd dataForKMeans = eigenSolver.eigenvectors().block(0, 0, N,
			parameters->getK()).transpose();

	prepareKMeansData(&dataForKMeans);

	if (parameters->getVerbose()) {
		std::cout << "Eigenvalues : " << std::endl
				<< getEigenvalues().transpose() << std::endl;
	}

	//TODO get rid of ugly constants
	std::shared_ptr<ClustererParameters> kMeansParams = std::make_shared<
			KMeansParameters>(parameters->getK(), 1000, 25, false);

	if (parameters->getVerbose()) {
		std::cout << "Calling KMeans." << std::endl;
	}

	KMeans_Clusterer kmeansClusterer(dataForKMeans, kMeansParams);
	kmeansClusterer.compute();
	clusters = kmeansClusterer.getClusters();
}

ClusterXX::UnnormalizedSpectralClustering::UnnormalizedSpectralClustering(
		const Eigen::MatrixXd &_data,
		const std::shared_ptr<ClustererParameters> &_params,
		bool dataIsDistanceMatrix) :
		Spectral_Clusterer(_data, _params, dataIsDistanceMatrix) {
}

void ClusterXX::UnnormalizedSpectralClustering::computeLaplacianMatrix() {
	unsigned int N = originalData.cols();
	Eigen::MatrixXd D = Eigen::MatrixXd::Zero(N, N);
	for (unsigned int i = 0; i < N; ++i) {
		D(i, i) = similarityMatrix.col(i).sum();
	}
	laplacianMatrix = D - similarityMatrix;
}

ClusterXX::NormalizedSpectralClustering_RandomWalk::NormalizedSpectralClustering_RandomWalk(
		const Eigen::MatrixXd &_data,
		const std::shared_ptr<ClustererParameters> &_params,
		bool dataIsDistanceMatrix) :
		Spectral_Clusterer(_data, _params, dataIsDistanceMatrix) {
}

void ClusterXX::NormalizedSpectralClustering_RandomWalk::computeLaplacianMatrix() {
	unsigned int N = originalData.cols();
	Eigen::MatrixXd D = Eigen::MatrixXd::Zero(N, N);
	for (unsigned int i = 0; i < N; ++i) {
		D(i, i) = similarityMatrix.col(i).sum();
	}
	laplacianMatrix = Eigen::MatrixXd::Identity(N, N)
			- D.inverse() * similarityMatrix;
}

ClusterXX::NormalizedSpectralClustering_Symmetric::NormalizedSpectralClustering_Symmetric(
		const Eigen::MatrixXd &_data,
		const std::shared_ptr<ClustererParameters> &_params,
		bool dataIsDistanceMatrix) :
		Spectral_Clusterer(_data, _params, dataIsDistanceMatrix) {
}

void ClusterXX::NormalizedSpectralClustering_Symmetric::computeLaplacianMatrix() {
	unsigned int N = originalData.cols();
	Eigen::MatrixXd D_1_2 = Eigen::MatrixXd::Zero(N, N);
	for (unsigned int i = 0; i < N; ++i) {
		D_1_2(i, i) = std::pow(similarityMatrix.col(i).sum(), -0.5);
	}
	laplacianMatrix = Eigen::MatrixXd::Identity(N, N)
			- D_1_2 * similarityMatrix * D_1_2;
}

void ClusterXX::NormalizedSpectralClustering_Symmetric::prepareKMeansData(
		Eigen::MatrixXd *kMeansData) {
	kMeansData->colwise().normalize();
}

ClusterXX::NormalizedSpectralClustering::NormalizedSpectralClustering(
		const Eigen::MatrixXd &_data,
		const std::shared_ptr<ClustererParameters> &_params,
		bool dataIsDistanceMatrix) :
		NormalizedSpectralClustering_Symmetric(_data, _params,
				dataIsDistanceMatrix) {
}
