/******************************************************************************
 **	Filename:	cluster.c
 **	Purpose:	Routines for clustering points in N-D space
 **	Author:		Dan Johnson
 **	History:	5/29/89, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
#include "const.h"
#include "cluster.h"
#include "emalloc.h"
#include "genericheap.h"
#include "helpers.h"
#include "kdpair.h"
#include "matrix.h"
#include "tprintf.h"
#include "danerror.h"
#include "freelist.h"
#include <math.h>

#define HOTELLING 1  // If true use Hotelling's test to decide where to split.
#define FTABLE_X 10  // Size of FTable.
#define FTABLE_Y 100  // Size of FTable.

// Table of values approximating the cumulative F-distribution for a confidence of 1%.
const double FTable[FTABLE_Y][FTABLE_X] = {
 {4052.19, 4999.52, 5403.34, 5624.62, 5763.65, 5858.97, 5928.33, 5981.10, 6022.50, 6055.85,},
  {98.502,  99.000,  99.166,  99.249,  99.300,  99.333,  99.356,  99.374,  99.388,  99.399,},
  {34.116,  30.816,  29.457,  28.710,  28.237,  27.911,  27.672,  27.489,  27.345,  27.229,},
  {21.198,  18.000,  16.694,  15.977,  15.522,  15.207,  14.976,  14.799,  14.659,  14.546,},
  {16.258,  13.274,  12.060,  11.392,  10.967,  10.672,  10.456,  10.289,  10.158,  10.051,},
  {13.745,  10.925,   9.780,   9.148,   8.746,   8.466,   8.260,   8.102,   7.976,   7.874,},
  {12.246,   9.547,   8.451,   7.847,   7.460,   7.191,   6.993,   6.840,   6.719,   6.620,},
  {11.259,   8.649,   7.591,   7.006,   6.632,   6.371,   6.178,   6.029,   5.911,   5.814,},
  {10.561,   8.022,   6.992,   6.422,   6.057,   5.802,   5.613,   5.467,   5.351,   5.257,},
  {10.044,   7.559,   6.552,   5.994,   5.636,   5.386,   5.200,   5.057,   4.942,   4.849,},
  { 9.646,   7.206,   6.217,   5.668,   5.316,   5.069,   4.886,   4.744,   4.632,   4.539,},
  { 9.330,   6.927,   5.953,   5.412,   5.064,   4.821,   4.640,   4.499,   4.388,   4.296,},
  { 9.074,   6.701,   5.739,   5.205,   4.862,   4.620,   4.441,   4.302,   4.191,   4.100,},
  { 8.862,   6.515,   5.564,   5.035,   4.695,   4.456,   4.278,   4.140,   4.030,   3.939,},
  { 8.683,   6.359,   5.417,   4.893,   4.556,   4.318,   4.142,   4.004,   3.895,   3.805,},
  { 8.531,   6.226,   5.292,   4.773,   4.437,   4.202,   4.026,   3.890,   3.780,   3.691,},
  { 8.400,   6.112,   5.185,   4.669,   4.336,   4.102,   3.927,   3.791,   3.682,   3.593,},
  { 8.285,   6.013,   5.092,   4.579,   4.248,   4.015,   3.841,   3.705,   3.597,   3.508,},
  { 8.185,   5.926,   5.010,   4.500,   4.171,   3.939,   3.765,   3.631,   3.523,   3.434,},
  { 8.096,   5.849,   4.938,   4.431,   4.103,   3.871,   3.699,   3.564,   3.457,   3.368,},
  { 8.017,   5.780,   4.874,   4.369,   4.042,   3.812,   3.640,   3.506,   3.398,   3.310,},
  { 7.945,   5.719,   4.817,   4.313,   3.988,   3.758,   3.587,   3.453,   3.346,   3.258,},
  { 7.881,   5.664,   4.765,   4.264,   3.939,   3.710,   3.539,   3.406,   3.299,   3.211,},
  { 7.823,   5.614,   4.718,   4.218,   3.895,   3.667,   3.496,   3.363,   3.256,   3.168,},
  { 7.770,   5.568,   4.675,   4.177,   3.855,   3.627,   3.457,   3.324,   3.217,   3.129,},
  { 7.721,   5.526,   4.637,   4.140,   3.818,   3.591,   3.421,   3.288,   3.182,   3.094,},
  { 7.677,   5.488,   4.601,   4.106,   3.785,   3.558,   3.388,   3.256,   3.149,   3.062,},
  { 7.636,   5.453,   4.568,   4.074,   3.754,   3.528,   3.358,   3.226,   3.120,   3.032,},
  { 7.598,   5.420,   4.538,   4.045,   3.725,   3.499,   3.330,   3.198,   3.092,   3.005,},
  { 7.562,   5.390,   4.510,   4.018,   3.699,   3.473,   3.305,   3.173,   3.067,   2.979,},
  { 7.530,   5.362,   4.484,   3.993,   3.675,   3.449,   3.281,   3.149,   3.043,   2.955,},
  { 7.499,   5.336,   4.459,   3.969,   3.652,   3.427,   3.258,   3.127,   3.021,   2.934,},
  { 7.471,   5.312,   4.437,   3.948,   3.630,   3.406,   3.238,   3.106,   3.000,   2.913,},
  { 7.444,   5.289,   4.416,   3.927,   3.611,   3.386,   3.218,   3.087,   2.981,   2.894,},
  { 7.419,   5.268,   4.396,   3.908,   3.592,   3.368,   3.200,   3.069,   2.963,   2.876,},
  { 7.396,   5.248,   4.377,   3.890,   3.574,   3.351,   3.183,   3.052,   2.946,   2.859,},
  { 7.373,   5.229,   4.360,   3.873,   3.558,   3.334,   3.167,   3.036,   2.930,   2.843,},
  { 7.353,   5.211,   4.343,   3.858,   3.542,   3.319,   3.152,   3.021,   2.915,   2.828,},
  { 7.333,   5.194,   4.327,   3.843,   3.528,   3.305,   3.137,   3.006,   2.901,   2.814,},
  { 7.314,   5.179,   4.313,   3.828,   3.514,   3.291,   3.124,   2.993,   2.888,   2.801,},
  { 7.296,   5.163,   4.299,   3.815,   3.501,   3.278,   3.111,   2.980,   2.875,   2.788,},
  { 7.280,   5.149,   4.285,   3.802,   3.488,   3.266,   3.099,   2.968,   2.863,   2.776,},
  { 7.264,   5.136,   4.273,   3.790,   3.476,   3.254,   3.087,   2.957,   2.851,   2.764,},
  { 7.248,   5.123,   4.261,   3.778,   3.465,   3.243,   3.076,   2.946,   2.840,   2.754,},
  { 7.234,   5.110,   4.249,   3.767,   3.454,   3.232,   3.066,   2.935,   2.830,   2.743,},
  { 7.220,   5.099,   4.238,   3.757,   3.444,   3.222,   3.056,   2.925,   2.820,   2.733,},
  { 7.207,   5.087,   4.228,   3.747,   3.434,   3.213,   3.046,   2.916,   2.811,   2.724,},
  { 7.194,   5.077,   4.218,   3.737,   3.425,   3.204,   3.037,   2.907,   2.802,   2.715,},
  { 7.182,   5.066,   4.208,   3.728,   3.416,   3.195,   3.028,   2.898,   2.793,   2.706,},
  { 7.171,   5.057,   4.199,   3.720,   3.408,   3.186,   3.020,   2.890,   2.785,   2.698,},
  { 7.159,   5.047,   4.191,   3.711,   3.400,   3.178,   3.012,   2.882,   2.777,   2.690,},
  { 7.149,   5.038,   4.182,   3.703,   3.392,   3.171,   3.005,   2.874,   2.769,   2.683,},
  { 7.139,   5.030,   4.174,   3.695,   3.384,   3.163,   2.997,   2.867,   2.762,   2.675,},
  { 7.129,   5.021,   4.167,   3.688,   3.377,   3.156,   2.990,   2.860,   2.755,   2.668,},
  { 7.119,   5.013,   4.159,   3.681,   3.370,   3.149,   2.983,   2.853,   2.748,   2.662,},
  { 7.110,   5.006,   4.152,   3.674,   3.363,   3.143,   2.977,   2.847,   2.742,   2.655,},
  { 7.102,   4.998,   4.145,   3.667,   3.357,   3.136,   2.971,   2.841,   2.736,   2.649,},
  { 7.093,   4.991,   4.138,   3.661,   3.351,   3.130,   2.965,   2.835,   2.730,   2.643,},
  { 7.085,   4.984,   4.132,   3.655,   3.345,   3.124,   2.959,   2.829,   2.724,   2.637,},
  { 7.077,   4.977,   4.126,   3.649,   3.339,   3.119,   2.953,   2.823,   2.718,   2.632,},
  { 7.070,   4.971,   4.120,   3.643,   3.333,   3.113,   2.948,   2.818,   2.713,   2.626,},
  { 7.062,   4.965,   4.114,   3.638,   3.328,   3.108,   2.942,   2.813,   2.708,   2.621,},
  { 7.055,   4.959,   4.109,   3.632,   3.323,   3.103,   2.937,   2.808,   2.703,   2.616,},
  { 7.048,   4.953,   4.103,   3.627,   3.318,   3.098,   2.932,   2.803,   2.698,   2.611,},
  { 7.042,   4.947,   4.098,   3.622,   3.313,   3.093,   2.928,   2.798,   2.693,   2.607,},
  { 7.035,   4.942,   4.093,   3.618,   3.308,   3.088,   2.923,   2.793,   2.689,   2.602,},
  { 7.029,   4.937,   4.088,   3.613,   3.304,   3.084,   2.919,   2.789,   2.684,   2.598,},
  { 7.023,   4.932,   4.083,   3.608,   3.299,   3.080,   2.914,   2.785,   2.680,   2.593,},
  { 7.017,   4.927,   4.079,   3.604,   3.295,   3.075,   2.910,   2.781,   2.676,   2.589,},
  { 7.011,   4.922,   4.074,   3.600,   3.291,   3.071,   2.906,   2.777,   2.672,   2.585,},
  { 7.006,   4.917,   4.070,   3.596,   3.287,   3.067,   2.902,   2.773,   2.668,   2.581,},
  { 7.001,   4.913,   4.066,   3.591,   3.283,   3.063,   2.898,   2.769,   2.664,   2.578,},
  { 6.995,   4.908,   4.062,   3.588,   3.279,   3.060,   2.895,   2.765,   2.660,   2.574,},
  { 6.990,   4.904,   4.058,   3.584,   3.275,   3.056,   2.891,   2.762,   2.657,   2.570,},
  { 6.985,   4.900,   4.054,   3.580,   3.272,   3.052,   2.887,   2.758,   2.653,   2.567,},
  { 6.981,   4.896,   4.050,   3.577,   3.268,   3.049,   2.884,   2.755,   2.650,   2.563,},
  { 6.976,   4.892,   4.047,   3.573,   3.265,   3.046,   2.881,   2.751,   2.647,   2.560,},
  { 6.971,   4.888,   4.043,   3.570,   3.261,   3.042,   2.877,   2.748,   2.644,   2.557,},
  { 6.967,   4.884,   4.040,   3.566,   3.258,   3.039,   2.874,   2.745,   2.640,   2.554,},
  { 6.963,   4.881,   4.036,   3.563,   3.255,   3.036,   2.871,   2.742,   2.637,   2.551,},
  { 6.958,   4.877,   4.033,   3.560,   3.252,   3.033,   2.868,   2.739,   2.634,   2.548,},
  { 6.954,   4.874,   4.030,   3.557,   3.249,   3.030,   2.865,   2.736,   2.632,   2.545,},
  { 6.950,   4.870,   4.027,   3.554,   3.246,   3.027,   2.863,   2.733,   2.629,   2.542,},
  { 6.947,   4.867,   4.024,   3.551,   3.243,   3.025,   2.860,   2.731,   2.626,   2.539,},
  { 6.943,   4.864,   4.021,   3.548,   3.240,   3.022,   2.857,   2.728,   2.623,   2.537,},
  { 6.939,   4.861,   4.018,   3.545,   3.238,   3.019,   2.854,   2.725,   2.621,   2.534,},
  { 6.935,   4.858,   4.015,   3.543,   3.235,   3.017,   2.852,   2.723,   2.618,   2.532,},
  { 6.932,   4.855,   4.012,   3.540,   3.233,   3.014,   2.849,   2.720,   2.616,   2.529,},
  { 6.928,   4.852,   4.010,   3.538,   3.230,   3.012,   2.847,   2.718,   2.613,   2.527,},
  { 6.925,   4.849,   4.007,   3.535,   3.228,   3.009,   2.845,   2.715,   2.611,   2.524,},
  { 6.922,   4.846,   4.004,   3.533,   3.225,   3.007,   2.842,   2.713,   2.609,   2.522,},
  { 6.919,   4.844,   4.002,   3.530,   3.223,   3.004,   2.840,   2.711,   2.606,   2.520,},
  { 6.915,   4.841,   3.999,   3.528,   3.221,   3.002,   2.838,   2.709,   2.604,   2.518,},
  { 6.912,   4.838,   3.997,   3.525,   3.218,   3.000,   2.835,   2.706,   2.602,   2.515,},
  { 6.909,   4.836,   3.995,   3.523,   3.216,   2.998,   2.833,   2.704,   2.600,   2.513,},
  { 6.906,   4.833,   3.992,   3.521,   3.214,   2.996,   2.831,   2.702,   2.598,   2.511,},
  { 6.904,   4.831,   3.990,   3.519,   3.212,   2.994,   2.829,   2.700,   2.596,   2.509,},
  { 6.901,   4.829,   3.988,   3.517,   3.210,   2.992,   2.827,   2.698,   2.594,   2.507,},
  { 6.898,   4.826,   3.986,   3.515,   3.208,   2.990,   2.825,   2.696,   2.592,   2.505,},
  { 6.895,   4.824,   3.984,   3.513,   3.206,   2.988,   2.823,   2.694,   2.590,   2.503}
};

/* define the variance which will be used as a minimum variance for any
  dimension of any feature. Since most features are calculated from numbers
  with a precision no better than 1 in 128, the variance should never be
  less than the square of this number for parameters whose range is 1. */
