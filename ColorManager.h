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
static std::array<uint8_t, 256*3> genBasicPalette(int r1, int g1, int b1, int r2, int g2, int b2, bool whitecenter = true){
	std::array<uint8_t, 256*3> paletteData;
    if (whitecenter) {
        for (int i = 0; i < 128; ++i)
        {
            paletteData[i*3] = r1+i*(255-r1)/128;
            paletteData[i*3+1] = g1+i*(255-g1)/128;
            paletteData[i*3+2] = b1+i*(255-b1)/128;
        }
        for (int i = 0; i < 128; ++i)
        {
            paletteData[(128+i)*3] = 255-i*(255-r2)/128;
            paletteData[(128+i)*3+1] = 255-i*(255-g2)/128;
            paletteData[(128+i)*3+2] = 255-i*(255-b2)/128;
        }
        return paletteData;
    }
    else {
        for (int i = 0; i < 256; ++i)
        {
            paletteData[i*3] = r1+i*(r2-r1)/256;
            paletteData[i*3+1] = g1+i*(g2-g1)/256;
            paletteData[i*3+2] = b1+i*(b2-b1)/256;
        }
        return paletteData;
    }

};

static std::array<uint8_t, 256*3> PALETTE = genBasicPalette(255,0,0,0,0,255,true);
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
