/******************************************************************************
 ** Filename: cluster.cpp
 ** Purpose:  Routines for clustering points in N-D space
 ** Author:   Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *****************************************************************************/

#define _USE_MATH_DEFINES // for M_PI

#include "cluster.h"

#include "genericheap.h"
#include "kdpair.h"
#include "matrix.h"
#include "tprintf.h"

#include "helpers.h"

#include <cfloat> // for FLT_MAX
#include <cmath>  // for M_PI
#include <vector> // for std::vector

namespace tesseract {

#define HOTELLING 1  // If true use Hotelling's test to decide where to split.
#define FTABLE_X 10  // Size of FTable.
#define FTABLE_Y 100 // Size of FTable.

// Table of values approximating the cumulative F-distribution for a confidence
// of 1%.
const double FTable[FTABLE_Y][FTABLE_X] = {
    {
        4052.19,
        4999.52,
        5403.34,
        5624.62,
        5763.65,
        5858.97,
        5928.33,
        5981.10,
        6022.50,
        6055.85,
    },
    {
        98.502,
        99.000,
        99.166,
        99.249,
        99.300,
        99.333,
        99.356,
        99.374,
        99.388,
        99.399,
    },
    {
        34.116,
        30.816,
        29.457,
        28.710,
        28.237,
        27.911,
        27.672,
        27.489,
        27.345,
        27.229,
    },
    {
        21.198,
        18.000,
        16.694,
        15.977,
        15.522,
        15.207,
        14.976,
        14.799,
        14.659,
        14.546,
    },
    {
        16.258,
        13.274,
        12.060,
        11.392,
        10.967,
        10.672,
        10.456,
        10.289,
        10.158,
        10.051,
    },
    {
        13.745,
        10.925,
        9.780,
        9.148,
        8.746,
        8.466,
        8.260,
        8.102,
        7.976,
        7.874,
    },
    {
        12.246,
        9.547,
        8.451,
        7.847,
        7.460,
        7.191,
        6.993,
        6.840,
        6.719,
        6.620,
    },
    {
        11.259,
        8.649,
        7.591,
        7.006,
        6.632,
        6.371,
        6.178,
        6.029,
        5.911,
        5.814,
    },
    {
        10.561,
        8.022,
        6.992,
        6.422,
        6.057,
        5.802,
        5.613,
        5.467,
        5.351,
        5.257,
    },
    {
        10.044,
        7.559,
        6.552,
        5.994,
        5.636,
        5.386,
        5.200,
        5.057,
        4.942,
        4.849,
    },
    {
        9.646,
        7.206,
        6.217,
        5.668,
        5.316,
        5.069,
        4.886,
        4.744,
        4.632,
        4.539,
    },
    {
        9.330,
        6.927,
        5.953,
        5.412,
        5.064,
        4.821,
        4.640,
        4.499,
        4.388,
        4.296,
    },
    {
        9.074,
        6.701,
        5.739,
        5.205,
        4.862,
        4.620,
        4.441,
        4.302,
        4.191,
        4.100,
    },
    {
        8.862,
        6.515,
        5.564,
        5.035,
        4.695,
        4.456,
        4.278,
        4.140,
        4.030,
        3.939,
    },
    {
        8.683,
        6.359,
        5.417,
        4.893,
        4.556,
        4.318,
        4.142,
        4.004,
        3.895,
        3.805,
    },
    {
        8.531,
        6.226,
        5.292,
        4.773,
        4.437,
        4.202,
        4.026,
        3.890,
        3.780,
        3.691,
    },
    {
        8.400,
        6.112,
        5.185,
        4.669,
        4.336,
        4.102,
        3.927,
        3.791,
        3.682,
        3.593,
    },
    {
        8.285,
        6.013,
        5.092,
        4.579,
        4.248,
        4.015,
        3.841,
        3.705,
        3.597,
        3.508,
    },
    {
        8.185,
        5.926,
        5.010,
        4.500,
        4.171,
        3.939,
        3.765,
        3.631,
        3.523,
        3.434,
    },
    {
        8.096,
        5.849,
        4.938,
        4.431,
        4.103,
        3.871,
        3.699,
        3.564,
        3.457,
        3.368,
    },
    {
        8.017,
        5.780,
        4.874,
        4.369,
        4.042,
        3.812,
        3.640,
        3.506,
        3.398,
        3.310,
    },
    {
        7.945,
        5.719,
        4.817,
        4.313,
        3.988,
        3.758,
        3.587,
        3.453,
        3.346,
        3.258,
    },
    {
        7.881,
        5.664,
        4.765,
        4.264,
        3.939,
        3.710,
        3.539,
        3.406,
        3.299,
        3.211,
    },
    {
        7.823,
        5.614,
        4.718,
        4.218,
        3.895,
        3.667,
        3.496,
        3.363,
        3.256,
        3.168,
    },
    {
        7.770,
        5.568,
        4.675,
        4.177,
        3.855,
        3.627,
        3.457,
        3.324,
        3.217,
        3.129,
    },
    {
        7.721,
        5.526,
        4.637,
        4.140,
        3.818,
        3.591,
        3.421,
        3.288,
        3.182,
        3.094,
    },
    {
        7.677,
        5.488,
        4.601,
        4.106,
        3.785,
        3.558,
        3.388,
        3.256,
        3.149,
        3.062,
    },
    {
        7.636,
        5.453,
        4.568,
        4.074,
        3.754,
        3.528,
        3.358,
        3.226,
        3.120,
        3.032,
    },
    {
        7.598,
        5.420,
        4.538,
        4.045,
        3.725,
        3.499,
        3.330,
        3.198,
        3.092,
        3.005,
    },
    {
        7.562,
        5.390,
        4.510,
        4.018,
        3.699,
        3.473,
        3.305,
        3.173,
        3.067,
        2.979,
    },
    {
        7.530,
        5.362,
        4.484,
        3.993,
        3.675,
        3.449,
        3.281,
        3.149,
        3.043,
        2.955,
    },
    {
        7.499,
        5.336,
        4.459,
        3.969,
        3.652,
        3.427,
        3.258,
        3.127,
        3.021,
        2.934,
    },
    {
        7.471,
        5.312,
        4.437,
        3.948,
        3.630,
        3.406,
        3.238,
        3.106,
        3.000,
        2.913,
    },
    {
        7.444,
        5.289,
        4.416,
        3.927,
        3.611,
        3.386,
        3.218,
        3.087,
        2.981,
        2.894,
    },
    {
        7.419,
        5.268,
        4.396,
        3.908,
        3.592,
        3.368,
        3.200,
        3.069,
        2.963,
        2.876,
    },
    {
        7.396,
        5.248,
        4.377,
        3.890,
        3.574,
        3.351,
        3.183,
        3.052,
        2.946,
        2.859,
    },
    {
        7.373,
        5.229,
        4.360,
        3.873,
        3.558,
        3.334,
        3.167,
        3.036,
        2.930,
        2.843,
    },
    {
        7.353,
        5.211,
        4.343,
        3.858,
        3.542,
        3.319,
        3.152,
        3.021,
        2.915,
        2.828,
    },
    {
        7.333,
        5.194,
        4.327,
        3.843,
        3.528,
        3.305,
        3.137,
        3.006,
        2.901,
        2.814,
    },
    {
        7.314,
        5.179,
        4.313,
        3.828,
        3.514,
        3.291,
        3.124,
        2.993,
        2.888,
        2.801,
    },
    {
        7.296,
        5.163,
        4.299,
        3.815,
        3.501,
        3.278,
        3.111,
        2.980,
        2.875,
        2.788,
    },
    {
        7.280,
        5.149,
        4.285,
        3.802,
        3.488,
        3.266,
        3.099,
        2.968,
        2.863,
        2.776,
    },
    {
        7.264,
        5.136,
        4.273,
        3.790,
        3.476,
        3.254,
        3.087,
        2.957,
        2.851,
        2.764,
    },
    {
        7.248,
        5.123,
        4.261,
        3.778,
        3.465,
        3.243,
        3.076,
        2.946,
        2.840,
        2.754,
    },
    {
        7.234,
        5.110,
        4.249,
        3.767,
        3.454,
        3.232,
        3.066,
        2.935,
        2.830,
        2.743,
    },
    {
        7.220,
        5.099,
        4.238,
        3.757,
        3.444,
        3.222,
        3.056,
        2.925,
        2.820,
        2.733,
    },
    {
        7.207,
        5.087,
        4.228,
        3.747,
        3.434,
        3.213,
        3.046,
        2.916,
        2.811,
        2.724,
    },
    {
        7.194,
        5.077,
        4.218,
        3.737,
        3.425,
        3.204,
        3.037,
        2.907,
        2.802,
        2.715,
    },
    {
        7.182,
        5.066,
        4.208,
        3.728,
        3.416,
        3.195,
        3.028,
        2.898,
        2.793,
        2.706,
    },
    {
        7.171,
        5.057,
        4.199,
        3.720,
        3.408,
        3.186,
        3.020,
        2.890,
        2.785,
        2.698,
    },
    {
        7.159,
        5.047,
        4.191,
        3.711,
        3.400,
        3.178,
        3.012,
        2.882,
        2.777,
        2.690,
    },
    {
        7.149,
        5.038,
        4.182,
        3.703,
        3.392,
        3.171,
        3.005,
        2.874,
        2.769,
        2.683,
    },
    {
        7.139,
        5.030,
        4.174,
        3.695,
        3.384,
        3.163,
        2.997,
        2.867,
        2.762,
        2.675,
    },
    {
        7.129,
        5.021,
        4.167,
        3.688,
        3.377,
        3.156,
        2.990,
        2.860,
        2.755,
        2.668,
    },
    {
        7.119,
        5.013,
        4.159,
        3.681,
        3.370,
        3.149,
        2.983,
        2.853,
        2.748,
        2.662,
    },
    {
        7.110,
        5.006,
        4.152,
        3.674,
        3.363,
        3.143,
        2.977,
        2.847,
        2.742,
        2.655,
    },
    {
        7.102,
        4.998,
        4.145,
        3.667,
        3.357,
        3.136,
        2.971,
        2.841,
        2.736,
        2.649,
    },
    {
        7.093,
        4.991,
        4.138,
        3.661,
        3.351,
        3.130,
        2.965,
        2.835,
        2.730,
        2.643,
    },
    {
        7.085,
        4.984,
        4.132,
        3.655,
        3.345,
        3.124,
        2.959,
        2.829,
        2.724,
        2.637,
    },
    {
        7.077,
        4.977,
        4.126,
        3.649,
        3.339,
        3.119,
        2.953,
        2.823,
        2.718,
        2.632,
    },
    {
        7.070,
        4.971,
        4.120,
        3.643,
        3.333,
        3.113,
        2.948,
        2.818,
        2.713,
        2.626,
    },
    {
        7.062,
        4.965,
        4.114,
        3.638,
        3.328,
        3.108,
        2.942,
        2.813,
        2.708,
        2.621,
    },
    {
        7.055,
        4.959,
        4.109,
        3.632,
        3.323,
        3.103,
        2.937,
        2.808,
        2.703,
        2.616,
    },
    {
        7.048,
        4.953,
        4.103,
        3.627,
        3.318,
        3.098,
        2.932,
        2.803,
        2.698,
        2.611,
    },
    {
        7.042,
        4.947,
        4.098,
        3.622,
        3.313,
        3.093,
        2.928,
        2.798,
        2.693,
        2.607,
    },
    {
        7.035,
        4.942,
        4.093,
        3.618,
        3.308,
        3.088,
        2.923,
        2.793,
        2.689,
        2.602,
    },
    {
        7.029,
        4.937,
        4.088,
        3.613,
        3.304,
        3.084,
        2.919,
        2.789,
        2.684,
        2.598,
    },
    {
        7.023,
        4.932,
        4.083,
        3.608,
        3.299,
        3.080,
        2.914,
        2.785,
        2.680,
        2.593,
    },
    {
        7.017,
        4.927,
        4.079,
        3.604,
        3.295,
        3.075,
        2.910,
        2.781,
        2.676,
        2.589,
    },
    {
        7.011,
        4.922,
        4.074,
        3.600,
        3.291,
        3.071,
        2.906,
        2.777,
        2.672,
        2.585,
    },
    {
        7.006,
        4.917,
        4.070,
        3.596,
        3.287,
        3.067,
        2.902,
        2.773,
        2.668,
        2.581,
    },
    {
        7.001,
        4.913,
        4.066,
        3.591,
        3.283,
        3.063,
        2.898,
        2.769,
        2.664,
        2.578,
    },
    {
        6.995,
        4.908,
        4.062,
        3.588,
        3.279,
        3.060,
        2.895,
        2.765,
        2.660,
        2.574,
    },
    {
        6.990,
        4.904,
        4.058,
        3.584,
        3.275,
        3.056,
        2.891,
        2.762,
        2.657,
        2.570,
    },
    {
        6.985,
        4.900,
        4.054,
        3.580,
        3.272,
        3.052,
        2.887,
        2.758,
        2.653,
        2.567,
    },
    {
        6.981,
        4.896,
        4.050,
        3.577,
        3.268,
        3.049,
        2.884,
        2.755,
        2.650,
        2.563,
    },
    {
        6.976,
        4.892,
        4.047,
        3.573,
        3.265,
        3.046,
        2.881,
        2.751,
        2.647,
        2.560,
    },
    {
        6.971,
        4.888,
        4.043,
        3.570,
        3.261,
        3.042,
        2.877,
        2.748,
        2.644,
        2.557,
    },
    {
        6.967,
        4.884,
        4.040,
        3.566,
        3.258,
        3.039,
        2.874,
        2.745,
        2.640,
        2.554,
    },
    {
        6.963,
        4.881,
        4.036,
        3.563,
        3.255,
        3.036,
        2.871,
        2.742,
        2.637,
        2.551,
    },
    {
        6.958,
        4.877,
        4.033,
        3.560,
        3.252,
        3.033,
        2.868,
        2.739,
        2.634,
        2.548,
    },
    {
        6.954,
        4.874,
        4.030,
        3.557,
        3.249,
        3.030,
        2.865,
        2.736,
        2.632,
        2.545,
    },
    {
        6.950,
        4.870,
        4.027,
        3.554,
        3.246,
        3.027,
        2.863,
        2.733,
        2.629,
        2.542,
    },
    {
        6.947,
        4.867,
        4.024,
        3.551,
        3.243,
        3.025,
        2.860,
        2.731,
        2.626,
        2.539,
    },
    {
        6.943,
        4.864,
        4.021,
        3.548,
        3.240,
        3.022,
        2.857,
        2.728,
        2.623,
        2.537,
    },
    {
        6.939,
        4.861,
        4.018,
        3.545,
        3.238,
        3.019,
        2.854,
        2.725,
        2.621,
        2.534,
    },
    {
        6.935,
        4.858,
        4.015,
        3.543,
        3.235,
        3.017,
        2.852,
        2.723,
        2.618,
        2.532,
    },
    {
        6.932,
        4.855,
        4.012,
        3.540,
        3.233,
        3.014,
        2.849,
        2.720,
        2.616,
        2.529,
    },
    {
        6.928,
        4.852,
        4.010,
        3.538,
        3.230,
        3.012,
        2.847,
        2.718,
        2.613,
        2.527,
    },
    {
        6.925,
        4.849,
        4.007,
        3.535,
        3.228,
        3.009,
        2.845,
        2.715,
        2.611,
        2.524,
    },
    {
        6.922,
        4.846,
        4.004,
        3.533,
        3.225,
        3.007,
        2.842,
        2.713,
        2.609,
        2.522,
    },
    {
        6.919,
        4.844,
        4.002,
        3.530,
        3.223,
        3.004,
        2.840,
        2.711,
        2.606,
        2.520,
    },
    {
        6.915,
        4.841,
        3.999,
        3.528,
        3.221,
        3.002,
        2.838,
        2.709,
        2.604,
        2.518,
    },
    {
        6.912,
        4.838,
        3.997,
        3.525,
        3.218,
        3.000,
        2.835,
        2.706,
        2.602,
        2.515,
    },
    {
        6.909,
        4.836,
        3.995,
        3.523,
        3.216,
        2.998,
        2.833,
        2.704,
        2.600,
        2.513,
    },
    {
        6.906,
        4.833,
        3.992,
        3.521,
        3.214,
        2.996,
        2.831,
        2.702,
        2.598,
        2.511,
    },
    {
        6.904,
        4.831,
        3.990,
        3.519,
        3.212,
        2.994,
        2.829,
        2.700,
        2.596,
        2.509,
    },
    {
        6.901,
        4.829,
        3.988,
        3.517,
        3.210,
        2.992,
        2.827,
        2.698,
        2.594,
        2.507,
    },
    {
        6.898,
        4.826,
        3.986,
        3.515,
        3.208,
        2.990,
        2.825,
        2.696,
        2.592,
        2.505,
    },
    {6.895, 4.824, 3.984, 3.513, 3.206, 2.988, 2.823, 2.694, 2.590, 2.503}};

/** define the variance which will be used as a minimum variance for any
  dimension of any feature. Since most features are calculated from numbers
  with a precision no better than 1 in 128, the variance should never be
  less than the square of this number for parameters whose range is 1. */
#define MINVARIANCE 0.0004

/** define the absolute minimum number of samples which must be present in
  order to accurately test hypotheses about underlying probability
  distributions.  Define separately the minimum samples that are needed
  before a statistical analysis is attempted; this number should be
  equal to MINSAMPLES but can be set to a lower number for early testing
  when very few samples are available. */
#define MINSAMPLESPERBUCKET 5
#define MINSAMPLES (MINBUCKETS * MINSAMPLESPERBUCKET)
#define MINSAMPLESNEEDED 1

/** define the size of the table which maps normalized samples to
  histogram buckets.  Also define the number of standard deviations
  in a normal distribution which are considered to be significant.
  The mapping table will be defined in such a way that it covers
  the specified number of standard deviations on either side of
  the mean.  BUCKETTABLESIZE should always be even. */
#define BUCKETTABLESIZE 1024
#define NORMALEXTENT 3.0

struct TEMPCLUSTER {
  CLUSTER *Cluster;
  CLUSTER *Neighbor;
};

using ClusterPair = tesseract::KDPairInc<float, TEMPCLUSTER *>;
using ClusterHeap = tesseract::GenericHeap<ClusterPair>;

struct STATISTICS {
  STATISTICS(size_t n) : CoVariance(n * n), Min(n), Max(n) {
  }
  float AvgVariance = 1.0f;
  std::vector<float> CoVariance;
  std::vector<float> Min; // largest negative distance from the mean
  std::vector<float> Max; // largest positive distance from the mean
};

struct BUCKETS {
  BUCKETS(size_t n) : NumberOfBuckets(n), Count(n), ExpectedCount(n) {
  }
  ~BUCKETS() {
  }
  DISTRIBUTION Distribution = normal; // distribution being tested for
  uint32_t SampleCount = 0;         // # of samples in histogram
  double Confidence = 0.0;          // confidence level of test
  double ChiSquared = 0.0;          // test threshold
  uint16_t NumberOfBuckets;         // number of cells in histogram
  uint16_t Bucket[BUCKETTABLESIZE]; // mapping to histogram buckets
  std::vector<uint32_t> Count;      // frequency of occurrence histogram
  std::vector<float> ExpectedCount; // expected histogram
};

struct CHISTRUCT {
  /// This constructor allocates a new data structure which is used
  /// to hold a chi-squared value along with its associated
  /// number of degrees of freedom and alpha value.
  ///
  /// @param degreesOfFreedom  degrees of freedom for new chi value
  /// @param alpha     confidence level for new chi value
  CHISTRUCT(uint16_t degreesOfFreedom, double alpha) : DegreesOfFreedom(degreesOfFreedom), Alpha(alpha) {
  }
  uint16_t DegreesOfFreedom = 0;
  double Alpha = 0.0;
  double ChiSquared = 0.0;
};

// For use with KDWalk / MakePotentialClusters
struct ClusteringContext {
  ClusterHeap *heap;       // heap used to hold temp clusters, "best" on top
  TEMPCLUSTER *candidates; // array of potential clusters
  KDTREE *tree;            // kd-tree to be searched for neighbors
  int32_t next;            // next candidate to be used
};

using DENSITYFUNC = double (*)(int32_t);
using SOLVEFUNC = double (*)(CHISTRUCT *, double);

#define Odd(N) ((N) % 2)
#define Mirror(N, R) ((R) - (N)-1)
#define Abs(N) (((N) < 0) ? (-(N)) : (N))

//--------------Global Data Definitions and Declarations----------------------
/** the following variables describe a discrete normal distribution
  which is used by NormalDensity() and NormalBucket().  The
  constant NORMALEXTENT determines how many standard
  deviations of the distribution are mapped onto the fixed
  discrete range of x.  x=0 is mapped to -NORMALEXTENT standard
  deviations and x=BUCKETTABLESIZE is mapped to
  +NORMALEXTENT standard deviations. */
#define SqrtOf2Pi 2.506628275
static const double kNormalStdDev = BUCKETTABLESIZE / (2.0 * NORMALEXTENT);
static const double kNormalVariance =
    (BUCKETTABLESIZE * BUCKETTABLESIZE) / (4.0 * NORMALEXTENT * NORMALEXTENT);
static const double kNormalMagnitude = (2.0 * NORMALEXTENT) / (SqrtOf2Pi * BUCKETTABLESIZE);
static const double kNormalMean = BUCKETTABLESIZE / 2;

/** define lookup tables used to compute the number of histogram buckets
  that should be used for a given number of samples. */
#define LOOKUPTABLESIZE 8
#define MAXDEGREESOFFREEDOM MAXBUCKETS

static const uint32_t kCountTable[LOOKUPTABLESIZE] = {MINSAMPLES, 200,  400, 600, 800,
                                                      1000,       1500, 2000}; // number of samples

static const uint16_t kBucketsTable[LOOKUPTABLESIZE] = {
    MINBUCKETS, 16, 20, 24, 27, 30, 35, MAXBUCKETS}; // number of buckets

/*-------------------------------------------------------------------------
          Private Function Prototypes
--------------------------------------------------------------------------*/
static void CreateClusterTree(CLUSTERER *Clusterer);

static void MakePotentialClusters(ClusteringContext *context, CLUSTER *Cluster, int32_t Level);

static CLUSTER *FindNearestNeighbor(KDTREE *Tree, CLUSTER *Cluster, float *Distance);

static CLUSTER *MakeNewCluster(CLUSTERER *Clusterer, TEMPCLUSTER *TempCluster);

static void ComputePrototypes(CLUSTERER *Clusterer, CLUSTERCONFIG *Config);

static PROTOTYPE *MakePrototype(CLUSTERER *Clusterer, CLUSTERCONFIG *Config, CLUSTER *Cluster);

static PROTOTYPE *MakeDegenerateProto(uint16_t N, CLUSTER *Cluster, STATISTICS *Statistics,
                                      PROTOSTYLE Style, int32_t MinSamples);

static PROTOTYPE *TestEllipticalProto(CLUSTERER *Clusterer, CLUSTERCONFIG *Config, CLUSTER *Cluster,
                                      STATISTICS *Statistics);

static PROTOTYPE *MakeSphericalProto(CLUSTERER *Clusterer, CLUSTER *Cluster, STATISTICS *Statistics,
                                     BUCKETS *Buckets);

static PROTOTYPE *MakeEllipticalProto(CLUSTERER *Clusterer, CLUSTER *Cluster,
                                      STATISTICS *Statistics, BUCKETS *Buckets);

static PROTOTYPE *MakeMixedProto(CLUSTERER *Clusterer, CLUSTER *Cluster, STATISTICS *Statistics,
                                 BUCKETS *NormalBuckets, double Confidence);

static void MakeDimRandom(uint16_t i, PROTOTYPE *Proto, PARAM_DESC *ParamDesc);

static void MakeDimUniform(uint16_t i, PROTOTYPE *Proto, STATISTICS *Statistics);

static STATISTICS *ComputeStatistics(int16_t N, PARAM_DESC ParamDesc[], CLUSTER *Cluster);

static PROTOTYPE *NewSphericalProto(uint16_t N, CLUSTER *Cluster, STATISTICS *Statistics);

static PROTOTYPE *NewEllipticalProto(int16_t N, CLUSTER *Cluster, STATISTICS *Statistics);

static PROTOTYPE *NewMixedProto(int16_t N, CLUSTER *Cluster, STATISTICS *Statistics);

static PROTOTYPE *NewSimpleProto(int16_t N, CLUSTER *Cluster);

static bool Independent(PARAM_DESC *ParamDesc, int16_t N, float *CoVariance, float Independence);

static BUCKETS *GetBuckets(CLUSTERER *clusterer, DISTRIBUTION Distribution, uint32_t SampleCount,
                           double Confidence);

static BUCKETS *MakeBuckets(DISTRIBUTION Distribution, uint32_t SampleCount, double Confidence);

static uint16_t OptimumNumberOfBuckets(uint32_t SampleCount);

static double ComputeChiSquared(uint16_t DegreesOfFreedom, double Alpha);

static double NormalDensity(int32_t x);

static double UniformDensity(int32_t x);

static double Integral(double f1, double f2, double Dx);

static void FillBuckets(BUCKETS *Buckets, CLUSTER *Cluster, uint16_t Dim, PARAM_DESC *ParamDesc,
                        float Mean, float StdDev);

static uint16_t NormalBucket(PARAM_DESC *ParamDesc, float x, float Mean, float StdDev);

static uint16_t UniformBucket(PARAM_DESC *ParamDesc, float x, float Mean, float StdDev);

static bool DistributionOK(BUCKETS *Buckets);

static uint16_t DegreesOfFreedom(DISTRIBUTION Distribution, uint16_t HistogramBuckets);

static void AdjustBuckets(BUCKETS *Buckets, uint32_t NewSampleCount);

static void InitBuckets(BUCKETS *Buckets);

static int AlphaMatch(void *arg1,  // CHISTRUCT *ChiStruct,
                      void *arg2); // CHISTRUCT *SearchKey);

static double Solve(SOLVEFUNC Function, void *FunctionParams, double InitialGuess, double Accuracy);

static double ChiArea(CHISTRUCT *ChiParams, double x);

static bool MultipleCharSamples(CLUSTERER *Clusterer, CLUSTER *Cluster, float MaxIllegal);

static double InvertMatrix(const float *input, int size, float *inv);

//--------------------------Public Code--------------------------------------
/**
 * This routine creates a new clusterer data structure,
 * initializes it, and returns a pointer to it.
 *
 * @param SampleSize  number of dimensions in feature space
 * @param ParamDesc description of each dimension
 * @return pointer to the new clusterer data structure
 */
CLUSTERER *MakeClusterer(int16_t SampleSize, const PARAM_DESC ParamDesc[]) {
  int i;

  // allocate main clusterer data structure and init simple fields
  auto Clusterer = new CLUSTERER;
  Clusterer->SampleSize = SampleSize;
  Clusterer->NumberOfSamples = 0;
  Clusterer->NumChar = 0;

  // init fields which will not be used initially
  Clusterer->Root = nullptr;
  Clusterer->ProtoList = NIL_LIST;

  // maintain a copy of param descriptors in the clusterer data structure
  Clusterer->ParamDesc = new PARAM_DESC[SampleSize];
  for (i = 0; i < SampleSize; i++) {
    Clusterer->ParamDesc[i].Circular = ParamDesc[i].Circular;
    Clusterer->ParamDesc[i].NonEssential = ParamDesc[i].NonEssential;
    Clusterer->ParamDesc[i].Min = ParamDesc[i].Min;
    Clusterer->ParamDesc[i].Max = ParamDesc[i].Max;
    Clusterer->ParamDesc[i].Range = ParamDesc[i].Max - ParamDesc[i].Min;
    Clusterer->ParamDesc[i].HalfRange = Clusterer->ParamDesc[i].Range / 2;
    Clusterer->ParamDesc[i].MidRange = (ParamDesc[i].Max + ParamDesc[i].Min) / 2;
  }

  // allocate a kd tree to hold the samples
  Clusterer->KDTree = MakeKDTree(SampleSize, ParamDesc);

  // Initialize cache of histogram buckets to minimize recomputing them.
  for (auto &d : Clusterer->bucket_cache) {
    for (auto &c : d) {
      c = nullptr;
    }
  }

  return Clusterer;
} // MakeClusterer

/**
 * This routine creates a new sample data structure to hold
 * the specified feature.  This sample is added to the clusterer
 * data structure (so that it knows which samples are to be
 * clustered later), and a pointer to the sample is returned to
 * the caller.
 *
 * @param Clusterer clusterer data structure to add sample to
 * @param Feature feature to be added to clusterer
 * @param CharID  unique ident. of char that sample came from
 *
 * @return    Pointer to the new sample data structure
 */
SAMPLE *MakeSample(CLUSTERER *Clusterer, const float *Feature, uint32_t CharID) {
  int i;

  // see if the samples have already been clustered - if so trap an error
  // Can't add samples after they have been clustered.
  ASSERT_HOST(Clusterer->Root == nullptr);

  // allocate the new sample and initialize it
  auto Sample = new SAMPLE(Clusterer->SampleSize);
  Sample->Clustered = false;
  Sample->Prototype = false;
  Sample->SampleCount = 1;
  Sample->Left = nullptr;
  Sample->Right = nullptr;
  Sample->CharID = CharID;

  for (i = 0; i < Clusterer->SampleSize; i++) {
    Sample->Mean[i] = Feature[i];
  }

  // add the sample to the KD tree - keep track of the total # of samples
  Clusterer->NumberOfSamples++;
  KDStore(Clusterer->KDTree, &Sample->Mean[0], Sample);
  if (CharID >= Clusterer->NumChar) {
    Clusterer->NumChar = CharID + 1;
  }

  // execute hook for monitoring clustering operation
  // (*SampleCreationHook)(Sample);

  return (Sample);
} // MakeSample

/**
 * This routine first checks to see if the samples in this
 * clusterer have already been clustered before; if so, it does
 * not bother to recreate the cluster tree.  It simply recomputes
 * the prototypes based on the new Config info.
 *
 * If the samples have not been clustered before, the
 * samples in the KD tree are formed into a cluster tree and then
 * the prototypes are computed from the cluster tree.
 *
 * In either case this routine returns a pointer to a
 * list of prototypes that best represent the samples given
 * the constraints specified in Config.
 *
 * @param Clusterer data struct containing samples to be clustered
 * @param Config  parameters which control clustering process
 *
 * @return Pointer to a list of prototypes
 */
LIST ClusterSamples(CLUSTERER *Clusterer, CLUSTERCONFIG *Config) {
  // only create cluster tree if samples have never been clustered before
  if (Clusterer->Root == nullptr) {
    CreateClusterTree(Clusterer);
  }

  // deallocate the old prototype list if one exists
  FreeProtoList(&Clusterer->ProtoList);
  Clusterer->ProtoList = NIL_LIST;

  // compute prototypes starting at the root node in the tree
  ComputePrototypes(Clusterer, Config);
  // We don't need the cluster pointers in the protos any more, so null them
  // out, which makes it safe to delete the clusterer.
  LIST proto_list = Clusterer->ProtoList;
  iterate(proto_list) {
    auto *proto = reinterpret_cast<PROTOTYPE *>(proto_list->first_node());
    proto->Cluster = nullptr;
  }
  return Clusterer->ProtoList;
} // ClusterSamples

/**
 * This routine frees all of the memory allocated to the
 * specified data structure.  It will not, however, free
 * the memory used by the prototype list.  The pointers to
 * the clusters for each prototype in the list will be set
 * to nullptr to indicate that the cluster data structures no
 * longer exist.  Any sample lists that have been obtained
 * via calls to GetSamples are no longer valid.
 * @param Clusterer pointer to data structure to be freed
 */
void FreeClusterer(CLUSTERER *Clusterer) {
  if (Clusterer != nullptr) {
    delete[] Clusterer->ParamDesc;
    delete Clusterer->KDTree;
    delete Clusterer->Root;
    // Free up all used buckets structures.
    for (auto &d : Clusterer->bucket_cache) {
      for (auto &c : d) {
        delete c;
      }
    }

    delete Clusterer;
  }
} // FreeClusterer

/**
 * This routine frees all of the memory allocated to the
 * specified list of prototypes.  The clusters which are
 * pointed to by the prototypes are not freed.
 * @param ProtoList pointer to list of prototypes to be freed
 */
void FreeProtoList(LIST *ProtoList) {
  destroy_nodes(*ProtoList, FreePrototype);
} // FreeProtoList

/**
 * This routine deallocates the memory consumed by the specified
 * prototype and modifies the corresponding cluster so that it
 * is no longer marked as a prototype.  The cluster is NOT
 * deallocated by this routine.
 * @param arg prototype data structure to be deallocated
 */
void FreePrototype(void *arg) { // PROTOTYPE     *Prototype)
  auto *Prototype = static_cast<PROTOTYPE *>(arg);

  // unmark the corresponding cluster (if there is one
  if (Prototype->Cluster != nullptr) {
    Prototype->Cluster->Prototype = false;
  }

  // deallocate the prototype statistics and then the prototype itself
  if (Prototype->Style != spherical) {
    delete[] Prototype->Variance.Elliptical;
    delete[] Prototype->Magnitude.Elliptical;
    delete[] Prototype->Weight.Elliptical;
  }
  delete Prototype;
} // FreePrototype

/**
 * This routine is used to find all of the samples which
 * belong to a cluster.  It starts by removing the top
 * cluster on the cluster list (SearchState).  If this cluster is
 * a leaf it is returned.  Otherwise, the right subcluster
 * is pushed on the list and we continue the search in the
 * left subcluster.  This continues until a leaf is found.
 * If all samples have been found, nullptr is returned.
 * InitSampleSearch() must be called
 * before NextSample() to initialize the search.
 * @param SearchState ptr to list containing clusters to be searched
 * @return  Pointer to the next leaf cluster (sample) or nullptr.
 */
CLUSTER *NextSample(LIST *SearchState) {
  CLUSTER *Cluster;

  if (*SearchState == NIL_LIST) {
    return (nullptr);
  }
  Cluster = reinterpret_cast<CLUSTER *>((*SearchState)->first_node());
  *SearchState = pop(*SearchState);
  for (;;) {
    if (Cluster->Left == nullptr) {
      return (Cluster);
    }
    *SearchState = push(*SearchState, Cluster->Right);
    Cluster = Cluster->Left;
  }
} // NextSample

/**
 * This routine returns the mean of the specified
 * prototype in the indicated dimension.
 * @param Proto prototype to return mean of
 * @param Dimension dimension whose mean is to be returned
 * @return  Mean of Prototype in Dimension
 */
float Mean(PROTOTYPE *Proto, uint16_t Dimension) {
  return (Proto->Mean[Dimension]);
} // Mean

/**
 * This routine returns the standard deviation of the
 * prototype in the indicated dimension.
 * @param Proto   prototype to return standard deviation of
 * @param Dimension dimension whose stddev is to be returned
 * @return  Standard deviation of Prototype in Dimension
 */
float StandardDeviation(PROTOTYPE *Proto, uint16_t Dimension) {
  switch (Proto->Style) {
    case spherical:
      return std::sqrt(Proto->Variance.Spherical);
    case elliptical:
      return std::sqrt(Proto->Variance.Elliptical[Dimension]);
    case mixed:
      switch (Proto->Distrib[Dimension]) {
        case normal:
          return std::sqrt(Proto->Variance.Elliptical[Dimension]);
        case uniform:
        case D_random:
          return Proto->Variance.Elliptical[Dimension];
        case DISTRIBUTION_COUNT:
          ASSERT_HOST(!"Distribution count not allowed!");
      }
  }
  return 0.0f;
} // StandardDeviation

/*---------------------------------------------------------------------------
            Private Code
----------------------------------------------------------------------------*/
/**
 * This routine performs a bottoms-up clustering on the samples
 * held in the kd-tree of the Clusterer data structure.  The
 * result is a cluster tree.  Each node in the tree represents
 * a cluster which conceptually contains a subset of the samples.
 * More precisely, the cluster contains all of the samples which
 * are contained in its two sub-clusters.  The leaves of the
 * tree are the individual samples themselves; they have no
 * sub-clusters.  The root node of the tree conceptually contains
 * all of the samples.
 * The Clusterer data structure is changed.
 * @param Clusterer data structure holdings samples to be clustered
 */
static void CreateClusterTree(CLUSTERER *Clusterer) {
  ClusteringContext context;
  ClusterPair HeapEntry;

  // each sample and its nearest neighbor form a "potential" cluster
  // save these in a heap with the "best" potential clusters on top
  context.tree = Clusterer->KDTree;
  context.candidates = new TEMPCLUSTER[Clusterer->NumberOfSamples];
  context.next = 0;
  context.heap = new ClusterHeap(Clusterer->NumberOfSamples);
  KDWalk(context.tree, MakePotentialClusters, &context);

  // form potential clusters into actual clusters - always do "best" first
  while (context.heap->Pop(&HeapEntry)) {
    TEMPCLUSTER *PotentialCluster = HeapEntry.data();

    // if main cluster of potential cluster is already in another cluster
    // then we don't need to worry about it
    if (PotentialCluster->Cluster->Clustered) {
      continue;
    }

    // if main cluster is not yet clustered, but its nearest neighbor is
    // then we must find a new nearest neighbor
    else if (PotentialCluster->Neighbor->Clustered) {
      PotentialCluster->Neighbor =
          FindNearestNeighbor(context.tree, PotentialCluster->Cluster, &HeapEntry.key());
      if (PotentialCluster->Neighbor != nullptr) {
        context.heap->Push(&HeapEntry);
      }
    }

    // if neither cluster is already clustered, form permanent cluster
    else {
      PotentialCluster->Cluster = MakeNewCluster(Clusterer, PotentialCluster);
      PotentialCluster->Neighbor =
          FindNearestNeighbor(context.tree, PotentialCluster->Cluster, &HeapEntry.key());
      if (PotentialCluster->Neighbor != nullptr) {
        context.heap->Push(&HeapEntry);
      }
    }
  }

  // the root node in the cluster tree is now the only node in the kd-tree
  Clusterer->Root = static_cast<CLUSTER *> RootOf(Clusterer->KDTree);

  // free up the memory used by the K-D tree, heap, and temp clusters
  delete context.tree;
  Clusterer->KDTree = nullptr;
  delete context.heap;
  delete[] context.candidates;
} // CreateClusterTree

/**
 * This routine is designed to be used in concert with the
 * KDWalk routine.  It will create a potential cluster for
 * each sample in the kd-tree that is being walked.  This
 * potential cluster will then be pushed on the heap.
 * @param context  ClusteringContext (see definition above)
 * @param Cluster  current cluster being visited in kd-tree walk
 * @param Level  level of this cluster in the kd-tree
 */
static void MakePotentialClusters(ClusteringContext *context, CLUSTER *Cluster, int32_t /*Level*/) {
  ClusterPair HeapEntry;
  int next = context->next;
  context->candidates[next].Cluster = Cluster;
  HeapEntry.data() = &(context->candidates[next]);
  context->candidates[next].Neighbor =
      FindNearestNeighbor(context->tree, context->candidates[next].Cluster, &HeapEntry.key());
  if (context->candidates[next].Neighbor != nullptr) {
    context->heap->Push(&HeapEntry);
    context->next++;
  }
} // MakePotentialClusters

/**
 * This routine searches the specified kd-tree for the nearest
 * neighbor of the specified cluster.  It actually uses the
 * kd routines to find the 2 nearest neighbors since one of them
 * will be the original cluster.  A pointer to the nearest
 * neighbor is returned, if it can be found, otherwise nullptr is
 * returned.  The distance between the 2 nodes is placed
 * in the specified variable.
 * @param Tree    kd-tree to search in for nearest neighbor
 * @param Cluster cluster whose nearest neighbor is to be found
 * @param Distance  ptr to variable to report distance found
 * @return  Pointer to the nearest neighbor of Cluster, or nullptr
 */
static CLUSTER *FindNearestNeighbor(KDTREE *Tree, CLUSTER *Cluster, float *Distance)
#define MAXNEIGHBORS 2
#define MAXDISTANCE FLT_MAX
{
  CLUSTER *Neighbor[MAXNEIGHBORS];
  float Dist[MAXNEIGHBORS];
  int NumberOfNeighbors;
  int32_t i;
  CLUSTER *BestNeighbor;

  // find the 2 nearest neighbors of the cluster
  KDNearestNeighborSearch(Tree, &Cluster->Mean[0], MAXNEIGHBORS, MAXDISTANCE, &NumberOfNeighbors,
                          reinterpret_cast<void **>(Neighbor), Dist);

  // search for the nearest neighbor that is not the cluster itself
  *Distance = MAXDISTANCE;
  BestNeighbor = nullptr;
  for (i = 0; i < NumberOfNeighbors; i++) {
    if ((Dist[i] < *Distance) && (Neighbor[i] != Cluster)) {
      *Distance = Dist[i];
      BestNeighbor = Neighbor[i];
    }
  }
  return BestNeighbor;
} // FindNearestNeighbor

/**
 * This routine creates a new permanent cluster from the
 * clusters specified in TempCluster.  The 2 clusters in
 * TempCluster are marked as "clustered" and deleted from
 * the kd-tree.  The new cluster is then added to the kd-tree.
 * @param Clusterer current clustering environment
 * @param TempCluster potential cluster to make permanent
 * @return Pointer to the new permanent cluster
 */
static CLUSTER *MakeNewCluster(CLUSTERER *Clusterer, TEMPCLUSTER *TempCluster) {
  // allocate the new cluster and initialize it
  auto Cluster = new CLUSTER(Clusterer->SampleSize);
  Cluster->Clustered = false;
  Cluster->Prototype = false;
  Cluster->Left = TempCluster->Cluster;
  Cluster->Right = TempCluster->Neighbor;
  Cluster->CharID = -1;

  // mark the old clusters as "clustered" and delete them from the kd-tree
  Cluster->Left->Clustered = true;
  Cluster->Right->Clustered = true;
  KDDelete(Clusterer->KDTree, &Cluster->Left->Mean[0], Cluster->Left);
  KDDelete(Clusterer->KDTree, &Cluster->Right->Mean[0], Cluster->Right);

  // compute the mean and sample count for the new cluster
  Cluster->SampleCount = MergeClusters(Clusterer->SampleSize, Clusterer->ParamDesc,
                                       Cluster->Left->SampleCount, Cluster->Right->SampleCount,
                                       &Cluster->Mean[0], &Cluster->Left->Mean[0], &Cluster->Right->Mean[0]);

  // add the new cluster to the KD tree
  KDStore(Clusterer->KDTree, &Cluster->Mean[0], Cluster);
  return Cluster;
} // MakeNewCluster

/**
 * This routine merges two clusters into one larger cluster.
 * To do this it computes the number of samples in the new
 * cluster and the mean of the new cluster.  The ParamDesc
 * information is used to ensure that circular dimensions
 * are handled correctly.
 * @param N # of dimensions (size of arrays)
 * @param ParamDesc array of dimension descriptions
 * @param n1, n2  number of samples in each old cluster
 * @param m array to hold mean of new cluster
 * @param m1, m2  arrays containing means of old clusters
 * @return  The number of samples in the new cluster.
 */
int32_t MergeClusters(int16_t N, PARAM_DESC ParamDesc[], int32_t n1, int32_t n2, float m[],
                      float m1[], float m2[]) {
  int32_t i, n;

  n = n1 + n2;
  for (i = N; i > 0; i--, ParamDesc++, m++, m1++, m2++) {
    if (ParamDesc->Circular) {
      // if distance between means is greater than allowed
      // reduce upper point by one "rotation" to compute mean
      // then normalize the mean back into the accepted range
      if ((*m2 - *m1) > ParamDesc->HalfRange) {
        *m = (n1 * *m1 + n2 * (*m2 - ParamDesc->Range)) / n;
        if (*m < ParamDesc->Min) {
          *m += ParamDesc->Range;
        }
      } else if ((*m1 - *m2) > ParamDesc->HalfRange) {
        *m = (n1 * (*m1 - ParamDesc->Range) + n2 * *m2) / n;
        if (*m < ParamDesc->Min) {
          *m += ParamDesc->Range;
        }
      } else {
        *m = (n1 * *m1 + n2 * *m2) / n;
      }
    } else {
      *m = (n1 * *m1 + n2 * *m2) / n;
    }
  }
  return n;
} // MergeClusters

/**
 * This routine decides which clusters in the cluster tree
 * should be represented by prototypes, forms a list of these
 * prototypes, and places the list in the Clusterer data
 * structure.
 * @param Clusterer data structure holding cluster tree
 * @param Config    parameters used to control prototype generation
 */
static void ComputePrototypes(CLUSTERER *Clusterer, CLUSTERCONFIG *Config) {
  LIST ClusterStack = NIL_LIST;
  CLUSTER *Cluster;
  PROTOTYPE *Prototype;

  // use a stack to keep track of clusters waiting to be processed
  // initially the only cluster on the stack is the root cluster
  if (Clusterer->Root != nullptr) {
    ClusterStack = push(NIL_LIST, Clusterer->Root);
  }

  // loop until we have analyzed all clusters which are potential prototypes
  while (ClusterStack != NIL_LIST) {
    // remove the next cluster to be analyzed from the stack
    // try to make a prototype from the cluster
    // if successful, put it on the proto list, else split the cluster
    Cluster = reinterpret_cast<CLUSTER *>(ClusterStack->first_node());
    ClusterStack = pop(ClusterStack);
    Prototype = MakePrototype(Clusterer, Config, Cluster);
    if (Prototype != nullptr) {
      Clusterer->ProtoList = push(Clusterer->ProtoList, Prototype);
    } else {
      ClusterStack = push(ClusterStack, Cluster->Right);
      ClusterStack = push(ClusterStack, Cluster->Left);
    }
  }
} // ComputePrototypes

/**
 * This routine attempts to create a prototype from the
 * specified cluster that conforms to the distribution
 * specified in Config.  If there are too few samples in the
 * cluster to perform a statistical analysis, then a prototype
 * is generated but labelled as insignificant.  If the
 * dimensions of the cluster are not independent, no prototype
 * is generated and nullptr is returned.  If a prototype can be
 * found that matches the desired distribution then a pointer
 * to it is returned, otherwise nullptr is returned.
 * @param Clusterer data structure holding cluster tree
 * @param Config  parameters used to control prototype generation
 * @param Cluster cluster to be made into a prototype
 * @return  Pointer to new prototype or nullptr
 */
static PROTOTYPE *MakePrototype(CLUSTERER *Clusterer, CLUSTERCONFIG *Config, CLUSTER *Cluster) {
  PROTOTYPE *Proto;
  BUCKETS *Buckets;

  // filter out clusters which contain samples from the same character
  if (MultipleCharSamples(Clusterer, Cluster, Config->MaxIllegal)) {
    return nullptr;
  }

  // compute the covariance matrix and ranges for the cluster
  auto Statistics = ComputeStatistics(Clusterer->SampleSize, Clusterer->ParamDesc, Cluster);

  // check for degenerate clusters which need not be analyzed further
  // note that the MinSamples test assumes that all clusters with multiple
  // character samples have been removed (as above)
  Proto = MakeDegenerateProto(Clusterer->SampleSize, Cluster, Statistics, Config->ProtoStyle,
                              static_cast<int32_t>(Config->MinSamples * Clusterer->NumChar));
  if (Proto != nullptr) {
    delete Statistics;
    return Proto;
  }
  // check to ensure that all dimensions are independent
  if (!Independent(Clusterer->ParamDesc, Clusterer->SampleSize, &Statistics->CoVariance[0],
                   Config->Independence)) {
    delete Statistics;
    return nullptr;
  }

  if (HOTELLING && Config->ProtoStyle == elliptical) {
    Proto = TestEllipticalProto(Clusterer, Config, Cluster, Statistics);
    if (Proto != nullptr) {
      delete Statistics;
      return Proto;
    }
  }

  // create a histogram data structure used to evaluate distributions
  Buckets = GetBuckets(Clusterer, normal, Cluster->SampleCount, Config->Confidence);

  // create a prototype based on the statistics and test it
  switch (Config->ProtoStyle) {
    case spherical:
      Proto = MakeSphericalProto(Clusterer, Cluster, Statistics, Buckets);
      break;
    case elliptical:
      Proto = MakeEllipticalProto(Clusterer, Cluster, Statistics, Buckets);
      break;
    case mixed:
      Proto = MakeMixedProto(Clusterer, Cluster, Statistics, Buckets, Config->Confidence);
      break;
    case automatic:
      Proto = MakeSphericalProto(Clusterer, Cluster, Statistics, Buckets);
      if (Proto != nullptr) {
        break;
      }
      Proto = MakeEllipticalProto(Clusterer, Cluster, Statistics, Buckets);
      if (Proto != nullptr) {
        break;
      }
      Proto = MakeMixedProto(Clusterer, Cluster, Statistics, Buckets, Config->Confidence);
      break;
  }
  delete Statistics;
  return Proto;
} // MakePrototype

/**
 * This routine checks for clusters which are degenerate and
 * therefore cannot be analyzed in a statistically valid way.
 * A cluster is defined as degenerate if it does not have at
 * least MINSAMPLESNEEDED samples in it.  If the cluster is
 * found to be degenerate, a prototype of the specified style
 * is generated and marked as insignificant.  A cluster is
 * also degenerate if it does not have at least MinSamples
 * samples in it.
 *
 * If the cluster is not degenerate, nullptr is returned.
 *
 * @param N   number of dimensions
 * @param Cluster   cluster being analyzed
 * @param Statistics  statistical info about cluster
 * @param Style   type of prototype to be generated
 * @param MinSamples  minimum number of samples in a cluster
 * @return  Pointer to degenerate prototype or nullptr.
 */
static PROTOTYPE *MakeDegenerateProto( // this was MinSample
    uint16_t N, CLUSTER *Cluster, STATISTICS *Statistics, PROTOSTYLE Style, int32_t MinSamples) {
  PROTOTYPE *Proto = nullptr;

  if (MinSamples < MINSAMPLESNEEDED) {
    MinSamples = MINSAMPLESNEEDED;
  }

  if (Cluster->SampleCount < MinSamples) {
    switch (Style) {
      case spherical:
        Proto = NewSphericalProto(N, Cluster, Statistics);
        break;
      case elliptical:
      case automatic:
        Proto = NewEllipticalProto(N, Cluster, Statistics);
        break;
      case mixed:
        Proto = NewMixedProto(N, Cluster, Statistics);
        break;
    }
    Proto->Significant = false;
  }
  return (Proto);
} // MakeDegenerateProto

/**
 * This routine tests the specified cluster to see if **
 * there is a statistically significant difference between
 * the sub-clusters that would be made if the cluster were to
 * be split. If not, then a new prototype is formed and
 * returned to the caller. If there is, then nullptr is returned
 * to the caller.
 * @param Clusterer data struct containing samples being clustered
 * @param Config provides the magic number of samples that make a good cluster
 * @param Cluster   cluster to be made into an elliptical prototype
 * @param Statistics  statistical info about cluster
 * @return Pointer to new elliptical prototype or nullptr.
 */
static PROTOTYPE *TestEllipticalProto(CLUSTERER *Clusterer, CLUSTERCONFIG *Config, CLUSTER *Cluster,
                                      STATISTICS *Statistics) {
  // Fraction of the number of samples used as a range around 1 within
  // which a cluster has the magic size that allows a boost to the
  // FTable by kFTableBoostMargin, thus allowing clusters near the
  // magic size (equal to the number of sample characters) to be more
  // likely to stay together.
  const double kMagicSampleMargin = 0.0625;
  const double kFTableBoostMargin = 2.0;

  int N = Clusterer->SampleSize;
  CLUSTER *Left = Cluster->Left;
  CLUSTER *Right = Cluster->Right;
  if (Left == nullptr || Right == nullptr) {
    return nullptr;
  }
  int TotalDims = Left->SampleCount + Right->SampleCount;
  if (TotalDims < N + 1 || TotalDims < 2) {
    return nullptr;
  }
  std::vector<float> Covariance(static_cast<size_t>(N) * N);
  std::vector<float> Inverse(static_cast<size_t>(N) * N);
  std::vector<float> Delta(N);
  // Compute a new covariance matrix that only uses essential features.
  for (int i = 0; i < N; ++i) {
    int row_offset = i * N;
    if (!Clusterer->ParamDesc[i].NonEssential) {
      for (int j = 0; j < N; ++j) {
        if (!Clusterer->ParamDesc[j].NonEssential) {
          Covariance[j + row_offset] = Statistics->CoVariance[j + row_offset];
        } else {
          Covariance[j + row_offset] = 0.0f;
        }
      }
    } else {
      for (int j = 0; j < N; ++j) {
        if (i == j) {
          Covariance[j + row_offset] = 1.0f;
        } else {
          Covariance[j + row_offset] = 0.0f;
        }
      }
    }
  }
  double err = InvertMatrix(&Covariance[0], N, &Inverse[0]);
  if (err > 1) {
    tprintf("Clustering error: Matrix inverse failed with error %g\n", err);
  }
  int EssentialN = 0;
  for (int dim = 0; dim < N; ++dim) {
    if (!Clusterer->ParamDesc[dim].NonEssential) {
      Delta[dim] = Left->Mean[dim] - Right->Mean[dim];
      ++EssentialN;
    } else {
      Delta[dim] = 0.0f;
    }
  }
  // Compute Hotelling's T-squared.
  double Tsq = 0.0;
  for (int x = 0; x < N; ++x) {
    double temp = 0.0;
    for (int y = 0; y < N; ++y) {
      temp += static_cast<double>(Inverse[y + N * x]) * Delta[y];
    }
    Tsq += Delta[x] * temp;
  }
  // Changed this function to match the formula in
  // Statistical Methods in Medical Research p 473
  // By Peter Armitage, Geoffrey Berry, J. N. S. Matthews.
  // Tsq *= Left->SampleCount * Right->SampleCount / TotalDims;
  double F = Tsq * (TotalDims - EssentialN - 1) / ((TotalDims - 2) * EssentialN);
  int Fx = EssentialN;
  if (Fx > FTABLE_X) {
    Fx = FTABLE_X;
  }
  --Fx;
  int Fy = TotalDims - EssentialN - 1;
  if (Fy > FTABLE_Y) {
    Fy = FTABLE_Y;
  }
  --Fy;
  double FTarget = FTable[Fy][Fx];
  if (Config->MagicSamples > 0 && TotalDims >= Config->MagicSamples * (1.0 - kMagicSampleMargin) &&
      TotalDims <= Config->MagicSamples * (1.0 + kMagicSampleMargin)) {
    // Give magic-sized clusters a magic FTable boost.
    FTarget += kFTableBoostMargin;
  }
  if (F < FTarget) {
    return NewEllipticalProto(Clusterer->SampleSize, Cluster, Statistics);
  }
  return nullptr;
}

/**
 * This routine tests the specified cluster to see if it can
 * be approximated by a spherical normal distribution.  If it
 * can be, then a new prototype is formed and returned to the
 * caller.  If it can't be, then nullptr is returned to the caller.
 * @param Clusterer data struct containing samples being clustered
 * @param Cluster   cluster to be made into a spherical prototype
 * @param Statistics  statistical info about cluster
 * @param Buckets   histogram struct used to analyze distribution
 * @return  Pointer to new spherical prototype or nullptr.
 */
static PROTOTYPE *MakeSphericalProto(CLUSTERER *Clusterer, CLUSTER *Cluster, STATISTICS *Statistics,
                                     BUCKETS *Buckets) {
  PROTOTYPE *Proto = nullptr;
  int i;

  // check that each dimension is a normal distribution
  for (i = 0; i < Clusterer->SampleSize; i++) {
    if (Clusterer->ParamDesc[i].NonEssential) {
      continue;
    }

    FillBuckets(Buckets, Cluster, i, &(Clusterer->ParamDesc[i]), Cluster->Mean[i],
                sqrt(static_cast<double>(Statistics->AvgVariance)));
    if (!DistributionOK(Buckets)) {
      break;
    }
  }
  // if all dimensions matched a normal distribution, make a proto
  if (i >= Clusterer->SampleSize) {
    Proto = NewSphericalProto(Clusterer->SampleSize, Cluster, Statistics);
  }
  return (Proto);
} // MakeSphericalProto

/**
 * This routine tests the specified cluster to see if it can
 * be approximated by an elliptical normal distribution.  If it
 * can be, then a new prototype is formed and returned to the
 * caller.  If it can't be, then nullptr is returned to the caller.
 * @param Clusterer data struct containing samples being clustered
 * @param Cluster   cluster to be made into an elliptical prototype
 * @param Statistics  statistical info about cluster
 * @param Buckets   histogram struct used to analyze distribution
 * @return  Pointer to new elliptical prototype or nullptr.
 */
static PROTOTYPE *MakeEllipticalProto(CLUSTERER *Clusterer, CLUSTER *Cluster,
                                      STATISTICS *Statistics, BUCKETS *Buckets) {
  PROTOTYPE *Proto = nullptr;
  int i;

  // check that each dimension is a normal distribution
  for (i = 0; i < Clusterer->SampleSize; i++) {
    if (Clusterer->ParamDesc[i].NonEssential) {
      continue;
    }

    FillBuckets(Buckets, Cluster, i, &(Clusterer->ParamDesc[i]), Cluster->Mean[i],
                sqrt(static_cast<double>(Statistics->CoVariance[i * (Clusterer->SampleSize + 1)])));
    if (!DistributionOK(Buckets)) {
      break;
    }
  }
  // if all dimensions matched a normal distribution, make a proto
  if (i >= Clusterer->SampleSize) {
    Proto = NewEllipticalProto(Clusterer->SampleSize, Cluster, Statistics);
  }
  return (Proto);
} // MakeEllipticalProto

/**
 * This routine tests each dimension of the specified cluster to
 * see what distribution would best approximate that dimension.
 * Each dimension is compared to the following distributions
 * in order: normal, random, uniform.  If each dimension can
 * be represented by one of these distributions,
 * then a new prototype is formed and returned to the
 * caller.  If it can't be, then nullptr is returned to the caller.
 * @param Clusterer data struct containing samples being clustered
 * @param Cluster   cluster to be made into a prototype
 * @param Statistics  statistical info about cluster
 * @param NormalBuckets histogram struct used to analyze distribution
 * @param Confidence  confidence level for alternate distributions
 * @return  Pointer to new mixed prototype or nullptr.
 */
static PROTOTYPE *MakeMixedProto(CLUSTERER *Clusterer, CLUSTER *Cluster, STATISTICS *Statistics,
                                 BUCKETS *NormalBuckets, double Confidence) {
  PROTOTYPE *Proto;
  int i;
  BUCKETS *UniformBuckets = nullptr;
  BUCKETS *RandomBuckets = nullptr;

  // create a mixed proto to work on - initially assume all dimensions normal
  Proto = NewMixedProto(Clusterer->SampleSize, Cluster, Statistics);

  // find the proper distribution for each dimension
  for (i = 0; i < Clusterer->SampleSize; i++) {
    if (Clusterer->ParamDesc[i].NonEssential) {
      continue;
    }

    FillBuckets(NormalBuckets, Cluster, i, &(Clusterer->ParamDesc[i]), Proto->Mean[i],
                std::sqrt(Proto->Variance.Elliptical[i]));
    if (DistributionOK(NormalBuckets)) {
      continue;
    }

    if (RandomBuckets == nullptr) {
      RandomBuckets = GetBuckets(Clusterer, D_random, Cluster->SampleCount, Confidence);
    }
    MakeDimRandom(i, Proto, &(Clusterer->ParamDesc[i]));
    FillBuckets(RandomBuckets, Cluster, i, &(Clusterer->ParamDesc[i]), Proto->Mean[i],
                Proto->Variance.Elliptical[i]);
    if (DistributionOK(RandomBuckets)) {
      continue;
    }

    if (UniformBuckets == nullptr) {
      UniformBuckets = GetBuckets(Clusterer, uniform, Cluster->SampleCount, Confidence);
    }
    MakeDimUniform(i, Proto, Statistics);
    FillBuckets(UniformBuckets, Cluster, i, &(Clusterer->ParamDesc[i]), Proto->Mean[i],
                Proto->Variance.Elliptical[i]);
    if (DistributionOK(UniformBuckets)) {
      continue;
    }
    break;
  }
  // if any dimension failed to match a distribution, discard the proto
  if (i < Clusterer->SampleSize) {
    FreePrototype(Proto);
    Proto = nullptr;
  }
  return (Proto);
} // MakeMixedProto

/**
 * This routine alters the ith dimension of the specified
 * mixed prototype to be D_random.
 * @param i index of dimension to be changed
 * @param Proto prototype whose dimension is to be altered
 * @param ParamDesc description of specified dimension
 */
static void MakeDimRandom(uint16_t i, PROTOTYPE *Proto, PARAM_DESC *ParamDesc) {
  Proto->Distrib[i] = D_random;
  Proto->Mean[i] = ParamDesc->MidRange;
  Proto->Variance.Elliptical[i] = ParamDesc->HalfRange;

  // subtract out the previous magnitude of this dimension from the total
  Proto->TotalMagnitude /= Proto->Magnitude.Elliptical[i];
  Proto->Magnitude.Elliptical[i] = 1.0 / ParamDesc->Range;
  Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
  Proto->LogMagnitude = log(static_cast<double>(Proto->TotalMagnitude));

  // note that the proto Weight is irrelevant for D_random protos
} // MakeDimRandom

/**
 * This routine alters the ith dimension of the specified
 * mixed prototype to be uniform.
 * @param i index of dimension to be changed
 * @param Proto   prototype whose dimension is to be altered
 * @param Statistics  statistical info about prototype
 */
static void MakeDimUniform(uint16_t i, PROTOTYPE *Proto, STATISTICS *Statistics) {
  Proto->Distrib[i] = uniform;
  Proto->Mean[i] = Proto->Cluster->Mean[i] + (Statistics->Min[i] + Statistics->Max[i]) / 2;
  Proto->Variance.Elliptical[i] = (Statistics->Max[i] - Statistics->Min[i]) / 2;
  if (Proto->Variance.Elliptical[i] < MINVARIANCE) {
    Proto->Variance.Elliptical[i] = MINVARIANCE;
  }

  // subtract out the previous magnitude of this dimension from the total
  Proto->TotalMagnitude /= Proto->Magnitude.Elliptical[i];
  Proto->Magnitude.Elliptical[i] = 1.0 / (2.0 * Proto->Variance.Elliptical[i]);
  Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
  Proto->LogMagnitude = log(static_cast<double>(Proto->TotalMagnitude));

  // note that the proto Weight is irrelevant for uniform protos
} // MakeDimUniform

/**
 * This routine searches the cluster tree for all leaf nodes
 * which are samples in the specified cluster.  It computes
 * a full covariance matrix for these samples as well as
 * keeping track of the ranges (min and max) for each
 * dimension.  A special data structure is allocated to
 * return this information to the caller.  An incremental
 * algorithm for computing statistics is not used because
 * it will not work with circular dimensions.
 * @param N number of dimensions
 * @param ParamDesc array of dimension descriptions
 * @param Cluster cluster whose stats are to be computed
 * @return  Pointer to new data structure containing statistics
 */
static STATISTICS *ComputeStatistics(int16_t N, PARAM_DESC ParamDesc[], CLUSTER *Cluster) {
  int i, j;
  LIST SearchState;
  SAMPLE *Sample;
  uint32_t SampleCountAdjustedForBias;

  // allocate memory to hold the statistics results
  auto Statistics = new STATISTICS(N);

  // allocate temporary memory to hold the sample to mean distances
  std::vector<float> Distance(N);

  // find each sample in the cluster and merge it into the statistics
  InitSampleSearch(SearchState, Cluster);
  while ((Sample = NextSample(&SearchState)) != nullptr) {
    for (i = 0; i < N; i++) {
      Distance[i] = Sample->Mean[i] - Cluster->Mean[i];
      if (ParamDesc[i].Circular) {
        if (Distance[i] > ParamDesc[i].HalfRange) {
          Distance[i] -= ParamDesc[i].Range;
        }
        if (Distance[i] < -ParamDesc[i].HalfRange) {
          Distance[i] += ParamDesc[i].Range;
        }
      }
      if (Distance[i] < Statistics->Min[i]) {
        Statistics->Min[i] = Distance[i];
      }
      if (Distance[i] > Statistics->Max[i]) {
        Statistics->Max[i] = Distance[i];
      }
    }
    auto CoVariance = &Statistics->CoVariance[0];
    for (i = 0; i < N; i++) {
      for (j = 0; j < N; j++, CoVariance++) {
        *CoVariance += Distance[i] * Distance[j];
      }
    }
  }
  // normalize the variances by the total number of samples
  // use SampleCount-1 instead of SampleCount to get an unbiased estimate
  // also compute the geometic mean of the diagonal variances
  // ensure that clusters with only 1 sample are handled correctly
  if (Cluster->SampleCount > 1) {
    SampleCountAdjustedForBias = Cluster->SampleCount - 1;
  } else {
    SampleCountAdjustedForBias = 1;
  }
  auto CoVariance = &Statistics->CoVariance[0];
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++, CoVariance++) {
      *CoVariance /= SampleCountAdjustedForBias;
      if (j == i) {
        if (*CoVariance < MINVARIANCE) {
          *CoVariance = MINVARIANCE;
        }
        Statistics->AvgVariance *= *CoVariance;
      }
    }
  }
  Statistics->AvgVariance =
      static_cast<float>(pow(static_cast<double>(Statistics->AvgVariance), 1.0 / N));

  return Statistics;
} // ComputeStatistics