#define MINVARIANCE     0.0004

/* define the absolute minimum number of samples which must be present in
  order to accurately test hypotheses about underlying probability
  distributions.  Define separately the minimum samples that are needed
  before a statistical analysis is attempted; this number should be
  equal to MINSAMPLES but can be set to a lower number for early testing
  when very few samples are available. */
#define MINSAMPLESPERBUCKET 5
#define MINSAMPLES    (MINBUCKETS * MINSAMPLESPERBUCKET)
#define MINSAMPLESNEEDED  1

/* define the size of the table which maps normalized samples to
  histogram buckets.  Also define the number of standard deviations
  in a normal distribution which are considered to be significant.
  The mapping table will be defined in such a way that it covers
  the specified number of standard deviations on either side of
  the mean.  BUCKETTABLESIZE should always be even. */
#define BUCKETTABLESIZE   1024
#define NORMALEXTENT    3.0

struct TEMPCLUSTER {
  CLUSTER *Cluster;
  CLUSTER *Neighbor;
};

typedef tesseract::KDPairInc<float, TEMPCLUSTER*> ClusterPair;
typedef tesseract::GenericHeap<ClusterPair> ClusterHeap;

struct STATISTICS {
  FLOAT32 AvgVariance;
  FLOAT32 *CoVariance;
  FLOAT32 *Min;                  // largest negative distance from the mean
  FLOAT32 *Max;                  // largest positive distance from the mean
};

struct BUCKETS {
  DISTRIBUTION Distribution;     // distribution being tested for
  uinT32 SampleCount;            // # of samples in histogram
  FLOAT64 Confidence;            // confidence level of test
  FLOAT64 ChiSquared;            // test threshold
  uinT16 NumberOfBuckets;        // number of cells in histogram
  uinT16 Bucket[BUCKETTABLESIZE];// mapping to histogram buckets
  uinT32 *Count;                 // frequency of occurence histogram
  FLOAT32 *ExpectedCount;        // expected histogram
};

struct CHISTRUCT{
  uinT16 DegreesOfFreedom;
  FLOAT64 Alpha;
  FLOAT64 ChiSquared;
};

// For use with KDWalk / MakePotentialClusters
struct ClusteringContext {
  ClusterHeap *heap;  // heap used to hold temp clusters, "best" on top
  TEMPCLUSTER *candidates;  // array of potential clusters
  KDTREE *tree;  // kd-tree to be searched for neighbors
  inT32 next;  // next candidate to be used
};

typedef FLOAT64 (*DENSITYFUNC) (inT32);
typedef FLOAT64 (*SOLVEFUNC) (CHISTRUCT *, double);

#define Odd(N) ((N)%2)
#define Mirror(N,R) ((R) - (N) - 1)
#define Abs(N) ( ( (N) < 0 ) ? ( -(N) ) : (N) )

//--------------Global Data Definitions and Declarations----------------------
/* the following variables describe a discrete normal distribution
  which is used by NormalDensity() and NormalBucket().  The
  constant NORMALEXTENT determines how many standard
  deviations of the distribution are mapped onto the fixed
  discrete range of x.  x=0 is mapped to -NORMALEXTENT standard
  deviations and x=BUCKETTABLESIZE is mapped to
  +NORMALEXTENT standard deviations. */
#define SqrtOf2Pi     2.506628275
static const FLOAT64 kNormalStdDev = BUCKETTABLESIZE / (2.0 * NORMALEXTENT);
static const FLOAT64 kNormalVariance =
    (BUCKETTABLESIZE * BUCKETTABLESIZE) / (4.0 * NORMALEXTENT * NORMALEXTENT);
static const FLOAT64 kNormalMagnitude =
    (2.0 * NORMALEXTENT) / (SqrtOf2Pi * BUCKETTABLESIZE);
static const FLOAT64 kNormalMean = BUCKETTABLESIZE / 2;

/* define lookup tables used to compute the number of histogram buckets
  that should be used for a given number of samples. */
#define LOOKUPTABLESIZE   8
#define MAXDEGREESOFFREEDOM MAXBUCKETS

static const uinT32 kCountTable[LOOKUPTABLESIZE] = {
  MINSAMPLES, 200, 400, 600, 800, 1000, 1500, 2000
};  // number of samples

static const uinT16 kBucketsTable[LOOKUPTABLESIZE] = {
  MINBUCKETS, 16, 20, 24, 27, 30, 35, MAXBUCKETS
};  // number of buckets

/*-------------------------------------------------------------------------
          Private Function Prototypes
--------------------------------------------------------------------------*/
void CreateClusterTree(CLUSTERER *Clusterer);

void MakePotentialClusters(ClusteringContext *context, CLUSTER *Cluster,
                           inT32 Level);

CLUSTER *FindNearestNeighbor(KDTREE *Tree,
                             CLUSTER *Cluster,
                             FLOAT32 *Distance);

CLUSTER *MakeNewCluster(CLUSTERER *Clusterer, TEMPCLUSTER *TempCluster);

inT32 MergeClusters (inT16 N,
register PARAM_DESC ParamDesc[],
register inT32 n1,
register inT32 n2,
register FLOAT32 m[],
register FLOAT32 m1[], register FLOAT32 m2[]);

void ComputePrototypes(CLUSTERER *Clusterer, CLUSTERCONFIG *Config);

PROTOTYPE *MakePrototype(CLUSTERER *Clusterer,
                         CLUSTERCONFIG *Config,
                         CLUSTER *Cluster);

PROTOTYPE *MakeDegenerateProto(uinT16 N,
                               CLUSTER *Cluster,
                               STATISTICS *Statistics,
                               PROTOSTYLE Style,
                               inT32 MinSamples);

PROTOTYPE *TestEllipticalProto(CLUSTERER *Clusterer,
                               CLUSTERCONFIG *Config,
                               CLUSTER *Cluster,
                               STATISTICS *Statistics);

PROTOTYPE *MakeSphericalProto(CLUSTERER *Clusterer,
                              CLUSTER *Cluster,
                              STATISTICS *Statistics,
                              BUCKETS *Buckets);

PROTOTYPE *MakeEllipticalProto(CLUSTERER *Clusterer,
                               CLUSTER *Cluster,
                               STATISTICS *Statistics,
                               BUCKETS *Buckets);

PROTOTYPE *MakeMixedProto(CLUSTERER *Clusterer,
                          CLUSTER *Cluster,
                          STATISTICS *Statistics,
                          BUCKETS *NormalBuckets,
                          FLOAT64 Confidence);

void MakeDimRandom(uinT16 i, PROTOTYPE *Proto, PARAM_DESC *ParamDesc);

void MakeDimUniform(uinT16 i, PROTOTYPE *Proto, STATISTICS *Statistics);

STATISTICS *ComputeStatistics (inT16 N,
PARAM_DESC ParamDesc[], CLUSTER * Cluster);

PROTOTYPE *NewSphericalProto(uinT16 N,
                             CLUSTER *Cluster,
                             STATISTICS *Statistics);

PROTOTYPE *NewEllipticalProto(inT16 N,
                              CLUSTER *Cluster,
                              STATISTICS *Statistics);

PROTOTYPE *NewMixedProto(inT16 N, CLUSTER *Cluster, STATISTICS *Statistics);

PROTOTYPE *NewSimpleProto(inT16 N, CLUSTER *Cluster);

BOOL8 Independent (PARAM_DESC ParamDesc[],
inT16 N, FLOAT32 * CoVariance, FLOAT32 Independence);

BUCKETS *GetBuckets(CLUSTERER* clusterer,
                    DISTRIBUTION Distribution,
                    uinT32 SampleCount,
                    FLOAT64 Confidence);

BUCKETS *MakeBuckets(DISTRIBUTION Distribution,
                     uinT32 SampleCount,
                     FLOAT64 Confidence);

uinT16 OptimumNumberOfBuckets(uinT32 SampleCount);

FLOAT64 ComputeChiSquared(uinT16 DegreesOfFreedom, FLOAT64 Alpha);

FLOAT64 NormalDensity(inT32 x);

FLOAT64 UniformDensity(inT32 x);

FLOAT64 Integral(FLOAT64 f1, FLOAT64 f2, FLOAT64 Dx);

void FillBuckets(BUCKETS *Buckets,
                 CLUSTER *Cluster,
                 uinT16 Dim,
                 PARAM_DESC *ParamDesc,
                 FLOAT32 Mean,
                 FLOAT32 StdDev);

uinT16 NormalBucket(PARAM_DESC *ParamDesc,
                    FLOAT32 x,
                    FLOAT32 Mean,
                    FLOAT32 StdDev);

uinT16 UniformBucket(PARAM_DESC *ParamDesc,
                     FLOAT32 x,
                     FLOAT32 Mean,
                     FLOAT32 StdDev);

BOOL8 DistributionOK(BUCKETS *Buckets);

void FreeStatistics(STATISTICS *Statistics);

void FreeBuckets(BUCKETS *Buckets);

void FreeCluster(CLUSTER *Cluster);

uinT16 DegreesOfFreedom(DISTRIBUTION Distribution, uinT16 HistogramBuckets);

int NumBucketsMatch(void *arg1,   // BUCKETS *Histogram,
                    void *arg2);  // uinT16 *DesiredNumberOfBuckets);

int ListEntryMatch(void *arg1, void *arg2);

void AdjustBuckets(BUCKETS *Buckets, uinT32 NewSampleCount);

void InitBuckets(BUCKETS *Buckets);

int AlphaMatch(void *arg1,   // CHISTRUCT *ChiStruct,
               void *arg2);  // CHISTRUCT *SearchKey);

CHISTRUCT *NewChiStruct(uinT16 DegreesOfFreedom, FLOAT64 Alpha);

FLOAT64 Solve(SOLVEFUNC Function,
              void *FunctionParams,
              FLOAT64 InitialGuess,
              FLOAT64 Accuracy);

FLOAT64 ChiArea(CHISTRUCT *ChiParams, FLOAT64 x);

BOOL8 MultipleCharSamples(CLUSTERER *Clusterer,
                          CLUSTER *Cluster,
                          FLOAT32 MaxIllegal);

double InvertMatrix(const float* input, int size, float* inv);

//--------------------------Public Code--------------------------------------
/** MakeClusterer **********************************************************
Parameters:	SampleSize	number of dimensions in feature space
      ParamDesc	description of each dimension
Operation:	This routine creates a new clusterer data structure,
      initializes it, and returns a pointer to it.
Return:		pointer to the new clusterer data structure
Exceptions:	None
History:	5/29/89, DSJ, Created.
****************************************************************************/
CLUSTERER *
MakeClusterer (inT16 SampleSize, const PARAM_DESC ParamDesc[]) {
  CLUSTERER *Clusterer;
  int i;

  // allocate main clusterer data structure and init simple fields
  Clusterer = (CLUSTERER *) Emalloc (sizeof (CLUSTERER));
  Clusterer->SampleSize = SampleSize;
  Clusterer->NumberOfSamples = 0;
  Clusterer->NumChar = 0;

  // init fields which will not be used initially
  Clusterer->Root = NULL;
  Clusterer->ProtoList = NIL_LIST;

  // maintain a copy of param descriptors in the clusterer data structure
  Clusterer->ParamDesc =
    (PARAM_DESC *) Emalloc (SampleSize * sizeof (PARAM_DESC));
  for (i = 0; i < SampleSize; i++) {
    Clusterer->ParamDesc[i].Circular = ParamDesc[i].Circular;
    Clusterer->ParamDesc[i].NonEssential = ParamDesc[i].NonEssential;
    Clusterer->ParamDesc[i].Min = ParamDesc[i].Min;
    Clusterer->ParamDesc[i].Max = ParamDesc[i].Max;
    Clusterer->ParamDesc[i].Range = ParamDesc[i].Max - ParamDesc[i].Min;
    Clusterer->ParamDesc[i].HalfRange = Clusterer->ParamDesc[i].Range / 2;
    Clusterer->ParamDesc[i].MidRange =
      (ParamDesc[i].Max + ParamDesc[i].Min) / 2;
  }

  // allocate a kd tree to hold the samples
  Clusterer->KDTree = MakeKDTree (SampleSize, ParamDesc);

  // Initialize cache of histogram buckets to minimize recomputing them.
  for (int d = 0; d < DISTRIBUTION_COUNT; ++d) {
    for (int c = 0; c < MAXBUCKETS + 1 - MINBUCKETS; ++c)
      Clusterer->bucket_cache[d][c] = NULL;
  }

  return Clusterer;
}                                // MakeClusterer


