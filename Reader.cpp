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
Traces readXlineRegular(std::wstring fName, int lineNumber) {
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
    fileRead = std::make_unique<TFileStream>(ChangeFileExt(fName.c_str(), L".xbm").c_str(), fmOpenRead | fmShareDenyWrite);
    int tr_inline = t3 - t1 + 1;
    int lines = l3 - l1 + 1;
    int samples = static_cast<int>(sampleCount);

    // Проверка валидности номера инлайна
    if (lineNumber < t1 || lineNumber > t3) {
        std::cerr << "Error: Inline number " << lineNumber
                  << " is out of range [" << t1 << ", " << t3 << "]" << std::endl;
        throw Exception("invalid inline number");
    }

    // ВЫЧИСЛЯЕМ ИНДЕКС ОТНОСИТЕЛЬНО НАЧАЛА
    int inlineIndex = lineNumber - t1;

    // РАЗМЕРЫ В БАЙТАХ
    const int traceSize = samples * 4; // размер одной трассы в байтах
    const int xlineSize = lines * traceSize; // размер одного инлайна в байтах

    // БАЗОВОЕ СМЕЩЕНИЕ (как в кросслайновой версии)
    const __int64 baseOffset = /*776 +*/ fHead + static_cast<__int64>(trHead) * tr_inline * lines;

    // СМЕЩЕНИЕ ДО НУЖНОГО ИНЛАЙНА
    const __int64 offset = baseOffset + inlineIndex * xlineSize;

    // ЧИТАЕМ ВЕСЬ ИНЛАЙН
    const int count = lines * samples;
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
    tr.init(lines, samples, static_cast<int>(dt) * 1000, lineData);
    return tr;
}
//--
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
	std::unique_ptr<TFileStream> fileRead(new TFileStream(ChangeFileExt(fName.c_str(), L".ibm").c_str(), fmOpenRead | fmShareDenyWrite));
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
    fileRead = std::make_unique<TFileStream>(ChangeFileExt(fName.c_str(), L".sbm").c_str(), fmOpenRead | fmShareDenyWrite);
	int tr_inline = t3 - t1 + 1;
	int lines = l3 - l1 + 1;
	const __int64 offset = /*776 +*/ fHead + static_cast<int>(trHead) * tr_inline * lines + sliceNumber*tr_inline*lines*4;
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
std::vector<Traces> tracesToSwanEn(const Traces& input, bool timeSwan, int nf, int fn,
									int fk, int filterWidth, float* freqs)
{
    const double dt_val = input.dt / 1000000.0;
    const double log_coeff = std::log(static_cast<double>(fk) / fn) / (nf-1);
    const double df = filterWidth / 100.0;
    const int l = input.samplesNumber;
    const int w = input.tracesNumber;
	const float* data = input.data;
    // Предварительные вычисления коэффициентов
    std::vector<double> f_(nf), at_(nf), b_(nf), ab_(nf), ab1_(nf);
	std::vector<float> f_factor_(nf);

	for (int i = 0; i < nf; ++i)
	{
        f_[i] = fn * std::exp(i * log_coeff);
        at_[i] = std::exp(-2 * f_[i] * df * dt_val);
        b_[i] = 2.0 * at_[i] * std::cos(2.0 * M_PI * dt_val * f_[i]);
        ab_[i] = at_[i] * at_[i];
        ab1_[i] = ab_[i] / 2.0;
		f_factor_[i] = std::pow(f_[i], 1.1f);
		freqs[i] = static_cast<float>(f_[i]);
	}
    // Подготовка выходных данных
    std::vector<Traces> output(nf);
	for (int i = 0; i < nf; ++i)
	{
		output[i].init(w, l, input.dt, new float[l * w]);
	}

    // Выделение памяти для данных с затуханием
	std::vector<float> data_tapered(l * w);
	#pragma omp parallel for
	for (int r = 0; r < w; ++r)
	{
        const int offset = r * l;
		for (int i = 0; i < l; ++i)
		{
            float val = data[i + offset];
			if (i < 4) val *= i * 0.2f;
			else if (i >= l-4) val *= (l - i - 1) * 0.2f;
			data_tapered[i + offset] = val;
		}
	}
    // Основной вычислительный блок
#pragma omp parallel
	{
		// Выделение буферов для каждого потока (исключаем аллокации в цикле)
		std::vector<float> h_buf(l, 0.0f);
		std::vector<float> out_buf(l, 0.0f);

        // Параллелизация по каналам
#pragma omp for schedule(guided)
		for (int r = 0; r < w; ++r)
		{
			const int trace_offset = r * l;
			float* trace_in = data_tapered.data() + trace_offset;

			for (int n2 = 0; n2 < nf; ++n2)
			{
                float* h = h_buf.data();
                float* out = out_buf.data();
				float* out_trace = output[n2].data + trace_offset;

                // Инициализация граничных условий
                h[0] = h[1] = 0.0f;
                out[l-1] = out[l-2] = 0.0f;

                const float b_val = b_[n2];
                const float ab_val = ab_[n2];
                const float ab1_val = ab1_[n2];
				const float f_factor = f_factor_[n2];
				// Прямой фильтр
				for (int n = 2; n < l; ++n)
				{
                    const float s_val = -trace_in[n] * 0.5f + ab1_val * trace_in[n-2];
                    h[n] = s_val + b_val * h[n-1] - ab_val * h[n-2];
                }

                // Обратный фильтр
				for (int n = l-3; n >= 0; --n)
				{
                    const float s_val = -h[n] * 0.5f + ab1_val * h[n+2];
                    out[n] = s_val + b_val * out[n+1] - ab_val * out[n+2];
				}
				// Частотная коррекция и квадрат
				for (int n = 0; n < l; ++n) {
					if (std::abs(out[n]) > 1e17)
					{
						out_trace[n] = FLT_MAX;
					} else
					{
						const double val = out[n] * f_factor;
						out_trace[n] = static_cast<float>(val * val);
					}
				}
			}
		}
    }

    return output;
}



