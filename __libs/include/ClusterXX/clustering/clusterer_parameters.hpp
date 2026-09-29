#ifndef SRC_ALGORITHMS_CLUSTERER_PARAMETERS_HPP_
#define SRC_ALGORITHMS_CLUSTERER_PARAMETERS_HPP_

#include <memory>
#include <cassert>
#include "../metrics/metrics.hpp"

namespace ClusterXX {

class ClustererParameters {
private:
	bool verbose;
public:
	ClustererParameters(bool b) :
			verbose(b) {
	}
	virtual ~ClustererParameters() = 0;
	bool getVerbose() {
		return verbose;
	}
};

class KMeansParameters: public ClustererParameters {
private:
	unsigned int K;
	unsigned int maxIterations;
	unsigned int numberOfParallelKMeans;
public:
	KMeansParameters(unsigned int _K, unsigned int _maxIter, bool verbose) :
			ClustererParameters(verbose), K(_K), maxIterations(_maxIter), numberOfParallelKMeans(
					1) {
	}

	KMeansParameters(unsigned int _K, unsigned int _maxIter,
			unsigned int _numberOfParallelKMeans = 1, bool verbose = false) :
			ClustererParameters(verbose), K(_K), maxIterations(_maxIter), numberOfParallelKMeans(
					_numberOfParallelKMeans) {
	}

	unsigned int getK() {
		return K;
	}

	unsigned int getMaxIterations() {
		return maxIterations;
	}

	unsigned int getNumberOfParallelKMeans() {
		return numberOfParallelKMeans;
	}
};

class HierarchicalParameters: public ClustererParameters {
public:
	enum LinkageMethod {
		AVERAGE, SINGLE, COMPLETE
	};
	HierarchicalParameters(unsigned int _K,
			const std::shared_ptr<Metric> &_metricPtr,
			const LinkageMethod &_linkageMethod = LinkageMethod::COMPLETE,
			bool verbose = false) :
			ClustererParameters(verbose), K(_K), metricPtr(_metricPtr), linkageMethod(
					_linkageMethod) {

	}

	HierarchicalParameters(unsigned int _K,
			const std::shared_ptr<Metric> &_metricPtr, bool verbose) :
			HierarchicalParameters(_K, _metricPtr, LinkageMethod::COMPLETE,
					verbose) {
	}

	//Set K=2
	HierarchicalParameters(const std::shared_ptr<Metric> &_metricPtr,
			const LinkageMethod &_linkageMethod = LinkageMethod::COMPLETE,
			bool verbose = false) :
			ClustererParameters(verbose), K(2), metricPtr(_metricPtr), linkageMethod(
					_linkageMethod) {

	}

	unsigned int getK() {
		return K;
	}

	std::shared_ptr<Metric> getMetric() {
		return metricPtr;
	}

	LinkageMethod getLinkageMethod() {
		return linkageMethod;
	}
private:
	unsigned int K;
	std::shared_ptr<Metric> metricPtr;
	LinkageMethod linkageMethod;
};

class SpectralParameters: public ClustererParameters {
public:
	class GraphTransformationMethod {
	public:
		enum GraphTransformationMethodName {
			NO_TRANSFORMATION, K_NEAREST_NEIGHBORS, GAUSSIAN_MIXTURE
		};
		GraphTransformationMethod() :
				kNearestNeighbors(0), guaussianModelStdDev(0.0), method(
						GraphTransformationMethodName::NO_TRANSFORMATION) {

		}

		GraphTransformationMethod(GraphTransformationMethodName _method) :
				kNearestNeighbors(0), guaussianModelStdDev(0.0), method(
						GraphTransformationMethodName::NO_TRANSFORMATION) {
			assert(_method == GraphTransformationMethodName::NO_TRANSFORMATION);
		}

		GraphTransformationMethod(GraphTransformationMethodName _method,
				double param) :
				kNearestNeighbors(0), guaussianModelStdDev(0.0), method(_method) {
			if (method == GraphTransformationMethodName::K_NEAREST_NEIGHBORS) {
				kNearestNeighbors = param;
			} else if (method
					== GraphTransformationMethodName::GAUSSIAN_MIXTURE) {
				guaussianModelStdDev = param;
			} else {
				assert(
						method
								== GraphTransformationMethodName::NO_TRANSFORMATION
								&& param == 0);
			}
		}

		GraphTransformationMethodName getMethodName() const {
			return method;
		}

		unsigned int getKNearestNeighbors() const {
			return kNearestNeighbors;
		}

		double getGaussianModelStdDev() const {
			return guaussianModelStdDev;
		}
	private:
		unsigned int kNearestNeighbors;
		double guaussianModelStdDev;
		GraphTransformationMethodName method;
	};
	SpectralParameters(unsigned int _K,
			const std::shared_ptr<Metric> &_metricPtr,
			const GraphTransformationMethod &_transformationMethod =
					GraphTransformationMethod(), bool verbose = false) :
			ClustererParameters(verbose), K(_K), metricPtr(_metricPtr), transformationMethod(
					_transformationMethod) {
	}

	SpectralParameters(unsigned int _K,
			const std::shared_ptr<Metric> &_metricPtr, bool verbose) :
			SpectralParameters(_K, _metricPtr, GraphTransformationMethod(),
					verbose) {
	}

	unsigned int getK() {
		return K;
	}

	std::shared_ptr<Metric> getMetric() {
		return metricPtr;
	}

	GraphTransformationMethod getTransformationMethod() {
		return transformationMethod;
	}

private:
	unsigned int K;
	std::shared_ptr<Metric> metricPtr;
	GraphTransformationMethod transformationMethod;
};

} //End of namespace ClusterXX

#endif /* SRC_ALGORITHMS_CLUSTERER_PARAMETERS_HPP_ */
