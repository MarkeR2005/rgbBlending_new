//---------------------------------------------------------------------------
#include "Structures.h"
#ifndef DataMathH
#define DataMathH
//---------------------------------------------------------------------------
std::vector<Traces> tracesToSwanEn(Traces input, bool timeSwan, int nf, int fn,
									int fk, int filterWidth, float* freqs);
//--
Traces traceToSWAN(Traces& input, int traceno, bool timeSwan, int nf, int fn, int fk,
					int filterWidth, float* freqs);
                    //--
Traces traceToSWAN_E(Traces& input, int traceno, bool timeSwan, int nf, int fn, int fk,
					int filterWidth, float* freqs, int tWidth);
//--
std::vector<Traces> smoothingT(const std::vector<Traces>& input, const int tWidth);
//--
std::vector<Traces> smoothingF(const std::vector<Traces>& input, const int fWidth, const int nf1, const int nf2);
//--
std::vector<Traces> smoothing4D (const std::vector<std::vector<Traces>>& toSmooth, const int dt, const int nf);
//--
std::vector<Traces> smoothingX(std::vector<Traces>& input, const int xWidth);
//--
Traces getTrace(const Traces& input, int traceno);
#endif