void calculateCube(std::wstring fName, int nf, int fn, int fk, int filterWidth)
{
	double dt = 0.0;
	double sampleCount = 0;
	double trHead = 0;
	int fHead = 0;
	int t1 = 0, l1 = 0, t3 = 0, l3 = 0, hS = 0;
	// Чтение входного файла
    std::unique_ptr<TFileStream> fileRead(new TFileStream((fName+L".ibm").c_str(), fmOpenRead | fmShareDenyWrite));
	if (!fileRead || fileRead->Size == 0)
	{
		ShowMessage((L"fileError: Cannot open input file" + fName + L".ibm").c_str());
		return;
	}
	try
	{
		// Чтение заголовка
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
		std::cout << "dt=" << dt << std::endl;
		std::cout << "sampleCount=" << sampleCount << std::endl;
		std::cout << "trHead=" << trHead << std::endl;
		std::cout << "fHead=" << fHead << std::endl;
		std::cout << "t1=" << t1 << std::endl;
		std::cout << "l1=" << l1 << std::endl;
		std::cout << "t3=" << t3 << std::endl;
        std::cout << "l3=" << l3 << std::endl;
		std::cout << "hS=" << hS << std::endl;
		int tr_inline = t3 - t1 + 1;
		int lines = l3 - l1 + 1;
		const __int64 offset = 776 + fHead + static_cast<int>(trHead) * tr_inline * lines;
		const int count = static_cast<int>(sampleCount) * tr_inline;
		// Создание выходного файла
		std::wstring outputFileName = fName + L"\\" + std::to_wstring(nf) + L"_" + std::to_wstring(fn)
		+ L"_" + std::to_wstring(fk) + L".irgb";
		std::unique_ptr<TFileStream> fileWrite(new TFileStream(outputFileName.c_str(), fmCreate | fmShareExclusive));
        // Запись заголовка
		int dt_toWrite = static_cast<int>(dt);
        fileWrite->Write(&dt_toWrite, 4);
		int sampleCount_toWrite = static_cast<int>(sampleCount);
		fileWrite->Write(&sampleCount_toWrite, 4);
		fileWrite->Write(&tr_inline, 4);        // Traces in line
		fileWrite->Write(&lines, 4);            // Lines count
		fileWrite->Write(&nf, 4);               // nf
		fileWrite->Write(&fn, 4);               // fn
		fileWrite->Write(&fk, 4);               // fk
		fileWrite->Write(&filterWidth, 4);      // fw
		// Запись частот
		const double log_coeff = std::log(static_cast<double>(fk) / fn) / (nf - 1);
		float* freqs = new float[nf];
		for (int i = 0; i < nf; ++i)
		{
			float f = fn * std::exp(i * log_coeff);
			freqs[i] = f;
            fileWrite->Write(&f, 4);
			//std::cout << "Frequency " << i << ": " << f << std::endl;
		}
		// Буфер для чтения данных
		std::vector<float> dataBuffer(count);
		fileRead->Seek(offset, soBeginning);
		// Обработка каждой линии
        TLoading* load = new TLoading(Application->MainForm);
        load->setDuration(lines);
        load->Show();
		for (int line = 0; line < lines; ++line)
		{
			// Чтение данных для текущей линии
			fileRead->Read(dataBuffer.data(), count * 4);
			// Создание копии данных для Traces
			float* lineData = new float[count];
			std::copy(dataBuffer.begin(), dataBuffer.end(), lineData);
			// Обработка данных
			Traces tr;
			tr.init(tr_inline, static_cast<int>(sampleCount), static_cast<int>(dt) * 1000, lineData);
			std::vector<Traces> out = tracesToSwanEn(tr, false, nf, fn, fk, filterWidth, freqs);
			// Запись обработанных данных в правильном порядке
			for (int n = 0; n < nf; ++n)
			{
                // Для каждой частоты записываем все трассы текущей линии
				fileWrite->Write(out[n].data, count * 4);
			}

            //std::cout << "Finished line " << line << " of " << lines << std::endl;
            // Проверка на прерывание
			if (Application->Terminated) {
				break;
			}

            load->update(1);
		}
        delete[] freqs;
		ShowMessage("File successfully written");
	} catch (const Exception& e)
	{
		ShowMessage((L"Error: " + e.Message).c_str());
	}
}
//--
void convertCube(const std::wstring& fName)
{
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
		return;
	}
	// Чтение заголовка
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
	const __int64 offset = 776 + fHead + static_cast<int>(trHead) * tr_inline * lines;
	const int count = static_cast<int>(sampleCount) * tr_inline;
	// Создаем выходной файл
	HANDLE hOutputFile = CreateFile((fName + L"s").c_str(),
								   GENERIC_READ | GENERIC_WRITE,
								   0,
								   NULL,
								   CREATE_ALWAYS,
								   FILE_ATTRIBUTE_NORMAL,
								   NULL);
	if (hOutputFile == INVALID_HANDLE_VALUE) {
		throw std::system_error(GetLastError(), std::system_category(), "Cannot create output file");
	}
        LARGE_INTEGER fileSize;
	fileSize.QuadPart = offset + (__int64)sampleCount * lines * tr_inline * 4;
	if (!SetFilePointerEx(hOutputFile, fileSize, NULL, FILE_BEGIN) || !SetEndOfFile(hOutputFile)) {
		CloseHandle(hOutputFile);
		throw std::system_error(GetLastError(), std::system_category(), "Cannot set file size");
	}
    // Создаем memory mapping
	HANDLE hMapping = CreateFileMapping(hOutputFile, NULL, PAGE_READWRITE,
									   fileSize.HighPart, fileSize.LowPart, NULL);
	if (!hMapping) {
		CloseHandle(hOutputFile);
		throw std::system_error(GetLastError(), std::system_category(), "Cannot create file mapping");
	}
    // Маппим файл
	BYTE* pData = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, fileSize.QuadPart);
	if (!pData) {
		CloseHandle(hMapping);
		CloseHandle(hOutputFile);
		throw std::system_error(GetLastError(), std::system_category(), "Cannot map view of file");
	}
                     // Копируем заголовок
	fileRead->Seek(static_cast<__int64>(0), soBeginning);
	fileRead->Read(pData, offset);
    // Буфер для чтения одной линии
	std::vector<float> lineData(tr_inline * sampleCount);
	fileRead->Seek(offset, soBeginning);
	const size_t lineSizeInBytes = tr_inline * sampleCount * 4;
		// Обрабатываем каждую линию
	TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(lines);
	load->Show();
	for (int line = 0; line < lines; ++line) {
        // Читаем данные линии
		fileRead->Read(lineData.data(), lineSizeInBytes);
		// Обрабатываем каждый временной срез и частоту
		for (int t = 0; t < sampleCount; ++t) {
			// Вычисляем позицию в выходном файле
			const __int64 offset1 = offset +
				(t * (__int64)lines * tr_inline +
				 line * (__int64)tr_inline) * 4;
			float* target = (float*)(pData + offset1);
			// Копируем данные для всех трасс
			for (int tr = 0; tr < tr_inline; ++tr) {
				const size_t srcIndex = tr * sampleCount + t;
				target[tr] = lineData[srcIndex];
			}
		}
		load->update(1);
	}
	ShowMessage("FinishedConversion");
	// Убираем mapping и закрываем файлы
	UnmapViewOfFile(pData);
	CloseHandle(hMapping);
	CloseHandle(hOutputFile);
}
//void convertToSliceOrder_b(const std::wstring& inputFileName, TComponent* Sender) {
//	 // Открываем исходный файл
//	auto start_time = std::chrono::high_resolution_clock::now();
//	std::unique_ptr<TFileStream> fileRead(new TFileStream(inputFileName.c_str(), fmOpenRead | fmShareDenyWrite));
//    if (!fileRead || fileRead->Size == 0) {
//		std::cerr << "Cannot open input file" << std::endl;
//		return;
//    }
//
//    // Читаем заголовок
//    int dt, sampleCount, tracesCount, linesCount, nf, fn, fk, fw;
//	fileRead->Read(&dt, 4);
//	fileRead->Read(&sampleCount, 4);
//	fileRead->Read(&tracesCount, 4);
//	fileRead->Read(&linesCount, 4);
//	fileRead->Read(&nf, 4);
//    fileRead->Read(&fn, 4);
//	fileRead->Read(&fk, 4);
//	fileRead->Read(&fw, 4);
//
//	// Читаем частоты
//	fileRead->Seek((__int64)32, soBeginning);
//	std::vector<float> freqs(nf);
//    for (int i = 0; i < nf; ++i) {
//        fileRead->Read(&freqs[i], 4);
//    }
//
//    const size_t headerSize = 32 + nf * 4;
//    const size_t lineSizeInBytes = static_cast<size_t>(tracesCount) * sampleCount * nf * 4;
//
//    // Создаем выходной файл
//    HANDLE hOutputFile = CreateFileW((inputFileName + L"s").c_str(),
//                                    GENERIC_READ | GENERIC_WRITE,
//                                    0,
//                                    NULL,
//                                    CREATE_ALWAYS,
//                                    FILE_ATTRIBUTE_NORMAL,
//									NULL);
//    if (hOutputFile == INVALID_HANDLE_VALUE) {
//        throw std::system_error(GetLastError(), std::system_category(), "Cannot create output file");
//    }
//
//    // Устанавливаем размер файла
//    LARGE_INTEGER fileSize;
//    fileSize.QuadPart = headerSize + static_cast<__int64>(sampleCount) * nf * linesCount * tracesCount * 4;
//	if (!SetFilePointerEx(hOutputFile, fileSize, NULL, FILE_BEGIN) || !SetEndOfFile(hOutputFile)) {
//        CloseHandle(hOutputFile);
//        throw std::system_error(GetLastError(), std::system_category(), "Cannot set file size");
//    }
//
//    // Создаем memory mapping
//    HANDLE hMapping = CreateFileMappingW(hOutputFile, NULL, PAGE_READWRITE,
//                                        fileSize.HighPart, fileSize.LowPart, NULL);
//	if (!hMapping) {
//        CloseHandle(hOutputFile);
//        throw std::system_error(GetLastError(), std::system_category(), "Cannot create file mapping");
//    }
//
//    // Маппим файл
//    BYTE* pData = static_cast<BYTE*>(MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, fileSize.QuadPart));
//    if (!pData) {
//		CloseHandle(hMapping);
//        CloseHandle(hOutputFile);
//        throw std::system_error(GetLastError(), std::system_category(), "Cannot map view of file");
//    }
//
//	// Копируем заголовок
//	fileRead->Seek((__int64)0, soBeginning);
//	fileRead->Read(pData, headerSize);
//
//    // Размер буфера для обработки (50 МБ)
//    const size_t bufferSize = 50 * 1024 * 1024;
//    const size_t floatsInBuffer = bufferSize / sizeof(float);
//    std::vector<float> buffer(floatsInBuffer);
//
//	// Рассчитываем размер данных для одной частоты в одной линии
//    const size_t floatsPerFrequency = tracesCount * sampleCount;
//	TLoading* load = new TLoading(Sender);
//	load->setDuration(linesCount);
//	load->Show();
//	//fileRead->Seek(headerSize, soBeginning);
//	// Обрабатываем данные частями
//	for (int line = 0; line < linesCount; ++line) {
//        //const __int64 lineOffset = headerSize + static_cast<__int64>(line) * lineSizeInBytes;
//		//fileRead->Seek(lineOffset, soBeginning);
//
//        // Читаем всю линию частями
//        size_t totalFloatsRead = 0;
//		while (totalFloatsRead < static_cast<size_t>(tracesCount) * sampleCount * nf) {
//            size_t floatsToRead = std::min(floatsInBuffer,
//                static_cast<size_t>(tracesCount) * sampleCount * nf - totalFloatsRead);
//            fileRead->Read(buffer.data(), floatsToRead * sizeof(float));
//
//			// Обрабатываем прочитанные данные
//            for (size_t i = 0; i < floatsToRead; ++i) {
//                size_t totalIndex = totalFloatsRead + i;
//                int n = totalIndex / floatsPerFrequency;
//                int remainder = totalIndex % floatsPerFrequency;
//				int tr = remainder / sampleCount;
//                int t = remainder % sampleCount;
//
//                __int64 outputOffset = headerSize +
//                    (static_cast<__int64>(t) * nf * linesCount * tracesCount +
//					 static_cast<__int64>(n) * linesCount * tracesCount +
//                     static_cast<__int64>(line) * tracesCount + tr) * sizeof(float);
//
//                *reinterpret_cast<float*>(pData + outputOffset) = buffer[i];
//            }
//			totalFloatsRead += floatsToRead;
//        }
//
//        load->update(1);
//    }
//
//    // Убираем mapping и закрываем файлы
//	UnmapViewOfFile(pData);
//    CloseHandle(hMapping);
//	CloseHandle(hOutputFile);
//	auto end_time = std::chrono::high_resolution_clock::now();
//	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time-start_time);
//	ShowMessage("Time takenText: " + IntToStr(duration.count()) + " microseconds");
//}