/** MakeSample ***********************************************************
Parameters:	Clusterer	clusterer data structure to add sample to
      Feature		feature to be added to clusterer
      CharID		unique ident. of char that sample came from
Operation:	This routine creates a new sample data structure to hold
      the specified feature.  This sample is added to the clusterer
      data structure (so that it knows which samples are to be
      clustered later), and a pointer to the sample is returned to
      the caller.
Return:		Pointer to the new sample data structure
Exceptions:	ALREADYCLUSTERED	MakeSample can't be called after
      ClusterSamples has been called
History:	5/29/89, DSJ, Created.
*****************************************************************************/
SAMPLE* MakeSample(CLUSTERER * Clusterer, const FLOAT32* Feature,
                   inT32 CharID) {
  SAMPLE *Sample;
  int i;

  // see if the samples have already been clustered - if so trap an error
  if (Clusterer->Root != NULL)
    DoError (ALREADYCLUSTERED,
      "Can't add samples after they have been clustered");

  // allocate the new sample and initialize it
  Sample = (SAMPLE *) Emalloc (sizeof (SAMPLE) +
    (Clusterer->SampleSize -
    1) * sizeof (FLOAT32));
  Sample->Clustered = FALSE;
  Sample->Prototype = FALSE;
  Sample->SampleCount = 1;
  Sample->Left = NULL;
  Sample->Right = NULL;
  Sample->CharID = CharID;

  for (i = 0; i < Clusterer->SampleSize; i++)
    Sample->Mean[i] = Feature[i];

  // add the sample to the KD tree - keep track of the total # of samples
  Clusterer->NumberOfSamples++;
  KDStore (Clusterer->KDTree, Sample->Mean, (char *) Sample);
  if (CharID >= Clusterer->NumChar)
    Clusterer->NumChar = CharID + 1;

  // execute hook for monitoring clustering operation
  // (*SampleCreationHook)( Sample );

  return (Sample);
}                                // MakeSample


/** ClusterSamples ***********************************************************
Parameters:	Clusterer	data struct containing samples to be clustered
      Config		parameters which control clustering process
Operation:	This routine first checks to see if the samples in this
      clusterer have already been clustered before; if so, it does
      not bother to recreate the cluster tree.  It simply recomputes
      the prototypes based on the new Config info.
        If the samples have not been clustered before, the
      samples in the KD tree are formed into a cluster tree and then
      the prototypes are computed from the cluster tree.
        In either case this routine returns a pointer to a
      list of prototypes that best represent the samples given
      the constraints specified in Config.
Return:		Pointer to a list of prototypes
Exceptions:	None
History:	5/29/89, DSJ, Created.
*******************************************************************************/
LIST ClusterSamples(CLUSTERER *Clusterer, CLUSTERCONFIG *Config) {
  //only create cluster tree if samples have never been clustered before
  if (Clusterer->Root == NULL)
    CreateClusterTree(Clusterer);

  //deallocate the old prototype list if one exists
  FreeProtoList (&Clusterer->ProtoList);
  Clusterer->ProtoList = NIL_LIST;

  //compute prototypes starting at the root node in the tree
  ComputePrototypes(Clusterer, Config);
  return (Clusterer->ProtoList);
}                                // ClusterSamples


/** FreeClusterer *************************************************************
Parameters:	Clusterer	pointer to data structure to be freed
Operation:	This routine frees all of the memory allocated to the
      specified data structure.  It will not, however, free
      the memory used by the prototype list.  The pointers to
      the clusters for each prototype in the list will be set
      to NULL to indicate that the cluster data structures no
      longer exist.  Any sample lists that have been obtained
      via calls to GetSamples are no longer valid.
Return:		None
Exceptions:	None
History:	6/6/89, DSJ, Created.
*******************************************************************************/
void FreeClusterer(CLUSTERER *Clusterer) {
  if (Clusterer != NULL) {
    memfree (Clusterer->ParamDesc);
    if (Clusterer->KDTree != NULL)
      FreeKDTree (Clusterer->KDTree);
    if (Clusterer->Root != NULL)
      FreeCluster (Clusterer->Root);
    // Free up all used buckets structures.
    for (int d = 0; d < DISTRIBUTION_COUNT; ++d) {
      for (int c = 0; c < MAXBUCKETS + 1 - MINBUCKETS; ++c)
        if (Clusterer->bucket_cache[d][c] != NULL)
          FreeBuckets(Clusterer->bucket_cache[d][c]);
    }

    memfree(Clusterer);
  }
}                                // FreeClusterer


/** FreeProtoList ************************************************************
Parameters:	ProtoList	pointer to list of prototypes to be freed
Operation:	This routine frees all of the memory allocated to the
      specified list of prototypes.  The clusters which are
      pointed to by the prototypes are not freed.
Return:		None
Exceptions:	None
History:	6/6/89, DSJ, Created.
*****************************************************************************/
void FreeProtoList(LIST *ProtoList) {
  destroy_nodes(*ProtoList, FreePrototype);
}                                // FreeProtoList


/** FreePrototype ************************************************************
Parameters:	Prototype	prototype data structure to be deallocated
Operation:	This routine deallocates the memory consumed by the specified
      prototype and modifies the corresponding cluster so that it
      is no longer marked as a prototype.  The cluster is NOT
      deallocated by this routine.
Return:		None
Exceptions:	None
History:	5/30/89, DSJ, Created.
*******************************************************************************/
void FreePrototype(void *arg) {  //PROTOTYPE     *Prototype)
  PROTOTYPE *Prototype = (PROTOTYPE *) arg;

  // unmark the corresponding cluster (if there is one
  if (Prototype->Cluster != NULL)
    Prototype->Cluster->Prototype = FALSE;

  // deallocate the prototype statistics and then the prototype itself
  if (Prototype->Distrib != NULL)
    memfree (Prototype->Distrib);
  if (Prototype->Mean != NULL)
    memfree (Prototype->Mean);
  if (Prototype->Style != spherical) {
    if (Prototype->Variance.Elliptical != NULL)
      memfree (Prototype->Variance.Elliptical);
    if (Prototype->Magnitude.Elliptical != NULL)
      memfree (Prototype->Magnitude.Elliptical);
    if (Prototype->Weight.Elliptical != NULL)
      memfree (Prototype->Weight.Elliptical);
  }
  memfree(Prototype);
}                                // FreePrototype


/** NextSample ************************************************************
Parameters:	SearchState	ptr to list containing clusters to be searched
Operation:	This routine is used to find all of the samples which
      belong to a cluster.  It starts by removing the top
      cluster on the cluster list (SearchState).  If this cluster is
      a leaf it is returned.  Otherwise, the right subcluster
      is pushed on the list and we continue the search in the
      left subcluster.  This continues until a leaf is found.
      If all samples have been found, NULL is returned.
      InitSampleSearch() must be called
      before NextSample() to initialize the search.
Return:		Pointer to the next leaf cluster (sample) or NULL.
Exceptions:	None
History:	6/16/89, DSJ, Created.
****************************************************************************/
CLUSTER *NextSample(LIST *SearchState) {
  CLUSTER *Cluster;

  if (*SearchState == NIL_LIST)
    return (NULL);
  Cluster = (CLUSTER *) first_node (*SearchState);
  *SearchState = pop (*SearchState);
  while (TRUE) {
    if (Cluster->Left == NULL)
      return (Cluster);
    *SearchState = push (*SearchState, Cluster->Right);
    Cluster = Cluster->Left;
  }
}                                // NextSample


/** Mean ***********************************************************
Parameters:	Proto		prototype to return mean of
      Dimension	dimension whose mean is to be returned
Operation:	This routine returns the mean of the specified
      prototype in the indicated dimension.
Return:		Mean of Prototype in Dimension
Exceptions: none
History:	7/6/89, DSJ, Created.
*********************************************************************/
FLOAT32 Mean(PROTOTYPE *Proto, uinT16 Dimension) {
  return (Proto->Mean[Dimension]);
}                                // Mean


/** StandardDeviation *************************************************
Parameters:	Proto		prototype to return standard deviation of
      Dimension	dimension whose stddev is to be returned
Operation:	This routine returns the standard deviation of the
      prototype in the indicated dimension.
Return:		Standard deviation of Prototype in Dimension
Exceptions: none
History:	7/6/89, DSJ, Created.
**********************************************************************/
FLOAT32 StandardDeviation(PROTOTYPE *Proto, uinT16 Dimension) {
  switch (Proto->Style) {
    case spherical:
      return ((FLOAT32) sqrt ((double) Proto->Variance.Spherical));
    case elliptical:
      return ((FLOAT32)
        sqrt ((double) Proto->Variance.Elliptical[Dimension]));
    case mixed:
      switch (Proto->Distrib[Dimension]) {
        case normal:
          return ((FLOAT32)
            sqrt ((double) Proto->Variance.Elliptical[Dimension]));
        case uniform:
        case D_random:
          return (Proto->Variance.Elliptical[Dimension]);
        case DISTRIBUTION_COUNT:
          ASSERT_HOST(!"Distribution count not allowed!");
      }
  }
  return 0.0f;
}                                // StandardDeviation


/*---------------------------------------------------------------------------
            Private Code
----------------------------------------------------------------------------*/
/** CreateClusterTree *******************************************************
Parameters:	Clusterer	data structure holdings samples to be clustered
Operation:	This routine performs a bottoms-up clustering on the samples
      held in the kd-tree of the Clusterer data structure.  The
      result is a cluster tree.  Each node in the tree represents
      a cluster which conceptually contains a subset of the samples.
      More precisely, the cluster contains all of the samples which
      are contained in its two sub-clusters.  The leaves of the
      tree are the individual samples themselves; they have no
      sub-clusters.  The root node of the tree conceptually contains
      all of the samples.
Return:		None (the Clusterer data structure is changed)
Exceptions:	None
History:	5/29/89, DSJ, Created.
******************************************************************************/
void CreateClusterTree(CLUSTERER *Clusterer) {
  ClusteringContext context;
  ClusterPair HeapEntry;
  TEMPCLUSTER *PotentialCluster;

  // each sample and its nearest neighbor form a "potential" cluster
  // save these in a heap with the "best" potential clusters on top
  context.tree = Clusterer->KDTree;
  context.candidates = (TEMPCLUSTER *)
    Emalloc(Clusterer->NumberOfSamples * sizeof(TEMPCLUSTER));
  context.next = 0;
  context.heap = new ClusterHeap(Clusterer->NumberOfSamples);
  KDWalk(context.tree, (void_proc)MakePotentialClusters, &context);

  // form potential clusters into actual clusters - always do "best" first
  while (context.heap->Pop(&HeapEntry)) {
    PotentialCluster = HeapEntry.data;

    // if main cluster of potential cluster is already in another cluster
    // then we don't need to worry about it
    if (PotentialCluster->Cluster->Clustered) {
      continue;
    }

    // if main cluster is not yet clustered, but its nearest neighbor is
    // then we must find a new nearest neighbor
    else if (PotentialCluster->Neighbor->Clustered) {
      PotentialCluster->Neighbor =
        FindNearestNeighbor(context.tree, PotentialCluster->Cluster,
                            &HeapEntry.key);
      if (PotentialCluster->Neighbor != NULL) {
        context.heap->Push(&HeapEntry);
      }
    }

    // if neither cluster is already clustered, form permanent cluster
    else {
      PotentialCluster->Cluster =
          MakeNewCluster(Clusterer, PotentialCluster);
      PotentialCluster->Neighbor =
          FindNearestNeighbor(context.tree, PotentialCluster->Cluster,
                              &HeapEntry.key);
      if (PotentialCluster->Neighbor != NULL) {
        context.heap->Push(&HeapEntry);
      }
    }
  }

  // the root node in the cluster tree is now the only node in the kd-tree
  Clusterer->Root = (CLUSTER *) RootOf(Clusterer->KDTree);

  // free up the memory used by the K-D tree, heap, and temp clusters
  FreeKDTree(context.tree);
  Clusterer->KDTree = NULL;
  delete context.heap;
  memfree(context.candidates);
}                                // CreateClusterTree


/** MakePotentialClusters **************************************************
  Parameters:
      context  ClusteringContext (see definition above)
      Cluster  current cluster being visited in kd-tree walk
      Level  level of this cluster in the kd-tree
  Operation:
      This routine is designed to be used in concert with the
      KDWalk routine.  It will create a potential cluster for
      each sample in the kd-tree that is being walked.  This
      potential cluster will then be pushed on the heap.
******************************************************************************/
void MakePotentialClusters(ClusteringContext *context,
                           CLUSTER *Cluster, inT32 Level) {
  ClusterPair HeapEntry;
  int next = context->next;
  context->candidates[next].Cluster = Cluster;
  HeapEntry.data = &(context->candidates[next]);
  context->candidates[next].Neighbor =
      FindNearestNeighbor(context->tree,
                          context->candidates[next].Cluster,
                          &HeapEntry.key);
  if (context->candidates[next].Neighbor != NULL) {
    context->heap->Push(&HeapEntry);
    context->next++;
  }
}                                // MakePotentialClusters


/** FindNearestNeighbor *********************************************************
Parameters:	Tree		kd-tree to search in for nearest neighbor
      Cluster		cluster whose nearest neighbor is to be found
      Distance	ptr to variable to report distance found
Operation:	This routine searches the specified kd-tree for the nearest
      neighbor of the specified cluster.  It actually uses the
      kd routines to find the 2 nearest neighbors since one of them
      will be the original cluster.  A pointer to the nearest
      neighbor is returned, if it can be found, otherwise NULL is
      returned.  The distance between the 2 nodes is placed
      in the specified variable.
Return:		Pointer to the nearest neighbor of Cluster, or NULL
Exceptions: none
History:	5/29/89, DSJ, Created.
      7/13/89, DSJ, Removed visibility of kd-tree node data struct
********************************************************************************/
CLUSTER *
FindNearestNeighbor(KDTREE * Tree, CLUSTER * Cluster, FLOAT32 * Distance)
#define MAXNEIGHBORS  2
#define MAXDISTANCE   MAX_FLOAT32
{
  CLUSTER *Neighbor[MAXNEIGHBORS];
  FLOAT32 Dist[MAXNEIGHBORS];
  int NumberOfNeighbors;
  inT32 i;
  CLUSTER *BestNeighbor;

  // find the 2 nearest neighbors of the cluster
  KDNearestNeighborSearch(Tree, Cluster->Mean, MAXNEIGHBORS, MAXDISTANCE,
                          &NumberOfNeighbors, (void **)Neighbor, Dist);

  // search for the nearest neighbor that is not the cluster itself
  *Distance = MAXDISTANCE;
  BestNeighbor = NULL;
  for (i = 0; i < NumberOfNeighbors; i++) {
    if ((Dist[i] < *Distance) && (Neighbor[i] != Cluster)) {
      *Distance = Dist[i];
      BestNeighbor = Neighbor[i];
    }
  }
  return BestNeighbor;
}                                // FindNearestNeighbor


