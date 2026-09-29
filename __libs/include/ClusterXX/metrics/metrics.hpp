#ifndef SRC_METRICS_METRICS_HPP_
#define SRC_METRICS_METRICS_HPP_

#include <memory>
#include <iostream>
#include <Eigen/Dense>

namespace ClusterXX{

enum MetricName {
	PEARSON_CORRELATION,
	PEARSON_ABSOLUTE_CORRELATION,
	PEARSON_DISTANCE,
	SPEARMAN_CORRELATION,
	SPEARMAN_ABSOLUTE_CORRELATION,
	SPEARMAN_DISTANCE,
	COSINE_SIMILARITY,
	COSINE_ABSOLUTE_SIMILARITY,
	COSINE_DISTANCE,
	EUCLIDEAN_DISTANCE,
	SQUARED_EUCLIDEAN_DISTANCE,
	MANHATTAN_DISTANCE
};

/*
 *
 * Virtual Base class. All metrics must implement its methods
 *
 */

class Metric {
private:
	bool verbose = false;
public:
	void setVerbose(bool b) {
		verbose = b;
	}
	bool getVerbose() const {
		return verbose;
	}
	virtual double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const = 0;
	virtual Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const = 0;
	virtual Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const = 0;
	virtual bool isDistanceMetric() const = 0;
	virtual std::string toString() const = 0;
	virtual ~Metric() = default;
};

/*
 *
 * Metric implementations
 *
 */

class SquaredEuclideanDistance: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return true;
	}
	std::string toString() const{
		return "squared-euclidean-distance";
	}
};

class EuclideanDistance: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	double computeVector(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return true;
	}
	std::string toString() const{
		return "euclidean-distance";
	}
};

class ManhattanDistance: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return true;
	}
	std::string toString() const{
		return "manhattan-distance";
	}
};

class CosineSimilarity: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return false;
	}
	std::string toString() const{
		return "cosine-similarity";
	}
};

class CosineAbsoluteSimilarity: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return false;
	}
	std::string toString() const{
		return "cosine-absolute-similarity";
	}
};

class CosineDistance: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return true;
	}
	std::string toString() const{
		return "cosine-distance";
	}
};

class PearsonCorrelation: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return false;
	}
	std::string toString() const{
		return "pearson-correlation";
	}
};

class PearsonAbsoluteCorrelation: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return false;
	}
	std::string toString() const{
		return "pearson-absolute-correlation";
	}
};

class PearsonDistance: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return true;
	}
	std::string toString() const{
		return "pearson-distance";
	}
};

class SpearmanCorrelation: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return false;
	}
	std::string toString() const{
		return "spearman-correlation";
	}
};

class SpearmanAbsoluteCorrelation: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return false;
	}
	std::string toString() const{
		return "spearman-absolute-correlation";
	}
};

class SpearmanDistance: public Metric {
public:
	double compute(const Eigen::VectorXd &left,
			const Eigen::VectorXd &right) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X,
			const Eigen::MatrixXd &Y) const;
	Eigen::MatrixXd computeMatrix(const Eigen::MatrixXd &X) const;
	bool isDistanceMetric() const {
		return true;
	}
	std::string toString() const{
		return "spearman-distance";
	}
};

//utility functions

std::shared_ptr<Metric> buildMetric(MetricName metricName);
std::shared_ptr<Metric> buildMetric(const std::string &s);


} //End of namespace ClusterXX

#endif /* SRC_METRICS_METRICS_HPP_ */
