/* -*-C-*-
 ********************************************************************************
 *
 * File:        measure.h  (Formerly measure.h)
 * Description:  Statistics for a group of single measurements
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon Apr  8 09:42:28 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 ********************************************************************************
 */

#ifndef MEASURE_H
#define MEASURE_H

/*
----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------
*/

#include <math.h>

/*
----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------
*/

typedef struct
{
  long num_samples;
  float sum_of_samples;
  float sum_of_squares;
} MEASUREMENT;

/*
----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------
*/

/**********************************************************************
 * add_sample
 *
 * Add one more sample to a measurement.
 **********************************************************************/

#define ADD_SAMPLE(m,s)                           \
(m.sum_of_samples += (float) (s),               \
	m.sum_of_squares += (float) (s) * (float) (s), \
	++m.num_samples)

/**********************************************************************
 * mean
 *
 * Return the mean value of the measurement.
 **********************************************************************/

#define MEAN(m)                                       \
((m).num_samples ?                                  \
	((float) ((m).sum_of_samples / (m).num_samples)) : \
	0)

/**********************************************************************
 * new_measurement
 *
 * Initialize a record to hold a measurement of a group of individual
 * samples.
 **********************************************************************/

#define new_measurement(m)   \
((m).num_samples    = 0, \
	(m).sum_of_samples = 0, \
	(m).sum_of_squares = 0)

/**********************************************************************
 * number_of_samples
 *
 * Return the number of samples in a measurement.
 **********************************************************************/

#define number_of_samples(m)  \
((m).num_samples)

/**********************************************************************
 * standard_deviation
 *
 * Return the standard deviation of the measurement.
 **********************************************************************/

#define standard_deviation(m)                                \
((float) sqrt (VARIANCE (m)))

/**********************************************************************
 * variance
 *
 * Return the variance of the measurement.
 **********************************************************************/

#define VARIANCE(m)                                   \
(((m).num_samples > 1) ?                            \
	((float)                                           \
	(((m).num_samples * (m).sum_of_squares -          \
		(m).sum_of_samples * (m).sum_of_samples) /      \
	(((m).num_samples - 1) * (m).num_samples)))   :  \
	0)

/**********************************************************************
 * print_summary
 *
 * Summarize a MEASUREMENT record.
 **********************************************************************/

#define print_summary(string,measure)                       \
cprintf ("\t%-20s \tn = %d, \tm = %4.2f, \ts = %4.2f\n ",  \
			string,                                          \
			number_of_samples  (measure),                    \
			MEAN           (measure),                    \
			standard_deviation (measure))
#endif
