/******************************************************************************
 ** Filename: clusttool.h
 ** Purpose:  Definition of clustering utility tools
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
 ******************************************************************************/

#ifndef TESSERACT_CLASSIFY_CLUSTTOOL_H_
#define TESSERACT_CLASSIFY_CLUSTTOOL_H_

#include "cluster.h"

#include "serialis.h"

#include <cstdio>

namespace tesseract {

uint16_t ReadSampleSize(tesseract::TFile *fp);

PARAM_DESC *ReadParamDesc(tesseract::TFile *fp, uint16_t N);

PROTOTYPE *ReadPrototype(tesseract::TFile *fp, uint16_t N);

TESS_API
void WriteParamDesc(FILE *File, uint16_t N, const PARAM_DESC ParamDesc[]);

TESS_API
void WritePrototype(FILE *File, uint16_t N, PROTOTYPE *Proto);

} // namespace tesseract

#endif // TESSERACT_CLASSIFY_CLUSTTOOL_H_