/** MakeNewCluster *************************************************************
Parameters:	Clusterer	current clustering environment
      TempCluster	potential cluster to make permanent
Operation:	This routine creates a new permanent cluster from the
      clusters specified in TempCluster.  The 2 clusters in
      TempCluster are marked as "clustered" and deleted from
      the kd-tree.  The new cluster is then added to the kd-tree.
      Return: Pointer to the new permanent cluster
Exceptions:	none
History:	5/29/89, DSJ, Created.
      7/13/89, DSJ, Removed visibility of kd-tree node data struct
********************************************************************************/
CLUSTER *MakeNewCluster(CLUSTERER *Clusterer, TEMPCLUSTER *TempCluster) {
  CLUSTER *Cluster;

  // allocate the new cluster and initialize it
  Cluster = (CLUSTER *) Emalloc(
      sizeof(CLUSTER) + (Clusterer->SampleSize - 1) * sizeof(FLOAT32));
  Cluster->Clustered = FALSE;
  Cluster->Prototype = FALSE;
  Cluster->Left = TempCluster->Cluster;
  Cluster->Right = TempCluster->Neighbor;
  Cluster->CharID = -1;

  // mark the old clusters as "clustered" and delete them from the kd-tree
  Cluster->Left->Clustered = TRUE;
  Cluster->Right->Clustered = TRUE;
  KDDelete(Clusterer->KDTree, Cluster->Left->Mean, Cluster->Left);
  KDDelete(Clusterer->KDTree, Cluster->Right->Mean, Cluster->Right);

  // compute the mean and sample count for the new cluster
  Cluster->SampleCount =
      MergeClusters(Clusterer->SampleSize, Clusterer->ParamDesc,
                    Cluster->Left->SampleCount, Cluster->Right->SampleCount,
                    Cluster->Mean, Cluster->Left->Mean, Cluster->Right->Mean);

  // add the new cluster to the KD tree
  KDStore(Clusterer->KDTree, Cluster->Mean, Cluster);
  return Cluster;
}                                // MakeNewCluster


/** MergeClusters ************************************************************
Parameters:	N	# of dimensions (size of arrays)
      ParamDesc	array of dimension descriptions
      n1, n2	number of samples in each old cluster
      m	array to hold mean of new cluster
      m1, m2	arrays containing means of old clusters
Operation:	This routine merges two clusters into one larger cluster.
      To do this it computes the number of samples in the new
      cluster and the mean of the new cluster.  The ParamDesc
      information is used to ensure that circular dimensions
      are handled correctly.
Return:		The number of samples in the new cluster.
Exceptions:	None
History:	5/31/89, DSJ, Created.
*********************************************************************************/
inT32 MergeClusters(inT16 N,
                    PARAM_DESC ParamDesc[],
                    inT32 n1,
                    inT32 n2,
                    FLOAT32 m[],
                    FLOAT32 m1[], FLOAT32 m2[]) {
  inT32 i, n;

  n = n1 + n2;
  for (i = N; i > 0; i--, ParamDesc++, m++, m1++, m2++) {
    if (ParamDesc->Circular) {
      // if distance between means is greater than allowed
      // reduce upper point by one "rotation" to compute mean
      // then normalize the mean back into the accepted range
      if ((*m2 - *m1) > ParamDesc->HalfRange) {
        *m = (n1 * *m1 + n2 * (*m2 - ParamDesc->Range)) / n;
        if (*m < ParamDesc->Min)
          *m += ParamDesc->Range;
      }
      else if ((*m1 - *m2) > ParamDesc->HalfRange) {
        *m = (n1 * (*m1 - ParamDesc->Range) + n2 * *m2) / n;
        if (*m < ParamDesc->Min)
          *m += ParamDesc->Range;
      }
      else
        *m = (n1 * *m1 + n2 * *m2) / n;
    }
    else
      *m = (n1 * *m1 + n2 * *m2) / n;
  }
  return n;
}                                // MergeClusters


/** ComputePrototypes *******************************************************
Parameters:	Clusterer	data structure holding cluster tree
      Config		parameters used to control prototype generation
Operation:	This routine decides which clusters in the cluster tree
      should be represented by prototypes, forms a list of these
      prototypes, and places the list in the Clusterer data
      structure.
Return:		None
Exceptions:	None
History:	5/30/89, DSJ, Created.
*******************************************************************************/
void ComputePrototypes(CLUSTERER *Clusterer, CLUSTERCONFIG *Config) {
  LIST ClusterStack = NIL_LIST;
  CLUSTER *Cluster;
  PROTOTYPE *Prototype;

  // use a stack to keep track of clusters waiting to be processed
  // initially the only cluster on the stack is the root cluster
  if (Clusterer->Root != NULL)
    ClusterStack = push (NIL_LIST, Clusterer->Root);

  // loop until we have analyzed all clusters which are potential prototypes
  while (ClusterStack != NIL_LIST) {
    // remove the next cluster to be analyzed from the stack
    // try to make a prototype from the cluster
    // if successful, put it on the proto list, else split the cluster
    Cluster = (CLUSTER *) first_node (ClusterStack);
    ClusterStack = pop (ClusterStack);
    Prototype = MakePrototype(Clusterer, Config, Cluster);
    if (Prototype != NULL) {
      Clusterer->ProtoList = push (Clusterer->ProtoList, Prototype);
    }
    else {
      ClusterStack = push (ClusterStack, Cluster->Right);
      ClusterStack = push (ClusterStack, Cluster->Left);
    }
  }
}                                // ComputePrototypes


/** MakePrototype ***********************************************************
Parameters:
      Clusterer	data structure holding cluster tree
      Config		parameters used to control prototype generation
      Cluster		cluster to be made into a prototype
Operation:	This routine attempts to create a prototype from the
      specified cluster that conforms to the distribution
      specified in Config.  If there are too few samples in the
      cluster to perform a statistical analysis, then a prototype
      is generated but labelled as insignificant.  If the
      dimensions of the cluster are not independent, no prototype
      is generated and NULL is returned.  If a prototype can be
      found that matches the desired distribution then a pointer
      to it is returned, otherwise NULL is returned.
Return:		Pointer to new prototype or NULL
Exceptions:	None
History:	6/19/89, DSJ, Created.
*******************************************************************************/
PROTOTYPE *MakePrototype(CLUSTERER *Clusterer,
                         CLUSTERCONFIG *Config,
                         CLUSTER *Cluster) {
  STATISTICS *Statistics;
  PROTOTYPE *Proto;
  BUCKETS *Buckets;

  // filter out clusters which contain samples from the same character
  if (MultipleCharSamples (Clusterer, Cluster, Config->MaxIllegal))
    return NULL;

  // compute the covariance matrix and ranges for the cluster
  Statistics =
      ComputeStatistics(Clusterer->SampleSize, Clusterer->ParamDesc, Cluster);

  // check for degenerate clusters which need not be analyzed further
  // note that the MinSamples test assumes that all clusters with multiple
  // character samples have been removed (as above)
  Proto = MakeDegenerateProto(
      Clusterer->SampleSize, Cluster, Statistics, Config->ProtoStyle,
      (inT32) (Config->MinSamples * Clusterer->NumChar));
  if (Proto != NULL) {
    FreeStatistics(Statistics);
    return Proto;
  }
  // check to ensure that all dimensions are independent
  if (!Independent(Clusterer->ParamDesc, Clusterer->SampleSize,
                   Statistics->CoVariance, Config->Independence)) {
    FreeStatistics(Statistics);
    return NULL;
  }

  if (HOTELLING && Config->ProtoStyle == elliptical) {
    Proto = TestEllipticalProto(Clusterer, Config, Cluster, Statistics);
    if (Proto != NULL) {
      FreeStatistics(Statistics);
      return Proto;
    }
  }

  // create a histogram data structure used to evaluate distributions
  Buckets = GetBuckets(Clusterer, normal, Cluster->SampleCount,
                       Config->Confidence);

  // create a prototype based on the statistics and test it
  switch (Config->ProtoStyle) {
    case spherical:
      Proto = MakeSphericalProto(Clusterer, Cluster, Statistics, Buckets);
      break;
    case elliptical:
      Proto = MakeEllipticalProto(Clusterer, Cluster, Statistics, Buckets);
      break;
    case mixed:
      Proto = MakeMixedProto(Clusterer, Cluster, Statistics, Buckets,
                             Config->Confidence);
      break;
    case automatic:
      Proto = MakeSphericalProto(Clusterer, Cluster, Statistics, Buckets);
      if (Proto != NULL)
        break;
      Proto = MakeEllipticalProto(Clusterer, Cluster, Statistics, Buckets);
      if (Proto != NULL)
        break;
      Proto = MakeMixedProto(Clusterer, Cluster, Statistics, Buckets,
                             Config->Confidence);
      break;
  }
  FreeStatistics(Statistics);
  return Proto;
}                                // MakePrototype


/** MakeDegenerateProto ******************************************************
Parameters:	N		number of dimensions
      Cluster		cluster being analyzed
      Statistics	statistical info about cluster
      Style		type of prototype to be generated
      MinSamples	minimum number of samples in a cluster
Operation:	This routine checks for clusters which are degenerate and
      therefore cannot be analyzed in a statistically valid way.
      A cluster is defined as degenerate if it does not have at
      least MINSAMPLESNEEDED samples in it.  If the cluster is
      found to be degenerate, a prototype of the specified style
      is generated and marked as insignificant.  A cluster is
      also degenerate if it does not have at least MinSamples
      samples in it.
      If the cluster is not degenerate, NULL is returned.
Return:		Pointer to degenerate prototype or NULL.
Exceptions:	None
History:	6/20/89, DSJ, Created.
      7/12/89, DSJ, Changed name and added check for 0 stddev.
      8/8/89, DSJ, Removed check for 0 stddev (handled elsewhere).
********************************************************************************/
PROTOTYPE *MakeDegenerateProto(  //this was MinSample
                               uinT16 N,
                               CLUSTER *Cluster,
                               STATISTICS *Statistics,
                               PROTOSTYLE Style,
                               inT32 MinSamples) {
  PROTOTYPE *Proto = NULL;

  if (MinSamples < MINSAMPLESNEEDED)
    MinSamples = MINSAMPLESNEEDED;

  if (Cluster->SampleCount < MinSamples) {
    switch (Style) {
      case spherical:
        Proto = NewSphericalProto (N, Cluster, Statistics);
        break;
      case elliptical:
      case automatic:
        Proto = NewEllipticalProto (N, Cluster, Statistics);
        break;
      case mixed:
        Proto = NewMixedProto (N, Cluster, Statistics);
        break;
    }
    Proto->Significant = FALSE;
  }
  return (Proto);
}                                // MakeDegenerateProto

/** TestEllipticalProto ****************************************************
Parameters:	Clusterer	data struct containing samples being clustered
      Config provides the magic number of samples that make a good cluster
      Cluster		cluster to be made into an elliptical prototype
      Statistics	statistical info about cluster
Operation:	This routine tests the specified cluster to see if **
*     there is a statistically significant difference between
*     the sub-clusters that would be made if the cluster were to
*     be split. If not, then a new prototype is formed and
*     returned to the caller. If there is, then NULL is returned
*     to the caller.
Return:		Pointer to new elliptical prototype or NULL.
****************************************************************************/
PROTOTYPE *TestEllipticalProto(CLUSTERER *Clusterer,
                               CLUSTERCONFIG *Config,
                               CLUSTER *Cluster,
                               STATISTICS *Statistics) {
  // Fraction of the number of samples used as a range around 1 within
  // which a cluster has the magic size that allows a boost to the
  // FTable by kFTableBoostMargin, thus allowing clusters near the
  // magic size (equal to the number of sample characters) to be more
  // likely to stay together.
  const double kMagicSampleMargin = 0.0625;
  const double kFTableBoostMargin = 2.0;

  int N = Clusterer->SampleSize;
  CLUSTER* Left = Cluster->Left;
  CLUSTER* Right = Cluster->Right;
  if (Left == NULL || Right == NULL)
    return NULL;
  int TotalDims = Left->SampleCount + Right->SampleCount;
  if (TotalDims < N + 1 || TotalDims < 2)
    return NULL;
  const int kMatrixSize = N * N * sizeof(FLOAT32);
  FLOAT32* Covariance = reinterpret_cast<FLOAT32 *>(Emalloc(kMatrixSize));
  FLOAT32* Inverse = reinterpret_cast<FLOAT32 *>(Emalloc(kMatrixSize));
  FLOAT32* Delta = reinterpret_cast<FLOAT32*>(Emalloc(N * sizeof(FLOAT32)));
  // Compute a new covariance matrix that only uses essential features.
  for (int i = 0; i < N; ++i) {
    int row_offset = i * N;
    if (!Clusterer->ParamDesc[i].NonEssential) {
      for (int j = 0; j < N; ++j) {
        if (!Clusterer->ParamDesc[j].NonEssential)
          Covariance[j + row_offset] = Statistics->CoVariance[j + row_offset];
        else
          Covariance[j + row_offset] = 0.0f;
      }
    } else {
      for (int j = 0; j < N; ++j) {
        if (i == j)
          Covariance[j + row_offset] = 1.0f;
        else
          Covariance[j + row_offset] = 0.0f;
      }
    }
  }
  double err = InvertMatrix(Covariance, N, Inverse);
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
      temp += Inverse[y + N*x] * Delta[y];
    }
    Tsq += Delta[x] * temp;
  }
  memfree(Covariance);
  memfree(Inverse);
  memfree(Delta);
  // Changed this function to match the formula in
  // Statistical Methods in Medical Research p 473
  // By Peter Armitage, Geoffrey Berry, J. N. S. Matthews.
  // Tsq *= Left->SampleCount * Right->SampleCount / TotalDims;
  double F = Tsq * (TotalDims - EssentialN - 1) / ((TotalDims - 2)*EssentialN);
  int Fx = EssentialN;
  if (Fx > FTABLE_X)
    Fx = FTABLE_X;
  --Fx;
  int Fy = TotalDims - EssentialN - 1;
  if (Fy > FTABLE_Y)
    Fy = FTABLE_Y;
  --Fy;
  double FTarget = FTable[Fy][Fx];
  if (Config->MagicSamples > 0 &&
      TotalDims >= Config->MagicSamples * (1.0 - kMagicSampleMargin) &&
      TotalDims <= Config->MagicSamples * (1.0 + kMagicSampleMargin)) {
    // Give magic-sized clusters a magic FTable boost.
    FTarget += kFTableBoostMargin;
  }
  if (F < FTarget) {
    return NewEllipticalProto (Clusterer->SampleSize, Cluster, Statistics);
  }
  return NULL;
}

