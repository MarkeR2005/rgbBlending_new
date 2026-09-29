#include <cassert>
#include <memory>
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>

#include <Eigen/Dense>
#include <ClusterXX/metrics/metrics.hpp>
#include <ClusterXX/utils/utils.hpp>
#include <ClusterXX/utils/math.hpp>

using Eigen::MatrixXd;
using Eigen::VectorXd;

double ClusterXX::SquaredEuclideanDistance::compute(const Eigen::VectorXd &left,
		const Eigen::VectorXd &right) const {
	return (left - right).squaredNorm();
}

MatrixXd ClusterXX::SquaredEuclideanDistance::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Squared Euclidean Distance..." << std::endl;
	}
	unsigned int N = X.cols();
	unsigned int M = Y.cols();
	assert(X.rows() == Y.rows());

	MatrixXd D = MatrixXd::Zero(N, M);
	D.colwise() += X.colwise().squaredNorm().transpose().col(0);
	D.rowwise() += Y.colwise().squaredNorm().row(0);
	return D - 2 * X.transpose() * Y;
}

MatrixXd ClusterXX::SquaredEuclideanDistance::computeMatrix(
		const MatrixXd &X) const {
	if (getVerbose()) {
		std::cout << "Computing Squared Euclidean Distance..." << std::endl;
	}
	unsigned int N = X.cols();
	MatrixXd D;
	D = MatrixXd::Zero(N, N);
	D.colwise() += X.colwise().squaredNorm().transpose().col(0);
	D.rowwise() += X.colwise().squaredNorm().row(0);
	return D - 2 * (X.transpose() * X);
}

double ClusterXX::EuclideanDistance::compute(const Eigen::VectorXd &left,
		const Eigen::VectorXd &right) const {
	return std::sqrt(SquaredEuclideanDistance().compute(left, right));
}

MatrixXd ClusterXX::EuclideanDistance::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Euclidean Distance..." << std::endl;
	}
	std::cout << "OK" << std::endl;
	return SquaredEuclideanDistance().computeMatrix(X, Y).array().sqrt();
}

MatrixXd ClusterXX::EuclideanDistance::computeMatrix(const MatrixXd &X) const {
	if (getVerbose()) {
		std::cout << "Computing Euclidean Distance..." << std::endl;
	}
	return SquaredEuclideanDistance().computeMatrix(X).array().sqrt();
}

double ClusterXX::ManhattanDistance::compute(const Eigen::VectorXd &left,
		const Eigen::VectorXd &right) const {
	return (left - right).lpNorm<1>();
}

MatrixXd ClusterXX::ManhattanDistance::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Manhattan Distance..." << std::endl;
	}
	unsigned int N = X.cols();
	unsigned int M = Y.cols();
	MatrixXd D(N, M);
	assert(X.rows() == Y.rows());
	for (unsigned int i = 0; i < N; ++i) {
		for (unsigned int j = 0; j < M; ++j) {
			D(i, j) = (X.col(i) - Y.col(j)).lpNorm<1>();
		}
	}
	return D;
}

MatrixXd ClusterXX::ManhattanDistance::computeMatrix(const MatrixXd &X) const {
	unsigned int N = X.cols();
	if (getVerbose()) {
		std::cout << "Computing Manhattan Distance..." << std::endl;
	}
	MatrixXd D(N, N);
	for (unsigned int i = 0; i < N; ++i) {
		D(i, i) = 0;
		for (unsigned int j = i + 1; j < N; ++j) {
			double dist = (X.col(i) - X.col(j)).lpNorm<1>();
			D(i, j) = dist;
			D(j, i) = dist;
		}
	}
	return D;
}

double ClusterXX::CosineSimilarity::compute(const VectorXd &left,
		const VectorXd &right) const {
	return left.dot(right) / (left.norm() * right.norm());
}

MatrixXd ClusterXX::CosineSimilarity::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Cosine Similarity..." << std::endl;
	}
	MatrixXd D = X.transpose() * Y;
	D.array().rowwise() /= X.colwise().norm().array();
	D.array().colwise() /= Y.colwise().norm().transpose().array();
	return D;
}