/**
 * This routine creates a spherical prototype data structure to
 * approximate the samples in the specified cluster.
 * Spherical prototypes have a single variance which is
 * common across all dimensions.  All dimensions are normally
 * distributed and independent.
 * @param N number of dimensions
 * @param Cluster cluster to be made into a spherical prototype
 * @param Statistics  statistical info about samples in cluster
 * @return  Pointer to a new spherical prototype data structure
 */
static PROTOTYPE *NewSphericalProto(uint16_t N, CLUSTER *Cluster, STATISTICS *Statistics) {
  PROTOTYPE *Proto;

  Proto = NewSimpleProto(N, Cluster);

  Proto->Variance.Spherical = Statistics->AvgVariance;
  if (Proto->Variance.Spherical < MINVARIANCE) {
    Proto->Variance.Spherical = MINVARIANCE;
  }

  Proto->Magnitude.Spherical = 1.0 / sqrt(2.0 * M_PI * Proto->Variance.Spherical);
  Proto->TotalMagnitude = static_cast<float>(
      pow(static_cast<double>(Proto->Magnitude.Spherical), static_cast<double>(N)));
  Proto->Weight.Spherical = 1.0 / Proto->Variance.Spherical;
  Proto->LogMagnitude = log(static_cast<double>(Proto->TotalMagnitude));

  return (Proto);
} // NewSphericalProto