/* MakeSphericalProto *******************************************************
Parameters:	Clusterer	data struct containing samples being clustered
      Cluster		cluster to be made into a spherical prototype
      Statistics	statistical info about cluster
      Buckets		histogram struct used to analyze distribution
Operation:	This routine tests the specified cluster to see if it can
      be approximated by a spherical normal distribution.  If it
      can be, then a new prototype is formed and returned to the
      caller.  If it can't be, then NULL is returned to the caller.
Return:		Pointer to new spherical prototype or NULL.
Exceptions:	None
History:	6/1/89, DSJ, Created.
******************************************************************************/
PROTOTYPE *MakeSphericalProto(CLUSTERER *Clusterer,
                              CLUSTER *Cluster,
                              STATISTICS *Statistics,
                              BUCKETS *Buckets) {
  PROTOTYPE *Proto = NULL;
  int i;

  // check that each dimension is a normal distribution
  for (i = 0; i < Clusterer->SampleSize; i++) {
    if (Clusterer->ParamDesc[i].NonEssential)
      continue;

    FillBuckets (Buckets, Cluster, i, &(Clusterer->ParamDesc[i]),
      Cluster->Mean[i],
      sqrt ((FLOAT64) (Statistics->AvgVariance)));
    if (!DistributionOK (Buckets))
      break;
  }
  // if all dimensions matched a normal distribution, make a proto
  if (i >= Clusterer->SampleSize)
    Proto = NewSphericalProto (Clusterer->SampleSize, Cluster, Statistics);
  return (Proto);
}                                // MakeSphericalProto


/** MakeEllipticalProto ****************************************************
Parameters:	Clusterer	data struct containing samples being clustered
      Cluster		cluster to be made into an elliptical prototype
      Statistics	statistical info about cluster
      Buckets		histogram struct used to analyze distribution
Operation:	This routine tests the specified cluster to see if it can
      be approximated by an elliptical normal distribution.  If it
      can be, then a new prototype is formed and returned to the
      caller.  If it can't be, then NULL is returned to the caller.
Return:		Pointer to new elliptical prototype or NULL.
Exceptions:	None
History:	6/12/89, DSJ, Created.
****************************************************************************/
PROTOTYPE *MakeEllipticalProto(CLUSTERER *Clusterer,
                               CLUSTER *Cluster,
                               STATISTICS *Statistics,
                               BUCKETS *Buckets) {
  PROTOTYPE *Proto = NULL;
  int i;

  // check that each dimension is a normal distribution
  for (i = 0; i < Clusterer->SampleSize; i++) {
    if (Clusterer->ParamDesc[i].NonEssential)
      continue;

    FillBuckets (Buckets, Cluster, i, &(Clusterer->ParamDesc[i]),
      Cluster->Mean[i],
      sqrt ((FLOAT64) Statistics->
      CoVariance[i * (Clusterer->SampleSize + 1)]));
    if (!DistributionOK (Buckets))
      break;
  }
  // if all dimensions matched a normal distribution, make a proto
  if (i >= Clusterer->SampleSize)
    Proto = NewEllipticalProto (Clusterer->SampleSize, Cluster, Statistics);
  return (Proto);
}                                // MakeEllipticalProto


/** MakeMixedProto ***********************************************************
Parameters:
      Clusterer	data struct containing samples being clustered
      Cluster		cluster to be made into a prototype
      Statistics	statistical info about cluster
      NormalBuckets	histogram struct used to analyze distribution
      Confidence	confidence level for alternate distributions
Operation:	This routine tests each dimension of the specified cluster to
      see what distribution would best approximate that dimension.
      Each dimension is compared to the following distributions
      in order: normal, random, uniform.  If each dimension can
      be represented by one of these distributions,
      then a new prototype is formed and returned to the
      caller.  If it can't be, then NULL is returned to the caller.
Return:		Pointer to new mixed prototype or NULL.
Exceptions:	None
History:	6/12/89, DSJ, Created.
********************************************************************************/
PROTOTYPE *MakeMixedProto(CLUSTERER *Clusterer,
                          CLUSTER *Cluster,
                          STATISTICS *Statistics,
                          BUCKETS *NormalBuckets,
                          FLOAT64 Confidence) {
  PROTOTYPE *Proto;
  int i;
  BUCKETS *UniformBuckets = NULL;
  BUCKETS *RandomBuckets = NULL;

  // create a mixed proto to work on - initially assume all dimensions normal*/
  Proto = NewMixedProto (Clusterer->SampleSize, Cluster, Statistics);

  // find the proper distribution for each dimension
  for (i = 0; i < Clusterer->SampleSize; i++) {
    if (Clusterer->ParamDesc[i].NonEssential)
      continue;

    FillBuckets (NormalBuckets, Cluster, i, &(Clusterer->ParamDesc[i]),
      Proto->Mean[i],
      sqrt ((FLOAT64) Proto->Variance.Elliptical[i]));
    if (DistributionOK (NormalBuckets))
      continue;

    if (RandomBuckets == NULL)
      RandomBuckets =
        GetBuckets(Clusterer, D_random, Cluster->SampleCount, Confidence);
    MakeDimRandom (i, Proto, &(Clusterer->ParamDesc[i]));
    FillBuckets (RandomBuckets, Cluster, i, &(Clusterer->ParamDesc[i]),
      Proto->Mean[i], Proto->Variance.Elliptical[i]);
    if (DistributionOK (RandomBuckets))
      continue;

    if (UniformBuckets == NULL)
      UniformBuckets =
        GetBuckets(Clusterer, uniform, Cluster->SampleCount, Confidence);
    MakeDimUniform(i, Proto, Statistics);
    FillBuckets (UniformBuckets, Cluster, i, &(Clusterer->ParamDesc[i]),
      Proto->Mean[i], Proto->Variance.Elliptical[i]);
    if (DistributionOK (UniformBuckets))
      continue;
    break;
  }
  // if any dimension failed to match a distribution, discard the proto
  if (i < Clusterer->SampleSize) {
    FreePrototype(Proto);
    Proto = NULL;
  }
  return (Proto);
}                                // MakeMixedProto


/* MakeDimRandom *************************************************************
Parameters:	i		index of dimension to be changed
      Proto		prototype whose dimension is to be altered
      ParamDesc	description of specified dimension
Operation:	This routine alters the ith dimension of the specified
      mixed prototype to be D_random.
Return:		None
Exceptions:	None
History:	6/20/89, DSJ, Created.
******************************************************************************/
void MakeDimRandom(uinT16 i, PROTOTYPE *Proto, PARAM_DESC *ParamDesc) {
  Proto->Distrib[i] = D_random;
  Proto->Mean[i] = ParamDesc->MidRange;
  Proto->Variance.Elliptical[i] = ParamDesc->HalfRange;

  // subtract out the previous magnitude of this dimension from the total
  Proto->TotalMagnitude /= Proto->Magnitude.Elliptical[i];
  Proto->Magnitude.Elliptical[i] = 1.0 / ParamDesc->Range;
  Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
  Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);

  // note that the proto Weight is irrelevant for D_random protos
}                                // MakeDimRandom


/** MakeDimUniform ***********************************************************
Parameters:	i		index of dimension to be changed
      Proto		prototype whose dimension is to be altered
      Statistics	statistical info about prototype
Operation:	This routine alters the ith dimension of the specified
      mixed prototype to be uniform.
Return:		None
Exceptions:	None
History:	6/20/89, DSJ, Created.
******************************************************************************/
void MakeDimUniform(uinT16 i, PROTOTYPE *Proto, STATISTICS *Statistics) {
  Proto->Distrib[i] = uniform;
  Proto->Mean[i] = Proto->Cluster->Mean[i] +
    (Statistics->Min[i] + Statistics->Max[i]) / 2;
  Proto->Variance.Elliptical[i] =
    (Statistics->Max[i] - Statistics->Min[i]) / 2;
  if (Proto->Variance.Elliptical[i] < MINVARIANCE)
    Proto->Variance.Elliptical[i] = MINVARIANCE;

  // subtract out the previous magnitude of this dimension from the total
  Proto->TotalMagnitude /= Proto->Magnitude.Elliptical[i];
  Proto->Magnitude.Elliptical[i] =
    1.0 / (2.0 * Proto->Variance.Elliptical[i]);
  Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
  Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);

  // note that the proto Weight is irrelevant for uniform protos
}                                // MakeDimUniform


/** ComputeStatistics *********************************************************
Parameters:	N		number of dimensions
      ParamDesc	array of dimension descriptions
      Cluster		cluster whose stats are to be computed
Operation:	This routine searches the cluster tree for all leaf nodes
      which are samples in the specified cluster.  It computes
      a full covariance matrix for these samples as well as
      keeping track of the ranges (min and max) for each
      dimension.  A special data structure is allocated to
      return this information to the caller.  An incremental
      algorithm for computing statistics is not used because
      it will not work with circular dimensions.
Return:		Pointer to new data structure containing statistics
Exceptions:	None
History:	6/2/89, DSJ, Created.
*********************************************************************************/
STATISTICS *
ComputeStatistics (inT16 N, PARAM_DESC ParamDesc[], CLUSTER * Cluster) {
  STATISTICS *Statistics;
  int i, j;
  FLOAT32 *CoVariance;
  FLOAT32 *Distance;
  LIST SearchState;
  SAMPLE *Sample;
  uinT32 SampleCountAdjustedForBias;

  // allocate memory to hold the statistics results
  Statistics = (STATISTICS *) Emalloc (sizeof (STATISTICS));
  Statistics->CoVariance = (FLOAT32 *) Emalloc (N * N * sizeof (FLOAT32));
  Statistics->Min = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
  Statistics->Max = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));

  // allocate temporary memory to hold the sample to mean distances
  Distance = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));

  // initialize the statistics
  Statistics->AvgVariance = 1.0;
  CoVariance = Statistics->CoVariance;
  for (i = 0; i < N; i++) {
    Statistics->Min[i] = 0.0;
    Statistics->Max[i] = 0.0;
    for (j = 0; j < N; j++, CoVariance++)
      *CoVariance = 0;
  }
  // find each sample in the cluster and merge it into the statistics
  InitSampleSearch(SearchState, Cluster);
  while ((Sample = NextSample (&SearchState)) != NULL) {
    for (i = 0; i < N; i++) {
      Distance[i] = Sample->Mean[i] - Cluster->Mean[i];
      if (ParamDesc[i].Circular) {
        if (Distance[i] > ParamDesc[i].HalfRange)
          Distance[i] -= ParamDesc[i].Range;
        if (Distance[i] < -ParamDesc[i].HalfRange)
          Distance[i] += ParamDesc[i].Range;
      }
      if (Distance[i] < Statistics->Min[i])
        Statistics->Min[i] = Distance[i];
      if (Distance[i] > Statistics->Max[i])
        Statistics->Max[i] = Distance[i];
    }
    CoVariance = Statistics->CoVariance;
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++, CoVariance++)
        *CoVariance += Distance[i] * Distance[j];
  }
  // normalize the variances by the total number of samples
  // use SampleCount-1 instead of SampleCount to get an unbiased estimate
  // also compute the geometic mean of the diagonal variances
  // ensure that clusters with only 1 sample are handled correctly
  if (Cluster->SampleCount > 1)
    SampleCountAdjustedForBias = Cluster->SampleCount - 1;
  else
    SampleCountAdjustedForBias = 1;
  CoVariance = Statistics->CoVariance;
  for (i = 0; i < N; i++)
  for (j = 0; j < N; j++, CoVariance++) {
    *CoVariance /= SampleCountAdjustedForBias;
    if (j == i) {
      if (*CoVariance < MINVARIANCE)
        *CoVariance = MINVARIANCE;
      Statistics->AvgVariance *= *CoVariance;
    }
  }
  Statistics->AvgVariance = (float)pow((double)Statistics->AvgVariance,
                                       1.0 / N);

  // release temporary memory and return
  memfree(Distance);
  return (Statistics);
}                                // ComputeStatistics


