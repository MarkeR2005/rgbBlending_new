//---------------------------------------------------------------------------
#include "InterfacesData.h"
#include "SeismicData.h"
#include "Structures.h"

#include <vector>

#ifndef RgbDataH
#define RgbDataH
//---------------------------------------------------------------------------
class RgbData : public IBaseData
{
private:
	//Data
	std::vector<std::vector<std::vector<float>>> data;
	//Params
	float dT;
	int samples;
	int traces;
	int filters;
	//
	std::vector<float> frequencies;
	//Metadata
	std::string name;
	std::string procedures;
	//ColorManager
	std::shared_ptr<ColorManager> mng;
    std::function<void()> update;

public:
	//Constructors
	RgbData (const std::vector<std::vector<std::vector<float>>>& data_ = {}, const float dT_ = 0.0f,
	const float freqStart = 0, const float freqFinish = 0, const std::string name_ = "rgb", const std::string procedures_ = "n");
	RgbData (SeismicData* base, int freqStart, int freqFinish, int filtersNumber, int filterWidth);
	RgbData (const RgbData& other);
    ~RgbData();
	//Setters
	void setCM(std::shared_ptr<ColorManager> cm) override;
    std::shared_ptr<ColorManager> getCM() override {return mng;};
	//Getters
	std::vector<std::vector<std::vector<float>>> getRawData(){return data;};
	std::vector<std::vector<std::vector<float>>>& getRawDataRef(){return data;};
	virtual std::string getName() override {return name;};
	virtual std::string getProcedures() override {return procedures;};
	size3 getSize(){return {traces, samples, filters};};
	virtual float getDT() override {return dT;};
	std::vector<float> getFreqs(){return frequencies;};
	std::vector<bitMap> getTexture();
	//--------------------------------------------------------------------------
	//File
	void saveFile(const std::wstring& loc);
	static RgbData loadFile(const std::wstring& loc);
	void setUpdateCallback(std::function<void()> callback) override {update = callback;};
};
#endif
