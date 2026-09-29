/*
 * math.hpp
 *
 *  Created on: Jun 25, 2015
 *      Author: nicolas
 */

#ifndef INCLUDES_CLUSTERXX_UTILS_MATH_HPP_
#define INCLUDES_CLUSTERXX_UTILS_MATH_HPP_

namespace ClusterXX {
namespace Utilities {
namespace Math{
	const double EPSILON = 0.00001;
	bool isZero(double x);
	bool equal(double x, double y);
	bool greaterThanZero(double x);
	bool greaterThan(double x, double y);
	bool lessThanZero(double x);
	bool lessThan(double x, double y);
}
}
}



#endif /* INCLUDES_CLUSTERXX_UTILS_MATH_HPP_ */