MatrixXd ClusterXX::CosineSimilarity::computeMatrix(const MatrixXd &X) const {
	if (getVerbose()) {
		std::cout << "Computing Cosine Similarity..." << std::endl;
	}
	MatrixXd D = X.transpose() * X;
	D.array().rowwise() /= X.colwise().norm().array();
	D.array().colwise() /= X.colwise().norm().transpose().array();
	return D;
}

double ClusterXX::CosineAbsoluteSimilarity::compute(const VectorXd &left,
		const VectorXd &right) const {
	return std::fabs(CosineSimilarity().compute(left, right));
}

MatrixXd ClusterXX::CosineAbsoluteSimilarity::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Cosine Absolute Similarity..." << std::endl;
	}
	return CosineSimilarity().computeMatrix(X, Y).array().abs();
}

MatrixXd ClusterXX::CosineAbsoluteSimilarity::computeMatrix(
		const MatrixXd &X) const {
	if (getVerbose()) {
		std::cout << "Computing Cosine Absolute Similarity..." << std::endl;
	}
	return CosineSimilarity().computeMatrix(X).array().abs();
}

double ClusterXX::CosineDistance::compute(const VectorXd &X,
		const VectorXd &Y) const {
	return 1.0 - CosineAbsoluteSimilarity().compute(X, Y);
}

MatrixXd ClusterXX::CosineDistance::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Cosine Distance..." << std::endl;
	}
	return 1.0 - CosineAbsoluteSimilarity().computeMatrix(X, Y).array();
}

MatrixXd ClusterXX::CosineDistance::computeMatrix(const MatrixXd &X) const {
	if (getVerbose()) {
		std::cout << "Computing Cosine Distance..." << std::endl;
	}
	return 1.0 - CosineAbsoluteSimilarity().computeMatrix(X).array();
}

double ClusterXX::PearsonCorrelation::compute(const VectorXd &left,
		const VectorXd &right) const {
	double mean_left = left.mean();
	double stddev_left = (left.array() * left.array()).matrix().mean()
			- mean_left * mean_left;
	stddev_left = std::sqrt(stddev_left);
	double mean_right = right.mean();
	double stddev_right = (right.array() * right.array()).matrix().mean()
			- mean_right * mean_right;
	stddev_right = std::sqrt(stddev_right);
	return ((left.array() - mean_left).matrix()).dot(
			(right.array() - mean_right).matrix())
			/ (left.rows() * stddev_left * stddev_right);
}

MatrixXd ClusterXX::PearsonCorrelation::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	if (getVerbose()) {
		std::cout << "Computing Pearson Correlation..." << std::endl;
	}
	Eigen::MatrixXd XMeans = X.colwise().mean();
	Eigen::MatrixXd YMeans = Y.colwise().mean();
	Eigen::MatrixXd XStdDevs =
			((X.array() * X.array()).matrix().colwise().mean()
					- (XMeans.array() * XMeans.array()).matrix()).array().sqrt();
	Eigen::MatrixXd YStdDevs =
			((Y.array() * Y.array()).matrix().colwise().mean()
					- (YMeans.array() * YMeans.array()).matrix()).array().sqrt();
	return (((X.rowwise() - XMeans.row(0)).transpose()
			* (Y.rowwise() - YMeans.row(0))).array()
			/ ((XStdDevs.transpose() * YStdDevs).array())).matrix()
			/ (double) X.rows();
}

MatrixXd ClusterXX::PearsonCorrelation::computeMatrix(const MatrixXd &X) const {
	if (getVerbose()) {
		std::cout << "Computing Pearson Correlation..." << std::endl;
	}
	Eigen::MatrixXd means = X.colwise().mean();
	Eigen::MatrixXd stdDevs = ((X.array() * X.array()).matrix().colwise().mean()
			- (means.array() * means.array()).matrix()).array().sqrt();
	return (((X.rowwise() - means.row(0)).transpose()
			* (X.rowwise() - means.row(0))).array()
			/ (stdDevs.transpose() * stdDevs).array()).matrix()
			/ (double) X.rows();
}

