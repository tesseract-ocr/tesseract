#ifndef TESSERACT_LSTM_TFLOAT_H
#define TESSERACT_LSTM_TFLOAT_H

namespace tesseract {

#define FAST_FLOAT 1

#ifdef FAST_FLOAT
	typedef float TFloat;
#else
	typedef double TFloat;
#endif

}

#endif
