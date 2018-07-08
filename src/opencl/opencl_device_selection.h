// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DEVICE_SELECTION_H
#define DEVICE_SELECTION_H

#ifdef USE_OPENCL

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

struct TessDeviceScore;

// device type
enum ds_device_type {
  DS_DEVICE_NATIVE_CPU = 0,
  DS_DEVICE_OPENCL_DEVICE
};

struct ds_device {
  ds_device_type type;
  cl_device_id oclDeviceID;
  char* oclDeviceName;
  char* oclDriverVersion;
  // a pointer to the score data, the content/format is application defined.
  TessDeviceScore* score;
};

#endif  // USE_OPENCL
#endif  // DEVICE_SELECTION_H
