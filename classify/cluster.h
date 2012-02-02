/******************************************************************************
 **	Filename:	cluster.h
 **	Purpose:	Definition of feature space clustering routines
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
#ifndef   CLUSTER_H
#define   CLUSTER_H

#include "kdtree.h"
#include "oldlist.h"

struct BUCKETS;

#define MINBUCKETS      5
#define MAXBUCKETS      39

/*----------------------------------------------------------------------
          Types
----------------------------------------------------------------------*/
typedef struct sample {
  unsigned Clustered:1;          // TRUE if included in a higher cluster
  unsigned Prototype:1;          // TRUE if cluster represented by a proto
  unsigned SampleCount:30;       // number of samples in this cluster
  struct sample *Left;           // ptr to left sub-cluster
  struct sample *Right;          // ptr to right sub-cluster
  inT32 CharID;                  // identifier of char sample came from
  FLOAT32 Mean[1];               // mean of cluster - SampleSize floats
} CLUSTER;

typedef CLUSTER SAMPLE;          // can refer to as either sample or cluster

typedef enum {
  spherical, elliptical, mixed, automatic
} PROTOSTYLE;

typedef struct {                 // parameters to control clustering
  PROTOSTYLE ProtoStyle;         // specifies types of protos to be made
  FLOAT32 MinSamples;            // min # of samples per proto - % of total
  FLOAT32 MaxIllegal;            // max percentage of samples in a cluster which have
  // more than 1 feature in that cluster
  FLOAT32 Independence;          // desired independence between dimensions
  FLOAT64 Confidence;            // desired confidence in prototypes created
  int MagicSamples;              // Ideal number of samples in a cluster.
} CLUSTERCONFIG;

typedef enum {
  normal, uniform, D_random, DISTRIBUTION_COUNT
} DISTRIBUTION;

typedef union {
  FLOAT32 Spherical;
  FLOAT32 *Elliptical;
} FLOATUNION;

typedef struct {
  unsigned Significant:1;        // TRUE if prototype is significant
  unsigned Merged:1;             // Merged after clustering so do not output
                                 // but kept for display purposes. If it has no
                                 // samples then it was actually merged.
                                 // Otherwise it matched an already significant
                                 // cluster.
  unsigned Style:2;              // spherical, elliptical, or mixed
  unsigned NumSamples:28;        // number of samples in the cluster
  CLUSTER *Cluster;              // ptr to cluster which made prototype
  DISTRIBUTION *Distrib;         // different distribution for each dimension
  FLOAT32 *Mean;                 // prototype mean
  FLOAT32 TotalMagnitude;        // total magnitude over all dimensions
  FLOAT32 LogMagnitude;          // log base e of TotalMagnitude
  FLOATUNION Variance;           // prototype variance
  FLOATUNION Magnitude;          // magnitude of density function
  FLOATUNION Weight;             // weight of density function
} PROTOTYPE;

typedef struct {
  inT16 SampleSize;              // number of parameters per sample
  PARAM_DESC *ParamDesc;         // description of each parameter
  inT32 NumberOfSamples;         // total number of samples being clustered
  KDTREE *KDTree;                // for optimal nearest neighbor searching
  CLUSTER *Root;                 // ptr to root cluster of cluster tree
  LIST ProtoList;                // list of prototypes
  inT32 NumChar;                 // # of characters represented by samples
  // cache of reusable histograms by distribution type and number of buckets.
  BUCKETS* bucket_cache[DISTRIBUTION_COUNT][MAXBUCKETS + 1 - MINBUCKETS];
} CLUSTERER;

typedef struct {
  inT32 NumSamples;              // number of samples in list
  inT32 MaxNumSamples;           // maximum size of list
  SAMPLE *Sample[1];             // array of ptrs to sample data structures
} SAMPLELIST;

// low level cluster tree analysis routines.
#define InitSampleSearch(S,C) (((C)==NULL)?(S=NIL_LIST):(S=push(NIL_LIST,(C))))

/*--------------------------------------------------------------------------
        Public Function Prototypes
--------------------------------------------------------------------------*/
CLUSTERER *MakeClusterer (inT16 SampleSize, const PARAM_DESC ParamDesc[]);

SAMPLE *MakeSample(CLUSTERER * Clusterer, const FLOAT32* Feature, inT32 CharID);

LIST ClusterSamples(CLUSTERER *Clusterer, CLUSTERCONFIG *Config);

void FreeClusterer(CLUSTERER *Clusterer);

void FreeProtoList(LIST *ProtoList);

void FreePrototype(void *arg);  // PROTOTYPE *Prototype);

CLUSTER *NextSample(LIST *SearchState);

FLOAT32 Mean(PROTOTYPE *Proto, uinT16 Dimension);

FLOAT32 StandardDeviation(PROTOTYPE *Proto, uinT16 Dimension);

inT32 MergeClusters(inT16 N, PARAM_DESC ParamDesc[], inT32 n1, inT32 n2,
                    FLOAT32 m[], FLOAT32 m1[], FLOAT32 m2[]);

//--------------Global Data Definitions and Declarations---------------------------
// define errors that can be trapped
#define ALREADYCLUSTERED  4000
#endif