/** NewSpericalProto *********************************************************
Parameters:	N		number of dimensions
      Cluster		cluster to be made into a spherical prototype
      Statistics	statistical info about samples in cluster
Operation:	This routine creates a spherical prototype data structure to
      approximate the samples in the specified cluster.
      Spherical prototypes have a single variance which is
      common across all dimensions.  All dimensions are normally
      distributed and independent.
Return:		Pointer to a new spherical prototype data structure
Exceptions:	None
History:	6/19/89, DSJ, Created.
******************************************************************************/
PROTOTYPE *NewSphericalProto(uinT16 N,
                             CLUSTER *Cluster,
                             STATISTICS *Statistics) {
  PROTOTYPE *Proto;

  Proto = NewSimpleProto (N, Cluster);

  Proto->Variance.Spherical = Statistics->AvgVariance;
  if (Proto->Variance.Spherical < MINVARIANCE)
    Proto->Variance.Spherical = MINVARIANCE;

  Proto->Magnitude.Spherical =
    1.0 / sqrt ((double) (2.0 * PI * Proto->Variance.Spherical));
  Proto->TotalMagnitude = (float)pow((double)Proto->Magnitude.Spherical,
                                     (double) N);
  Proto->Weight.Spherical = 1.0 / Proto->Variance.Spherical;
  Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);

  return (Proto);
}                                // NewSphericalProto


/** NewEllipticalProto *******************************************************
Parameters:	N		number of dimensions
      Cluster		cluster to be made into an elliptical prototype
      Statistics	statistical info about samples in cluster
Operation:	This routine creates an elliptical prototype data structure to
      approximate the samples in the specified cluster.
      Elliptical prototypes have a variance for each dimension.
      All dimensions are normally distributed and independent.
Return:		Pointer to a new elliptical prototype data structure
Exceptions:	None
History:	6/19/89, DSJ, Created.
*******************************************************************************/
PROTOTYPE *NewEllipticalProto(inT16 N,
                              CLUSTER *Cluster,
                              STATISTICS *Statistics) {
  PROTOTYPE *Proto;
  FLOAT32 *CoVariance;
  int i;

  Proto = NewSimpleProto (N, Cluster);
  Proto->Variance.Elliptical = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
  Proto->Magnitude.Elliptical = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
  Proto->Weight.Elliptical = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));

  CoVariance = Statistics->CoVariance;
  Proto->TotalMagnitude = 1.0;
  for (i = 0; i < N; i++, CoVariance += N + 1) {
    Proto->Variance.Elliptical[i] = *CoVariance;
    if (Proto->Variance.Elliptical[i] < MINVARIANCE)
      Proto->Variance.Elliptical[i] = MINVARIANCE;

    Proto->Magnitude.Elliptical[i] =
      1.0 / sqrt ((double) (2.0 * PI * Proto->Variance.Elliptical[i]));
    Proto->Weight.Elliptical[i] = 1.0 / Proto->Variance.Elliptical[i];
    Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
  }
  Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);
  Proto->Style = elliptical;
  return (Proto);
}                                // NewEllipticalProto


/** MewMixedProto ************************************************************
Parameters:	N		number of dimensions
      Cluster		cluster to be made into a mixed prototype
      Statistics	statistical info about samples in cluster
Operation:	This routine creates a mixed prototype data structure to
      approximate the samples in the specified cluster.
      Mixed prototypes can have different distributions for
      each dimension.  All dimensions are independent.  The
      structure is initially filled in as though it were an
      elliptical prototype.  The actual distributions of the
      dimensions can be altered by other routines.
Return:		Pointer to a new mixed prototype data structure
Exceptions:	None
History:	6/19/89, DSJ, Created.
********************************************************************************/
PROTOTYPE *NewMixedProto(inT16 N, CLUSTER *Cluster, STATISTICS *Statistics) {
  PROTOTYPE *Proto;
  int i;

  Proto = NewEllipticalProto (N, Cluster, Statistics);
  Proto->Distrib = (DISTRIBUTION *) Emalloc (N * sizeof (DISTRIBUTION));

  for (i = 0; i < N; i++) {
    Proto->Distrib[i] = normal;
  }
  Proto->Style = mixed;
  return (Proto);
}                                // NewMixedProto


/** NewSimpleProto ***********************************************************
Parameters:	N		number of dimensions
      Cluster		cluster to be made into a prototype
Operation:	This routine allocates memory to hold a simple prototype
      data structure, i.e. one without independent distributions
      and variances for each dimension.
Return:		Pointer to new simple prototype
Exceptions:	None
History:	6/19/89, DSJ, Created.
*******************************************************************************/
PROTOTYPE *NewSimpleProto(inT16 N, CLUSTER *Cluster) {
  PROTOTYPE *Proto;
  int i;

  Proto = (PROTOTYPE *) Emalloc (sizeof (PROTOTYPE));
  Proto->Mean = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));

  for (i = 0; i < N; i++)
    Proto->Mean[i] = Cluster->Mean[i];
  Proto->Distrib = NULL;

  Proto->Significant = TRUE;
  Proto->Merged = FALSE;
  Proto->Style = spherical;
  Proto->NumSamples = Cluster->SampleCount;
  Proto->Cluster = Cluster;
  Proto->Cluster->Prototype = TRUE;
  return (Proto);
}                                // NewSimpleProto


/** Independent ***************************************************************
Parameters:	ParamDesc	descriptions of each feature space dimension
      N		number of dimensions
      CoVariance	ptr to a covariance matrix
      Independence	max off-diagonal correlation coefficient
Operation:	This routine returns TRUE if the specified covariance
      matrix indicates that all N dimensions are independent of
      one another.  One dimension is judged to be independent of
      another when the magnitude of the corresponding correlation
      coefficient is
      less than the specified Independence factor.  The
      correlation coefficient is calculated as: (see Duda and
      Hart, pg. 247)
      coeff[ij] = stddev[ij] / sqrt (stddev[ii] * stddev[jj])
      The covariance matrix is assumed to be symmetric (which
      should always be true).
Return:		TRUE if dimensions are independent, FALSE otherwise
Exceptions:	None
History:	6/4/89, DSJ, Created.
*******************************************************************************/
BOOL8
Independent (PARAM_DESC ParamDesc[],
inT16 N, FLOAT32 * CoVariance, FLOAT32 Independence) {
  int i, j;
  FLOAT32 *VARii;                // points to ith on-diagonal element
  FLOAT32 *VARjj;                // points to jth on-diagonal element
  FLOAT32 CorrelationCoeff;

  VARii = CoVariance;
  for (i = 0; i < N; i++, VARii += N + 1) {
    if (ParamDesc[i].NonEssential)
      continue;

    VARjj = VARii + N + 1;
    CoVariance = VARii + 1;
    for (j = i + 1; j < N; j++, CoVariance++, VARjj += N + 1) {
      if (ParamDesc[j].NonEssential)
        continue;

      if ((*VARii == 0.0) || (*VARjj == 0.0))
        CorrelationCoeff = 0.0;
      else
        CorrelationCoeff =
          sqrt (sqrt (*CoVariance * *CoVariance / (*VARii * *VARjj)));
      if (CorrelationCoeff > Independence)
        return (FALSE);
    }
  }
  return (TRUE);
}                                // Independent


/** GetBuckets **************************************************************
  Parameters:
      Clusterer  which keeps a bucket_cache for us.
      Distribution	type of probability distribution to test for
      SampleCount	number of samples that are available
      Confidence	probability of a Type I error
Operation:	This routine returns a histogram data structure which can
      be used by other routines to place samples into histogram
      buckets, and then apply a goodness of fit test to the
      histogram data to determine if the samples belong to the
      specified probability distribution.  The routine keeps
      a list of bucket data structures which have already been
      created so that it minimizes the computation time needed
      to create a new bucket.
Return:		Bucket data structure
Exceptions: none
History:	Thu Aug  3 12:58:10 1989, DSJ, Created.
*****************************************************************************/
BUCKETS *GetBuckets(CLUSTERER* clusterer,
                    DISTRIBUTION Distribution,
                    uinT32 SampleCount,
                    FLOAT64 Confidence) {
  // Get an old bucket structure with the same number of buckets.
  uinT16 NumberOfBuckets = OptimumNumberOfBuckets(SampleCount);
  BUCKETS *Buckets =
      clusterer->bucket_cache[Distribution][NumberOfBuckets - MINBUCKETS];

  // If a matching bucket structure is not found, make one and save it.
  if (Buckets == NULL) {
    Buckets = MakeBuckets(Distribution, SampleCount, Confidence);
    clusterer->bucket_cache[Distribution][NumberOfBuckets - MINBUCKETS] =
        Buckets;
  } else {
    // Just adjust the existing buckets.
    if (SampleCount != Buckets->SampleCount)
      AdjustBuckets(Buckets, SampleCount);
    if (Confidence != Buckets->Confidence) {
      Buckets->Confidence = Confidence;
      Buckets->ChiSquared = ComputeChiSquared(
          DegreesOfFreedom(Distribution, Buckets->NumberOfBuckets),
          Confidence);
    }
    InitBuckets(Buckets);
  }
  return Buckets;
}                                // GetBuckets


/** Makebuckets *************************************************************
Parameters:
      Distribution	type of probability distribution to test for
      SampleCount	number of samples that are available
      Confidence	probability of a Type I error
Operation:
      This routine creates a histogram data structure which can
      be used by other routines to place samples into histogram
      buckets, and then apply a goodness of fit test to the
      histogram data to determine if the samples belong to the
      specified probability distribution.  The buckets are
      allocated in such a way that the expected frequency of
      samples in each bucket is approximately the same.  In
      order to make this possible, a mapping table is
      computed which maps "normalized" samples into the
      appropriate bucket.
Return:		Pointer to new histogram data structure
Exceptions:	None
History:	6/4/89, DSJ, Created.
*****************************************************************************/
BUCKETS *MakeBuckets(DISTRIBUTION Distribution,
                     uinT32 SampleCount,
                     FLOAT64 Confidence) {
  const DENSITYFUNC DensityFunction[] =
    { NormalDensity, UniformDensity, UniformDensity };
  int i, j;
  BUCKETS *Buckets;
  FLOAT64 BucketProbability;
  FLOAT64 NextBucketBoundary;
  FLOAT64 Probability;
  FLOAT64 ProbabilityDelta;
  FLOAT64 LastProbDensity;
  FLOAT64 ProbDensity;
  uinT16 CurrentBucket;
  BOOL8 Symmetrical;

  // allocate memory needed for data structure
  Buckets = reinterpret_cast<BUCKETS*>(Emalloc(sizeof(BUCKETS)));
  Buckets->NumberOfBuckets = OptimumNumberOfBuckets(SampleCount);
  Buckets->SampleCount = SampleCount;
  Buckets->Confidence = Confidence;
  Buckets->Count = reinterpret_cast<uinT32*>(
      Emalloc(Buckets->NumberOfBuckets * sizeof(uinT32)));
  Buckets->ExpectedCount = reinterpret_cast<FLOAT32*>(
      Emalloc(Buckets->NumberOfBuckets * sizeof(FLOAT32)));

  // initialize simple fields
  Buckets->Distribution = Distribution;
  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    Buckets->Count[i] = 0;
    Buckets->ExpectedCount[i] = 0.0;
  }

  // all currently defined distributions are symmetrical
  Symmetrical = TRUE;
  Buckets->ChiSquared = ComputeChiSquared(
      DegreesOfFreedom(Distribution, Buckets->NumberOfBuckets), Confidence);

  if (Symmetrical) {
    // allocate buckets so that all have approx. equal probability
    BucketProbability = 1.0 / (FLOAT64) (Buckets->NumberOfBuckets);

    // distribution is symmetric so fill in upper half then copy
    CurrentBucket = Buckets->NumberOfBuckets / 2;
    if (Odd (Buckets->NumberOfBuckets))
      NextBucketBoundary = BucketProbability / 2;
    else
      NextBucketBoundary = BucketProbability;

    Probability = 0.0;
    LastProbDensity =
      (*DensityFunction[(int) Distribution]) (BUCKETTABLESIZE / 2);
    for (i = BUCKETTABLESIZE / 2; i < BUCKETTABLESIZE; i++) {
      ProbDensity = (*DensityFunction[(int) Distribution]) (i + 1);
      ProbabilityDelta = Integral (LastProbDensity, ProbDensity, 1.0);
      Probability += ProbabilityDelta;
      if (Probability > NextBucketBoundary) {
        if (CurrentBucket < Buckets->NumberOfBuckets - 1)
          CurrentBucket++;
        NextBucketBoundary += BucketProbability;
      }
      Buckets->Bucket[i] = CurrentBucket;
      Buckets->ExpectedCount[CurrentBucket] +=
        (FLOAT32) (ProbabilityDelta * SampleCount);
      LastProbDensity = ProbDensity;
    }
    // place any leftover probability into the last bucket
    Buckets->ExpectedCount[CurrentBucket] +=
      (FLOAT32) ((0.5 - Probability) * SampleCount);

    // copy upper half of distribution to lower half
    for (i = 0, j = BUCKETTABLESIZE - 1; i < j; i++, j--)
      Buckets->Bucket[i] =
        Mirror(Buckets->Bucket[j], Buckets->NumberOfBuckets);

    // copy upper half of expected counts to lower half
    for (i = 0, j = Buckets->NumberOfBuckets - 1; i <= j; i++, j--)
      Buckets->ExpectedCount[i] += Buckets->ExpectedCount[j];
  }
  return Buckets;
}                                // MakeBuckets


