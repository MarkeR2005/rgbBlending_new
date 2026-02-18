//---------------------------------------------------------------------------
#include "SeismicData.h"
#include "RgbData.h"

#ifndef DataCalculatorH
#define DataCalculatorH
//---------------------------------------------------------------------------
class DataCalculator
{
	public:
	static std::shared_ptr<IBaseData> smoothT(const std::shared_ptr<IBaseData>& input, int window);
	static std::shared_ptr<IBaseData> smoothX(const std::shared_ptr<IBaseData>& input, int window);
	static std::shared_ptr<RgbData> smoothF(const std::shared_ptr<RgbData>& input, int window, int freqNo1 = -1, int freqNo2 = -1);

	static std::shared_ptr<IBaseData> getTrace(const std::shared_ptr<IBaseData>& input, int pos);
	static std::shared_ptr<SeismicData> getFilter(const std::shared_ptr<RgbData>& input, int pos);
	static std::shared_ptr<SeismicData> toEnergy(const std::shared_ptr<SeismicData>& input);
	static std::shared_ptr<SeismicData> filter(const std::shared_ptr<SeismicData>& input, float freq, int window);

	static std::shared_ptr<SeismicData> getSwan(const std::shared_ptr<SeismicData>& input, int pos, const float freqStart, const float freqFinish, const int filtersNumber, const int filterWidth);
	static std::shared_ptr<SeismicData> getSwan(const std::shared_ptr<RgbData>& input, int pos);
	static std::shared_ptr<RgbData> getSwanRGB(const std::shared_ptr<SeismicData>& input, int pos, const float freqStart, const float freqFinish, const int filtersNumber, const int filterWidth);

	static std::shared_ptr<IBaseData> mute(const std::shared_ptr<IBaseData>& input, int start, int stop);
	static std::shared_ptr<RgbData> SwanToRGB(const std::shared_ptr<SeismicData>& input, int window = 1);
	//static RgbData getRGB(SeismicData input, const float freqStart, const float freqFinish, const int filtersNumber, const int filterWidth);
};
#endif
