//---------------------------------------------------------------------------
#include "Reader.h"
#include <bitset>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>
#include <vcl.h>
#include "UnitLoading.h"
#include "RgbData.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define Write(x,y) Write(x, (Longint)(y))
#define Read(x,y) Read(x, (Longint)(y))

//--
Traces readInlineRegular(std::wstring fName, int lineNumber) {
    double dt = 0.0;
    double sampleCount = 0;
    double trHead = 0;
    int fHead = 0;
    int t1 = 0, l1 = 0, t3 = 0, l3 = 0, hS = 0;

    std::unique_ptr<TFileStream> fileRead(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
    if (!fileRead || fileRead->Size == 0) {
        std::cerr << "fileError: Cannot open input file" << std::endl;
        throw Exception("bad file");
    }

    fileRead->Seek(static_cast<__int64>(6 * 4), soBeginning);
    fileRead->Read(&dt, 8);
    fileRead->Seek(static_cast<__int64>(32 + 6 * 4), soBeginning);
    fileRead->Read(&sampleCount, 8);
    fileRead->Seek(static_cast<__int64>(9 * 32 + 6 * 4), soBeginning);
    fileRead->Read(&trHead, 8);
    fileRead->Seek(static_cast<__int64>(10 * 32 + 4 * 2), soBeginning);
    fileRead->Read(&fHead, 4);
    fileRead->Seek(static_cast<__int64>(12 * 32), soBeginning);
    fileRead->Read(&t1, 4);
    fileRead->Read(&l1, 4);
    fileRead->Read(&t3, 4);
    fileRead->Read(&l3, 4);
    fileRead->Read(&hS, 4);

    int tr_inline = t3 - t1 + 1;
    int lines = l3 - l1 + 1;
    int samples = static_cast<int>(sampleCount);

    // Проверка валидности номера инлайна
    if (lineNumber < l1 || lineNumber > l3) {
        std::cerr << "Error: Inline number " << lineNumber
                  << " is out of range [" << l1 << ", " << l3 << "]" << std::endl;
        throw Exception("invalid inline number");
    }

    // ВЫЧИСЛЯЕМ ИНДЕКС ОТНОСИТЕЛЬНО НАЧАЛА
    int inlineIndex = lineNumber - l1;

    // РАЗМЕРЫ В БАЙТАХ
    const int traceSize = samples * 4; // размер одной трассы в байтах
    const int inlineSize = tr_inline * traceSize; // размер одного инлайна в байтах

    // БАЗОВОЕ СМЕЩЕНИЕ (как в кросслайновой версии)
    const __int64 baseOffset = 776 + fHead + static_cast<__int64>(trHead) * tr_inline * lines;

    // СМЕЩЕНИЕ ДО НУЖНОГО ИНЛАЙНА
    const __int64 offset = baseOffset + inlineIndex * inlineSize;

    // ЧИТАЕМ ВЕСЬ ИНЛАЙН
    const int count = tr_inline * samples;
    std::vector<float> dataBuffer(count);
    fileRead->Seek(offset, soBeginning);
    fileRead->Read(dataBuffer.data(), count * 4);

    // ТЕПЕРЬ НУЖНО ПРАВИЛЬНО УПОРЯДОЧИТЬ ДАННЫЕ
    // В файле данные organized по кросслайнам внутри инлайна
    // Но для отображения инлайна нам нужно, чтобы данные были упорядочены правильно

    float* lineData = new float[count];

    // Копируем данные как есть - предполагая, что порядок в файле правильный
    std::copy(dataBuffer.begin(), dataBuffer.end(), lineData);

    Traces tr;
    tr.init(tr_inline, samples, static_cast<int>(dt) * 1000, lineData);
    return tr;
}
//--
Traces readIBM(std::wstring fName) {
	double dt = 0.0;
    double sampleCount = 0;
    double trHead = 0;
	int fHead = 0;
	double tr_inline = 0;

	std::unique_ptr<TFileStream> fileRead(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
    if (!fileRead || fileRead->Size == 0) {
		std::cerr << "fileError: Cannot open input file" << std::endl;
        throw Exception("bad file");
	}

	fileRead->Seek(static_cast<__int64>(6 * 4), soBeginning);
	fileRead->Read(&dt, 8);
	fileRead->Seek(static_cast<__int64>(32 + 6 * 4), soBeginning);
	fileRead->Read(&sampleCount, 8);
	fileRead->Seek(static_cast<__int64>(3 * 32 + 6 * 4), soBeginning);
	fileRead->Read(&tr_inline, 8);
	fileRead->Seek(static_cast<__int64>(9 * 32 + 6 * 4), soBeginning);
	fileRead->Read(&trHead, 8);
    fileRead->Seek(static_cast<__int64>(10 * 32 + 4 * 2), soBeginning);
	fileRead->Read(&fHead, 4);

    int samples = static_cast<int>(sampleCount);


    // РАЗМЕРЫ В БАЙТАХ
	const int traceSize = samples * 4; // размер одной трассы в байтах

    // БАЗОВОЕ СМЕЩЕНИЕ (как в кросслайновой версии)
	const __int64 baseOffset = 776 + fHead + static_cast<__int64>(trHead) * static_cast<__int64>(tr_inline);


    // ЧИТАЕМ ВЕСЬ ИНЛАЙН
	const int count = static_cast<int>(tr_inline) * samples;
    std::vector<float> dataBuffer(count);
	fileRead->Seek(baseOffset, soBeginning);
	fileRead->Read(dataBuffer.data(), count * 4);

    // ТЕПЕРЬ НУЖНО ПРАВИЛЬНО УПОРЯДОЧИТЬ ДАННЫЕ
    // В файле данные organized по кросслайнам внутри инлайна
    // Но для отображения инлайна нам нужно, чтобы данные были упорядочены правильно

    float* lineData = new float[count];

    // Копируем данные как есть - предполагая, что порядок в файле правильный
    std::copy(dataBuffer.begin(), dataBuffer.end(), lineData);

    Traces tr;
	tr.init(static_cast<int>(tr_inline), samples, static_cast<int>(dt) * 1000, lineData);
    return tr;
}
//--
Traces readCrosslineRegular(std::wstring fName, int crosslineNumber) {
	double dt = 0.0;
	double sampleCount = 0;
	double trHead = 0;
	int fHead = 0;
	int t1 = 0, l1 = 0, t3 = 0, l3 = 0, hS = 0;
	// Чтение входного файла
	std::unique_ptr<TFileStream> fileRead(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!fileRead || fileRead->Size == 0) {
		std::cerr << "fileError: Cannot open input file" << std::endl;
		throw Exception("bad file");
	}
	// Чтение заголовочной информации
	fileRead->Seek(static_cast<__int64>(6 * 4), soBeginning);
	fileRead->Read(&dt, 8);
	fileRead->Seek(static_cast<__int64>(32 + 6 * 4), soBeginning);
	fileRead->Read(&sampleCount, 8);
	fileRead->Seek(static_cast<__int64>(9 * 32 + 6 * 4), soBeginning);
	fileRead->Read(&trHead, 8);
	fileRead->Seek(static_cast<__int64>(10 * 32 + 4 * 2), soBeginning);
	fileRead->Read(&fHead, 4);
	fileRead->Seek(static_cast<__int64>(12 * 32), soBeginning);
	fileRead->Read(&t1, 4);
	fileRead->Read(&l1, 4);
	fileRead->Read(&t3, 4);
    fileRead->Read(&l3, 4);
    fileRead->Read(&hS, 4);
	int tr_inline = t3 - t1 + 1;
    int lines = l3 - l1 + 1;
	int samples = static_cast<int>(sampleCount);
	// Проверка валидности номера трассы для кросслайна
    if (crosslineNumber < t1 || crosslineNumber > t3) {
        std::cerr << "Error: Crossline trace number " << crosslineNumber
				  << " is out of range [" << t1 << ", " << t3 << "]" << std::endl;
		throw Exception("invalid crossline trace number");
	}
    const int traceIndex = crosslineNumber - t1; // индекс трассы в линии
	const int traceSize = samples * 4; // размер одной трассы в байтах
	const int lineSize = tr_inline * traceSize; // размер одной линии в байтах
	// Вычисление базового смещения до данных
	const __int64 baseOffset = 776 + fHead + static_cast<int>(trHead)*tr_inline*lines;
	// Буфер для данных кросслайна
	const int count = samples * lines;
	std::vector<float> dataBuffer(count);
	// Чтение данных кросслайна: для каждой линии читаем одну трассу
	for (int line = 0; line < lines; ++line) {
		// Смещение до нужной трассы в текущей линии
		__int64 traceOffset = baseOffset + line * lineSize + traceIndex * traceSize;
		// Чтение одной трассы
		fileRead->Seek(traceOffset, soBeginning);
		fileRead->Read(dataBuffer.data() + line * samples, traceSize);
	}
	// Создание копии данных для Traces
	float* crosslineData = new float[count];
	std::copy(dataBuffer.begin(), dataBuffer.end(), crosslineData);
    // Создание и инициализация объекта Traces
	Traces tr;
	tr.init(lines, samples, static_cast<int>(dt * 1000), crosslineData);
//tr.init(samples, lines, static_cast<int>(dt) * 1000, crosslineData);
	return tr;
}
//--
Traces readTimeSliceRegular(std::wstring fName, int sliceNumber) {
	double dt = 0.0;
	double sampleCount = 0;
	double trHead = 0;
	int fHead = 0;
	int t1 = 0, l1 = 0, t3 = 0, l3 = 0, hS = 0;
	// Чтение входного файла
	std::unique_ptr<TFileStream> fileRead(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!fileRead || fileRead->Size == 0)
	{
		std::cerr << "fileError: Cannot open input file" << std::endl;
		throw Exception("bad file");
	}

	fileRead->Seek(static_cast<__int64>(6 * 4), soBeginning);
	fileRead->Read(&dt, 8);
	fileRead->Seek(static_cast<__int64>(32 + 6 * 4), soBeginning);
	fileRead->Read(&sampleCount, 8);
	fileRead->Seek(static_cast<__int64>(9 * 32 + 6 * 4), soBeginning);
	fileRead->Read(&trHead, 8);
	fileRead->Seek(static_cast<__int64>(10 * 32 + 4 * 2), soBeginning);
	fileRead->Read(&fHead, 4);
	fileRead->Seek(static_cast<__int64>(12 * 32), soBeginning);
	fileRead->Read(&t1, 4);
	fileRead->Read(&l1, 4);
	fileRead->Read(&t3, 4);
	fileRead->Read(&l3, 4);
	fileRead->Read(&hS, 4);
	int tr_inline = t3 - t1 + 1;
	int lines = l3 - l1 + 1;
	const __int64 offset = 776 + fHead + static_cast<int>(trHead) * tr_inline * lines + sliceNumber*tr_inline*lines*4;
	const int count = static_cast<int>(lines) * tr_inline;
	std::vector<float> dataBuffer(count);
	fileRead->Seek(offset, soBeginning);
	// Чтение данных для текущей линии
	fileRead->Read(dataBuffer.data(), count * 4);
	// Создание копии данных для Traces
	float* lineData = new float[count];
	std::copy(dataBuffer.begin(), dataBuffer.end(), lineData);
	// Обработка данных
	Traces tr;
	tr.init(lines, tr_inline, static_cast<int>(dt) * 1000, lineData);
	return tr;
}

std::vector<Traces> readCrosslineRGB(std::wstring fName, int trace, float*& freqs,
								 int &fn, int &f1, int &f2, int &fk, int &nf, int &fw)
{
	int dt = 0, sampleCount = 0, linesCount = 0, tracesCount = 0;
	std::unique_ptr<TFileStream> file(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!file || file->Size == 0)
	{
		std::cerr << "fileError" << std::endl;
        return std::vector<Traces>();
    }
	try
	{
        // Чтение заголовка
        file->Read(&dt, 4);
        file->Read(&sampleCount, 4);
        file->Read(&tracesCount, 4);
        file->Read(&linesCount, 4);
		if (trace < 0 || trace >= tracesCount)
		{
            std::cout << "wrong trace: " << trace << ", available: 0-" << tracesCount-1 << std::endl;
            trace = 0;
        }
        file->Read(&nf, 4);
        file->Read(&fn, 4);
        file->Read(&fk, 4);
        file->Read(&fw, 4);
        // Чтение частот
        freqs = new float[nf];
        file->Seek(static_cast<__int64>(32), soBeginning);
		for (int i = 0; i < nf; ++i)
		{
            file->Read(&freqs[i], 4);
        }
        const __int64 headerSize = 32 + nf * 4;
        std::vector<Traces> out(nf);
        // Размеры блоков для правильного расчета смещений
        const __int64 samplesPerTrace = sampleCount;
        const __int64 tracesPerFrequency = tracesCount;
        const __int64 frequenciesPerLine = nf;
        // Размер одной полной линии
        const __int64 lineSize = frequenciesPerLine * tracesPerFrequency * samplesPerTrace * 4;
        // Размер данных для одной частоты в одной линии
        const __int64 frequencySizeInLine = tracesPerFrequency * samplesPerTrace * 4;
        // Размер одной трассы в одной частоте
        const __int64 traceSizeInFrequency = samplesPerTrace * 4;
		for (int freqIndex = 0; freqIndex < nf; ++freqIndex)
		{
            float* data = new float[linesCount * sampleCount];
			for (int line = 0; line < linesCount; ++line)
			{
                // ПРАВИЛЬНОЕ вычисление смещения
                const __int64 lineOffset = headerSize + line * lineSize;
                const __int64 freqOffset = lineOffset + freqIndex * frequencySizeInLine;
                const __int64 traceOffset = freqOffset + trace * traceSizeInFrequency;
                file->Seek(traceOffset, soBeginning);
                // Читаем одну трассу
                float* lineData = data + line * sampleCount;
                file->Read(lineData, sampleCount * 4);
            }
            Traces tr;
            tr.init(linesCount, sampleCount, dt, data);
            out[freqIndex] = tr;
        }
        return out;
	} catch (const Exception& e)
	{
        std::cerr << "Error reading file: " << e.Message.c_str() << std::endl;
        return std::vector<Traces>();
    }
}
//--
std::vector<Traces> readInlineRGB(std::wstring fName, int line, float*& freqs,
							  int &fn, int &f1, int &f2, int &fk, int &nf, int &fw)
{
    int dt = 0, sampleCount = 0, linesCount = 0, tracesCount = 0;
    std::unique_ptr<TFileStream> file(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!file || file->Size == 0)
	{
        std::cerr << "fileError" << std::endl;
		return std::vector<Traces>();
    }
	try
	{
        // Чтение заголовка
        file->Read(&dt, 4);
        file->Read(&sampleCount, 4);
        file->Read(&tracesCount, 4);
        file->Read(&linesCount, 4);
		if (line < 0 || line >= linesCount)
		{
            std::cout << "wrong line: " << line << ", available: 0-" << linesCount-1 << std::endl;
            line = 0;
        }
        file->Read(&nf, 4);
        file->Read(&fn, 4);
        file->Read(&fk, 4);
        file->Read(&fw, 4);
		if (f2 >= fk || f2 <= fn || f1 >= fk || f1 <= fn)
		{
            f1 = fn + (fk - fn) / 3;
            f2 = fn + 2 * (fk - fn) / 3;
        }
        // Чтение частот
        freqs = new float[nf];
        file->Seek(static_cast<__int64>(32), soBeginning);
		for (int i = 0; i < nf; ++i)
		{
            file->Read(&freqs[i], 4);
        }
        // Расчет правильного смещения
        const __int64 headerSize = 32 + nf * 4;
        const __int64 lineSize = tracesCount * sampleCount * nf * 4; // размер одной линии
        const __int64 offset = headerSize + line * lineSize;
        std::vector<Traces> out(nf);
        // Чтение данных для каждой частоты
		for (int n = 0; n < nf; ++n)
		{
            float* data = new float[tracesCount * sampleCount];
            // Для каждой частоты смещение внутри линии: n * (tracesCount * sampleCount * 4)
            __int64 freqOffset = offset + n * (tracesCount * sampleCount * 4);
            file->Seek(freqOffset, soBeginning);
            // Читаем все трассы для этой частоты
            file->Read(data, tracesCount * sampleCount * 4);
            Traces tr;
            tr.init(tracesCount, sampleCount, dt, data);
            out[n] = tr;
        }
        return out;
	} catch (const Exception& e)
	{
        std::cerr << "Error reading file: " << e.Message.c_str() << std::endl;
        return std::vector<Traces>();
	}
}
//--
std::vector<Traces> readTimeSliceRGB(std::wstring fileName, int timeSample,
													 float*& freqs, int& fn,
													 int& f1, int& f2, int& fk, int& nf, int& fw) {
	// Открываем файл
	int sampleCount;
	int tracesCount;
	int linesCount;
	int dt;
    std::unique_ptr<TFileStream> file(new TFileStream(fileName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!file || file->Size == 0) {
        throw std::runtime_error("Cannot open file");
    }
    // Читаем заголовок
    file->Read(&dt, 4);
    file->Read(&sampleCount, 4);
    file->Read(&tracesCount, 4);
    file->Read(&linesCount, 4);
    file->Read(&nf, 4);
    file->Read(&fn, 4);
    file->Read(&fk, 4);
    file->Read(&fw, 4);
    // Читаем частоты
    freqs = new float[nf];
    file->Seek(static_cast<__int64>(32), soBeginning);
    for (int i = 0; i < nf; ++i) {
        file->Read(&freqs[i], 4);
    }
    // Проверяем корректность временного среза
    if (timeSample < 0 || timeSample >= sampleCount) {
        timeSample = 0;
    }
    const size_t headerSize = 32 + nf * 4;
    const size_t timeSliceSize = nf * linesCount * tracesCount * 4;
    // Позиционируемся на нужный временной срез
    file->Seek(headerSize + timeSample * timeSliceSize, soBeginning);
    // Читаем данные для всех частот
    std::vector<Traces> result(nf);
    for (int n = 0; n < nf; ++n) {
        float* data = new float[linesCount * tracesCount];
        file->Read(data, linesCount * tracesCount * 4);
        Traces trace;
        trace.init(linesCount, tracesCount, dt, data);
        result[n] = trace;
    }
    return result;
}
//--

size readSize(std::wstring fName) {
	double sampleCount = 0;
	int t1 = 0, l1 = 0, t3 = 0, l3 = 0, hS = 0;
	// Чтение входного файла
	std::unique_ptr<TFileStream> fileRead(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!fileRead || fileRead->Size == 0)
	{
		std::cerr << "fileError: Cannot open input file" << std::endl;
		throw Exception("bad file");
	}

	fileRead->Seek(static_cast<__int64>(32 + 6 * 4), soBeginning);
	fileRead->Read(&sampleCount, 8);
	fileRead->Seek(static_cast<__int64>(12 * 32), soBeginning);
	fileRead->Read(&t1, 4);
	fileRead->Read(&l1, 4);
	fileRead->Read(&t3, 4);
	fileRead->Read(&l3, 4);
	fileRead->Read(&hS, 4);
	int tr_inline = t3 - t1 + 1;
	int lines = l3 - l1 + 1;
	fileRead->Seek((__int64)0, soBeginning);
	return {(int)sampleCount, tr_inline, lines};
}
size4 readSizeRGB(std::wstring fName, std::vector<float>& freqOut) {
		int dt = 0, sampleCount = 0, linesCount = 0, tracesCount = 0, nf = 0;
	std::unique_ptr<TFileStream> file(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!file || file->Size == 0)
	{
		std::cerr << "fileError" << std::endl;
		return {0,0,0,0};
	}
	try
	{
		// Чтение заголовка
		file->Read(&dt, 4);
		file->Read(&sampleCount, 4);
		file->Read(&tracesCount, 4);
		file->Read(&linesCount, 4);
		file->Read(&nf, 4);
		file->Seek(static_cast<__int64>(32), soBeginning);
        freqOut.resize(nf);
		for (int i = 0; i < nf; ++i)
		{
			float temp;
			file->Read(&temp, 4);
			freqOut[i] = temp;
		}
	}
	catch(...){
		return {0,0,0,0};
	}
	return {sampleCount, tracesCount, linesCount, nf};
}


std::shared_ptr<RgbData> readCrosslineRGB(std::wstring fName, int trace)
{
	float* freqs;
	int fn, fk, nf, fw;
	int dt = 0, sampleCount = 0, linesCount = 0, tracesCount = 0;
    std::unique_ptr<TFileStream> file(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
    if (!file || file->Size == 0)
    {
		std::cerr << "fileError" << std::endl;
        return nullptr;
    }
    try
    {
        // Чтение заголовка
        file->Read(&dt, 4);
        file->Read(&sampleCount, 4);
        file->Read(&tracesCount, 4);
        file->Read(&linesCount, 4);
        if (trace < 0 || trace >= tracesCount)
        {
            std::cout << "wrong trace: " << trace << ", available: 0-" << tracesCount-1 << std::endl;
            trace = 0;
        }
        file->Read(&nf, 4);
        file->Read(&fn, 4);
        file->Read(&fk, 4);
        file->Read(&fw, 4);

        // Чтение частот
        freqs = new float[nf];
		file->Seek(static_cast<__int64>(32), soBeginning);
		for (int i = 0; i < nf; ++i)
		{
			file->Read(&freqs[i], 4);
		}

        const __int64 headerSize = 32 + nf * 4;

        // Подготовка трехмерного вектора для RgbData
        std::vector<std::vector<std::vector<float>>> data3D(nf);

        const __int64 samplesPerTrace = sampleCount;
        const __int64 tracesPerFrequency = tracesCount;
        const __int64 frequenciesPerLine = nf;
        const __int64 lineSize = frequenciesPerLine * tracesPerFrequency * samplesPerTrace * 4;
        const __int64 frequencySizeInLine = tracesPerFrequency * samplesPerTrace * 4;
        const __int64 traceSizeInFrequency = samplesPerTrace * 4;

        for (int freqIndex = 0; freqIndex < nf; ++freqIndex)
        {
            std::vector<std::vector<float>> frequencyData(linesCount);
            float* data = new float[linesCount * sampleCount];

            for (int line = 0; line < linesCount; ++line)
            {
                const __int64 lineOffset = headerSize + line * lineSize;
                const __int64 freqOffset = lineOffset + freqIndex * frequencySizeInLine;
                const __int64 traceOffset = freqOffset + trace * traceSizeInFrequency;
                file->Seek(traceOffset, soBeginning);

                float* lineData = data + line * sampleCount;
                file->Read(lineData, sampleCount * 4);

                // Заполняем вектор для текущей линии
                std::vector<float> lineVector(sampleCount);
                for (int s = 0; s < sampleCount; ++s) {
                    lineVector[s] = lineData[s];
                }
                frequencyData[line] = lineVector;
            }

            data3D[freqIndex] = frequencyData;
            delete[] data; // Освобождаем временный массив
        }

        // Создаем RgbData
        auto rgbData = std::make_shared<RgbData>(data3D, dt*1000, freqs[0], freqs[nf-1], "", "");
        return rgbData;

    } catch (const Exception& e)
    {
        std::cerr << "Error reading file: " << e.Message.c_str() << std::endl;
        if (freqs) {
            delete[] freqs;
            freqs = nullptr;
        }
        return nullptr;
    }
}

std::shared_ptr<RgbData> readInlineRGB(std::wstring fName, int line)
{
	float* freqs;
	int fn, fk, nf, fw;
	int dt = 0, sampleCount = 0, linesCount = 0, tracesCount = 0;
	std::unique_ptr<TFileStream> file(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
	if (!file || file->Size == 0)
	{
		std::cerr << "fileError" << std::endl;
		return nullptr;
	}
	try
	{
		// Чтение заголовка
		file->Read(&dt, 4);
		file->Read(&sampleCount, 4);
		file->Read(&tracesCount, 4);
		file->Read(&linesCount, 4);
		if (line < 0 || line >= linesCount)
		{
			std::cout << "wrong line: " << line << ", available: 0-" << linesCount-1 << std::endl;
			line = 0;
		}
		file->Read(&nf, 4);
        file->Read(&fn, 4);
        file->Read(&fk, 4);
		file->Read(&fw, 4);

        // Чтение частот
        freqs = new float[nf];
        file->Seek(static_cast<__int64>(32), soBeginning);
        for (int i = 0; i < nf; ++i)
        {
            file->Read(&freqs[i], 4);
        }

        const __int64 headerSize = 32 + nf * 4;
        const __int64 lineSize = tracesCount * sampleCount * nf * 4;
        const __int64 offset = headerSize + line * lineSize;

        // Подготовка трехмерного вектора для RgbData
        std::vector<std::vector<std::vector<float>>> data3D(nf);

        for (int n = 0; n < nf; ++n)
        {
            std::vector<std::vector<float>> frequencyData(tracesCount);
            float* data = new float[tracesCount * sampleCount];

            __int64 freqOffset = offset + n * (tracesCount * sampleCount * 4);
            file->Seek(freqOffset, soBeginning);
            file->Read(data, tracesCount * sampleCount * 4);

            // Заполняем векторы
            for (int trace = 0; trace < tracesCount; ++trace) {
                std::vector<float> traceVector(sampleCount);
                for (int s = 0; s < sampleCount; ++s) {
                    traceVector[s] = data[trace * sampleCount + s];
                }
                frequencyData[trace] = traceVector;
            }

            data3D[n] = frequencyData;
            delete[] data; // Освобождаем временный массив
        }

        // Создаем RgbData
		auto rgbData = std::make_shared<RgbData>(data3D, dt*1000, freqs[0], freqs[nf-1], "", "");
        return rgbData;

	} catch (const Exception& e)
    {
        std::cerr << "Error reading file: " << e.Message.c_str() << std::endl;
        if (freqs) {
            delete[] freqs;
            freqs = nullptr;
        }
        return nullptr;
    }
}

std::shared_ptr<RgbData> readTimeSliceRGB(std::wstring fileName, int timeSample) {
	float* freqs;
	int fn, fk, nf, fw;
    int sampleCount;
    int tracesCount;
    int linesCount;
    int dt;
    std::unique_ptr<TFileStream> file(new TFileStream(fileName.c_str(), fmOpenRead | fmShareDenyWrite));
    if (!file || file->Size == 0) {
        throw std::runtime_error("Cannot open file");
    }

    try {
        // Читаем заголовок
        file->Read(&dt, 4);
        file->Read(&sampleCount, 4);
        file->Read(&tracesCount, 4);
        file->Read(&linesCount, 4);
        file->Read(&nf, 4);
        file->Read(&fn, 4);
        file->Read(&fk, 4);
        file->Read(&fw, 4);

        // Читаем частоты
        freqs = new float[nf];
        file->Seek(static_cast<__int64>(32), soBeginning);
        for (int i = 0; i < nf; ++i) {
            file->Read(&freqs[i], 4);
        }

        // Проверяем корректность временного среза
        if (timeSample < 0 || timeSample >= sampleCount) {
            timeSample = 0;
        }

        const size_t headerSize = 32 + nf * 4;
        const size_t timeSliceSize = nf * linesCount * tracesCount * 4;
        file->Seek(headerSize + timeSample * timeSliceSize, soBeginning);

        // Подготовка трехмерного вектора для RgbData
        std::vector<std::vector<std::vector<float>>> data3D(nf);

        for (int n = 0; n < nf; ++n) {
            std::vector<std::vector<float>> frequencyData(linesCount);
            float* data = new float[linesCount * tracesCount];
            file->Read(data, linesCount * tracesCount * 4);

            // Заполняем векторы
            for (int line = 0; line < linesCount; ++line) {
                std::vector<float> lineVector(tracesCount);
                for (int trace = 0; trace < tracesCount; ++trace) {
                    lineVector[trace] = data[line * tracesCount + trace];
                }
                frequencyData[line] = lineVector;
            }

            data3D[n] = frequencyData;
            delete[] data; // Освобождаем временный массив
        }

        // Создаем RgbData
		auto rgbData = std::make_shared<RgbData>(data3D, dt*1000, freqs[0], freqs[nf-1], "", "");
        return rgbData;

    } catch (const Exception& e) {
        std::cerr << "Error reading file: " << e.Message.c_str() << std::endl;
        if (freqs) {
            delete[] freqs;
            freqs = nullptr;
        }
        return nullptr;
    }
}
