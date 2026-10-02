//---------------------------------------------------------------------------

#ifndef StructuresH
#define StructuresH
#include <vector>
#include <cstdint>
#include <string>
#include <utility>
//---------------------------------------------------------------------------
static float stnFreqStart = 5;
static float stnFreqStop = 100;
static int stnNumFilters = 100;
static int stnFilterWindow = 100;
static int stnXWindow = 5;
static int stnTWindow = 5;
static int stnFWindow = 5;
static float stnFilterFreq = 15;
// One ordinate per trace; exactly -1 means no picked point.
struct Horizon {
    std::wstring name;
    std::vector<float> points;
    // Local appearance. Existing host transfer data still contains name/points.
    bool useColor = false;
    bool contrast = true;
    unsigned char red = 255, green = 255, blue = 255;
};
struct Cross {
    std::wstring name;
    int x = 0; // trace coordinate
};
struct bitMap
{
	std::vector<uint8_t> texture;
	int height;
	int width;
	bitMap(std::vector<uint8_t> texture_ = {}, int height_ = 0, int width_ = 0): texture(std::move(texture_)), height(height_), width(width_){}
};
struct size2
{
	int x;
	int t;
};
struct size3
{
	int x;
	int t;
	int f;
};
struct point2
{
	int x;
	int y;
};
struct point2F
{
	float x;
	float y;
};
struct point3F
{
	float x;
	float y;
    float z;
};
struct Traces
{
    int tracesNumber;
    int samplesNumber;
    int dt;
    float* data;
	void init (int trNum, int spNum, int dT, float* dat);
    ~Traces(){if (data != nullptr) delete[] data;};
};
//--
struct byteMap
{
    int height;
    int width;
    std::vector<uint8_t> color;
    void init (int h, int w);
};
//--
struct ArrayStats
{
    float median;
    float lower_bound;
    float upper_bound;
    void init (float m, float l, float r);
};
//--
struct FrequencyTexture
{
    int width;
    int height;
    std::vector<uint8_t> data;
};
//---------------------------------------------------
#endif
