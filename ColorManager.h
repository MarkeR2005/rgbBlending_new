//---------------------------------------------------------------------------
#include "Structures.h"
#include <vector>
#include <array>
#include <map>
#include <memory>
#include "vcl.h"


#ifndef ColorManagerH
#define ColorManagerH
//---------------------------------------------------------------------------
static std::array<uint8_t, 256*3> genBasicPalette(){
	std::array<uint8_t, 256*3> paletteData;
	for (int i = 0; i < 128; ++i)
	{
		paletteData[i*3] = i;
		paletteData[i*3+1] = i;
		paletteData[i*3+2] = i;
	}
	for (int i = 128; i < 256; ++i)
	{
		paletteData[i*3] = i;
		paletteData[i*3+1] = 127-(i-128);
		paletteData[i*3+2] = 127-(i-128);
	}
	return paletteData;
};

static const std::array<uint8_t, 256*3> PALETTE = genBasicPalette();
//
class IBaseData;

class ColorManager
{
private:
	//Parameters
	float boundPercent;
	bool zeroMode = false;
	//Data
	std::map<std::weak_ptr<IBaseData>, std::vector<float>, std::owner_less<std::weak_ptr<IBaseData>>> data;
	//Data derivatives
	float median, lowerBound, upperBound;
	float range_low;
	float range_high;
	float inv_range_low;
	float inv_range_high;
	//State
	bool needsUpdate = true;

public:
	//Constructor
	ColorManager(float p = 95.0f, bool zero = true);
	ColorManager(const ColorManager& other) = delete;
    ~ColorManager();
	//Data addition
	void addToSelection(std::weak_ptr<IBaseData> input);
	//Data delition
	void removeFromSelection(std::weak_ptr<IBaseData> ptr);
	void clear();
	//Enumeration
	void update();
	bitMap getTexture(const std::vector<std::vector<float>>& data);
};

#endif
