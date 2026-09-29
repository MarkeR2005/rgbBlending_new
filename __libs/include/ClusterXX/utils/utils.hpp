/*
 * utils.hpp
 *
 *  Created on: Jun 12, 2015
 *      Author: nicolas
 */

#ifndef SRC_UTILS_UTILS_HPP_
#define SRC_UTILS_UTILS_HPP_

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <Eigen/Dense>

namespace ClusterXX{

namespace Utilities {

const double EPSILON = 0.00001;

/*
 *
 * Useful templates
 *
 */

template<typename InputIter1, typename InputIter2, typename Function>
Function for_each_two_ranges(InputIter1 first1, InputIter1 last1,
		InputIter2 first2, Function f) {
	for (; first1 != last1; ++first1, ++first2) {
		f(*first1, *first2);
	}
	return f;
}

template<typename InputIter1, typename InputIter2, typename OutputIter>
//The second range should correspond to a container of boolean values saying whether the value should be added to the output
void copy_if_two_ranges(InputIter1 first1, InputIter1 last1, InputIter2 first2,
		OutputIter out) {
	for (; first1 != last1; ++first1, ++first2) {
		if (*first2) {
			*out = *first1;
		}
	}
}

//Print a vector
template<typename T>
void print_vector(const std::vector<T> &v) {
	std::cout << "{ ";
	for_each(v.cbegin(), v.cend(),
			[](const T &data) {std::cout << data << " ";});
	std::cout << "}" << std::endl;
}

//Print a map
template<typename T1, typename T2>
void print_map(const std::map<T1, T2> &m) {
	std::cout << "{" << std::endl;
	for_each(m.cbegin(), m.cend(),
			[](const std::pair<T1, T2> &data) {std::cout << data.first << " --> " << data.second << std::endl;});
	std::cout << "}" << std::endl;
}

//Sort the indexes of a vector in decreasing order
template<typename T>
std::vector<size_t> sort_indexes_decreasing(const std::vector<T> &v) {

	// initialize original index locations
	std::vector<size_t> idx(v.size());
	for (size_t i = 0; i != idx.size(); ++i)
		idx[i] = i;

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
			[&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

	return idx;
}

//Sort the indexes of an array in increasing order
template<typename T>
std::vector<size_t> sort_indexes_increasing(const std::vector<T> &v) {

	// initialize original index locations
	std::vector<size_t> idx(v.size());
	for (size_t i = 0; i != idx.size(); ++i)
		idx[i] = i;

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
			[&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

	return idx;
}

//Get the rank of each value of an array, where the rank is taken in increasing order
template<typename T>
std::vector<size_t> get_rank_increasing(const std::vector<T> &v) {
	std::vector<size_t> sortedIdx { sort_indexes_increasing(v) };
	std::vector<size_t> ranks(sortedIdx.size());
	for (size_t i = 0; i != ranks.size(); ++i) {
		ranks[sortedIdx[i]] = i;
	}
	return ranks;
}

//Build a map from a range
template<typename T, typename InputIterator>
std::map<T, unsigned int> buildIndexMap(InputIterator first,
		InputIterator last) {
	std::map<T, unsigned int> indexMap;
	unsigned int count = 0;
	for_each(first, last, [&indexMap, &count](const T &data) {
		indexMap.insert(std::make_pair(data, count));
		++count;
	});
	return indexMap;
}

/*
 *
 * Useful functions
 *
 */

std::vector<double> eigen2Stl(const Eigen::VectorXd &vec);
Eigen::VectorXd stl2Eigen(const std::vector<double> &vec);

void computeRank(std::vector<double> *v);

void printAdvancement(unsigned int currentCount, unsigned int totalCount);

} //End of namespace Utilities

} //End of namespace ClusterXX

#endif /* SRC_UTILS_UTILS_HPP_ */
