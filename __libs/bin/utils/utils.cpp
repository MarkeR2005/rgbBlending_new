/*
 * utils.cpp
 *
 *  Created on: Jun 12, 2015
 *      Author: nicolas
 */

#ifndef SRC_UTILS_UTILS_CPP_
#define SRC_UTILS_UTILS_CPP_

#include <vector>
#include <iostream>
#include <ClusterXX/utils/utils.hpp>

std::vector<double> ClusterXX::Utilities::eigen2Stl(const Eigen::VectorXd &vec){
	std::vector<double> v(vec.rows());
	for(unsigned int i=0; i<vec.rows(); ++i){
		v[i] = vec(i);
	}
	return v;
}

Eigen::VectorXd ClusterXX::Utilities::stl2Eigen(const std::vector<double> &vec){
	Eigen::VectorXd v(vec.size());
	for(unsigned int i=0; i<vec.size(); ++i){
		v(i) = vec[i];
	}
	return v;
}

//Utility for computeRank()
void computeRankSorted(std::vector<double> *x) {
	unsigned int current = 0;
	while (current != x->size()) {
		double d = (*x)[current];
		unsigned int next = current + 1;
		int sum = current;
		int count = 1;
		while (next < x->size() && d == (*x)[next]) {
			++count;
			sum += next;
			++next;
		}

		for (unsigned int j = current; j < next; ++j) {
			(*x)[j] = (double) sum / (double) count;
		}

		current = next;
	}
}

void ClusterXX::Utilities::computeRank(std::vector<double> *v) {
	std::vector<double> copyX(*v);
	std::vector<size_t> sortedXIndexes = Utilities::get_rank_increasing(copyX);
	std::sort(copyX.begin(), copyX.end());
	computeRankSorted(&copyX);
	for (unsigned int i = 0; i != v->size(); ++i) {
		(*v)[i] = copyX[sortedXIndexes[i]];
	}
}



void ClusterXX::Utilities::printAdvancement(unsigned int currentCount,
		unsigned int totalCount) {
	std::cout << (100.0 * (double) currentCount) / (double) (totalCount)
			<< "% \r" << std::flush;
}

#endif /* SRC_UTILS_UTILS_CPP_ */