void convertToSliceOrder(const std::wstring& inputFileName) {
    // Получаем информацию о системе для выравнивания
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD allocationGranularity = sysInfo.dwAllocationGranularity;
    // Открываем исходный файл
    HANDLE hInputFile = CreateFileW(ChangeFileExt(inputFileName.c_str(), L".irgb").c_str(),
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                   NULL);
    if (hInputFile == INVALID_HANDLE_VALUE) {
        ShowMessage(GetLastError());
        //throw std::system_error(GetLastError(), std::system_category(), "Cannot open input file");
    }
	// Читаем заголовок
    int dt, sampleCount, tracesCount, linesCount, nf, fn, fk, fw;
    DWORD bytesRead;
    ReadFile(hInputFile, &dt, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &sampleCount, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &tracesCount, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &linesCount, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &nf, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &fn, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &fk, 4, &bytesRead, NULL);
    ReadFile(hInputFile, &fw, 4, &bytesRead, NULL);
//    std::cout << "Parameters: lines=" << linesCount << ", traces=" << tracesCount
//              << ", samples=" << sampleCount << ", frequencies=" << nf << std::endl;

    // Читаем частоты
    SetFilePointer(hInputFile, 32, NULL, FILE_BEGIN);
    std::vector<float> freqs(nf);
    for (int i = 0; i < nf; ++i) {
        ReadFile(hInputFile, &freqs[i], 4, &bytesRead, NULL);
    }
    const size_t headerSize = 32 + nf * 4;
    const size_t inputLineSize = static_cast<size_t>(tracesCount) * sampleCount * nf * 4;
    const size_t outputSliceSize = static_cast<size_t>(nf) * linesCount * tracesCount * 4;
    const size_t outputFileSize = headerSize + static_cast<size_t>(sampleCount) * outputSliceSize;

    // Создаем выходной файл
    HANDLE hOutputFile = CreateFileW(ChangeFileExt(inputFileName.c_str(), L".srgb").c_str(),
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
    if (hOutputFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hInputFile);
        throw std::system_error(GetLastError(), std::system_category(), "Cannot create output file");
    }
    // Устанавливаем размер файла
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = outputFileSize;
    if (!SetFilePointerEx(hOutputFile, fileSize, NULL, FILE_BEGIN) || !SetEndOfFile(hOutputFile)) {
        CloseHandle(hInputFile);
        CloseHandle(hOutputFile);
        throw std::system_error(GetLastError(), std::system_category(), "Cannot set file size");
    }
    // Записываем заголовок
    SetFilePointer(hInputFile, 0, NULL, FILE_BEGIN);
    std::vector<BYTE> headerBuffer(headerSize);
    ReadFile(hInputFile, headerBuffer.data(), headerSize, &bytesRead, NULL);

    DWORD bytesWritten;
    SetFilePointer(hOutputFile, 0, NULL, FILE_BEGIN);
    WriteFile(hOutputFile, headerBuffer.data(), headerSize, &bytesWritten, NULL);

    TLoading* load = new TLoading(Application->MainForm);

	// ОПРЕДЕЛЯЕМ СТРАТЕГИЮ: разбиваем на куски по 2 ГБ для избежания своппинга
	const size_t CHUNK_SIZE = 2ULL * 1024 * 1024 * 1024; // 2 ГБ chunks
	const size_t floatsPerChunk = CHUNK_SIZE / sizeof(float);

    // Вычисляем, сколько временных срезов помещается в один chunk
    size_t slicesPerChunk = CHUNK_SIZE / outputSliceSize;

    if (slicesPerChunk == 0) slicesPerChunk = 1; // Если срез больше chunk, обрабатываем по одному

//	std::cout << "Slices per chunk: " << slicesPerChunk << std::endl;
//	std::cout << "Total chunks: " << ((sampleCount + slicesPerChunk - 1) / slicesPerChunk) << std::endl;

	load->setDuration((sampleCount + slicesPerChunk - 1) / slicesPerChunk*(int)(linesCount/10)); // Прогресс по временным срезам
    load->Show();

    // Буфер для чтения одной линии
    const size_t lineSizeInFloats = tracesCount * sampleCount * nf;
	std::vector<float> lineBuffer(lineSizeInFloats);

    // ОСНОВНОЙ АЛГОРИТМ: обработка по chunks временных срезов
    for (size_t chunkStart = 0; chunkStart < sampleCount; chunkStart += slicesPerChunk) {
        size_t chunkEnd = std::min(chunkStart + slicesPerChunk, static_cast<size_t>(sampleCount));
		size_t chunkSlices = chunkEnd - chunkStart;

		//std::cout << "Processing chunk: " << chunkStart << " to " << chunkEnd-1
				  //<< " (" << chunkSlices << " slices)" << std::endl;

        // Вычисляем размер и смещение для текущего chunk
        LARGE_INTEGER chunkOffset;
		chunkOffset.QuadPart = headerSize + static_cast<__int64>(chunkStart) * outputSliceSize;

        SIZE_T chunkSize = chunkSlices * outputSliceSize;

		if (chunkSize == 0) {
			std::cerr << "Warning: chunkSize is 0, skipping" << std::endl;
            continue;
		}
        // ВЫРАВНИВАЕМ смещение и размер для memory mapping
		LARGE_INTEGER alignedOffset;
		alignedOffset.QuadPart = (chunkOffset.QuadPart / allocationGranularity) * allocationGranularity;
        DWORD offsetInBlock = static_cast<DWORD>(chunkOffset.QuadPart - alignedOffset.QuadPart);
		SIZE_T alignedSize = ((chunkSize + offsetInBlock + allocationGranularity - 1) / allocationGranularity) * allocationGranularity;
		if (alignedSize > 4096ULL * 1024 * 1024) { // Ограничиваем 1 ГБ
            std::cerr << "Warning: alignedSize too large: " << alignedSize << ", reducing chunk" << std::endl;
            slicesPerChunk = slicesPerChunk / 2;
			if (slicesPerChunk == 0) slicesPerChunk = 1;
            continue; // Перезапускаем с меньшим chunk
		}
        // СОЗДАЕМ MEMORY MAPPING только для текущего chunk
		HANDLE hChunkMapping = CreateFileMappingW(hOutputFile, NULL, PAGE_READWRITE,
												(alignedOffset.QuadPart + alignedSize) >> 32, (alignedOffset.QuadPart + alignedSize) & 0xFFFFFFFF, NULL);
		if (!hChunkMapping) {
            DWORD err = GetLastError();
			//std::cerr << "CreateFileMapping failed for chunk at offset " << chunkOffset.QuadPart
					  //<< ", error: " << err << std::endl;
			ShowMessage("problem1");
			ShowMessage(err);
			if (slicesPerChunk > 1) {
                slicesPerChunk = slicesPerChunk / 2;
                std::cout << "Reducing slicesPerChunk to: " << slicesPerChunk << std::endl;
                chunkStart -= slicesPerChunk; // Откатываемся назад
			}
			continue; // Пропускаем этот chunk, продолжаем со следующим
        }

		BYTE* pChunkData = static_cast<BYTE*>(MapViewOfFile(hChunkMapping, FILE_MAP_WRITE,
														   alignedOffset.HighPart, alignedOffset.LowPart, alignedSize));
        if (!pChunkData) {
			DWORD err = GetLastError();
			//std::cerr << "MapViewOfFile failed for chunk, error: " << err << std::endl;
			CloseHandle(hChunkMapping);
			ShowMessage("problem2");
			ShowMessage(err);
			if (offsetInBlock > 0) {
                std::cout << "Trying alternative mapping from offset 0" << std::endl;
                alignedOffset.QuadPart = 0;
                offsetInBlock = static_cast<DWORD>(chunkOffset.QuadPart);
                alignedSize = ((chunkSize + offsetInBlock + allocationGranularity - 1) / allocationGranularity) * allocationGranularity;

                hChunkMapping = CreateFileMappingW(hOutputFile, NULL, PAGE_READWRITE,
                                                 alignedOffset.HighPart, alignedOffset.LowPart + alignedSize, NULL);
                if (hChunkMapping) {
                    pChunkData = static_cast<BYTE*>(MapViewOfFile(hChunkMapping, FILE_MAP_WRITE,
                                                                alignedOffset.HighPart, alignedOffset.LowPart, alignedSize));
                }
            }

            if (!pChunkData) {
                std::cerr << "Alternative mapping also failed, skipping chunk" << std::endl;
                if (slicesPerChunk > 1) {
                    slicesPerChunk = slicesPerChunk / 2;
                    chunkStart -= slicesPerChunk;
                }
                continue;
			}
            continue;
        }

        // Указатель на начало данных chunk (с учетом выравнивания)
        BYTE* pData = pChunkData + offsetInBlock;

        // ОБРАБАТЫВАЕМ все линии для текущего chunk
        for (int line = 0; line < linesCount; ++line) {
			// Читаем текущую линию из входного файла
            LARGE_INTEGER lineOffset;
            lineOffset.QuadPart = headerSize + static_cast<__int64>(line) * inputLineSize;
            SetFilePointerEx(hInputFile, lineOffset, NULL, FILE_BEGIN);

            // Читаем линию целиком или по частям если очень большая
            if (lineSizeInFloats * sizeof(float) <= CHUNK_SIZE / 4) {
                // Читаем целиком если помещается в буфер
                ReadFile(hInputFile, lineBuffer.data(), inputLineSize, &bytesRead, NULL);
            } else {
                // Читаем по частям
                size_t totalRead = 0;
                while (totalRead < lineSizeInFloats) {
                    size_t floatsToRead = std::min(floatsPerChunk / 4, lineSizeInFloats - totalRead);
                    ReadFile(hInputFile, lineBuffer.data() + totalRead,
                            floatsToRead * sizeof(float), &bytesRead, NULL);
                    totalRead += floatsToRead;
                }
            }

            // ОБРАБАТЫВАЕМ данные для текущего chunk временных срезов
            for (size_t t = chunkStart; t < chunkEnd; ++t) {
                for (int n = 0; n < nf; ++n) {
                    for (int tr = 0; tr < tracesCount; ++tr) {
                        // Вычисляем позицию в исходных данных
                        size_t inputIndex = n * tracesCount * sampleCount +
                                          tr * sampleCount +
                                          t;

                        // Вычисляем позицию в выходном chunk
                        size_t outputIndex = (t - chunkStart) * nf * linesCount * tracesCount +
                                           n * linesCount * tracesCount +
                                           line * tracesCount +
                                           tr;

                        // Записываем данные через memory mapping
                        *reinterpret_cast<float*>(pData + outputIndex * sizeof(float)) = lineBuffer[inputIndex];
                    }
                }
			}
			if (line%10 == 0) {
			load -> update(1);
			}
		}

		// ОСВОБОЖДАЕМ ресурсы chunk
		if (pChunkData) {
		UnmapViewOfFile(pChunkData);
        }
        CloseHandle(hChunkMapping);

		//std::cout << "Completed chunk " << chunkStart/slicesPerChunk + 1
				  //<< " of " << ((sampleCount + slicesPerChunk - 1) / slicesPerChunk) << std::endl;
//    	if (Application->Terminated) {
//            break;
//        }
	}

	CloseHandle(hInputFile);
//	CloseHandle(hOutputFile);
	ShowMessage("Processing completed");
}

