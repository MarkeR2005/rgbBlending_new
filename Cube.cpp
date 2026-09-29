//---------------------------------------------------------------------------

#pragma hdrstop

#include "Cube.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
 #include "SeismicData.h"
 #include "RgbData.h"
 #include "unitLoading.h"
 #include <memory>
 

std::shared_ptr<SeismicData> NreadInlineRegular(std::wstring fName, int lineNumber) {
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
    return std::move(std::make_shared<SeismicData>(lineData, static_cast<int>(dt) * 1000, samples, tr_inline));
}
//--
//--
std::shared_ptr<SeismicData> NreadCrosslineRegular(std::wstring fName, int crosslineNumber) {
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
	return std::move(std::make_shared<SeismicData>(crosslineData, static_cast<int>(dt) * 1000, samples, tr_inline));
}
//--
std::shared_ptr<SeismicData> NreadTimeSliceRegular(std::wstring fName, int sliceNumber) {
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
	return std::move(std::make_shared<SeismicData>(lineData, static_cast<int>(dt) * 1000, tr_inline, lines));
}
//--

//void NcalculateCube(std::wstring fName, int nf, int fn, int fk, int filterWidth, TComponent* Sender)
//{
//	double dt = 0.0;
//	double sampleCount = 0;
//	double trHead = 0;
//	int fHead = 0;
//	int t1 = 0, l1 = 0, t3 = 0, l3 = 0, hS = 0;
//	// Чтение входного файла
//    std::unique_ptr<TFileStream> fileRead(new TFileStream(fName.c_str(), fmOpenRead | fmShareDenyWrite));
//	if (!fileRead || fileRead->Size == 0)
//	{
//		std::cerr << "fileError: Cannot open input file" << std::endl;
//		return;
//	}
//	try
//	{
//		// Чтение заголовка
//		fileRead->Seek(static_cast<__int64>(6 * 4), soBeginning);
//		fileRead->Read(&dt, 8);
//		fileRead->Seek(static_cast<__int64>(32 + 6 * 4), soBeginning);
//		fileRead->Read(&sampleCount, 8);
//		fileRead->Seek(static_cast<__int64>(9 * 32 + 6 * 4), soBeginning);
//		fileRead->Read(&trHead, 8);
//		fileRead->Seek(static_cast<__int64>(10 * 32 + 4 * 2), soBeginning);
//        fileRead->Read(&fHead, 4);
//		fileRead->Seek(static_cast<__int64>(12 * 32), soBeginning);
//		fileRead->Read(&t1, 4);
//		fileRead->Read(&l1, 4);
//		fileRead->Read(&t3, 4);
//		fileRead->Read(&l3, 4);
//		fileRead->Read(&hS, 4);
//		std::cout << "dt=" << dt << std::endl;
//		std::cout << "sampleCount=" << sampleCount << std::endl;
//		std::cout << "trHead=" << trHead << std::endl;
//		std::cout << "fHead=" << fHead << std::endl;
//		std::cout << "t1=" << t1 << std::endl;
//		std::cout << "l1=" << l1 << std::endl;
//		std::cout << "t3=" << t3 << std::endl;
//        std::cout << "l3=" << l3 << std::endl;
//		std::cout << "hS=" << hS << std::endl;
//		int tr_inline = t3 - t1 + 1;
//		int lines = l3 - l1 + 1;
//		const __int64 offset = 776 + fHead + static_cast<int>(trHead) * tr_inline * lines;
//		const int count = static_cast<int>(sampleCount) * tr_inline;
//		// Создание выходного файла
//		std::wstring outputFileName = fName + L"_" + std::to_wstring(nf) + L"_" + std::to_wstring(fn)
//		+ L"_" + std::to_wstring(fk) + L".rgb";
//		std::unique_ptr<TFileStream> fileWrite(new TFileStream(outputFileName.c_str(), fmCreate | fmShareExclusive));
//        // Запись заголовка
//		int dt_toWrite = static_cast<int>(dt);
//        fileWrite->Write(&dt_toWrite, 4);
//		int sampleCount_toWrite = static_cast<int>(sampleCount);
//		fileWrite->Write(&sampleCount_toWrite, 4);
//		fileWrite->Write(&tr_inline, 4);        // Traces in line
//		fileWrite->Write(&lines, 4);            // Lines count
//		fileWrite->Write(&nf, 4);               // nf
//		fileWrite->Write(&fn, 4);               // fn
//		fileWrite->Write(&fk, 4);               // fk
//		fileWrite->Write(&filterWidth, 4);      // fw
//		// Запись частот
//		const double log_coeff = std::log(static_cast<double>(fk) / fn) / (nf - 1);
//		float* freqs = new float[nf];
//		for (int i = 0; i < nf; ++i)
//		{
//			float f = fn * std::exp(i * log_coeff);
//			freqs[i] = f;
//            fileWrite->Write(&f, 4);
//			std::cout << "Frequency " << i << ": " << f << std::endl;
//		}
//		// Буфер для чтения данных
//		std::vector<float> dataBuffer(count);
//		fileRead->Seek(offset, soBeginning);
//		TLoading* load = new TLoading(Sender);
//		load->setDuration(lines);
//        load->Show();
//		// Обработка каждой линии
//		for (int line = 0; line < lines; ++line)
//		{
//			// Чтение данных для текущей линии
//			fileRead->Read(dataBuffer.data(), count * 4);
//			// Создание копии данных для Traces
//			float* lineData = new float[count];
//			std::copy(dataBuffer.begin(), dataBuffer.end(), lineData);
//			// Обработка данных
//			Traces tr;
//			tr.init(tr_inline, static_cast<int>(sampleCount), static_cast<int>(dt) * 1000, lineData);
//			std::vector<Traces> out = tracesToSwanEn(tr, false, nf, fn, fk, filterWidth, freqs);
//			// Запись обработанных данных в правильном порядке
//			for (int n = 0; n < nf; ++n)
//			{
//                // Для каждой частоты записываем все трассы текущей линии
//				fileWrite->Write(out[n].data, count * 4);
//				// Отладочный вывод
//				if (line == 168) {
//					for (int x = 0; x < std::min(10, count); ++x)
//					{
//						std::cout << "Line " << line << ", Freq " << n << ", Sample " << x
//                                  << ": " << out[n].data[x] << std::endl;
//					}
//				}
//			}
//			// Очистка памяти
//            delete[] lineData;
//			for (auto& trace : out)
//			{
//				delete[] trace.data;
//			}
//            std::cout << "Finished line " << line << " of " << lines << std::endl;
//            // Проверка на прерывание
////			if (Application->ProcessMessages() && Application->Terminated) {
////				break;
////			}
//			load->update(1);
//		}
//        delete[] freqs;
//		ShowMessage("File successfully written");
//	} catch (const Exception& e)
//	{
//		std::cerr << "Error: " << e.Message.c_str() << std::endl;
//	} catch (const std::exception& e)
//	{
//		std::cerr << "Error: " << e.what() << std::endl;
//    }
//}
//--
void NconvertCube(const std::wstring& fName, TComponent* Sender)
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
	TLoading* load = new TLoading(Sender);
	load->setDuration(lines);
	load->Show();
	for (int line = 0; line < lines; ++line) {
        // Читаем данные линии
		fileRead->Read(lineData.data(), (Longint)lineSizeInBytes);
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

std::shared_ptr<SeismicData> NreadIBM(std::wstring fName) {
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

    return std::move(std::make_shared<SeismicData>(lineData, static_cast<int>(dt) * 1000, samples, static_cast<int>(tr_inline)));
}