/**
 * This routine creates an elliptical prototype data structure to
 * approximate the samples in the specified cluster.
 * Elliptical prototypes have a variance for each dimension.
 * All dimensions are normally distributed and independent.
 * @param N number of dimensions
 * @param Cluster cluster to be made into an elliptical prototype
 * @param Statistics  statistical info about samples in cluster
 * @return  Pointer to a new elliptical prototype data structure
 */
static PROTOTYPE *NewEllipticalProto(int16_t N, CLUSTER *Cluster, STATISTICS *Statistics) {
  PROTOTYPE *Proto;
  int i;

  Proto = NewSimpleProto(N, Cluster);
  Proto->Variance.Elliptical = new float[N];
  Proto->Magnitude.Elliptical = new float[N];
  Proto->Weight.Elliptical = new float[N];

  auto CoVariance = &Statistics->CoVariance[0];
  Proto->TotalMagnitude = 1.0;
  for (i = 0; i < N; i++, CoVariance += N + 1) {
    Proto->Variance.Elliptical[i] = *CoVariance;
    if (Proto->Variance.Elliptical[i] < MINVARIANCE) {
      Proto->Variance.Elliptical[i] = MINVARIANCE;
    }

    Proto->Magnitude.Elliptical[i] = 1.0f / sqrt(2.0f * M_PI * Proto->Variance.Elliptical[i]);
    Proto->Weight.Elliptical[i] = 1.0f / Proto->Variance.Elliptical[i];
    Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
  }
  Proto->LogMagnitude = log(static_cast<double>(Proto->TotalMagnitude));
  Proto->Style = elliptical;
  return (Proto);
} // NewEllipticalProto