//---------------------------------------------------------------------------
uinT16 OptimumNumberOfBuckets(uinT32 SampleCount) {
/*
 **	Parameters:
 **		SampleCount	number of samples to be tested
  **	Operation:
 **		This routine computes the optimum number of histogram
 **		buckets that should be used in a chi-squared goodness of
 **		fit test for the specified number of samples.  The optimum
 **		number is computed based on Table 4.1 on pg. 147 of
 **		"Measurement and Analysis of Random Data" by Bendat & Piersol.
 **		Linear interpolation is used to interpolate between table
 **		values.  The table is intended for a 0.05 level of
 **		significance (alpha).  This routine assumes that it is
 **		equally valid for other alpha's, which may not be true.
 **	Return:
 **		Optimum number of histogram buckets
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  uinT8 Last, Next;
  FLOAT32 Slope;

  if (SampleCount < kCountTable[0])
    return kBucketsTable[0];

  for (Last = 0, Next = 1; Next < LOOKUPTABLESIZE; Last++, Next++) {
    if (SampleCount <= kCountTable[Next]) {
      Slope = (FLOAT32) (kBucketsTable[Next] - kBucketsTable[Last]) /
          (FLOAT32) (kCountTable[Next] - kCountTable[Last]);
      return ((uinT16) (kBucketsTable[Last] +
          Slope * (SampleCount - kCountTable[Last])));
    }
  }
  return kBucketsTable[Last];
}                                // OptimumNumberOfBuckets


//---------------------------------------------------------------------------
FLOAT64
ComputeChiSquared (uinT16 DegreesOfFreedom, FLOAT64 Alpha)
/*
 **	Parameters:
 **		DegreesOfFreedom	determines shape of distribution
 **		Alpha			probability of right tail
 **	Operation:
 **		This routine computes the chi-squared value which will
 **		leave a cumulative probability of Alpha in the right tail
 **		of a chi-squared distribution with the specified number of
 **		degrees of freedom.  Alpha must be between 0 and 1.
 **		DegreesOfFreedom must be even.  The routine maintains an
 **		array of lists.  Each list corresponds to a different
 **		number of degrees of freedom.  Each entry in the list
 **		corresponds to a different alpha value and its corresponding
 **		chi-squared value.  Therefore, once a particular chi-squared
 **		value is computed, it is stored in the list and never
 **		needs to be computed again.
 **	Return: Desired chi-squared value
 **	Exceptions: none
 **	History: 6/5/89, DSJ, Created.
 */
#define CHIACCURACY     0.01
#define MINALPHA  (1e-200)
{
  static LIST ChiWith[MAXDEGREESOFFREEDOM + 1];

  CHISTRUCT *OldChiSquared;
  CHISTRUCT SearchKey;

  // limit the minimum alpha that can be used - if alpha is too small
  //      it may not be possible to compute chi-squared.
  Alpha = ClipToRange(Alpha, MINALPHA, 1.0);
  if (Odd (DegreesOfFreedom))
    DegreesOfFreedom++;

  /* find the list of chi-squared values which have already been computed
     for the specified number of degrees of freedom.  Search the list for
     the desired chi-squared. */
  SearchKey.Alpha = Alpha;
  OldChiSquared = (CHISTRUCT *) first_node (search (ChiWith[DegreesOfFreedom],
    &SearchKey, AlphaMatch));

  if (OldChiSquared == NULL) {
    OldChiSquared = NewChiStruct (DegreesOfFreedom, Alpha);
    OldChiSquared->ChiSquared = Solve (ChiArea, OldChiSquared,
      (FLOAT64) DegreesOfFreedom,
      (FLOAT64) CHIACCURACY);
    ChiWith[DegreesOfFreedom] = push (ChiWith[DegreesOfFreedom],
      OldChiSquared);
  }
  else {
    // further optimization might move OldChiSquared to front of list
  }

  return (OldChiSquared->ChiSquared);

}                                // ComputeChiSquared


//---------------------------------------------------------------------------
FLOAT64 NormalDensity(inT32 x) {
/*
 **	Parameters:
 **		x	number to compute the normal probability density for
 **	Globals:
 **		kNormalMean	mean of a discrete normal distribution
 **		kNormalVariance	variance of a discrete normal distribution
 **		kNormalMagnitude	magnitude of a discrete normal distribution
 **	Operation:
 **		This routine computes the probability density function
 **		of a discrete normal distribution defined by the global
 **		variables kNormalMean, kNormalVariance, and kNormalMagnitude.
 **		Normal magnitude could, of course, be computed in terms of
 **		the normal variance but it is precomputed for efficiency.
 **	Return:
 **		The value of the normal distribution at x.
 **	Exceptions:
 **		None
 **	History:
 **		6/4/89, DSJ, Created.
 */
  FLOAT64 Distance;

  Distance = x - kNormalMean;
  return kNormalMagnitude * exp(-0.5 * Distance * Distance / kNormalVariance);
}                                // NormalDensity


//---------------------------------------------------------------------------
FLOAT64 UniformDensity(inT32 x) {
/*
 **	Parameters:
 **		x	number to compute the uniform probability density for
 **	Operation:
 **		This routine computes the probability density function
 **		of a uniform distribution at the specified point.  The
 **		range of the distribution is from 0 to BUCKETTABLESIZE.
 **	Return:
 **		The value of the uniform distribution at x.
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  static FLOAT64 UniformDistributionDensity = (FLOAT64) 1.0 / BUCKETTABLESIZE;

  if ((x >= 0.0) && (x <= BUCKETTABLESIZE))
    return UniformDistributionDensity;
  else
    return (FLOAT64) 0.0;
}                                // UniformDensity


//---------------------------------------------------------------------------
FLOAT64 Integral(FLOAT64 f1, FLOAT64 f2, FLOAT64 Dx) {
/*
 **	Parameters:
 **		f1	value of function at x1
 **		f2	value of function at x2
 **		Dx	x2 - x1 (should always be positive)
 **	Operation:
 **		This routine computes a trapezoidal approximation to the
 **		integral of a function over a small delta in x.
 **	Return:
 **		Approximation of the integral of the function from x1 to x2.
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  return (f1 + f2) * Dx / 2.0;
}                                // Integral


//---------------------------------------------------------------------------
void FillBuckets(BUCKETS *Buckets,
                 CLUSTER *Cluster,
                 uinT16 Dim,
                 PARAM_DESC *ParamDesc,
                 FLOAT32 Mean,
                 FLOAT32 StdDev) {
/*
 **	Parameters:
 **		Buckets		histogram buckets to count samples
 **		Cluster		cluster whose samples are being analyzed
 **		Dim		dimension of samples which is being analyzed
 **		ParamDesc	description of the dimension
 **		Mean		"mean" of the distribution
 **		StdDev		"standard deviation" of the distribution
 **	Operation:
 **		This routine counts the number of cluster samples which
 **		fall within the various histogram buckets in Buckets.  Only
 **		one dimension of each sample is examined.  The exact meaning
 **		of the Mean and StdDev parameters depends on the
 **		distribution which is being analyzed (this info is in the
 **		Buckets data structure).  For normal distributions, Mean
 **		and StdDev have the expected meanings.  For uniform and
 **		random distributions the Mean is the center point of the
 **		range and the StdDev is 1/2 the range.  A dimension with
 **		zero standard deviation cannot be statistically analyzed.
 **		In this case, a pseudo-analysis is used.
 **	Return:
 **		None (the Buckets data structure is filled in)
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  uinT16 BucketID;
  int i;
  LIST SearchState;
  SAMPLE *Sample;

  // initialize the histogram bucket counts to 0
  for (i = 0; i < Buckets->NumberOfBuckets; i++)
    Buckets->Count[i] = 0;

  if (StdDev == 0.0) {
    /* if the standard deviation is zero, then we can't statistically
       analyze the cluster.  Use a pseudo-analysis: samples exactly on
       the mean are distributed evenly across all buckets.  Samples greater
       than the mean are placed in the last bucket; samples less than the
       mean are placed in the first bucket. */

    InitSampleSearch(SearchState, Cluster);
    i = 0;
    while ((Sample = NextSample (&SearchState)) != NULL) {
      if (Sample->Mean[Dim] > Mean)
        BucketID = Buckets->NumberOfBuckets - 1;
      else if (Sample->Mean[Dim] < Mean)
        BucketID = 0;
      else
        BucketID = i;
      Buckets->Count[BucketID] += 1;
      i++;
      if (i >= Buckets->NumberOfBuckets)
        i = 0;
    }
  }
  else {
    // search for all samples in the cluster and add to histogram buckets
    InitSampleSearch(SearchState, Cluster);
    while ((Sample = NextSample (&SearchState)) != NULL) {
      switch (Buckets->Distribution) {
        case normal:
          BucketID = NormalBucket (ParamDesc, Sample->Mean[Dim],
            Mean, StdDev);
          break;
        case D_random:
        case uniform:
          BucketID = UniformBucket (ParamDesc, Sample->Mean[Dim],
            Mean, StdDev);
          break;
        default:
          BucketID = 0;
      }
      Buckets->Count[Buckets->Bucket[BucketID]] += 1;
    }
  }
}                                // FillBuckets


//---------------------------------------------------------------------------*/
uinT16 NormalBucket(PARAM_DESC *ParamDesc,
                    FLOAT32 x,
                    FLOAT32 Mean,
                    FLOAT32 StdDev) {
/*
 **	Parameters:
 **		ParamDesc	used to identify circular dimensions
 **		x		value to be normalized
 **		Mean		mean of normal distribution
 **		StdDev		standard deviation of normal distribution
 **	Operation:
 **		This routine determines which bucket x falls into in the
 **		discrete normal distribution defined by kNormalMean
 **		and kNormalStdDev.  x values which exceed the range of
 **		the discrete distribution are clipped.
 **	Return:
 **		Bucket number into which x falls
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  FLOAT32 X;

  // wraparound circular parameters if necessary
  if (ParamDesc->Circular) {
    if (x - Mean > ParamDesc->HalfRange)
      x -= ParamDesc->Range;
    else if (x - Mean < -ParamDesc->HalfRange)
      x += ParamDesc->Range;
  }

  X = ((x - Mean) / StdDev) * kNormalStdDev + kNormalMean;
  if (X < 0)
    return 0;
  if (X > BUCKETTABLESIZE - 1)
    return ((uinT16) (BUCKETTABLESIZE - 1));
  return (uinT16) floor((FLOAT64) X);
}                                // NormalBucket


//---------------------------------------------------------------------------
uinT16 UniformBucket(PARAM_DESC *ParamDesc,
                     FLOAT32 x,
                     FLOAT32 Mean,
                     FLOAT32 StdDev) {
/*
 **	Parameters:
 **		ParamDesc	used to identify circular dimensions
 **		x		value to be normalized
 **		Mean		center of range of uniform distribution
 **		StdDev		1/2 the range of the uniform distribution
 **	Operation:
 **		This routine determines which bucket x falls into in the
 **		discrete uniform distribution defined by
 **		BUCKETTABLESIZE.  x values which exceed the range of
 **		the discrete distribution are clipped.
 **	Return:
 **		Bucket number into which x falls
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  FLOAT32 X;

  // wraparound circular parameters if necessary
  if (ParamDesc->Circular) {
    if (x - Mean > ParamDesc->HalfRange)
      x -= ParamDesc->Range;
    else if (x - Mean < -ParamDesc->HalfRange)
      x += ParamDesc->Range;
  }

  X = ((x - Mean) / (2 * StdDev) * BUCKETTABLESIZE + BUCKETTABLESIZE / 2.0);
  if (X < 0)
    return 0;
  if (X > BUCKETTABLESIZE - 1)
    return (uinT16) (BUCKETTABLESIZE - 1);
  return (uinT16) floor((FLOAT64) X);
}                                // UniformBucket


//---------------------------------------------------------------------------
BOOL8 DistributionOK(BUCKETS *Buckets) {
/*
 **	Parameters:
 **		Buckets		histogram data to perform chi-square test on
 **	Operation:
 **		This routine performs a chi-square goodness of fit test
 **		on the histogram data in the Buckets data structure.  TRUE
 **		is returned if the histogram matches the probability
 **		distribution which was specified when the Buckets
 **		structure was originally created.  Otherwise FALSE is
 **		returned.
 **	Return:
 **		TRUE if samples match distribution, FALSE otherwise
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  FLOAT32 FrequencyDifference;
  FLOAT32 TotalDifference;
  int i;

  // compute how well the histogram matches the expected histogram
  TotalDifference = 0.0;
  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    FrequencyDifference = Buckets->Count[i] - Buckets->ExpectedCount[i];
    TotalDifference += (FrequencyDifference * FrequencyDifference) /
      Buckets->ExpectedCount[i];
  }

  // test to see if the difference is more than expected
  if (TotalDifference > Buckets->ChiSquared)
    return FALSE;
  else
    return TRUE;
}                                // DistributionOK


//---------------------------------------------------------------------------
void FreeStatistics(STATISTICS *Statistics) {
/*
 **	Parameters:
 **		Statistics	pointer to data structure to be freed
 **	Operation:
 **		This routine frees the memory used by the statistics
 **		data structure.
 **	Return:
 **		None
 **	Exceptions:
 **		None
 **	History:
 **		6/5/89, DSJ, Created.
 */
  memfree (Statistics->CoVariance);
  memfree (Statistics->Min);
  memfree (Statistics->Max);
  memfree(Statistics);
}                                // FreeStatistics


//---------------------------------------------------------------------------
void FreeBuckets(BUCKETS *buckets) {
/*
 **  Parameters:
 **      buckets  pointer to data structure to be freed
 **  Operation:
 **      This routine properly frees the memory used by a BUCKETS.
 */
  Efree(buckets->Count);
  Efree(buckets->ExpectedCount);
  Efree(buckets);
}                                // FreeBuckets


//---------------------------------------------------------------------------
void FreeCluster(CLUSTER *Cluster) {
/*
 **	Parameters:
 **		Cluster		pointer to cluster to be freed
 **	Operation:
 **		This routine frees the memory consumed by the specified
 **		cluster and all of its subclusters.  This is done by
 **		recursive calls to FreeCluster().
 **	Return:
 **		None
 **	Exceptions:
 **		None
 **	History:
 **		6/6/89, DSJ, Created.
 */
  if (Cluster != NULL) {
    FreeCluster (Cluster->Left);
    FreeCluster (Cluster->Right);
    memfree(Cluster);
  }
}                                // FreeCluster