double ClusterXX::PearsonAbsoluteCorrelation::compute(const VectorXd &left,
		const VectorXd &right) const {
	return std::fabs(PearsonCorrelation().compute(left, right));
}

MatrixXd ClusterXX::PearsonAbsoluteCorrelation::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	return PearsonCorrelation().computeMatrix(X, Y).array().abs();
}

MatrixXd ClusterXX::PearsonAbsoluteCorrelation::computeMatrix(
		const MatrixXd &X) const {
	return PearsonCorrelation().computeMatrix(X).array().abs();
}

double ClusterXX::PearsonDistance::compute(const VectorXd &X,
		const VectorXd &Y) const {
	return std::fabs(1.0 - PearsonAbsoluteCorrelation().compute(X, Y));
}

MatrixXd ClusterXX::PearsonDistance::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	return 1.0 - PearsonAbsoluteCorrelation().computeMatrix(X, Y).array();
}

MatrixXd ClusterXX::PearsonDistance::computeMatrix(const MatrixXd &X) const {
	return 1.0 - PearsonAbsoluteCorrelation().computeMatrix(X).array();
}

double ClusterXX::SpearmanCorrelation::compute(const VectorXd &left,
		const VectorXd &right) const {
	std::vector<double> l = Utilities::eigen2Stl(left);
	std::vector<double> r = Utilities::eigen2Stl(right);
	Utilities::computeRank(&l);
	Utilities::computeRank(&r);
	return PearsonCorrelation().compute(Utilities::stl2Eigen(l),
			Utilities::stl2Eigen(l));
}

MatrixXd ClusterXX::SpearmanCorrelation::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	unsigned int N = X.cols();
	unsigned int M = Y.cols();
	unsigned int dim = X.rows();
	MatrixXd XCopy(dim, N);
	MatrixXd YCopy(dim, M);
	for (unsigned int i = 0; i < N; ++i) {
		std::vector<double> v = Utilities::eigen2Stl(X.col(i));
		Utilities::computeRank(&v);
		XCopy.col(i) = Utilities::stl2Eigen(v);
	}
	for (unsigned int i = 0; i < M; ++i) {
		std::vector<double> v = Utilities::eigen2Stl(Y.col(i));
		Utilities::computeRank(&v);
		YCopy.col(i) = Utilities::stl2Eigen(v);
	}
	return PearsonCorrelation().computeMatrix(XCopy, YCopy);
}

MatrixXd ClusterXX::SpearmanCorrelation::computeMatrix(
		const MatrixXd &X) const {
	unsigned int N = X.cols();
	unsigned int dim = X.rows();
	MatrixXd XCopy(dim, N);
	for (unsigned int i = 0; i < N; ++i) {
		std::vector<double> v = Utilities::eigen2Stl(X.col(i));
		Utilities::computeRank(&v);
		XCopy.col(i) = Utilities::stl2Eigen(v);
	}
	return PearsonCorrelation().computeMatrix(XCopy);
}

double ClusterXX::SpearmanAbsoluteCorrelation::compute(const VectorXd &left,
		const VectorXd &right) const {
	return std::fabs(SpearmanCorrelation().compute(left, right));
}

MatrixXd ClusterXX::SpearmanAbsoluteCorrelation::computeMatrix(
		const MatrixXd &X, const MatrixXd &Y) const {
	return SpearmanCorrelation().computeMatrix(X, Y).array().abs();
}

MatrixXd ClusterXX::SpearmanAbsoluteCorrelation::computeMatrix(
		const MatrixXd &X) const {
	return SpearmanCorrelation().computeMatrix(X).array().abs();
}

double ClusterXX::SpearmanDistance::compute(const VectorXd &X,
		const VectorXd &Y) const {
	return 1.0 - SpearmanAbsoluteCorrelation().compute(X, Y);
}