/**
 * This routine creates a mixed prototype data structure to
 * approximate the samples in the specified cluster.
 * Mixed prototypes can have different distributions for
 * each dimension.  All dimensions are independent.  The
 * structure is initially filled in as though it were an
 * elliptical prototype.  The actual distributions of the
 * dimensions can be altered by other routines.
 * @param N number of dimensions
 * @param Cluster cluster to be made into a mixed prototype
 * @param Statistics  statistical info about samples in cluster
 * @return  Pointer to a new mixed prototype data structure
 */
static PROTOTYPE *NewMixedProto(int16_t N, CLUSTER *Cluster, STATISTICS *Statistics) {
  auto Proto = NewEllipticalProto(N, Cluster, Statistics);
  Proto->Distrib.clear();
  Proto->Distrib.resize(N, normal);
  Proto->Style = mixed;
  return Proto;
} // NewMixedProto

/**
 * This routine allocates memory to hold a simple prototype
 * data structure, i.e. one without independent distributions
 * and variances for each dimension.
 * @param N number of dimensions
 * @param Cluster cluster to be made into a prototype
 * @return  Pointer to new simple prototype
 */
static PROTOTYPE *NewSimpleProto(int16_t N, CLUSTER *Cluster) {
  auto Proto = new PROTOTYPE;
  Proto->Mean = Cluster->Mean;
  Proto->Distrib.clear();
  Proto->Significant = true;
  Proto->Merged = false;
  Proto->Style = spherical;
  Proto->NumSamples = Cluster->SampleCount;
  Proto->Cluster = Cluster;
  Proto->Cluster->Prototype = true;
  return Proto;
} // NewSimpleProto

