#ifndef SRC_HEATMAPBUILDER_HPP_
#define SRC_HEATMAPBUILDER_HPP_

#include <vector>
#include <array>
#include <Eigen/Dense>

namespace ClusterXX {

namespace Utilities {

class HeatMapBuilder {
public:
	void build(const Eigen::MatrixXd &distanceMatrix,
			const std::string &filename,
			std::vector<unsigned int> classDivision =
					std::vector<unsigned int>(),
			unsigned int divisionLineWidth = 0,
			std::array<unsigned char, 3> separatorColor = std::array<
					unsigned char, 3> { static_cast<unsigned char>(255),
					static_cast<unsigned char>(155), 0 });
};

} //End of namespace Utilities

} //End of namespace ClusterXX

#endif /* SRC_HEATMAPBUILDER_HPP_ */
