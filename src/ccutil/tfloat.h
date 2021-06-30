#ifndef TESSERACT_LSTM_TFLOAT_H
#define TESSERACT_LSTM_TFLOAT_H

#define FAST_FLOAT

#ifdef FAST_FLOAT
typedef float TFloat;
#else
typedef double TFloat;
#endif

#endif