/**
 * This routine returns true if the specified covariance
 * matrix indicates that all N dimensions are independent of
 * one another.  One dimension is judged to be independent of
 * another when the magnitude of the corresponding correlation
 * coefficient is
 * less than the specified Independence factor.  The
 * correlation coefficient is calculated as: (see Duda and
 * Hart, pg. 247)
 * coeff[ij] = stddev[ij] / sqrt (stddev[ii] * stddev[jj])
 * The covariance matrix is assumed to be symmetric (which
 * should always be true).
 * @param ParamDesc descriptions of each feature space dimension
 * @param N number of dimensions
 * @param CoVariance  ptr to a covariance matrix
 * @param Independence  max off-diagonal correlation coefficient
 * @return true if dimensions are independent, false otherwise
 */
static bool Independent(PARAM_DESC *ParamDesc, int16_t N, float *CoVariance, float Independence) {
  int i, j;
  float *VARii; // points to ith on-diagonal element
  float *VARjj; // points to jth on-diagonal element
  float CorrelationCoeff;

  VARii = CoVariance;
  for (i = 0; i < N; i++, VARii += N + 1) {
    if (ParamDesc[i].NonEssential) {
      continue;
    }

    VARjj = VARii + N + 1;
    CoVariance = VARii + 1;
    for (j = i + 1; j < N; j++, CoVariance++, VARjj += N + 1) {
      if (ParamDesc[j].NonEssential) {
        continue;
      }

      if ((*VARii == 0.0) || (*VARjj == 0.0)) {
        CorrelationCoeff = 0.0;
      } else {
        CorrelationCoeff = sqrt(std::sqrt(*CoVariance * *CoVariance / (*VARii * *VARjj)));
      }
      if (CorrelationCoeff > Independence) {
        return false;
      }
    }
  }
  return true;
} // Independent

