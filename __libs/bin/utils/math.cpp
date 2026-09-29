/*
 * math.cpp
 *
 *  Created on: Jun 25, 2015
 *      Author: nicolas
 */

#include <ClusterXX/utils/math.hpp>
#include <cmath>

bool ClusterXX::Utilities::Math::isZero(double x){
	return std::fabs(x) < EPSILON;
}

bool ClusterXX::Utilities::Math::equal(double x, double y){
	return isZero(x-y);
}

bool ClusterXX::Utilities::Math::greaterThanZero(double x){
	return x>-1.0*EPSILON;
}

bool ClusterXX::Utilities::Math::lessThanZero(double x){
	return x<EPSILON;
}

bool ClusterXX::Utilities::Math::greaterThan(double x, double y){
	return greaterThanZero(x-y);
}

bool ClusterXX::Utilities::Math::lessThan(double x, double y){
	return lessThanZero(x-y);
}