MatrixXd ClusterXX::SpearmanDistance::computeMatrix(const MatrixXd &X,
		const MatrixXd &Y) const {
	return 1.0 - SpearmanAbsoluteCorrelation().computeMatrix(X, Y).array();
}

MatrixXd ClusterXX::SpearmanDistance::computeMatrix(const MatrixXd &X) const {
	return 1.0 - SpearmanAbsoluteCorrelation().computeMatrix(X).array();
}

std::shared_ptr<ClusterXX::Metric> ClusterXX::buildMetric(
		MetricName metricName) {
	switch (metricName) {
	case MetricName::COSINE_ABSOLUTE_SIMILARITY:
		return std::make_shared<CosineAbsoluteSimilarity>();
	case MetricName::COSINE_DISTANCE:
		return std::make_shared<CosineDistance>();
	case MetricName::COSINE_SIMILARITY:
		return std::make_shared<CosineSimilarity>();
	case MetricName::EUCLIDEAN_DISTANCE:
		return std::make_shared<EuclideanDistance>();
	case MetricName::SQUARED_EUCLIDEAN_DISTANCE:
		return std::make_shared<SquaredEuclideanDistance>();
	case MetricName::MANHATTAN_DISTANCE:
		return std::make_shared<ManhattanDistance>();
	case MetricName::PEARSON_ABSOLUTE_CORRELATION:
		return std::make_shared<PearsonAbsoluteCorrelation>();
	case MetricName::PEARSON_CORRELATION:
		return std::make_shared<PearsonCorrelation>();
	case MetricName::PEARSON_DISTANCE:
		return std::make_shared<PearsonDistance>();
	case MetricName::SPEARMAN_ABSOLUTE_CORRELATION:
		return std::make_shared<SpearmanAbsoluteCorrelation>();
	case MetricName::SPEARMAN_CORRELATION:
		return std::make_shared<SpearmanCorrelation>();
	case MetricName::SPEARMAN_DISTANCE:
		return std::make_shared<SpearmanDistance>();
	default:
		assert(false);
		return std::make_shared<EuclideanDistance>(); //Eclipse complains otherwise
	}
}

std::shared_ptr<ClusterXX::Metric> ClusterXX::buildMetric(
		const std::string &s) {
	if (s == "absolute-cosine" || s == "cosine-absolute-similarity") {
		return std::make_shared<CosineAbsoluteSimilarity>();
	} else if (s == "cosine-distance") {
		return std::make_shared<CosineDistance>();
	} else if (s == "cosine" || s == "cosine-smilarity") {
		return std::make_shared<CosineSimilarity>();
	} else if (s == "euclidean" || s == "euclidean-distance") {
		return std::make_shared<EuclideanDistance>();
	} else if (s == "squared-euclidean" || s == "squared-euclidean-distance") {
		return std::make_shared<SquaredEuclideanDistance>();
	} else if (s == "manhattan" || s == "manhattan-distance") {
		return std::make_shared<ManhattanDistance>();
	} else if (s == "absolute-pearson" || s == "pearson-absolute-correlation") {
		return std::make_shared<PearsonAbsoluteCorrelation>();
	} else if (s == "pearson" || s == "pearson-correlation") {
		return std::make_shared<PearsonCorrelation>();
	} else if (s == "pearson-distance") {
		return std::make_shared<PearsonDistance>();
	} else if (s == "absolute-spearman"
			|| s == "spearman-absolute-correlation") {
		return std::make_shared<SpearmanAbsoluteCorrelation>();
	} else if (s == "spearman" || s == "spearman-correlation") {
		return std::make_shared<SpearmanCorrelation>();
	} else if (s == "spearman-distance") {
		return std::make_shared<SpearmanDistance>();
	} else {
		std::cerr
				<< "Couldn't match metric with name '" + s
						+ "', returning default." << std::endl;
		return std::make_shared<EuclideanDistance>();
	}
}