/**
 * This routine returns a histogram data structure which can
 * be used by other routines to place samples into histogram
 * buckets, and then apply a goodness of fit test to the
 * histogram data to determine if the samples belong to the
 * specified probability distribution.  The routine keeps
 * a list of bucket data structures which have already been
 * created so that it minimizes the computation time needed
 * to create a new bucket.
 * @param clusterer  which keeps a bucket_cache for us.
 * @param Distribution  type of probability distribution to test for
 * @param SampleCount number of samples that are available
 * @param Confidence  probability of a Type I error
 * @return  Bucket data structure
 */
static BUCKETS *GetBuckets(CLUSTERER *clusterer, DISTRIBUTION Distribution, uint32_t SampleCount,
                           double Confidence) {
  // Get an old bucket structure with the same number of buckets.
  uint16_t NumberOfBuckets = OptimumNumberOfBuckets(SampleCount);
  BUCKETS *Buckets = clusterer->bucket_cache[Distribution][NumberOfBuckets - MINBUCKETS];

  // If a matching bucket structure is not found, make one and save it.
  if (Buckets == nullptr) {
    Buckets = MakeBuckets(Distribution, SampleCount, Confidence);
    clusterer->bucket_cache[Distribution][NumberOfBuckets - MINBUCKETS] = Buckets;
  } else {
    // Just adjust the existing buckets.
    if (SampleCount != Buckets->SampleCount) {
      AdjustBuckets(Buckets, SampleCount);
    }
    if (Confidence != Buckets->Confidence) {
      Buckets->Confidence = Confidence;
      Buckets->ChiSquared =
          ComputeChiSquared(DegreesOfFreedom(Distribution, Buckets->NumberOfBuckets), Confidence);
    }
    InitBuckets(Buckets);
  }
  return Buckets;
} // GetBuckets

/**
 * This routine creates a histogram data structure which can
 * be used by other routines to place samples into histogram
 * buckets, and then apply a goodness of fit test to the
 * histogram data to determine if the samples belong to the
 * specified probability distribution.  The buckets are
 * allocated in such a way that the expected frequency of
 * samples in each bucket is approximately the same.  In
 * order to make this possible, a mapping table is
 * computed which maps "normalized" samples into the
 * appropriate bucket.
 * @param Distribution  type of probability distribution to test for
 * @param SampleCount number of samples that are available
 * @param Confidence  probability of a Type I error
 * @return Pointer to new histogram data structure
 */
