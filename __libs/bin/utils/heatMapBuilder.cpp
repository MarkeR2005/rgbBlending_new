#include <vector>
#include <array>
#include <climits>
#include <iostream>
#include <lodepng.h>
#include <ClusterXX/utils/heatMapBuilder.hpp>

void ClusterXX::Utilities::HeatMapBuilder::build(const Eigen::MatrixXd &distanceMatrix,
		const std::string &filename, std::vector<unsigned int> classDivision,
		unsigned int divisionLineWidth, std::array<unsigned char, 3> separatorColor) {

	std::vector<unsigned char> image;

	double min = distanceMatrix.minCoeff();
	double max = distanceMatrix.maxCoeff();
	double range = max - min;
	unsigned int N = distanceMatrix.cols();

	unsigned int lineSeparator = UINT_MAX;
	auto iteratorLineSeparator = classDivision.begin();
	if (iteratorLineSeparator != classDivision.end()) {
		//Get first element
		lineSeparator = *iteratorLineSeparator;
	}

	for (unsigned int line = 0; line < N; ++line) {

		if (line == lineSeparator) {
			for (unsigned int i = 0; i < divisionLineWidth; ++i) {
				for (unsigned int j = 0;
						j < N + classDivision.size() * divisionLineWidth; ++j) {
					image.push_back(separatorColor[0]);
					image.push_back(separatorColor[1]);
					image.push_back(separatorColor[2]);
					image.push_back(255);
				}
			}
			++iteratorLineSeparator;
			if (iteratorLineSeparator == classDivision.end()) {
				lineSeparator = UINT_MAX;
			} else {
				lineSeparator = *iteratorLineSeparator;
			}
		}

		unsigned int columnSeparator = UINT_MAX;
		auto iteratorColumnSeparator = classDivision.begin();
		if (iteratorColumnSeparator != classDivision.end()) {
			columnSeparator = *iteratorColumnSeparator;
		}

		for (unsigned int column = 0; column < N; ++column) {

			if (column == columnSeparator) {
				for (unsigned int i = 0; i < divisionLineWidth; ++i) {
					image.push_back(separatorColor[0]);
					image.push_back(separatorColor[1]);
					image.push_back(separatorColor[2]);
					image.push_back(255);
				}

				++iteratorColumnSeparator;
				if (iteratorColumnSeparator == classDivision.end()) {
					columnSeparator = UINT_MAX;
				} else {
					columnSeparator = *iteratorColumnSeparator;
				}
			}

			double d = distanceMatrix(line, column);
			image.push_back(255 * ((d - min) / range));
			image.push_back(255 * ((d - min) / range));
			image.push_back(255 * ((d - min) / range));
			image.push_back(255);
		}
	}

	unsigned error = lodepng::encode(filename.c_str(), image,
			N + classDivision.size() * divisionLineWidth,
			N + classDivision.size() * divisionLineWidth);

	//if there's an error, display it
	if (error)
		std::cout << "encoder error " << error << ": "
				<< lodepng_error_text(error) << std::endl;
}