//---------------------------------------------------------------------------
uinT16 DegreesOfFreedom(DISTRIBUTION Distribution, uinT16 HistogramBuckets) {
/*
 **	Parameters:
 **		Distribution		distribution being tested for
 **		HistogramBuckets	number of buckets in chi-square test
 **	Operation:
 **		This routine computes the degrees of freedom that should
 **		be used in a chi-squared test with the specified number of
 **		histogram buckets.  The result is always rounded up to
 **		the next even number so that the value of chi-squared can be
 **		computed more easily.  This will cause the value of
 **		chi-squared to be higher than the optimum value, resulting
 **		in the chi-square test being more lenient than optimum.
 **	Return: The number of degrees of freedom for a chi-square test
 **	Exceptions: none
 **	History: Thu Aug  3 14:04:18 1989, DSJ, Created.
 */
  static uinT8 DegreeOffsets[] = { 3, 3, 1 };

  uinT16 AdjustedNumBuckets;

  AdjustedNumBuckets = HistogramBuckets - DegreeOffsets[(int) Distribution];
  if (Odd (AdjustedNumBuckets))
    AdjustedNumBuckets++;
  return (AdjustedNumBuckets);

}                                // DegreesOfFreedom


//---------------------------------------------------------------------------
int NumBucketsMatch(void *arg1,    // BUCKETS *Histogram,
                    void *arg2) {  // uinT16 *DesiredNumberOfBuckets)
/*
 **	Parameters:
 **		Histogram	current histogram being tested for a match
 **		DesiredNumberOfBuckets	match key
 **	Operation:
 **		This routine is used to search a list of histogram data
 **		structures to find one with the specified number of
 **		buckets.  It is called by the list search routines.
 **	Return: TRUE if Histogram matches DesiredNumberOfBuckets
 **	Exceptions: none
 **	History: Thu Aug  3 14:17:33 1989, DSJ, Created.
 */
  BUCKETS *Histogram = (BUCKETS *) arg1;
  uinT16 *DesiredNumberOfBuckets = (uinT16 *) arg2;

  return (*DesiredNumberOfBuckets == Histogram->NumberOfBuckets);

}                                // NumBucketsMatch


//---------------------------------------------------------------------------
int ListEntryMatch(void *arg1,    //ListNode
                   void *arg2) {  //Key
/*
 **	Parameters: none
 **	Operation:
 **		This routine is used to search a list for a list node
 **		whose contents match Key.  It is called by the list
 **		delete_d routine.
 **	Return: TRUE if ListNode matches Key
 **	Exceptions: none
 **	History: Thu Aug  3 14:23:58 1989, DSJ, Created.
 */
  return (arg1 == arg2);

}                                // ListEntryMatch


//---------------------------------------------------------------------------
void AdjustBuckets(BUCKETS *Buckets, uinT32 NewSampleCount) {
/*
 **	Parameters:
 **		Buckets		histogram data structure to adjust
 **		NewSampleCount	new sample count to adjust to
 **	Operation:
 **		This routine multiplies each ExpectedCount histogram entry
 **		by NewSampleCount/OldSampleCount so that the histogram
 **		is now adjusted to the new sample count.
 **	Return: none
 **	Exceptions: none
 **	History: Thu Aug  3 14:31:14 1989, DSJ, Created.
 */
  int i;
  FLOAT64 AdjustFactor;

  AdjustFactor = (((FLOAT64) NewSampleCount) /
    ((FLOAT64) Buckets->SampleCount));

  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    Buckets->ExpectedCount[i] *= AdjustFactor;
  }

  Buckets->SampleCount = NewSampleCount;

}                                // AdjustBuckets


//---------------------------------------------------------------------------
void InitBuckets(BUCKETS *Buckets) {
/*
 **	Parameters:
 **		Buckets		histogram data structure to init
 **	Operation:
 **		This routine sets the bucket counts in the specified histogram
 **		to zero.
 **	Return: none
 **	Exceptions: none
 **	History: Thu Aug  3 14:31:14 1989, DSJ, Created.
 */
  int i;

  for (i = 0; i < Buckets->NumberOfBuckets; i++) {
    Buckets->Count[i] = 0;
  }

}                                // InitBuckets


//---------------------------------------------------------------------------
int AlphaMatch(void *arg1,    //CHISTRUCT                             *ChiStruct,
               void *arg2) {  //CHISTRUCT                             *SearchKey)
/*
 **	Parameters:
 **		ChiStruct	chi-squared struct being tested for a match
 **		SearchKey	chi-squared struct that is the search key
 **	Operation:
 **		This routine is used to search a list of structures which
 **		hold pre-computed chi-squared values for a chi-squared
 **		value whose corresponding alpha field matches the alpha
 **		field of SearchKey.
 **		It is called by the list search routines.
 **	Return: TRUE if ChiStruct's Alpha matches SearchKey's Alpha
 **	Exceptions: none
 **	History: Thu Aug  3 14:17:33 1989, DSJ, Created.
 */
  CHISTRUCT *ChiStruct = (CHISTRUCT *) arg1;
  CHISTRUCT *SearchKey = (CHISTRUCT *) arg2;

  return (ChiStruct->Alpha == SearchKey->Alpha);

}                                // AlphaMatch


//---------------------------------------------------------------------------
CHISTRUCT *NewChiStruct(uinT16 DegreesOfFreedom, FLOAT64 Alpha) {
/*
 **	Parameters:
 **		DegreesOfFreedom	degrees of freedom for new chi value
 **		Alpha			confidence level for new chi value
 **	Operation:
 **		This routine allocates a new data structure which is used
 **		to hold a chi-squared value along with its associated
 **		number of degrees of freedom and alpha value.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Aug  4 11:04:59 1989, DSJ, Created.
 */
  CHISTRUCT *NewChiStruct;

  NewChiStruct = (CHISTRUCT *) Emalloc (sizeof (CHISTRUCT));
  NewChiStruct->DegreesOfFreedom = DegreesOfFreedom;
  NewChiStruct->Alpha = Alpha;
  return (NewChiStruct);

}                                // NewChiStruct


//---------------------------------------------------------------------------
FLOAT64
Solve (SOLVEFUNC Function,
void *FunctionParams, FLOAT64 InitialGuess, FLOAT64 Accuracy)
/*
 **	Parameters:
 **		Function	function whose zero is to be found
 **		FunctionParams	arbitrary data to pass to function
 **		InitialGuess	point to start solution search at
 **		Accuracy	maximum allowed error
 **	Operation:
 **		This routine attempts to find an x value at which Function
 **		goes to zero (i.e. a root of the function ).  It will only
 **		work correctly if a solution actually exists and there
 **		are no extrema between the solution and the InitialGuess.
 **		The algorithms used are extremely primitive.
 **	Return: Solution of function ( x for which f(x) = 0 ).
 **	Exceptions: none
 **	History: Fri Aug  4 11:08:59 1989, DSJ, Created.
 */
#define INITIALDELTA    0.1
#define  DELTARATIO     0.1
{
  FLOAT64 x;
  FLOAT64 f;
  FLOAT64 Slope;
  FLOAT64 Delta;
  FLOAT64 NewDelta;
  FLOAT64 xDelta;
  FLOAT64 LastPosX, LastNegX;

  x = InitialGuess;
  Delta = INITIALDELTA;
  LastPosX = MAX_FLOAT32;
  LastNegX = -MAX_FLOAT32;
  f = (*Function) ((CHISTRUCT *) FunctionParams, x);
  while (Abs (LastPosX - LastNegX) > Accuracy) {
    // keep track of outer bounds of current estimate
    if (f < 0)
      LastNegX = x;
    else
      LastPosX = x;

    // compute the approx. slope of f(x) at the current point
    Slope =
      ((*Function) ((CHISTRUCT *) FunctionParams, x + Delta) - f) / Delta;

    // compute the next solution guess */
    xDelta = f / Slope;
    x -= xDelta;

    // reduce the delta used for computing slope to be a fraction of
    //the amount moved to get to the new guess
    NewDelta = Abs (xDelta) * DELTARATIO;
    if (NewDelta < Delta)
      Delta = NewDelta;

    // compute the value of the function at the new guess
    f = (*Function) ((CHISTRUCT *) FunctionParams, x);
  }
  return (x);

}                                // Solve


//---------------------------------------------------------------------------
FLOAT64 ChiArea(CHISTRUCT *ChiParams, FLOAT64 x) {
/*
 **	Parameters:
 **		ChiParams	contains degrees of freedom and alpha
 **		x		value of chi-squared to evaluate
 **	Operation:
 **		This routine computes the area under a chi density curve
 **		from 0 to x, minus the desired area under the curve.  The
 **		number of degrees of freedom of the chi curve is specified
 **		in the ChiParams structure.  The desired area is also
 **		specified in the ChiParams structure as Alpha ( or 1 minus
 **		the desired area ).  This routine is intended to be passed
 **		to the Solve() function to find the value of chi-squared
 **		which will yield a desired area under the right tail of
 **		the chi density curve.  The function will only work for
 **		even degrees of freedom.  The equations are based on
 **		integrating the chi density curve in parts to obtain
 **		a series that can be used to compute the area under the
 **		curve.
 **	Return: Error between actual and desired area under the chi curve.
 **	Exceptions: none
 **	History: Fri Aug  4 12:48:41 1989, DSJ, Created.
 */
  int i, N;
  FLOAT64 SeriesTotal;
  FLOAT64 Denominator;
  FLOAT64 PowerOfx;

  N = ChiParams->DegreesOfFreedom / 2 - 1;
  SeriesTotal = 1;
  Denominator = 1;
  PowerOfx = 1;
  for (i = 1; i <= N; i++) {
    Denominator *= 2 * i;
    PowerOfx *= x;
    SeriesTotal += PowerOfx / Denominator;
  }
  return ((SeriesTotal * exp (-0.5 * x)) - ChiParams->Alpha);

}                                // ChiArea


//---------------------------------------------------------------------------
BOOL8
MultipleCharSamples (CLUSTERER * Clusterer,
CLUSTER * Cluster, FLOAT32 MaxIllegal)
/*
 **	Parameters:
 **		Clusterer	data structure holding cluster tree
 **		Cluster		cluster containing samples to be tested
 **		MaxIllegal	max percentage of samples allowed to have
 **				more than 1 feature in the cluster
 **	Operation:
 **		This routine looks at all samples in the specified cluster.
 **		It computes a running estimate of the percentage of the
 **		charaters which have more than 1 sample in the cluster.
 **		When this percentage exceeds MaxIllegal, TRUE is returned.
 **		Otherwise FALSE is returned.  The CharID
 **		fields must contain integers which identify the training
 **		characters which were used to generate the sample.  One
 **		integer is used for each sample.  The NumChar field in
 **		the Clusterer must contain the number of characters in the
 **		training set.  All CharID fields must be between 0 and
 **		NumChar-1.  The main function of this routine is to help
 **		identify clusters which need to be split further, i.e. if
 **		numerous training characters have 2 or more features which are
 **		contained in the same cluster, then the cluster should be
 **		split.
 **	Return: TRUE if the cluster should be split, FALSE otherwise.
 **	Exceptions: none
 **	History: Wed Aug 30 11:13:05 1989, DSJ, Created.
 **		2/22/90, DSJ, Added MaxIllegal control rather than always
 **				splitting illegal clusters.
 */
#define ILLEGAL_CHAR    2
{
  static BOOL8 *CharFlags = NULL;
  static inT32 NumFlags = 0;
  int i;
  LIST SearchState;
  SAMPLE *Sample;
  inT32 CharID;
  inT32 NumCharInCluster;
  inT32 NumIllegalInCluster;
  FLOAT32 PercentIllegal;

  // initial estimate assumes that no illegal chars exist in the cluster
  NumCharInCluster = Cluster->SampleCount;
  NumIllegalInCluster = 0;

  if (Clusterer->NumChar > NumFlags) {
    if (CharFlags != NULL)
      memfree(CharFlags);
    NumFlags = Clusterer->NumChar;
    CharFlags = (BOOL8 *) Emalloc (NumFlags * sizeof (BOOL8));
  }

  for (i = 0; i < NumFlags; i++)
    CharFlags[i] = FALSE;

  // find each sample in the cluster and check if we have seen it before
  InitSampleSearch(SearchState, Cluster);
  while ((Sample = NextSample (&SearchState)) != NULL) {
    CharID = Sample->CharID;
    if (CharFlags[CharID] == FALSE) {
      CharFlags[CharID] = TRUE;
    }
    else {
      if (CharFlags[CharID] == TRUE) {
        NumIllegalInCluster++;
        CharFlags[CharID] = ILLEGAL_CHAR;
      }
      NumCharInCluster--;
      PercentIllegal = (FLOAT32) NumIllegalInCluster / NumCharInCluster;
      if (PercentIllegal > MaxIllegal) {
        destroy(SearchState);
        return (TRUE);
      }
    }
  }
  return (FALSE);

}                                // MultipleCharSamples

// Compute the inverse of a matrix using LU decomposition with partial pivoting.
// The return value is the sum of norms of the off-diagonal terms of the
// product of a and inv. (A measure of the error.)
double InvertMatrix(const float* input, int size, float* inv) {
  // Allocate memory for the 2D arrays.
  GENERIC_2D_ARRAY<double> U(size, size, 0.0);
  GENERIC_2D_ARRAY<double> U_inv(size, size, 0.0);
  GENERIC_2D_ARRAY<double> L(size, size, 0.0);

  // Initialize the working matrices. U starts as input, L as I and U_inv as O.
  int row;
  int col;
  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      U[row][col] = input[row*size + col];
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
      inv[row*size + col] = sum;
    }
  }
  // Check matrix product.
  double error_sum = 0.0;
  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      double sum = 0.0;
      for (int k = 0; k < size; ++k) {
        sum += input[row*size + k] * inv[k *size + col];
      }
      if (row != col) {
        error_sum += Abs(sum);
      }
    }
  }
  return error_sum;
}