static BUCKETS *MakeBuckets(DISTRIBUTION Distribution, uint32_t SampleCount, double Confidence) {
  const DENSITYFUNC DensityFunction[] = {NormalDensity, UniformDensity, UniformDensity};
  int i, j;
  double BucketProbability;
  double NextBucketBoundary;
  double Probability;
  double ProbabilityDelta;
  double LastProbDensity;
  double ProbDensity;
  uint16_t CurrentBucket;
  bool Symmetrical;

  // allocate memory needed for data structure
  auto Buckets = new BUCKETS(OptimumNumberOfBuckets(SampleCount));
  Buckets->SampleCount = SampleCount;
  Buckets->Confidence = Confidence;

  // initialize simple fields
  Buckets->Distribution = Distribution;

  // all currently defined distributions are symmetrical
  Symmetrical = true;
  Buckets->ChiSquared =
      ComputeChiSquared(DegreesOfFreedom(Distribution, Buckets->NumberOfBuckets), Confidence);

  if (Symmetrical) {
    // allocate buckets so that all have approx. equal probability
    BucketProbability = 1.0 / static_cast<double>(Buckets->NumberOfBuckets);

    // distribution is symmetric so fill in upper half then copy
    CurrentBucket = Buckets->NumberOfBuckets / 2;
    if (Odd(Buckets->NumberOfBuckets)) {
      NextBucketBoundary = BucketProbability / 2;
    } else {
      NextBucketBoundary = BucketProbability;
    }

    Probability = 0.0;
    LastProbDensity = (*DensityFunction[static_cast<int>(Distribution)])(BUCKETTABLESIZE / 2);
    for (i = BUCKETTABLESIZE / 2; i < BUCKETTABLESIZE; i++) {
      ProbDensity = (*DensityFunction[static_cast<int>(Distribution)])(i + 1);
      ProbabilityDelta = Integral(LastProbDensity, ProbDensity, 1.0);
      Probability += ProbabilityDelta;
      if (Probability > NextBucketBoundary) {
        if (CurrentBucket < Buckets->NumberOfBuckets - 1) {
          CurrentBucket++;
        }
        NextBucketBoundary += BucketProbability;
      }
      Buckets->Bucket[i] = CurrentBucket;
      Buckets->ExpectedCount[CurrentBucket] += static_cast<float>(ProbabilityDelta * SampleCount);
      LastProbDensity = ProbDensity;
    }
    // place any leftover probability into the last bucket
    Buckets->ExpectedCount[CurrentBucket] += static_cast<float>((0.5 - Probability) * SampleCount);

    // copy upper half of distribution to lower half
    for (i = 0, j = BUCKETTABLESIZE - 1; i < j; i++, j--) {
      Buckets->Bucket[i] = Mirror(Buckets->Bucket[j], Buckets->NumberOfBuckets);
    }

    // copy upper half of expected counts to lower half
    for (i = 0, j = Buckets->NumberOfBuckets - 1; i <= j; i++, j--) {
      Buckets->ExpectedCount[i] += Buckets->ExpectedCount[j];
    }
  }
  return Buckets;
} // MakeBuckets

/**
 * This routine computes the optimum number of histogram
 * buckets that should be used in a chi-squared goodness of
 * fit test for the specified number of samples.  The optimum
 * number is computed based on Table 4.1 on pg. 147 of
 * "Measurement and Analysis of Random Data" by Bendat & Piersol.
 * Linear interpolation is used to interpolate between table
 * values.  The table is intended for a 0.05 level of
 * significance (alpha).  This routine assumes that it is
 * equally valid for other alpha's, which may not be true.
 * @param SampleCount number of samples to be tested
 * @return Optimum number of histogram buckets
 */
static uint16_t OptimumNumberOfBuckets(uint32_t SampleCount) {
  uint8_t Last, Next;
  float Slope;

  if (SampleCount < kCountTable[0]) {
    return kBucketsTable[0];
  }

  for (Last = 0, Next = 1; Next < LOOKUPTABLESIZE; Last++, Next++) {
    if (SampleCount <= kCountTable[Next]) {
      Slope = static_cast<float>(kBucketsTable[Next] - kBucketsTable[Last]) /
              static_cast<float>(kCountTable[Next] - kCountTable[Last]);
      return (
          static_cast<uint16_t>(kBucketsTable[Last] + Slope * (SampleCount - kCountTable[Last])));
    }
  }
  return kBucketsTable[Last];
} // OptimumNumberOfBuckets

/**
 * This routine computes the chi-squared value which will
 * leave a cumulative probability of Alpha in the right tail
 * of a chi-squared distribution with the specified number of
 * degrees of freedom.  Alpha must be between 0 and 1.
 * DegreesOfFreedom must be even.  The routine maintains an
 * array of lists.  Each list corresponds to a different
 * number of degrees of freedom.  Each entry in the list
 * corresponds to a different alpha value and its corresponding
 * chi-squared value.  Therefore, once a particular chi-squared
 * value is computed, it is stored in the list and never
 * needs to be computed again.
 * @param DegreesOfFreedom  determines shape of distribution
 * @param Alpha probability of right tail
 * @return Desired chi-squared value
 */
static double ComputeChiSquared(uint16_t DegreesOfFreedom, double Alpha)
#define CHIACCURACY 0.01
#define MINALPHA (1e-200)
{
  static LIST ChiWith[MAXDEGREESOFFREEDOM + 1];

  // limit the minimum alpha that can be used - if alpha is too small
  //      it may not be possible to compute chi-squared.
  Alpha = ClipToRange(Alpha, MINALPHA, 1.0);
  if (Odd(DegreesOfFreedom)) {
    DegreesOfFreedom++;
  }

  /* find the list of chi-squared values which have already been computed
   for the specified number of degrees of freedom.  Search the list for
   the desired chi-squared. */
  CHISTRUCT SearchKey(0.0, Alpha);
  auto *found = search(ChiWith[DegreesOfFreedom], &SearchKey, AlphaMatch);
  auto OldChiSquared = reinterpret_cast<CHISTRUCT *>(found ? found->first_node() : nullptr);

  if (OldChiSquared == nullptr) {
    OldChiSquared = new CHISTRUCT(DegreesOfFreedom, Alpha);
    OldChiSquared->ChiSquared =
        Solve(ChiArea, OldChiSquared, static_cast<double>(DegreesOfFreedom), CHIACCURACY);
    ChiWith[DegreesOfFreedom] = push(ChiWith[DegreesOfFreedom], OldChiSquared);
  } else {
    // further optimization might move OldChiSquared to front of list
  }

  return (OldChiSquared->ChiSquared);

} // ComputeChiSquared

/**
 * This routine computes the probability density function
 * of a discrete normal distribution defined by the global
 * variables kNormalMean, kNormalVariance, and kNormalMagnitude.
 * Normal magnitude could, of course, be computed in terms of
 * the normal variance but it is precomputed for efficiency.
 * @param x number to compute the normal probability density for
 * @note Globals:
 *    kNormalMean mean of a discrete normal distribution
 *    kNormalVariance variance of a discrete normal distribution
 *    kNormalMagnitude  magnitude of a discrete normal distribution
 * @return  The value of the normal distribution at x.
 */
static double NormalDensity(int32_t x) {
  double Distance;

  Distance = x - kNormalMean;
  return kNormalMagnitude * exp(-0.5 * Distance * Distance / kNormalVariance);
} // NormalDensity

/**
 * This routine computes the probability density function
 * of a uniform distribution at the specified point.  The
 * range of the distribution is from 0 to BUCKETTABLESIZE.
 * @param x number to compute the uniform probability density for
 * @return The value of the uniform distribution at x.
 */
static double UniformDensity(int32_t x) {
  constexpr auto UniformDistributionDensity = 1.0 / BUCKETTABLESIZE;

  if ((x >= 0) && (x <= BUCKETTABLESIZE)) {
    return UniformDistributionDensity;
  } else {
    return 0.0;
  }
} // UniformDensity

/**
 * This routine computes a trapezoidal approximation to the
 * integral of a function over a small delta in x.
 * @param f1  value of function at x1
 * @param f2  value of function at x2
 * @param Dx  x2 - x1 (should always be positive)
 * @return Approximation of the integral of the function from x1 to x2.
 */
static double Integral(double f1, double f2, double Dx) {
  return (f1 + f2) * Dx / 2.0;
} // Integral

/**
 * This routine counts the number of cluster samples which
 * fall within the various histogram buckets in Buckets.  Only
 * one dimension of each sample is examined.  The exact meaning
 * of the Mean and StdDev parameters depends on the
 * distribution which is being analyzed (this info is in the
 * Buckets data structure).  For normal distributions, Mean
 * and StdDev have the expected meanings.  For uniform and
 * random distributions the Mean is the center point of the
 * range and the StdDev is 1/2 the range.  A dimension with
 * zero standard deviation cannot be statistically analyzed.
 * In this case, a pseudo-analysis is used.
 * The Buckets data structure is filled in.
 * @param Buckets histogram buckets to count samples
 * @param Cluster cluster whose samples are being analyzed
 * @param Dim dimension of samples which is being analyzed
 * @param ParamDesc description of the dimension
 * @param Mean  "mean" of the distribution
 * @param StdDev  "standard deviation" of the distribution
 */
static void FillBuckets(BUCKETS *Buckets, CLUSTER *Cluster, uint16_t Dim, PARAM_DESC *ParamDesc,
                        float Mean, float StdDev) {
  uint16_t BucketID;
  int i;
  LIST SearchState;
  SAMPLE *Sample;

  // initialize the histogram bucket counts to 0
  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    Buckets->Count[i] = 0;
  }

  if (StdDev == 0.0) {
    /* if the standard deviation is zero, then we can't statistically
   analyze the cluster.  Use a pseudo-analysis: samples exactly on
   the mean are distributed evenly across all buckets.  Samples greater
   than the mean are placed in the last bucket; samples less than the
   mean are placed in the first bucket. */

    InitSampleSearch(SearchState, Cluster);
    i = 0;
    while ((Sample = NextSample(&SearchState)) != nullptr) {
      if (Sample->Mean[Dim] > Mean) {
        BucketID = Buckets->NumberOfBuckets - 1;
      } else if (Sample->Mean[Dim] < Mean) {
        BucketID = 0;
      } else {
        BucketID = i;
      }
      Buckets->Count[BucketID] += 1;
      i++;
      if (i >= Buckets->NumberOfBuckets) {
        i = 0;
      }
    }
  } else {
    // search for all samples in the cluster and add to histogram buckets
    InitSampleSearch(SearchState, Cluster);
    while ((Sample = NextSample(&SearchState)) != nullptr) {
      switch (Buckets->Distribution) {
        case normal:
          BucketID = NormalBucket(ParamDesc, Sample->Mean[Dim], Mean, StdDev);
          break;
        case D_random:
        case uniform:
          BucketID = UniformBucket(ParamDesc, Sample->Mean[Dim], Mean, StdDev);
          break;
        default:
          BucketID = 0;
      }
      Buckets->Count[Buckets->Bucket[BucketID]] += 1;
    }
  }
} // FillBuckets

/**
 * This routine determines which bucket x falls into in the
 * discrete normal distribution defined by kNormalMean
 * and kNormalStdDev.  x values which exceed the range of
 * the discrete distribution are clipped.
 * @param ParamDesc used to identify circular dimensions
 * @param x value to be normalized
 * @param Mean  mean of normal distribution
 * @param StdDev  standard deviation of normal distribution
 * @return Bucket number into which x falls
 */
static uint16_t NormalBucket(PARAM_DESC *ParamDesc, float x, float Mean, float StdDev) {
  float X;

  // wraparound circular parameters if necessary
  if (ParamDesc->Circular) {
    if (x - Mean > ParamDesc->HalfRange) {
      x -= ParamDesc->Range;
    } else if (x - Mean < -ParamDesc->HalfRange) {
      x += ParamDesc->Range;
    }
  }

  X = ((x - Mean) / StdDev) * kNormalStdDev + kNormalMean;
  if (X < 0) {
    return 0;
  }
  if (X > BUCKETTABLESIZE - 1) {
    return (static_cast<uint16_t>(BUCKETTABLESIZE - 1));
  }
  return static_cast<uint16_t>(floor(static_cast<double>(X)));
} // NormalBucket

/**
 * This routine determines which bucket x falls into in the
 * discrete uniform distribution defined by
 * BUCKETTABLESIZE.  x values which exceed the range of
 * the discrete distribution are clipped.
 * @param ParamDesc used to identify circular dimensions
 * @param x value to be normalized
 * @param Mean  center of range of uniform distribution
 * @param StdDev  1/2 the range of the uniform distribution
 * @return Bucket number into which x falls
 */
static uint16_t UniformBucket(PARAM_DESC *ParamDesc, float x, float Mean, float StdDev) {
  float X;

  // wraparound circular parameters if necessary
  if (ParamDesc->Circular) {
    if (x - Mean > ParamDesc->HalfRange) {
      x -= ParamDesc->Range;
    } else if (x - Mean < -ParamDesc->HalfRange) {
      x += ParamDesc->Range;
    }
  }

  X = ((x - Mean) / (2 * StdDev) * BUCKETTABLESIZE + BUCKETTABLESIZE / 2.0);
  if (X < 0) {
    return 0;
  }
  if (X > BUCKETTABLESIZE - 1) {
    return static_cast<uint16_t>(BUCKETTABLESIZE - 1);
  }
  return static_cast<uint16_t>(floor(static_cast<double>(X)));
} // UniformBucket

