//---------------------------------------------------------------------------
#include "InterfacesData.h"
#include "Structures.h"

#include <vector>

#ifndef SeismicDataH
#define SeismicDataH
//---------------------------------------------------------------------------

enum class DataType{
	TRACE,
	BASE,
	SWAN,
	EN_SWAN,
	EN_BASE,
	EN_TRACE
};

//Добавить класс функции над данными и метод выполнения функции.
class SeismicData : public IBaseData
{
private:
	//Data
	std::vector<std::vector<float>> data;
	//Params
	float dT;
	int samples;
	int traces;
	//Metadata
	std::string name;
	std::string procedures;
	//ColorManager
	std::shared_ptr<ColorManager> mng;
	std::function<void()> update;

	DataType type = DataType::BASE;

    std::vector<float> freq = {};
public:
	//constructors
	SeismicData (const std::vector<std::vector<float>> data_ = {},
	 const float dT_ = 0.0f, const DataType type_ = DataType::BASE, const std::string name_ = "base", const std::string procedures_ = "n");
	SeismicData (const float* data_, const float dT_, const int samples_,
	 const int traces_, const DataType type_ = DataType::BASE, const std::string name_ = "base", const std::string procedures_ = "n");
	SeismicData (const SeismicData& other);
	~SeismicData();
	 //Setters
	void setCM(std::shared_ptr<ColorManager> cm) override;
	std::shared_ptr<ColorManager> getCM() override {return mng;};
	void setFreq(std::vector<float> f){freq=f;};
	//Getters
	std::vector<std::vector<float>> getRawData(){return data;};
	std::vector<std::vector<float>>& getRawDataRef(){return data;};
	std::string getName() override {return name;};
	std::string getProcedures() override {return procedures;};
	size2 getSize() {return {traces, samples};};
	float getDT()override {return dT;};
	DataType getType() {return type;};
	bitMap getTexture();
    std::vector<float> getFreq(){return freq;};
	//Derivative data
	//File
	void saveFile(const std::wstring& loc);
	static SeismicData loadFile(const std::wstring& loc);
	void setUpdateCallback(std::function<void()> callback) override {update = callback;};
};

#endif