/**
 * This routine performs a chi-square goodness of fit test
 * on the histogram data in the Buckets data structure.
 * true is returned if the histogram matches the probability
 * distribution which was specified when the Buckets
 * structure was originally created.  Otherwise false is
 * returned.
 * @param Buckets   histogram data to perform chi-square test on
 * @return true if samples match distribution, false otherwise
 */
static bool DistributionOK(BUCKETS *Buckets) {
  float FrequencyDifference;
  float TotalDifference;
  int i;

  // compute how well the histogram matches the expected histogram
  TotalDifference = 0.0;
  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    FrequencyDifference = Buckets->Count[i] - Buckets->ExpectedCount[i];
    TotalDifference += (FrequencyDifference * FrequencyDifference) / Buckets->ExpectedCount[i];
  }

  // test to see if the difference is more than expected
  if (TotalDifference > Buckets->ChiSquared) {
    return false;
  } else {
    return true;
  }
} // DistributionOK

/**
 * This routine computes the degrees of freedom that should
 * be used in a chi-squared test with the specified number of
 * histogram buckets.  The result is always rounded up to
 * the next even number so that the value of chi-squared can be
 * computed more easily.  This will cause the value of
 * chi-squared to be higher than the optimum value, resulting
 * in the chi-square test being more lenient than optimum.
 * @param Distribution    distribution being tested for
 * @param HistogramBuckets  number of buckets in chi-square test
 * @return The number of degrees of freedom for a chi-square test
 */
static uint16_t DegreesOfFreedom(DISTRIBUTION Distribution, uint16_t HistogramBuckets) {
  static uint8_t DegreeOffsets[] = {3, 3, 1};

  uint16_t AdjustedNumBuckets;

  AdjustedNumBuckets = HistogramBuckets - DegreeOffsets[static_cast<int>(Distribution)];
  if (Odd(AdjustedNumBuckets)) {
    AdjustedNumBuckets++;
  }
  return (AdjustedNumBuckets);

} // DegreesOfFreedom

/**
 * This routine multiplies each ExpectedCount histogram entry
 * by NewSampleCount/OldSampleCount so that the histogram
 * is now adjusted to the new sample count.
 * @param Buckets histogram data structure to adjust
 * @param NewSampleCount  new sample count to adjust to
 */
static void AdjustBuckets(BUCKETS *Buckets, uint32_t NewSampleCount) {
  int i;
  double AdjustFactor;

  AdjustFactor =
      ((static_cast<double>(NewSampleCount)) / (static_cast<double>(Buckets->SampleCount)));

  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    Buckets->ExpectedCount[i] *= AdjustFactor;
  }

  Buckets->SampleCount = NewSampleCount;

} // AdjustBuckets

/**
 * This routine sets the bucket counts in the specified histogram
 * to zero.
 * @param Buckets histogram data structure to init
 */
static void InitBuckets(BUCKETS *Buckets) {
  int i;

  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    Buckets->Count[i] = 0;
  }

} // InitBuckets

/**
 * This routine is used to search a list of structures which
 * hold pre-computed chi-squared values for a chi-squared
 * value whose corresponding alpha field matches the alpha
 * field of SearchKey.
 *
 * It is called by the list search routines.
 *
 * @param arg1 chi-squared struct being tested for a match
 * @param arg2 chi-squared struct that is the search key
 * @return true if ChiStruct's Alpha matches SearchKey's Alpha
 */
static int AlphaMatch(void *arg1,   // CHISTRUCT *ChiStruct,
                      void *arg2) { // CHISTRUCT *SearchKey)
  auto *ChiStruct = static_cast<CHISTRUCT *>(arg1);
  auto *SearchKey = static_cast<CHISTRUCT *>(arg2);

  return (ChiStruct->Alpha == SearchKey->Alpha);

} // AlphaMatch

/**
 * This routine attempts to find an x value at which Function
 * goes to zero (i.e. a root of the function).  It will only
 * work correctly if a solution actually exists and there
 * are no extrema between the solution and the InitialGuess.
 * The algorithms used are extremely primitive.
 *
 * @param Function  function whose zero is to be found
 * @param FunctionParams  arbitrary data to pass to function
 * @param InitialGuess  point to start solution search at
 * @param Accuracy  maximum allowed error
 * @return Solution of function (x for which f(x) = 0).
 */
static double Solve(SOLVEFUNC Function, void *FunctionParams, double InitialGuess, double Accuracy)
#define INITIALDELTA 0.1
#define DELTARATIO 0.1
{
  double x;
  double f;
  double Slope;
  double Delta;
  double NewDelta;
  double xDelta;
  double LastPosX, LastNegX;

  x = InitialGuess;
  Delta = INITIALDELTA;
  LastPosX = FLT_MAX;
  LastNegX = -FLT_MAX;
  f = (*Function)(static_cast<CHISTRUCT *>(FunctionParams), x);
  while (Abs(LastPosX - LastNegX) > Accuracy) {
    // keep track of outer bounds of current estimate
    if (f < 0) {
      LastNegX = x;
    } else {
      LastPosX = x;
    }

    // compute the approx. slope of f(x) at the current point
    Slope = ((*Function)(static_cast<CHISTRUCT *>(FunctionParams), x + Delta) - f) / Delta;

    // compute the next solution guess */
    xDelta = f / Slope;
    x -= xDelta;

    // reduce the delta used for computing slope to be a fraction of
    // the amount moved to get to the new guess
    NewDelta = Abs(xDelta) * DELTARATIO;
    if (NewDelta < Delta) {
      Delta = NewDelta;
    }

    // compute the value of the function at the new guess
    f = (*Function)(static_cast<CHISTRUCT *>(FunctionParams), x);
  }
  return (x);

} // Solve

/**
 * This routine computes the area under a chi density curve
 * from 0 to x, minus the desired area under the curve.  The
 * number of degrees of freedom of the chi curve is specified
 * in the ChiParams structure.  The desired area is also
 * specified in the ChiParams structure as Alpha (or 1 minus
 * the desired area).  This routine is intended to be passed
 * to the Solve() function to find the value of chi-squared
 * which will yield a desired area under the right tail of
 * the chi density curve.  The function will only work for
 * even degrees of freedom.  The equations are based on
 * integrating the chi density curve in parts to obtain
 * a series that can be used to compute the area under the
 * curve.
 * @param ChiParams contains degrees of freedom and alpha
 * @param x   value of chi-squared to evaluate
 * @return Error between actual and desired area under the chi curve.
 */
static double ChiArea(CHISTRUCT *ChiParams, double x) {
  int i, N;
  double SeriesTotal;
  double Denominator;
  double PowerOfx;

  N = ChiParams->DegreesOfFreedom / 2 - 1;
  SeriesTotal = 1;
  Denominator = 1;
  PowerOfx = 1;
  for (i = 1; i <= N; i++) {
    Denominator *= 2 * i;
    PowerOfx *= x;
    SeriesTotal += PowerOfx / Denominator;
  }
  return ((SeriesTotal * exp(-0.5 * x)) - ChiParams->Alpha);

} // ChiArea

/**
 * This routine looks at all samples in the specified cluster.
 * It computes a running estimate of the percentage of the
 * characters which have more than 1 sample in the cluster.
 * When this percentage exceeds MaxIllegal, true is returned.
 * Otherwise false is returned.  The CharID
 * fields must contain integers which identify the training
 * characters which were used to generate the sample.  One
 * integer is used for each sample.  The NumChar field in
 * the Clusterer must contain the number of characters in the
 * training set.  All CharID fields must be between 0 and
 * NumChar-1.  The main function of this routine is to help
 * identify clusters which need to be split further, i.e. if
 * numerous training characters have 2 or more features which are
 * contained in the same cluster, then the cluster should be
 * split.
 *
 * @param Clusterer data structure holding cluster tree
 * @param Cluster   cluster containing samples to be tested
 * @param MaxIllegal  max percentage of samples allowed to have
 *        more than 1 feature in the cluster
 * @return true if the cluster should be split, false otherwise.
 */
static bool MultipleCharSamples(CLUSTERER *Clusterer, CLUSTER *Cluster, float MaxIllegal)
#define ILLEGAL_CHAR 2
{
  static std::vector<uint8_t> CharFlags;
  LIST SearchState;
  SAMPLE *Sample;
  int32_t CharID;
  int32_t NumCharInCluster;
  int32_t NumIllegalInCluster;
  float PercentIllegal;

  // initial estimate assumes that no illegal chars exist in the cluster
  NumCharInCluster = Cluster->SampleCount;
  NumIllegalInCluster = 0;

  if (Clusterer->NumChar > CharFlags.size()) {
    CharFlags.resize(Clusterer->NumChar);
  }

  for (auto &CharFlag : CharFlags) {
    CharFlag = false;
  }

  // find each sample in the cluster and check if we have seen it before
  InitSampleSearch(SearchState, Cluster);
  while ((Sample = NextSample(&SearchState)) != nullptr) {
    CharID = Sample->CharID;
    if (CharFlags[CharID] == 0) {
      CharFlags[CharID] = true;
    } else {
      if (CharFlags[CharID] == 1) {
        NumIllegalInCluster++;
        CharFlags[CharID] = ILLEGAL_CHAR;
      }
      NumCharInCluster--;
      PercentIllegal = static_cast<float>(NumIllegalInCluster) / NumCharInCluster;
      if (PercentIllegal > MaxIllegal) {
        destroy(SearchState);
        return true;
      }
    }
  }
  return false;

} // MultipleCharSamples

/**
 * Compute the inverse of a matrix using LU decomposition with partial pivoting.
 * The return value is the sum of norms of the off-diagonal terms of the
 * product of a and inv. (A measure of the error.)
 */
static double InvertMatrix(const float *input, int size, float *inv) {
  // Allocate memory for the 2D arrays.
  GENERIC_2D_ARRAY<double> U(size, size, 0.0);
  GENERIC_2D_ARRAY<double> U_inv(size, size, 0.0);
  GENERIC_2D_ARRAY<double> L(size, size, 0.0);

  // Initialize the working matrices. U starts as input, L as I and U_inv as O.
  int row;
  int col;
  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      U[row][col] = input[row * size + col];
      L[row][col] = row == col ? 1.0 : 0.0;
      U_inv[row][col] = 0.0;
    }
  }

  // Compute forward matrix by inversion by LU decomposition of input.
  for (col = 0; col < size; ++col) {
    // Find best pivot
    int best_row = 0;
    double best_pivot = -1.0;
    for (row = col; row < size; ++row) {
      if (Abs(U[row][col]) > best_pivot) {
        best_pivot = Abs(U[row][col]);
        best_row = row;
      }
    }
    // Exchange pivot rows.
    if (best_row != col) {
      for (int k = 0; k < size; ++k) {
        double tmp = U[best_row][k];
        U[best_row][k] = U[col][k];
        U[col][k] = tmp;
        tmp = L[best_row][k];
        L[best_row][k] = L[col][k];
        L[col][k] = tmp;
      }
    }
    // Now do the pivot itself.
    for (row = col + 1; row < size; ++row) {
      double ratio = -U[row][col] / U[col][col];
      for (int j = col; j < size; ++j) {
        U[row][j] += U[col][j] * ratio;
      }
      for (int k = 0; k < size; ++k) {
        L[row][k] += L[col][k] * ratio;
      }
    }
  }
  // Next invert U.
  for (col = 0; col < size; ++col) {
    U_inv[col][col] = 1.0 / U[col][col];
    for (row = col - 1; row >= 0; --row) {
      double total = 0.0;
      for (int k = col; k > row; --k) {
        total += U[row][k] * U_inv[k][col];
      }
      U_inv[row][col] = -total / U[row][row];
    }
  }
  // Now the answer is U_inv.L.
  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      double sum = 0.0;
      for (int k = row; k < size; ++k) {
        sum += U_inv[row][k] * L[k][col];
      }
      inv[row * size + col] = sum;
    }
  }
  // Check matrix product.
  double error_sum = 0.0;
  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      double sum = 0.0;
      for (int k = 0; k < size; ++k) {
        sum += static_cast<double>(input[row * size + k]) * inv[k * size + col];
      }
      if (row != col) {
        error_sum += Abs(sum);
      }
    }
  }
  return error_sum;
}

} // namespace tesseract
