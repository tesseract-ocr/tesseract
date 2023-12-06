// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Include automatically generated configuration file
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#ifdef USE_OPENCL

#  ifdef _WIN32
#    include <io.h>
#    include <windows.h>
#  else
#    include <sys/types.h>
#    include <unistd.h>
#  endif
#  include <cfloat>
#  include <ctime> // for clock_gettime

#  include "oclkernels.h"
#  include "openclwrapper.h"

// for micro-benchmark
#  include "otsuthr.h"
#  include "thresholder.h"

// platform preprocessor commands
#  if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN__) || \
      defined(__MINGW32__)
#    define ON_WINDOWS 1
#    define ON_APPLE 0
#  elif defined(__linux__)
#    define ON_WINDOWS 0
#    define ON_APPLE 0
#  elif defined(__APPLE__)
#    define ON_WINDOWS 0
#    define ON_APPLE 1
#  else
#    define ON_WINDOWS 0
#    define ON_APPLE 0
#  endif

#  if ON_APPLE
#    include <mach/mach_time.h>
#  endif

#  include <cstdio>
#  include <cstdlib>
#  include <cstring> // for memset, strcpy, ...
#  include <vector>

#  include "errcode.h" // for ASSERT_HOST
#  include "image.h"   // for Image

namespace tesseract {

GPUEnv OpenclDevice::gpuEnv;

bool OpenclDevice::deviceIsSelected = false;
ds_device OpenclDevice::selectedDevice;

int OpenclDevice::isInited = 0;

static l_int32 MORPH_BC = ASYMMETRIC_MORPH_BC;

static const l_uint32 lmask32[] = {
    0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000, 0xfc000000, 0xfe000000, 0xff000000,
    0xff800000, 0xffc00000, 0xffe00000, 0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000,
    0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00,
    0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff};

static const l_uint32 rmask32[] = {
    0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
    0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
    0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
    0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff};

static cl_mem pixsCLBuffer, pixdCLBuffer,
    pixdCLIntermediate;    // Morph operations buffers
static cl_mem pixThBuffer; // output from thresholdtopix calculation
static cl_int clStatus;
static KernelEnv rEnv;

#  define DS_TAG_VERSION "<version>"
#  define DS_TAG_VERSION_END "</version>"
#  define DS_TAG_DEVICE "<device>"
#  define DS_TAG_DEVICE_END "</device>"
#  define DS_TAG_SCORE "<score>"
#  define DS_TAG_SCORE_END "</score>"
#  define DS_TAG_DEVICE_TYPE "<type>"
#  define DS_TAG_DEVICE_TYPE_END "</type>"
#  define DS_TAG_DEVICE_NAME "<name>"
#  define DS_TAG_DEVICE_NAME_END "</name>"
#  define DS_TAG_DEVICE_DRIVER_VERSION "<driver>"
#  define DS_TAG_DEVICE_DRIVER_VERSION_END "</driver>"

#  define DS_DEVICE_NATIVE_CPU_STRING "native_cpu"

#  define DS_DEVICE_NAME_LENGTH 256

enum ds_evaluation_type { DS_EVALUATE_ALL, DS_EVALUATE_NEW_ONLY };

struct ds_profile {
  std::vector<ds_device> devices;
  unsigned int numDevices;
  const char *version;
};

enum ds_status {
  DS_SUCCESS = 0,
  DS_INVALID_PROFILE = 1000,
  DS_MEMORY_ERROR,
  DS_INVALID_PERF_EVALUATOR_TYPE,
  DS_INVALID_PERF_EVALUATOR,
  DS_PERF_EVALUATOR_ERROR,
  DS_FILE_ERROR,
  DS_UNKNOWN_DEVICE_TYPE,
  DS_PROFILE_FILE_ERROR,
  DS_SCORE_SERIALIZER_ERROR,
  DS_SCORE_DESERIALIZER_ERROR
};

// Pointer to a function that calculates the score of a device (ex:
// device->score) update the data size of score. The encoding and the format
// of the score data is implementation defined. The function should return
// DS_SUCCESS if there's no error to be reported.
typedef ds_status (*ds_perf_evaluator)(ds_device *device, void *data);

// deallocate memory used by score
typedef ds_status (*ds_score_release)(TessDeviceScore *score);

static ds_status releaseDSProfile(ds_profile *profile, ds_score_release sr) {
  ds_status status = DS_SUCCESS;
  if (profile != nullptr) {
    if (sr != nullptr) {
      unsigned int i;
      for (i = 0; i < profile->numDevices; i++) {
        free(profile->devices[i].oclDeviceName);
        free(profile->devices[i].oclDriverVersion);
        status = sr(profile->devices[i].score);
        if (status != DS_SUCCESS)
          break;
      }
    }
    delete profile;
  }
  return status;
}

static ds_status initDSProfile(ds_profile **p, const char *version) {
  int numDevices;
  cl_uint numPlatforms;
  std::vector<cl_platform_id> platforms;
  std::vector<cl_device_id> devices;
  ds_status status = DS_SUCCESS;
  unsigned int next;
  unsigned int i;

  if (p == nullptr)
    return DS_INVALID_PROFILE;

  ds_profile *profile = new ds_profile;

  memset(profile, 0, sizeof(ds_profile));

  clGetPlatformIDs(0, nullptr, &numPlatforms);

  if (numPlatforms > 0) {
    platforms.resize(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
  }

  numDevices = 0;
  for (i = 0; i < numPlatforms; i++) {
    cl_uint num;
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &num);
    numDevices += num;
  }

  if (numDevices > 0) {
    devices.resize(numDevices);
  }

  profile->numDevices = numDevices + 1; // +1 to numDevices to include the native CPU
  profile->devices.resize(profile->numDevices);

  next = 0;
  for (i = 0; i < numPlatforms; i++) {
    cl_uint num;
    unsigned j;
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, &devices[0], &num);
    for (j = 0; j < num; j++, next++) {
      char buffer[DS_DEVICE_NAME_LENGTH];
      size_t length;

      profile->devices[next].type = DS_DEVICE_OPENCL_DEVICE;
      profile->devices[next].oclDeviceID = devices[j];

      clGetDeviceInfo(profile->devices[next].oclDeviceID, CL_DEVICE_NAME, DS_DEVICE_NAME_LENGTH,
                      &buffer, nullptr);
      length = strlen(buffer);
      profile->devices[next].oclDeviceName = (char *)malloc(length + 1);
      memcpy(profile->devices[next].oclDeviceName, buffer, length + 1);

      clGetDeviceInfo(profile->devices[next].oclDeviceID, CL_DRIVER_VERSION, DS_DEVICE_NAME_LENGTH,
                      &buffer, nullptr);
      length = strlen(buffer);
      profile->devices[next].oclDriverVersion = (char *)malloc(length + 1);
      memcpy(profile->devices[next].oclDriverVersion, buffer, length + 1);
    }
  }
  profile->devices[next].type = DS_DEVICE_NATIVE_CPU;
  profile->version = version;

  *p = profile;
  return status;
}

static ds_status profileDevices(ds_profile *profile, const ds_evaluation_type type,
                                ds_perf_evaluator evaluator, void *evaluatorData,
                                unsigned int *numUpdates) {
  ds_status status = DS_SUCCESS;
  unsigned int i;
  unsigned int updates = 0;

  if (profile == nullptr) {
    return DS_INVALID_PROFILE;
  }
  if (evaluator == nullptr) {
    return DS_INVALID_PERF_EVALUATOR;
  }

  for (i = 0; i < profile->numDevices; i++) {
    ds_status evaluatorStatus;

    switch (type) {
      case DS_EVALUATE_NEW_ONLY:
        if (profile->devices[i].score != nullptr)
          break;
      //  else fall through
      case DS_EVALUATE_ALL:
        evaluatorStatus = evaluator(&profile->devices[i], evaluatorData);
        if (evaluatorStatus != DS_SUCCESS) {
          status = evaluatorStatus;
          return status;
        }
        updates++;
        break;
      default:
        return DS_INVALID_PERF_EVALUATOR_TYPE;
        break;
    };
  }
  if (numUpdates)
    *numUpdates = updates;
  return status;
}

static const char *findString(const char *contentStart, const char *contentEnd,
                              const char *string) {
  size_t stringLength;
  const char *currentPosition;
  const char *found = nullptr;
  stringLength = strlen(string);
  currentPosition = contentStart;
  for (currentPosition = contentStart; currentPosition < contentEnd; currentPosition++) {
    if (*currentPosition == string[0]) {
      if (currentPosition + stringLength < contentEnd) {
        if (strncmp(currentPosition, string, stringLength) == 0) {
          found = currentPosition;
          break;
        }
      }
    }
  }
  return found;
}

static ds_status readProFile(const char *fileName, char **content, size_t *contentSize) {
  *contentSize = 0;
  *content = nullptr;
  ds_status status = DS_SUCCESS;
  FILE *input = fopen(fileName, "rb");
  if (input == nullptr) {
    status = DS_FILE_ERROR;
  } else {
    fseek(input, 0L, SEEK_END);
    auto pos = std::ftell(input);
    rewind(input);
    if (pos > 0) {
      size_t size = pos;
      char *binary = new char[size];
      if (fread(binary, sizeof(char), size, input) != size) {
        status = DS_FILE_ERROR;
        delete[] binary;
      } else {
        *contentSize = size;
        *content = binary;
      }
    }
    fclose(input);
  }
  return status;
}

typedef ds_status (*ds_score_deserializer)(ds_device *device, const uint8_t *serializedScore,
                                           unsigned int serializedScoreSize);

static ds_status readProfileFromFile(ds_profile *profile, ds_score_deserializer deserializer,
                                     const char *file) {
  ds_status status = DS_SUCCESS;
  char *contentStart;
  size_t contentSize;

  if (profile == nullptr)
    return DS_INVALID_PROFILE;

  status = readProFile(file, &contentStart, &contentSize);
  if (status == DS_SUCCESS) {
    const char *currentPosition;
    const char *dataStart;
    const char *dataEnd;

    const char *contentEnd = contentStart + contentSize;
    currentPosition = contentStart;

    // parse the version string
    dataStart = findString(currentPosition, contentEnd, DS_TAG_VERSION);
    if (dataStart == nullptr) {
      status = DS_PROFILE_FILE_ERROR;
      goto cleanup;
    }
    dataStart += strlen(DS_TAG_VERSION);

    dataEnd = findString(dataStart, contentEnd, DS_TAG_VERSION_END);
    if (dataEnd == nullptr) {
      status = DS_PROFILE_FILE_ERROR;
      goto cleanup;
    }

    size_t versionStringLength = strlen(profile->version);
    if (versionStringLength + dataStart != dataEnd ||
        strncmp(profile->version, dataStart, versionStringLength) != 0) {
      // version mismatch
      status = DS_PROFILE_FILE_ERROR;
      goto cleanup;
    }
    currentPosition = dataEnd + strlen(DS_TAG_VERSION_END);

    // parse the device information
    while (1) {
      unsigned int i;

      const char *deviceTypeStart;
      const char *deviceTypeEnd;
      ds_device_type deviceType;

      const char *deviceNameStart;
      const char *deviceNameEnd;

      const char *deviceScoreStart;
      const char *deviceScoreEnd;

      const char *deviceDriverStart;
      const char *deviceDriverEnd;

      dataStart = findString(currentPosition, contentEnd, DS_TAG_DEVICE);
      if (dataStart == nullptr) {
        // nothing useful remain, quit...
        break;
      }
      dataStart += strlen(DS_TAG_DEVICE);
      dataEnd = findString(dataStart, contentEnd, DS_TAG_DEVICE_END);
      if (dataEnd == nullptr) {
        status = DS_PROFILE_FILE_ERROR;
        goto cleanup;
      }

      // parse the device type
      deviceTypeStart = findString(dataStart, contentEnd, DS_TAG_DEVICE_TYPE);
      if (deviceTypeStart == nullptr) {
        status = DS_PROFILE_FILE_ERROR;
        goto cleanup;
      }
      deviceTypeStart += strlen(DS_TAG_DEVICE_TYPE);
      deviceTypeEnd = findString(deviceTypeStart, contentEnd, DS_TAG_DEVICE_TYPE_END);
      if (deviceTypeEnd == nullptr) {
        status = DS_PROFILE_FILE_ERROR;
        goto cleanup;
      }
      memcpy(&deviceType, deviceTypeStart, sizeof(ds_device_type));

      // parse the device name
      if (deviceType == DS_DEVICE_OPENCL_DEVICE) {
        deviceNameStart = findString(dataStart, contentEnd, DS_TAG_DEVICE_NAME);
        if (deviceNameStart == nullptr) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;
        }
        deviceNameStart += strlen(DS_TAG_DEVICE_NAME);
        deviceNameEnd = findString(deviceNameStart, contentEnd, DS_TAG_DEVICE_NAME_END);
        if (deviceNameEnd == nullptr) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;
        }

        deviceDriverStart = findString(dataStart, contentEnd, DS_TAG_DEVICE_DRIVER_VERSION);
        if (deviceDriverStart == nullptr) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;
        }
        deviceDriverStart += strlen(DS_TAG_DEVICE_DRIVER_VERSION);
        deviceDriverEnd =
            findString(deviceDriverStart, contentEnd, DS_TAG_DEVICE_DRIVER_VERSION_END);
        if (deviceDriverEnd == nullptr) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;
        }

        // check if this device is on the system
        for (i = 0; i < profile->numDevices; i++) {
          if (profile->devices[i].type == DS_DEVICE_OPENCL_DEVICE) {
            size_t actualDeviceNameLength;
            size_t driverVersionLength;

            actualDeviceNameLength = strlen(profile->devices[i].oclDeviceName);
            driverVersionLength = strlen(profile->devices[i].oclDriverVersion);
            if (deviceNameStart + actualDeviceNameLength == deviceNameEnd &&
                deviceDriverStart + driverVersionLength == deviceDriverEnd &&
                strncmp(profile->devices[i].oclDeviceName, deviceNameStart,
                        actualDeviceNameLength) == 0 &&
                strncmp(profile->devices[i].oclDriverVersion, deviceDriverStart,
                        driverVersionLength) == 0) {
              deviceScoreStart = findString(dataStart, contentEnd, DS_TAG_SCORE);
              deviceScoreStart += strlen(DS_TAG_SCORE);
              deviceScoreEnd = findString(deviceScoreStart, contentEnd, DS_TAG_SCORE_END);
              status = deserializer(&profile->devices[i], (const unsigned char *)deviceScoreStart,
                                    deviceScoreEnd - deviceScoreStart);
              if (status != DS_SUCCESS) {
                goto cleanup;
              }
            }
          }
        }
      } else if (deviceType == DS_DEVICE_NATIVE_CPU) {
        for (i = 0; i < profile->numDevices; i++) {
          if (profile->devices[i].type == DS_DEVICE_NATIVE_CPU) {
            deviceScoreStart = findString(dataStart, contentEnd, DS_TAG_SCORE);
            if (deviceScoreStart == nullptr) {
              status = DS_PROFILE_FILE_ERROR;
              goto cleanup;
            }
            deviceScoreStart += strlen(DS_TAG_SCORE);
            deviceScoreEnd = findString(deviceScoreStart, contentEnd, DS_TAG_SCORE_END);
            status = deserializer(&profile->devices[i], (const unsigned char *)deviceScoreStart,
                                  deviceScoreEnd - deviceScoreStart);
            if (status != DS_SUCCESS) {
              goto cleanup;
            }
          }
        }
      }

      // skip over the current one to find the next device
      currentPosition = dataEnd + strlen(DS_TAG_DEVICE_END);
    }
  }
cleanup:
  delete[] contentStart;
  return status;
}

typedef ds_status (*ds_score_serializer)(ds_device *device, uint8_t **serializedScore,
                                         unsigned int *serializedScoreSize);
static ds_status writeProfileToFile(ds_profile *profile, ds_score_serializer serializer,
                                    const char *file) {
  ds_status status = DS_SUCCESS;

  if (profile == nullptr)
    return DS_INVALID_PROFILE;

  FILE *profileFile = fopen(file, "wb");
  if (profileFile == nullptr) {
    status = DS_FILE_ERROR;
  } else {
    unsigned int i;

    // write version string
    fwrite(DS_TAG_VERSION, sizeof(char), strlen(DS_TAG_VERSION), profileFile);
    fwrite(profile->version, sizeof(char), strlen(profile->version), profileFile);
    fwrite(DS_TAG_VERSION_END, sizeof(char), strlen(DS_TAG_VERSION_END), profileFile);
    fwrite("\n", sizeof(char), 1, profileFile);

    for (i = 0; i < profile->numDevices && status == DS_SUCCESS; i++) {
      uint8_t *serializedScore;
      unsigned int serializedScoreSize;

      fwrite(DS_TAG_DEVICE, sizeof(char), strlen(DS_TAG_DEVICE), profileFile);

      fwrite(DS_TAG_DEVICE_TYPE, sizeof(char), strlen(DS_TAG_DEVICE_TYPE), profileFile);
      fwrite(&profile->devices[i].type, sizeof(ds_device_type), 1, profileFile);
      fwrite(DS_TAG_DEVICE_TYPE_END, sizeof(char), strlen(DS_TAG_DEVICE_TYPE_END), profileFile);

      switch (profile->devices[i].type) {
        case DS_DEVICE_NATIVE_CPU: {
          // There's no need to emit a device name for the native CPU device.
          /*
fwrite(DS_TAG_DEVICE_NAME, sizeof(char), strlen(DS_TAG_DEVICE_NAME),
       profileFile);
fwrite(DS_DEVICE_NATIVE_CPU_STRING,sizeof(char),
       strlen(DS_DEVICE_NATIVE_CPU_STRING), profileFile);
fwrite(DS_TAG_DEVICE_NAME_END, sizeof(char),
       strlen(DS_TAG_DEVICE_NAME_END), profileFile);
*/
        } break;
        case DS_DEVICE_OPENCL_DEVICE: {
          fwrite(DS_TAG_DEVICE_NAME, sizeof(char), strlen(DS_TAG_DEVICE_NAME), profileFile);
          fwrite(profile->devices[i].oclDeviceName, sizeof(char),
                 strlen(profile->devices[i].oclDeviceName), profileFile);
          fwrite(DS_TAG_DEVICE_NAME_END, sizeof(char), strlen(DS_TAG_DEVICE_NAME_END), profileFile);

          fwrite(DS_TAG_DEVICE_DRIVER_VERSION, sizeof(char), strlen(DS_TAG_DEVICE_DRIVER_VERSION),
                 profileFile);
          fwrite(profile->devices[i].oclDriverVersion, sizeof(char),
                 strlen(profile->devices[i].oclDriverVersion), profileFile);
          fwrite(DS_TAG_DEVICE_DRIVER_VERSION_END, sizeof(char),
                 strlen(DS_TAG_DEVICE_DRIVER_VERSION_END), profileFile);
        } break;
        default:
          status = DS_UNKNOWN_DEVICE_TYPE;
          continue;
      };

      fwrite(DS_TAG_SCORE, sizeof(char), strlen(DS_TAG_SCORE), profileFile);
      status = serializer(&profile->devices[i], &serializedScore, &serializedScoreSize);
      if (status == DS_SUCCESS && serializedScore != nullptr && serializedScoreSize > 0) {
        fwrite(serializedScore, sizeof(char), serializedScoreSize, profileFile);
        delete[] serializedScore;
      }
      fwrite(DS_TAG_SCORE_END, sizeof(char), strlen(DS_TAG_SCORE_END), profileFile);
      fwrite(DS_TAG_DEVICE_END, sizeof(char), strlen(DS_TAG_DEVICE_END), profileFile);
      fwrite("\n", sizeof(char), 1, profileFile);
    }
    fclose(profileFile);
  }
  return status;
}

// substitute invalid characters in device name with _
static void legalizeFileName(char *fileName) {
  // tprintf("fileName: %s\n", fileName);
  const char *invalidChars = "/\?:*\"><| "; // space is valid but can cause headaches
  // for each invalid char
  for (unsigned i = 0; i < strlen(invalidChars); i++) {
    char invalidStr[4];
    invalidStr[0] = invalidChars[i];
    invalidStr[1] = '\0';
    // tprintf("eliminating %s\n", invalidStr);
    // char *pos = strstr(fileName, invalidStr);
    // initial ./ is valid for present directory
    // if (*pos == '.') pos++;
    // if (*pos == '/') pos++;
    for (char *pos = strstr(fileName, invalidStr); pos != nullptr;
         pos = strstr(pos + 1, invalidStr)) {
      // tprintf("\tfound: %s, ", pos);
      pos[0] = '_';
      // tprintf("fileName: %s\n", fileName);
    }
  }
}

static void populateGPUEnvFromDevice(GPUEnv *gpuInfo, cl_device_id device) {
  // tprintf("[DS] populateGPUEnvFromDevice\n");
  size_t size;
  gpuInfo->mnIsUserCreated = 1;
  // device
  gpuInfo->mpDevID = device;
  gpuInfo->mpArryDevsID = new cl_device_id[1];
  gpuInfo->mpArryDevsID[0] = gpuInfo->mpDevID;
  clStatus = clGetDeviceInfo(gpuInfo->mpDevID, CL_DEVICE_TYPE, sizeof(cl_device_type),
                             &gpuInfo->mDevType, &size);
  CHECK_OPENCL(clStatus, "populateGPUEnv::getDeviceInfo(TYPE)");
  // platform
  clStatus = clGetDeviceInfo(gpuInfo->mpDevID, CL_DEVICE_PLATFORM, sizeof(cl_platform_id),
                             &gpuInfo->mpPlatformID, &size);
  CHECK_OPENCL(clStatus, "populateGPUEnv::getDeviceInfo(PLATFORM)");
  // context
  cl_context_properties props[3];
  props[0] = CL_CONTEXT_PLATFORM;
  props[1] = (cl_context_properties)gpuInfo->mpPlatformID;
  props[2] = 0;
  gpuInfo->mpContext = clCreateContext(props, 1, &gpuInfo->mpDevID, nullptr, nullptr, &clStatus);
  CHECK_OPENCL(clStatus, "populateGPUEnv::createContext");
  // queue
  cl_command_queue_properties queueProperties = 0;
  gpuInfo->mpCmdQueue =
      clCreateCommandQueue(gpuInfo->mpContext, gpuInfo->mpDevID, queueProperties, &clStatus);
  CHECK_OPENCL(clStatus, "populateGPUEnv::createCommandQueue");
}

int OpenclDevice::LoadOpencl() {
#  ifdef WIN32
  HINSTANCE HOpenclDll = nullptr;
  void *OpenclDll = nullptr;
  // fprintf(stderr, " LoadOpenclDllxx... \n");
  OpenclDll = static_cast<HINSTANCE>(HOpenclDll);
  OpenclDll = LoadLibrary("openCL.dll");
  if (!static_cast<HINSTANCE>(OpenclDll)) {
    fprintf(stderr, "[OD] Load opencl.dll failed!\n");
    FreeLibrary(static_cast<HINSTANCE>(OpenclDll));
    return 0;
  }
  fprintf(stderr, "[OD] Load opencl.dll successful!\n");
#  endif
  return 1;
}
int OpenclDevice::SetKernelEnv(KernelEnv *envInfo) {
  envInfo->mpkContext = gpuEnv.mpContext;
  envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
  envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];

  return 1;
}

static cl_mem allocateZeroCopyBuffer(const KernelEnv &rEnv, l_uint32 *hostbuffer, size_t nElements,
                                     cl_mem_flags flags, cl_int *pStatus) {
  cl_mem membuffer = clCreateBuffer(rEnv.mpkContext, (cl_mem_flags)(flags),
                                    nElements * sizeof(l_uint32), hostbuffer, pStatus);

  return membuffer;
}

static Image mapOutputCLBuffer(const KernelEnv &rEnv, cl_mem clbuffer, Image pixd, Image pixs,
                              int elements, cl_mem_flags flags, bool memcopy = false,
                              bool sync = true) {
  if (!pixd) {
    if (memcopy) {
      if ((pixd = pixCreateTemplate(pixs)) == nullptr)
        tprintf("pixd not made\n");
    } else {
      if ((pixd = pixCreateHeader(pixGetWidth(pixs), pixGetHeight(pixs), pixGetDepth(pixs))) ==
          nullptr)
        tprintf("pixd not made\n");
    }
  }
  l_uint32 *pValues =
      (l_uint32 *)clEnqueueMapBuffer(rEnv.mpkCmdQueue, clbuffer, CL_TRUE, flags, 0,
                                     elements * sizeof(l_uint32), 0, nullptr, nullptr, nullptr);

  if (memcopy) {
    memcpy(pixGetData(pixd), pValues, elements * sizeof(l_uint32));
  } else {
    pixSetData(pixd, pValues);
  }

  clEnqueueUnmapMemObject(rEnv.mpkCmdQueue, clbuffer, pValues, 0, nullptr, nullptr);

  if (sync) {
    clFinish(rEnv.mpkCmdQueue);
  }

  return pixd;
}

void OpenclDevice::releaseMorphCLBuffers() {
  if (pixdCLIntermediate != nullptr)
    clReleaseMemObject(pixdCLIntermediate);
  if (pixsCLBuffer != nullptr)
    clReleaseMemObject(pixsCLBuffer);
  if (pixdCLBuffer != nullptr)
    clReleaseMemObject(pixdCLBuffer);
  if (pixThBuffer != nullptr)
    clReleaseMemObject(pixThBuffer);
  pixdCLIntermediate = pixsCLBuffer = pixdCLBuffer = pixThBuffer = nullptr;
}

int OpenclDevice::initMorphCLAllocations(l_int32 wpl, l_int32 h, Image pixs) {
  SetKernelEnv(&rEnv);

  if (pixThBuffer != nullptr) {
    pixsCLBuffer = allocateZeroCopyBuffer(rEnv, nullptr, wpl * h, CL_MEM_ALLOC_HOST_PTR, &clStatus);

    // Get the output from ThresholdToPix operation
    clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixThBuffer, pixsCLBuffer, 0, 0,
                                   sizeof(l_uint32) * wpl * h, 0, nullptr, nullptr);
  } else {
    // Get data from the source image
    l_uint32 *srcdata = reinterpret_cast<l_uint32 *>(malloc(wpl * h * sizeof(l_uint32)));
    memcpy(srcdata, pixGetData(pixs), wpl * h * sizeof(l_uint32));

    pixsCLBuffer = allocateZeroCopyBuffer(rEnv, srcdata, wpl * h, CL_MEM_USE_HOST_PTR, &clStatus);
  }

  pixdCLBuffer = allocateZeroCopyBuffer(rEnv, nullptr, wpl * h, CL_MEM_ALLOC_HOST_PTR, &clStatus);

  pixdCLIntermediate =
      allocateZeroCopyBuffer(rEnv, nullptr, wpl * h, CL_MEM_ALLOC_HOST_PTR, &clStatus);

  return (int)clStatus;
}

int OpenclDevice::InitEnv() {
//    tprintf("[OD] OpenclDevice::InitEnv()\n");
#  ifdef SAL_WIN32
  while (1) {
    if (1 == LoadOpencl())
      break;
  }
#  endif
  // sets up environment, compiles programs

  InitOpenclRunEnv_DeviceSelection(0);
  return 1;
}

int OpenclDevice::ReleaseOpenclRunEnv() {
  ReleaseOpenclEnv(&gpuEnv);
#  ifdef SAL_WIN32
  FreeOpenclDll();
#  endif
  return 1;
}

inline int OpenclDevice::AddKernelConfig(int kCount, const char *kName) {
  ASSERT_HOST(kCount > 0);
  ASSERT_HOST(strlen(kName) < sizeof(gpuEnv.mArrykernelNames[kCount - 1]));
  strcpy(gpuEnv.mArrykernelNames[kCount - 1], kName);
  gpuEnv.mnKernelCount++;
  return 0;
}

int OpenclDevice::RegistOpenclKernel() {
  if (!gpuEnv.mnIsUserCreated)
    memset(&gpuEnv, 0, sizeof(gpuEnv));

  gpuEnv.mnFileCount = 0; // argc;
  gpuEnv.mnKernelCount = 0UL;

  AddKernelConfig(1, "oclAverageSub1");
  return 0;
}

int OpenclDevice::InitOpenclRunEnv_DeviceSelection(int argc) {
  if (!isInited) {
    // after programs compiled, selects best device
    ds_device bestDevice_DS = getDeviceSelection();
    cl_device_id bestDevice = bestDevice_DS.oclDeviceID;
    // overwrite global static GPUEnv with new device
    if (selectedDeviceIsOpenCL()) {
      // tprintf("[DS] InitOpenclRunEnv_DS::Calling populateGPUEnvFromDevice()
      // for selected device\n");
      populateGPUEnvFromDevice(&gpuEnv, bestDevice);
      gpuEnv.mnFileCount = 0; // argc;
      gpuEnv.mnKernelCount = 0UL;
      CompileKernelFile(&gpuEnv, "");
    } else {
      // tprintf("[DS] InitOpenclRunEnv_DS::Skipping populateGPUEnvFromDevice()
      // b/c native cpu selected\n");
    }
    isInited = 1;
  }
  return 0;
}

OpenclDevice::OpenclDevice() {
  // InitEnv();
}

OpenclDevice::~OpenclDevice() {
  // ReleaseOpenclRunEnv();
}

int OpenclDevice::ReleaseOpenclEnv(GPUEnv *gpuInfo) {
  int i = 0;
  int clStatus = 0;

  if (!isInited) {
    return 1;
  }

  for (i = 0; i < gpuEnv.mnFileCount; i++) {
    if (gpuEnv.mpArryPrograms[i]) {
      clStatus = clReleaseProgram(gpuEnv.mpArryPrograms[i]);
      CHECK_OPENCL(clStatus, "clReleaseProgram");
      gpuEnv.mpArryPrograms[i] = nullptr;
    }
  }
  if (gpuEnv.mpCmdQueue) {
    clReleaseCommandQueue(gpuEnv.mpCmdQueue);
    gpuEnv.mpCmdQueue = nullptr;
  }
  if (gpuEnv.mpContext) {
    clReleaseContext(gpuEnv.mpContext);
    gpuEnv.mpContext = nullptr;
  }
  isInited = 0;
  gpuInfo->mnIsUserCreated = 0;
  delete[] gpuInfo->mpArryDevsID;
  return 1;
}
int OpenclDevice::BinaryGenerated(const char *clFileName, FILE **fhandle) {
  unsigned int i = 0;
  cl_int clStatus;
  int status = 0;
  FILE *fd = nullptr;
  char fileName[256];
  char cl_name[128];
  char deviceName[1024];
  clStatus = clGetDeviceInfo(gpuEnv.mpArryDevsID[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName,
                             nullptr);
  CHECK_OPENCL(clStatus, "clGetDeviceInfo");
  const char *str = strstr(clFileName, ".cl");
  memcpy(cl_name, clFileName, str - clFileName);
  cl_name[str - clFileName] = '\0';
  snprintf(fileName, sizeof(fileName), "%s-%s.bin", cl_name, deviceName);
  legalizeFileName(fileName);
  fd = fopen(fileName, "rb");
  status = (fd != nullptr) ? 1 : 0;
  if (fd != nullptr) {
    *fhandle = fd;
  }
  return status;
}
int OpenclDevice::CachedOfKernerPrg(const GPUEnv *gpuEnvCached, const char *clFileName) {
  int i;
  for (i = 0; i < gpuEnvCached->mnFileCount; i++) {
    if (strcasecmp(gpuEnvCached->mArryKnelSrcFile[i], clFileName) == 0) {
      if (gpuEnvCached->mpArryPrograms[i] != nullptr) {
        return 1;
      }
    }
  }

  return 0;
}
int OpenclDevice::WriteBinaryToFile(const char *fileName, const char *birary, size_t numBytes) {
  FILE *output = nullptr;
  output = fopen(fileName, "wb");
  if (output == nullptr) {
    return 0;
  }

  fwrite(birary, sizeof(char), numBytes, output);
  fclose(output);

  return 1;
}

int OpenclDevice::GeneratBinFromKernelSource(cl_program program, const char *clFileName) {
  unsigned int i = 0;
  cl_int clStatus;
  cl_uint numDevices;

  clStatus =
      clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(numDevices), &numDevices, nullptr);
  CHECK_OPENCL(clStatus, "clGetProgramInfo");

  std::vector<cl_device_id> mpArryDevsID(numDevices);

  /* grab the handles to all of the devices in the program. */
  clStatus = clGetProgramInfo(program, CL_PROGRAM_DEVICES, sizeof(cl_device_id) * numDevices,
                              &mpArryDevsID[0], nullptr);
  CHECK_OPENCL(clStatus, "clGetProgramInfo");

  /* figure out the sizes of each of the binaries. */
  std::vector<size_t> binarySizes(numDevices);

  clStatus = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * numDevices,
                              &binarySizes[0], nullptr);
  CHECK_OPENCL(clStatus, "clGetProgramInfo");

  /* copy over all of the generated binaries. */
  std::vector<char *> binaries(numDevices);

  for (i = 0; i < numDevices; i++) {
    if (binarySizes[i] != 0) {
      binaries[i] = new char[binarySizes[i]];
    } else {
      binaries[i] = nullptr;
    }
  }

  clStatus = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(char *) * numDevices,
                              &binaries[0], nullptr);
  CHECK_OPENCL(clStatus, "clGetProgramInfo");

  /* dump out each binary into its own separate file. */
  for (i = 0; i < numDevices; i++) {
    if (binarySizes[i] != 0) {
      char fileName[256];
      char cl_name[128];
      char deviceName[1024];
      clStatus =
          clGetDeviceInfo(mpArryDevsID[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, nullptr);
      CHECK_OPENCL(clStatus, "clGetDeviceInfo");

      const char *str = strstr(clFileName, ".cl");
      memcpy(cl_name, clFileName, str - clFileName);
      cl_name[str - clFileName] = '\0';
      snprintf(fileName, sizeof(fileName), "%s-%s.bin", cl_name, deviceName);
      legalizeFileName(fileName);
      if (!WriteBinaryToFile(fileName, binaries[i], binarySizes[i])) {
        tprintf("[OD] write binary[%s] failed\n", fileName);
        return 0;
      } // else
      tprintf("[OD] write binary[%s] successfully\n", fileName);
    }
  }

  // Release all resources and memory
  for (i = 0; i < numDevices; i++) {
    delete[] binaries[i];
  }

  return 1;
}

int OpenclDevice::CompileKernelFile(GPUEnv *gpuInfo, const char *buildOption) {
  cl_int clStatus = 0;
  const char *source;
  size_t source_size[1];
  int binary_status, binaryExisted, idx;
  cl_uint numDevices;
  FILE *fd, *fd1;
  const char *filename = "kernel.cl";
  // fprintf(stderr, "[OD] CompileKernelFile ... \n");
  if (CachedOfKernerPrg(gpuInfo, filename) == 1) {
    return 1;
  }

  idx = gpuInfo->mnFileCount;

  source = kernel_src;

  source_size[0] = strlen(source);
  binaryExisted = 0;
  binaryExisted = BinaryGenerated(filename, &fd); // don't check for binary during microbenchmark
  if (binaryExisted == 1) {
    clStatus = clGetContextInfo(gpuInfo->mpContext, CL_CONTEXT_NUM_DEVICES, sizeof(numDevices),
                                &numDevices, nullptr);
    CHECK_OPENCL(clStatus, "clGetContextInfo");

    std::vector<cl_device_id> mpArryDevsID(numDevices);
    bool b_error = fseek(fd, 0, SEEK_END) < 0;
    auto pos = std::ftell(fd);
    b_error |= (pos <= 0);
    size_t length = pos;
    b_error |= fseek(fd, 0, SEEK_SET) < 0;
    if (b_error) {
      fclose(fd);
      return 0;
    }

    std::vector<uint8_t> binary(length + 2);

    memset(&binary[0], 0, length + 2);
    b_error |= fread(&binary[0], 1, length, fd) != length;

    fclose(fd);
    fd = nullptr;
    // grab the handles to all of the devices in the context.
    clStatus = clGetContextInfo(gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                                sizeof(cl_device_id) * numDevices, &mpArryDevsID[0], nullptr);
    CHECK_OPENCL(clStatus, "clGetContextInfo");
    // fprintf(stderr, "[OD] Create kernel from binary\n");
    const uint8_t *c_binary = &binary[0];
    gpuInfo->mpArryPrograms[idx] =
        clCreateProgramWithBinary(gpuInfo->mpContext, numDevices, &mpArryDevsID[0], &length,
                                  &c_binary, &binary_status, &clStatus);
    CHECK_OPENCL(clStatus, "clCreateProgramWithBinary");
  } else {
    // create a CL program using the kernel source
    // fprintf(stderr, "[OD] Create kernel from source\n");
    gpuInfo->mpArryPrograms[idx] =
        clCreateProgramWithSource(gpuInfo->mpContext, 1, &source, source_size, &clStatus);
    CHECK_OPENCL(clStatus, "clCreateProgramWithSource");
  }

  if (gpuInfo->mpArryPrograms[idx] == (cl_program) nullptr) {
    return 0;
  }

  // char options[512];
  // create a cl program executable for all the devices specified
  // tprintf("[OD] BuildProgram.\n");
  if (!gpuInfo->mnIsUserCreated) {
    clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, gpuInfo->mpArryDevsID, buildOption,
                              nullptr, nullptr);
  } else {
    clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, &(gpuInfo->mpDevID), buildOption,
                              nullptr, nullptr);
  }
  if (clStatus != CL_SUCCESS) {
    tprintf("BuildProgram error!\n");
    size_t length;
    if (!gpuInfo->mnIsUserCreated) {
      clStatus = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                                       CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
    } else {
      clStatus = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                                       CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
    }
    if (clStatus != CL_SUCCESS) {
      tprintf("opencl create build log fail\n");
      return 0;
    }
    std::vector<char> buildLog(length);
    if (!gpuInfo->mnIsUserCreated) {
      clStatus = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                                       CL_PROGRAM_BUILD_LOG, length, &buildLog[0], &length);
    } else {
      clStatus = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                                       CL_PROGRAM_BUILD_LOG, length, &buildLog[0], &length);
    }
    if (clStatus != CL_SUCCESS) {
      tprintf("opencl program build info fail\n");
      return 0;
    }

    fd1 = fopen("kernel-build.log", "w+");
    if (fd1 != nullptr) {
      fwrite(&buildLog[0], sizeof(char), length, fd1);
      fclose(fd1);
    }

    return 0;
  }

  strcpy(gpuInfo->mArryKnelSrcFile[idx], filename);
  if (binaryExisted == 0) {
    GeneratBinFromKernelSource(gpuInfo->mpArryPrograms[idx], filename);
  }

  gpuInfo->mnFileCount += 1;
  return 1;
}

l_uint32 *OpenclDevice::pixReadFromTiffKernel(l_uint32 *tiffdata, l_int32 w, l_int32 h, l_int32 wpl,
                                              l_uint32 *line) {
  cl_int clStatus;
  KernelEnv rEnv;
  size_t globalThreads[2];
  size_t localThreads[2];
  int gsize;
  cl_mem valuesCl;
  cl_mem outputCl;

  // global and local work dimensions for Horizontal pass
  gsize = (w + GROUPSIZE_X - 1) / GROUPSIZE_X * GROUPSIZE_X;
  globalThreads[0] = gsize;
  gsize = (h + GROUPSIZE_Y - 1) / GROUPSIZE_Y * GROUPSIZE_Y;
  globalThreads[1] = gsize;
  localThreads[0] = GROUPSIZE_X;
  localThreads[1] = GROUPSIZE_Y;

  SetKernelEnv(&rEnv);

  l_uint32 *pResult = (l_uint32 *)malloc(w * h * sizeof(l_uint32));
  rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "composeRGBPixel", &clStatus);
  CHECK_OPENCL(clStatus, "clCreateKernel composeRGBPixel");

  // Allocate input and output OCL buffers
  valuesCl = allocateZeroCopyBuffer(rEnv, tiffdata, w * h, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                    &clStatus);
  outputCl = allocateZeroCopyBuffer(rEnv, pResult, w * h, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                    &clStatus);

  // Kernel arguments
  clStatus = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &valuesCl);
  CHECK_OPENCL(clStatus, "clSetKernelArg");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(w), &w);
  CHECK_OPENCL(clStatus, "clSetKernelArg");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(h), &h);
  CHECK_OPENCL(clStatus, "clSetKernelArg");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(wpl), &wpl);
  CHECK_OPENCL(clStatus, "clSetKernelArg");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(cl_mem), &outputCl);
  CHECK_OPENCL(clStatus, "clSetKernelArg");

  // Kernel enqueue
  clStatus = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);
  CHECK_OPENCL(clStatus, "clEnqueueNDRangeKernel");

  /* map results back from gpu */
  void *ptr = clEnqueueMapBuffer(rEnv.mpkCmdQueue, outputCl, CL_TRUE, CL_MAP_READ, 0,
                                 w * h * sizeof(l_uint32), 0, nullptr, nullptr, &clStatus);
  CHECK_OPENCL(clStatus, "clEnqueueMapBuffer outputCl");
  clEnqueueUnmapMemObject(rEnv.mpkCmdQueue, outputCl, ptr, 0, nullptr, nullptr);

  // Sync
  clFinish(rEnv.mpkCmdQueue);
  return pResult;
}

// Morphology Dilate operation for 5x5 structuring element. Invokes the relevant
// OpenCL kernels
static cl_int pixDilateCL_55(l_int32 wpl, l_int32 h) {
  size_t globalThreads[2];
  cl_mem pixtemp;
  cl_int status;
  int gsize;
  size_t localThreads[2];

  // Horizontal pass
  gsize = (wpl * h + GROUPSIZE_HMORX - 1) / GROUPSIZE_HMORX * GROUPSIZE_HMORX;
  globalThreads[0] = gsize;
  globalThreads[1] = GROUPSIZE_HMORY;
  localThreads[0] = GROUPSIZE_HMORX;
  localThreads[1] = GROUPSIZE_HMORY;

  rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoDilateHor_5x5", &status);
  CHECK_OPENCL(status, "clCreateKernel morphoDilateHor_5x5");

  status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(wpl), &wpl);
  status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(h), &h);

  status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                  localThreads, 0, nullptr, nullptr);

  // Swap source and dest buffers
  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixtemp;

  // Vertical
  gsize = (wpl + GROUPSIZE_X - 1) / GROUPSIZE_X * GROUPSIZE_X;
  globalThreads[0] = gsize;
  gsize = (h + GROUPSIZE_Y - 1) / GROUPSIZE_Y * GROUPSIZE_Y;
  globalThreads[1] = gsize;
  localThreads[0] = GROUPSIZE_X;
  localThreads[1] = GROUPSIZE_Y;

  rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoDilateVer_5x5", &status);
  CHECK_OPENCL(status, "clCreateKernel morphoDilateVer_5x5");

  status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(wpl), &wpl);
  status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(h), &h);
  status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                  localThreads, 0, nullptr, nullptr);

  return status;
}

// Morphology Erode operation for 5x5 structuring element. Invokes the relevant
// OpenCL kernels
static cl_int pixErodeCL_55(l_int32 wpl, l_int32 h) {
  size_t globalThreads[2];
  cl_mem pixtemp;
  cl_int status;
  int gsize;
  l_uint32 fwmask, lwmask;
  size_t localThreads[2];

  lwmask = lmask32[31 - 2];
  fwmask = rmask32[31 - 2];

  // Horizontal pass
  gsize = (wpl * h + GROUPSIZE_HMORX - 1) / GROUPSIZE_HMORX * GROUPSIZE_HMORX;
  globalThreads[0] = gsize;
  globalThreads[1] = GROUPSIZE_HMORY;
  localThreads[0] = GROUPSIZE_HMORX;
  localThreads[1] = GROUPSIZE_HMORY;

  rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoErodeHor_5x5", &status);
  CHECK_OPENCL(status, "clCreateKernel morphoErodeHor_5x5");

  status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(wpl), &wpl);
  status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(h), &h);

  status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                  localThreads, 0, nullptr, nullptr);

  // Swap source and dest buffers
  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixtemp;

  // Vertical
  gsize = (wpl + GROUPSIZE_X - 1) / GROUPSIZE_X * GROUPSIZE_X;
  globalThreads[0] = gsize;
  gsize = (h + GROUPSIZE_Y - 1) / GROUPSIZE_Y * GROUPSIZE_Y;
  globalThreads[1] = gsize;
  localThreads[0] = GROUPSIZE_X;
  localThreads[1] = GROUPSIZE_Y;

  rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoErodeVer_5x5", &status);
  CHECK_OPENCL(status, "clCreateKernel morphoErodeVer_5x5");

  status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
  status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(wpl), &wpl);
  status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(h), &h);
  status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(fwmask), &fwmask);
  status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(lwmask), &lwmask);
  status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                  localThreads, 0, nullptr, nullptr);

  return status;
}

// Morphology Dilate operation. Invokes the relevant OpenCL kernels
static cl_int pixDilateCL(l_int32 hsize, l_int32 vsize, l_int32 wpl, l_int32 h) {
  l_int32 xp, yp, xn, yn;
  SEL *sel;
  size_t globalThreads[2];
  cl_mem pixtemp;
  cl_int status = 0;
  int gsize;
  size_t localThreads[2];
  char isEven;

  OpenclDevice::SetKernelEnv(&rEnv);

  if (hsize == 5 && vsize == 5) {
    // Specific case for 5x5
    status = pixDilateCL_55(wpl, h);
    return status;
  }

  sel = selCreateBrick(vsize, hsize, vsize / 2, hsize / 2, SEL_HIT);

  selFindMaxTranslations(sel, &xp, &yp, &xn, &yn);
  selDestroy(&sel);
  // global and local work dimensions for Horizontal pass
  gsize = (wpl + GROUPSIZE_X - 1) / GROUPSIZE_X * GROUPSIZE_X;
  globalThreads[0] = gsize;
  gsize = (h + GROUPSIZE_Y - 1) / GROUPSIZE_Y * GROUPSIZE_Y;
  globalThreads[1] = gsize;
  localThreads[0] = GROUPSIZE_X;
  localThreads[1] = GROUPSIZE_Y;

  if (xp > 31 || xn > 31) {
    // Generic case.
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoDilateHor", &status);
    CHECK_OPENCL(status, "clCreateKernel morphoDilateHor");

    status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(xp), &xp);
    status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(xn), &xn);
    status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(wpl), &wpl);
    status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(h), &h);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);

    if (yp > 0 || yn > 0) {
      pixtemp = pixsCLBuffer;
      pixsCLBuffer = pixdCLBuffer;
      pixdCLBuffer = pixtemp;
    }
  } else if (xp > 0 || xn > 0) {
    // Specific Horizontal pass kernel for half width < 32
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoDilateHor_32word", &status);
    CHECK_OPENCL(status, "clCreateKernel morphoDilateHor_32word");
    isEven = (xp != xn);

    status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(xp), &xp);
    status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(wpl), &wpl);
    status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(h), &h);
    status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(isEven), &isEven);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);

    if (yp > 0 || yn > 0) {
      pixtemp = pixsCLBuffer;
      pixsCLBuffer = pixdCLBuffer;
      pixdCLBuffer = pixtemp;
    }
  }

  if (yp > 0 || yn > 0) {
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoDilateVer", &status);
    CHECK_OPENCL(status, "clCreateKernel morphoDilateVer");

    status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(yp), &yp);
    status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(wpl), &wpl);
    status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(h), &h);
    status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(yn), &yn);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);
  }

  return status;
}

// Morphology Erode operation. Invokes the relevant OpenCL kernels
static cl_int pixErodeCL(l_int32 hsize, l_int32 vsize, l_uint32 wpl, l_uint32 h) {
  l_int32 xp, yp, xn, yn;
  SEL *sel;
  size_t globalThreads[2];
  size_t localThreads[2];
  cl_mem pixtemp;
  cl_int status = 0;
  int gsize;
  char isAsymmetric = (MORPH_BC == ASYMMETRIC_MORPH_BC);
  l_uint32 rwmask, lwmask;
  char isEven;

  sel = selCreateBrick(vsize, hsize, vsize / 2, hsize / 2, SEL_HIT);

  selFindMaxTranslations(sel, &xp, &yp, &xn, &yn);
  selDestroy(&sel);
  OpenclDevice::SetKernelEnv(&rEnv);

  if (hsize == 5 && vsize == 5 && isAsymmetric) {
    // Specific kernel for 5x5
    status = pixErodeCL_55(wpl, h);
    return status;
  }

  lwmask = lmask32[31 - (xn & 31)];
  rwmask = rmask32[31 - (xp & 31)];

  // global and local work dimensions for Horizontal pass
  gsize = (wpl + GROUPSIZE_X - 1) / GROUPSIZE_X * GROUPSIZE_X;
  globalThreads[0] = gsize;
  gsize = (h + GROUPSIZE_Y - 1) / GROUPSIZE_Y * GROUPSIZE_Y;
  globalThreads[1] = gsize;
  localThreads[0] = GROUPSIZE_X;
  localThreads[1] = GROUPSIZE_Y;

  // Horizontal Pass
  if (xp > 31 || xn > 31) {
    // Generic case.
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoErodeHor", &status);

    status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(xp), &xp);
    status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(xn), &xn);
    status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(wpl), &wpl);
    status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(h), &h);
    status = clSetKernelArg(rEnv.mpkKernel, 6, sizeof(isAsymmetric), &isAsymmetric);
    status = clSetKernelArg(rEnv.mpkKernel, 7, sizeof(rwmask), &rwmask);
    status = clSetKernelArg(rEnv.mpkKernel, 8, sizeof(lwmask), &lwmask);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);

    if (yp > 0 || yn > 0) {
      pixtemp = pixsCLBuffer;
      pixsCLBuffer = pixdCLBuffer;
      pixdCLBuffer = pixtemp;
    }
  } else if (xp > 0 || xn > 0) {
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoErodeHor_32word", &status);
    isEven = (xp != xn);

    status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(xp), &xp);
    status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(wpl), &wpl);
    status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(h), &h);
    status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(isAsymmetric), &isAsymmetric);
    status = clSetKernelArg(rEnv.mpkKernel, 6, sizeof(rwmask), &rwmask);
    status = clSetKernelArg(rEnv.mpkKernel, 7, sizeof(lwmask), &lwmask);
    status = clSetKernelArg(rEnv.mpkKernel, 8, sizeof(isEven), &isEven);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);

    if (yp > 0 || yn > 0) {
      pixtemp = pixsCLBuffer;
      pixsCLBuffer = pixdCLBuffer;
      pixdCLBuffer = pixtemp;
    }
  }

  // Vertical Pass
  if (yp > 0 || yn > 0) {
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "morphoErodeVer", &status);
    CHECK_OPENCL(status, "clCreateKernel morphoErodeVer");

    status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(yp), &yp);
    status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(wpl), &wpl);
    status = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(h), &h);
    status = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(isAsymmetric), &isAsymmetric);
    status = clSetKernelArg(rEnv.mpkKernel, 6, sizeof(yn), &yn);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                    localThreads, 0, nullptr, nullptr);
  }

  return status;
}

// Morphology Open operation. Invokes the relevant OpenCL kernels
static cl_int pixOpenCL(l_int32 hsize, l_int32 vsize, l_int32 wpl, l_int32 h) {
  cl_int status;
  cl_mem pixtemp;

  // Erode followed by Dilate
  status = pixErodeCL(hsize, vsize, wpl, h);

  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixtemp;

  status = pixDilateCL(hsize, vsize, wpl, h);

  return status;
}

// Morphology Close operation. Invokes the relevant OpenCL kernels
static cl_int pixCloseCL(l_int32 hsize, l_int32 vsize, l_int32 wpl, l_int32 h) {
  cl_int status;
  cl_mem pixtemp;

  // Dilate followed by Erode
  status = pixDilateCL(hsize, vsize, wpl, h);

  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixtemp;

  status = pixErodeCL(hsize, vsize, wpl, h);

  return status;
}

// output = buffer1 & ~(buffer2)
static cl_int pixSubtractCL_work(l_uint32 wpl, l_uint32 h, cl_mem buffer1, cl_mem buffer2) {
  cl_int status;
  size_t globalThreads[2];
  int gsize;
  size_t localThreads[] = {GROUPSIZE_X, GROUPSIZE_Y};

  gsize = (wpl + GROUPSIZE_X - 1) / GROUPSIZE_X * GROUPSIZE_X;
  globalThreads[0] = gsize;
  gsize = (h + GROUPSIZE_Y - 1) / GROUPSIZE_Y * GROUPSIZE_Y;
  globalThreads[1] = gsize;

  rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "pixSubtract_inplace", &status);
  CHECK_OPENCL(status, "clCreateKernel pixSubtract_inplace");

  // Enqueue a kernel run call.
  status = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &buffer1);
  status = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(cl_mem), &buffer2);
  status = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(wpl), &wpl);
  status = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(h), &h);
  status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, nullptr, globalThreads,
                                  localThreads, 0, nullptr, nullptr);

  return status;
}

// OpenCL implementation of Get Lines from pix function
// Note: Assumes the source and dest opencl buffer are initialized. No check
// done
void OpenclDevice::pixGetLinesCL(Image pixd, Image pixs, Image *pix_vline, Image *pix_hline,
                                 Image *pixClosed, bool getpixClosed, l_int32 close_hsize,
                                 l_int32 close_vsize, l_int32 open_hsize, l_int32 open_vsize,
                                 l_int32 line_hsize, l_int32 line_vsize) {
  l_uint32 wpl, h;
  cl_mem pixtemp;

  wpl = pixGetWpl(pixs);
  h = pixGetHeight(pixs);

  // First step : Close Morph operation: Dilate followed by Erode
  clStatus = pixCloseCL(close_hsize, close_vsize, wpl, h);

  // Copy the Close output to CPU buffer
  if (getpixClosed) {
    *pixClosed =
        mapOutputCLBuffer(rEnv, pixdCLBuffer, *pixClosed, pixs, wpl * h, CL_MAP_READ, true, false);
  }

  // Store the output of close operation in an intermediate buffer
  // this will be later used for pixsubtract
  clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixdCLBuffer, pixdCLIntermediate, 0, 0,
                                 sizeof(int) * wpl * h, 0, nullptr, nullptr);

  // Second step: Open Operation - Erode followed by Dilate
  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixtemp;

  clStatus = pixOpenCL(open_hsize, open_vsize, wpl, h);

  // Third step: Subtract : (Close - Open)
  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixdCLIntermediate;
  pixdCLIntermediate = pixtemp;

  clStatus = pixSubtractCL_work(wpl, h, pixdCLBuffer, pixsCLBuffer);

  // Store the output of Hollow operation in an intermediate buffer
  // this will be later used
  clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixdCLBuffer, pixdCLIntermediate, 0, 0,
                                 sizeof(int) * wpl * h, 0, nullptr, nullptr);

  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLBuffer;
  pixdCLBuffer = pixtemp;

  // Fourth step: Get vertical line
  // pixOpenBrick(nullptr, pix_hollow, 1, min_line_length);
  clStatus = pixOpenCL(1, line_vsize, wpl, h);

  // Copy the vertical line output to CPU buffer
  *pix_vline =
      mapOutputCLBuffer(rEnv, pixdCLBuffer, *pix_vline, pixs, wpl * h, CL_MAP_READ, true, false);

  pixtemp = pixsCLBuffer;
  pixsCLBuffer = pixdCLIntermediate;
  pixdCLIntermediate = pixtemp;

  // Fifth step: Get horizontal line
  // pixOpenBrick(nullptr, pix_hollow, min_line_length, 1);
  clStatus = pixOpenCL(line_hsize, 1, wpl, h);

  // Copy the horizontal line output to CPU buffer
  *pix_hline =
      mapOutputCLBuffer(rEnv, pixdCLBuffer, *pix_hline, pixs, wpl * h, CL_MAP_READ, true, true);

  return;
}

/*************************************************************************
 *  HistogramRect
 *  Otsu Thresholding Operations
 *  histogramAllChannels is laid out as all channel 0, then all channel 1...
 *  only supports 1 or 4 channels (bytes_per_pixel)
 ************************************************************************/
int OpenclDevice::HistogramRectOCL(void *imageData, int bytes_per_pixel, int bytes_per_line,
                                   int left, // always 0
                                   int top,  // always 0
                                   int width, int height, int kHistogramSize,
                                   int *histogramAllChannels) {
  cl_int clStatus;
  int retVal = 0;
  KernelEnv histKern;
  SetKernelEnv(&histKern);
  KernelEnv histRedKern;
  SetKernelEnv(&histRedKern);
  /* map imagedata to device as read only */
  // USE_HOST_PTR uses onion+ bus which is slowest option; also happens to be
  // coherent which we don't need.
  // faster option would be to allocate initial image buffer
  // using a garlic bus memory type
  cl_mem imageBuffer =
      clCreateBuffer(histKern.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     width * height * bytes_per_pixel * sizeof(char), imageData, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer imageBuffer");

  /* setup work group size parameters */
  int block_size = 256;
  cl_uint numCUs;
  clStatus = clGetDeviceInfo(gpuEnv.mpDevID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numCUs), &numCUs,
                             nullptr);
  CHECK_OPENCL(clStatus, "clCreateBuffer imageBuffer");

  int requestedOccupancy = 10;
  int numWorkGroups = numCUs * requestedOccupancy;
  int numThreads = block_size * numWorkGroups;
  size_t local_work_size[] = {static_cast<size_t>(block_size)};
  size_t global_work_size[] = {static_cast<size_t>(numThreads)};
  size_t red_global_work_size[] = {
      static_cast<size_t>(block_size * kHistogramSize * bytes_per_pixel)};

  /* map histogramAllChannels as write only */

  cl_mem histogramBuffer = clCreateBuffer(
      histKern.mpkContext, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
      kHistogramSize * bytes_per_pixel * sizeof(int), histogramAllChannels, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer histogramBuffer");

  /* intermediate histogram buffer */
  int histRed = 256;
  int tmpHistogramBins = kHistogramSize * bytes_per_pixel * histRed;

  cl_mem tmpHistogramBuffer =
      clCreateBuffer(histKern.mpkContext, CL_MEM_READ_WRITE, tmpHistogramBins * sizeof(cl_uint),
                     nullptr, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer tmpHistogramBuffer");

  /* atomic sync buffer */
  int *zeroBuffer = new int[1];
  zeroBuffer[0] = 0;
  cl_mem atomicSyncBuffer =
      clCreateBuffer(histKern.mpkContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int),
                     zeroBuffer, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer atomicSyncBuffer");
  delete[] zeroBuffer;
  // Create kernel objects based on bytes_per_pixel
  if (bytes_per_pixel == 1) {
    histKern.mpkKernel =
        clCreateKernel(histKern.mpkProgram, "kernel_HistogramRectOneChannel", &clStatus);
    CHECK_OPENCL(clStatus, "clCreateKernel kernel_HistogramRectOneChannel");

    histRedKern.mpkKernel = clCreateKernel(histRedKern.mpkProgram,
                                           "kernel_HistogramRectOneChannelReduction", &clStatus);
    CHECK_OPENCL(clStatus, "clCreateKernel kernel_HistogramRectOneChannelReduction");
  } else {
    histKern.mpkKernel =
        clCreateKernel(histKern.mpkProgram, "kernel_HistogramRectAllChannels", &clStatus);
    CHECK_OPENCL(clStatus, "clCreateKernel kernel_HistogramRectAllChannels");

    histRedKern.mpkKernel = clCreateKernel(histRedKern.mpkProgram,
                                           "kernel_HistogramRectAllChannelsReduction", &clStatus);
    CHECK_OPENCL(clStatus, "clCreateKernel kernel_HistogramRectAllChannelsReduction");
  }

  void *ptr;

  // Initialize tmpHistogramBuffer buffer
  ptr = clEnqueueMapBuffer(histKern.mpkCmdQueue, tmpHistogramBuffer, CL_TRUE, CL_MAP_WRITE, 0,
                           tmpHistogramBins * sizeof(cl_uint), 0, nullptr, nullptr, &clStatus);
  CHECK_OPENCL(clStatus, "clEnqueueMapBuffer tmpHistogramBuffer");

  memset(ptr, 0, tmpHistogramBins * sizeof(cl_uint));
  clEnqueueUnmapMemObject(histKern.mpkCmdQueue, tmpHistogramBuffer, ptr, 0, nullptr, nullptr);

  /* set kernel 1 arguments */
  clStatus = clSetKernelArg(histKern.mpkKernel, 0, sizeof(cl_mem), &imageBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg imageBuffer");
  cl_uint numPixels = width * height;
  clStatus = clSetKernelArg(histKern.mpkKernel, 1, sizeof(cl_uint), &numPixels);
  CHECK_OPENCL(clStatus, "clSetKernelArg numPixels");
  clStatus = clSetKernelArg(histKern.mpkKernel, 2, sizeof(cl_mem), &tmpHistogramBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg tmpHistogramBuffer");

  /* set kernel 2 arguments */
  int n = numThreads / bytes_per_pixel;
  clStatus = clSetKernelArg(histRedKern.mpkKernel, 0, sizeof(cl_int), &n);
  CHECK_OPENCL(clStatus, "clSetKernelArg imageBuffer");
  clStatus = clSetKernelArg(histRedKern.mpkKernel, 1, sizeof(cl_mem), &tmpHistogramBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg tmpHistogramBuffer");
  clStatus = clSetKernelArg(histRedKern.mpkKernel, 2, sizeof(cl_mem), &histogramBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg histogramBuffer");

  /* launch histogram */
  clStatus = clEnqueueNDRangeKernel(histKern.mpkCmdQueue, histKern.mpkKernel, 1, nullptr,
                                    global_work_size, local_work_size, 0, nullptr, nullptr);
  CHECK_OPENCL(clStatus, "clEnqueueNDRangeKernel kernel_HistogramRectAllChannels");
  clFinish(histKern.mpkCmdQueue);
  if (clStatus != 0) {
    retVal = -1;
  }
  /* launch histogram */
  clStatus = clEnqueueNDRangeKernel(histRedKern.mpkCmdQueue, histRedKern.mpkKernel, 1, nullptr,
                                    red_global_work_size, local_work_size, 0, nullptr, nullptr);
  CHECK_OPENCL(clStatus, "clEnqueueNDRangeKernel kernel_HistogramRectAllChannelsReduction");
  clFinish(histRedKern.mpkCmdQueue);
  if (clStatus != 0) {
    retVal = -1;
  }

  /* map results back from gpu */
  ptr = clEnqueueMapBuffer(histRedKern.mpkCmdQueue, histogramBuffer, CL_TRUE, CL_MAP_READ, 0,
                           kHistogramSize * bytes_per_pixel * sizeof(int), 0, nullptr, nullptr,
                           &clStatus);
  CHECK_OPENCL(clStatus, "clEnqueueMapBuffer histogramBuffer");
  if (clStatus != 0) {
    retVal = -1;
  }
  clEnqueueUnmapMemObject(histRedKern.mpkCmdQueue, histogramBuffer, ptr, 0, nullptr, nullptr);

  clReleaseMemObject(histogramBuffer);
  clReleaseMemObject(imageBuffer);
  return retVal;
}

/*************************************************************************
 * Threshold the rectangle, taking everything except the image buffer pointer
 * from the class, using thresholds/hi_values to the output IMAGE.
 * only supports 1 or 4 channels
 ************************************************************************/
int OpenclDevice::ThresholdRectToPixOCL(unsigned char *imageData, int bytes_per_pixel,
                                        int bytes_per_line, int *thresholds, int *hi_values,
                                        Image *pix, int height, int width, int top, int left) {
  int retVal = 0;
  /* create pix result buffer */
  *pix = pixCreate(width, height, 1);
  uint32_t *pixData = pixGetData(*pix);
  int wpl = pixGetWpl(*pix);
  int pixSize = wpl * height * sizeof(uint32_t); // number of pixels

  cl_int clStatus;
  KernelEnv rEnv;
  SetKernelEnv(&rEnv);

  /* setup work group size parameters */
  int block_size = 256;
  cl_uint numCUs = 6;
  clStatus = clGetDeviceInfo(gpuEnv.mpDevID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numCUs), &numCUs,
                             nullptr);
  CHECK_OPENCL(clStatus, "clCreateBuffer imageBuffer");

  int requestedOccupancy = 10;
  int numWorkGroups = numCUs * requestedOccupancy;
  int numThreads = block_size * numWorkGroups;
  size_t local_work_size[] = {(size_t)block_size};
  size_t global_work_size[] = {(size_t)numThreads};

  /* map imagedata to device as read only */
  // USE_HOST_PTR uses onion+ bus which is slowest option; also happens to be
  // coherent which we don't need.
  // faster option would be to allocate initial image buffer
  // using a garlic bus memory type
  cl_mem imageBuffer =
      clCreateBuffer(rEnv.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     width * height * bytes_per_pixel * sizeof(char), imageData, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer imageBuffer");

  /* map pix as write only */
  pixThBuffer = clCreateBuffer(rEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, pixSize,
                               pixData, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer pix");

  /* map thresholds and hi_values */
  cl_mem thresholdsBuffer = clCreateBuffer(rEnv.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                           bytes_per_pixel * sizeof(int), thresholds, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer thresholdBuffer");
  cl_mem hiValuesBuffer = clCreateBuffer(rEnv.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                         bytes_per_pixel * sizeof(int), hi_values, &clStatus);
  CHECK_OPENCL(clStatus, "clCreateBuffer hiValuesBuffer");

  /* compile kernel */
  if (bytes_per_pixel == 4) {
    rEnv.mpkKernel = clCreateKernel(rEnv.mpkProgram, "kernel_ThresholdRectToPix", &clStatus);
    CHECK_OPENCL(clStatus, "clCreateKernel kernel_ThresholdRectToPix");
  } else {
    rEnv.mpkKernel =
        clCreateKernel(rEnv.mpkProgram, "kernel_ThresholdRectToPix_OneChan", &clStatus);
    CHECK_OPENCL(clStatus, "clCreateKernel kernel_ThresholdRectToPix_OneChan");
  }

  /* set kernel arguments */
  clStatus = clSetKernelArg(rEnv.mpkKernel, 0, sizeof(cl_mem), &imageBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg imageBuffer");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 1, sizeof(int), &height);
  CHECK_OPENCL(clStatus, "clSetKernelArg height");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 2, sizeof(int), &width);
  CHECK_OPENCL(clStatus, "clSetKernelArg width");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 3, sizeof(int), &wpl);
  CHECK_OPENCL(clStatus, "clSetKernelArg wpl");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 4, sizeof(cl_mem), &thresholdsBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg thresholdsBuffer");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 5, sizeof(cl_mem), &hiValuesBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg hiValuesBuffer");
  clStatus = clSetKernelArg(rEnv.mpkKernel, 6, sizeof(cl_mem), &pixThBuffer);
  CHECK_OPENCL(clStatus, "clSetKernelArg pixThBuffer");

  /* launch kernel & wait */
  clStatus = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue, rEnv.mpkKernel, 1, nullptr, global_work_size,
                                    local_work_size, 0, nullptr, nullptr);
  CHECK_OPENCL(clStatus, "clEnqueueNDRangeKernel kernel_ThresholdRectToPix");
  clFinish(rEnv.mpkCmdQueue);
  if (clStatus != 0) {
    tprintf("Setting return value to -1\n");
    retVal = -1;
  }
  /* map results back from gpu */
  void *ptr = clEnqueueMapBuffer(rEnv.mpkCmdQueue, pixThBuffer, CL_TRUE, CL_MAP_READ, 0, pixSize, 0,
                                 nullptr, nullptr, &clStatus);
  CHECK_OPENCL(clStatus, "clEnqueueMapBuffer histogramBuffer");
  clEnqueueUnmapMemObject(rEnv.mpkCmdQueue, pixThBuffer, ptr, 0, nullptr, nullptr);

  clReleaseMemObject(imageBuffer);
  clReleaseMemObject(thresholdsBuffer);
  clReleaseMemObject(hiValuesBuffer);

  return retVal;
}

/******************************************************************************
 * Data Types for Device Selection
 *****************************************************************************/

struct TessScoreEvaluationInputData {
  int height;
  int width;
  int numChannels;
  unsigned char *imageData;
  Image pix;
};

static void populateTessScoreEvaluationInputData(TessScoreEvaluationInputData *input) {
  srand(1);
  // 8.5x11 inches @ 300dpi rounded to clean multiples
  int height = 3328; // %256
  int width = 2560;  // %512
  int numChannels = 4;
  input->height = height;
  input->width = width;
  input->numChannels = numChannels;
  unsigned char(*imageData4)[4] = (unsigned char(*)[4])malloc(
      height * width * numChannels * sizeof(unsigned char)); // new unsigned char[4][height*width];
  input->imageData = (unsigned char *)&imageData4[0];

  // zero out image
  unsigned char pixelWhite[4] = {0, 0, 0, 255};
  unsigned char pixelBlack[4] = {255, 255, 255, 255};
  for (int p = 0; p < height * width; p++) {
    // unsigned char tmp[4] = imageData4[0];
    imageData4[p][0] = pixelWhite[0];
    imageData4[p][1] = pixelWhite[1];
    imageData4[p][2] = pixelWhite[2];
    imageData4[p][3] = pixelWhite[3];
  }
  // random lines to be eliminated
  int maxLineWidth = 64; // pixels wide
  int numLines = 10;
  // vertical lines
  for (int i = 0; i < numLines; i++) {
    int lineWidth = rand() % maxLineWidth;
    int vertLinePos = lineWidth + rand() % (width - 2 * lineWidth);
    // tprintf("[PI] VerticalLine @ %i (w=%i)\n", vertLinePos, lineWidth);
    for (int row = vertLinePos - lineWidth / 2; row < vertLinePos + lineWidth / 2; row++) {
      for (int col = 0; col < height; col++) {
        // imageData4[row*width+col] = pixelBlack;
        imageData4[row * width + col][0] = pixelBlack[0];
        imageData4[row * width + col][1] = pixelBlack[1];
        imageData4[row * width + col][2] = pixelBlack[2];
        imageData4[row * width + col][3] = pixelBlack[3];
      }
    }
  }
  // horizontal lines
  for (int i = 0; i < numLines; i++) {
    int lineWidth = rand() % maxLineWidth;
    int horLinePos = lineWidth + rand() % (height - 2 * lineWidth);
    // tprintf("[PI] HorizontalLine @ %i (w=%i)\n", horLinePos, lineWidth);
    for (int row = 0; row < width; row++) {
      for (int col = horLinePos - lineWidth / 2; col < horLinePos + lineWidth / 2;
           col++) { // for (int row = vertLinePos-lineWidth/2; row <
                    // vertLinePos+lineWidth/2; row++) {
        // tprintf("[PI] HoizLine pix @ (%3i, %3i)\n", row, col);
        // imageData4[row*width+col] = pixelBlack;
        imageData4[row * width + col][0] = pixelBlack[0];
        imageData4[row * width + col][1] = pixelBlack[1];
        imageData4[row * width + col][2] = pixelBlack[2];
        imageData4[row * width + col][3] = pixelBlack[3];
      }
    }
  }
  // spots (noise, squares)
  float fractionBlack = 0.1; // how much of the image should be blackened
  int numSpots = (height * width) * fractionBlack / (maxLineWidth * maxLineWidth / 2 / 2);
  for (int i = 0; i < numSpots; i++) {
    int lineWidth = rand() % maxLineWidth;
    int col = lineWidth + rand() % (width - 2 * lineWidth);
    int row = lineWidth + rand() % (height - 2 * lineWidth);
    // tprintf("[PI] Spot[%i/%i] @ (%3i, %3i)\n", i, numSpots, row, col );
    for (int r = row - lineWidth / 2; r < row + lineWidth / 2; r++) {
      for (int c = col - lineWidth / 2; c < col + lineWidth / 2; c++) {
        // tprintf("[PI] \tSpot[%i/%i] @ (%3i, %3i)\n", i, numSpots, r, c );
        // imageData4[row*width+col] = pixelBlack;
        imageData4[r * width + c][0] = pixelBlack[0];
        imageData4[r * width + c][1] = pixelBlack[1];
        imageData4[r * width + c][2] = pixelBlack[2];
        imageData4[r * width + c][3] = pixelBlack[3];
      }
    }
  }

  input->pix = pixCreate(input->width, input->height, 8 * input->numChannels);
}

struct TessDeviceScore {
  float time;   // small time means faster device
  bool clError; // were there any opencl errors
  bool valid;   // was the correct response generated
};

/******************************************************************************
 * Micro Benchmarks for Device Selection
 *****************************************************************************/

static double composeRGBPixelMicroBench(GPUEnv *env, TessScoreEvaluationInputData input,
                                        ds_device_type type) {
  double time = 0;
#  if ON_WINDOWS
  LARGE_INTEGER freq, time_funct_start, time_funct_end;
  QueryPerformanceFrequency(&freq);
#  elif ON_APPLE
  mach_timebase_info_data_t info = {0, 0};
  mach_timebase_info(&info);
  long long start, stop;
#  else
  timespec time_funct_start, time_funct_end;
#  endif
  // input data
  l_uint32 *tiffdata = (l_uint32 *)input.imageData; // same size and random data; data doesn't
                                                    // change workload

  // function call
  if (type == DS_DEVICE_OPENCL_DEVICE) {
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif

    OpenclDevice::gpuEnv = *env;
    int wpl = pixGetWpl(input.pix);
    OpenclDevice::pixReadFromTiffKernel(tiffdata, input.width, input.height, wpl, nullptr);
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif

  } else {
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif
    Image pix = pixCreate(input.width, input.height, 32);
    l_uint32 *pixData = pixGetData(pix);
    int i, j;
    int idx = 0;
    for (i = 0; i < input.height; i++) {
      for (j = 0; j < input.width; j++) {
        l_uint32 tiffword = tiffdata[i * input.width + j];
        l_int32 rval = ((tiffword)&0xff);
        l_int32 gval = (((tiffword) >> 8) & 0xff);
        l_int32 bval = (((tiffword) >> 16) & 0xff);
        l_uint32 value = (rval << 24) | (gval << 16) | (bval << 8);
        pixData[idx] = value;
        idx++;
      }
    }
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
    pix.destroy();
  }

  return time;
}

static double histogramRectMicroBench(GPUEnv *env, TessScoreEvaluationInputData input,
                                      ds_device_type type) {
  double time;
#  if ON_WINDOWS
  LARGE_INTEGER freq, time_funct_start, time_funct_end;
  QueryPerformanceFrequency(&freq);
#  elif ON_APPLE
  mach_timebase_info_data_t info = {0, 0};
  mach_timebase_info(&info);
  long long start, stop;
#  else
  timespec time_funct_start, time_funct_end;
#  endif

  const int left = 0;
  const int top = 0;
  int kHistogramSize = 256;
  int bytes_per_line = input.width * input.numChannels;
  int *histogramAllChannels = new int[kHistogramSize * input.numChannels];
  // function call
  if (type == DS_DEVICE_OPENCL_DEVICE) {
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif

    OpenclDevice::gpuEnv = *env;
    int retVal = OpenclDevice::HistogramRectOCL(input.imageData, input.numChannels, bytes_per_line,
                                                left, top, input.width, input.height,
                                                kHistogramSize, histogramAllChannels);

#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    if (retVal == 0) {
      time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
    } else {
      time = FLT_MAX;
    }
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
  } else {
    int *histogram = new int[kHistogramSize];
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif
    for (int ch = 0; ch < input.numChannels; ++ch) {
      tesseract::HistogramRect(input.pix, input.numChannels, left, top, input.width, input.height,
                               histogram);
    }
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
    delete[] histogram;
  }

  // cleanup
  delete[] histogramAllChannels;
  return time;
}

// Reproducing the ThresholdRectToPix native version
static void ThresholdRectToPix_Native(const unsigned char *imagedata, int bytes_per_pixel,
                                      int bytes_per_line, const int *thresholds,
                                      const int *hi_values, Image *pix) {
  int top = 0;
  int left = 0;
  int width = pixGetWidth(*pix);
  int height = pixGetHeight(*pix);

  *pix = pixCreate(width, height, 1);
  uint32_t *pixdata = pixGetData(*pix);
  int wpl = pixGetWpl(*pix);
  const unsigned char *srcdata = imagedata + top * bytes_per_line + left * bytes_per_pixel;
  for (int y = 0; y < height; ++y) {
    const uint8_t *linedata = srcdata;
    uint32_t *pixline = pixdata + y * wpl;
    for (int x = 0; x < width; ++x, linedata += bytes_per_pixel) {
      bool white_result = true;
      for (int ch = 0; ch < bytes_per_pixel; ++ch) {
        if (hi_values[ch] >= 0 && (linedata[ch] > thresholds[ch]) == (hi_values[ch] == 0)) {
          white_result = false;
          break;
        }
      }
      if (white_result)
        CLEAR_DATA_BIT(pixline, x);
      else
        SET_DATA_BIT(pixline, x);
    }
    srcdata += bytes_per_line;
  }
}

static double thresholdRectToPixMicroBench(GPUEnv *env, TessScoreEvaluationInputData input,
                                           ds_device_type type) {
  double time;
#  if ON_WINDOWS
  LARGE_INTEGER freq, time_funct_start, time_funct_end;
  QueryPerformanceFrequency(&freq);
#  elif ON_APPLE
  mach_timebase_info_data_t info = {0, 0};
  mach_timebase_info(&info);
  long long start, stop;
#  else
  timespec time_funct_start, time_funct_end;
#  endif

  // input data
  unsigned char pixelHi = (unsigned char)255;
  int thresholds[4] = {pixelHi, pixelHi, pixelHi, pixelHi};

  // Pix* pix = pixCreate(width, height, 1);
  int top = 0;
  int left = 0;
  int bytes_per_line = input.width * input.numChannels;

  // function call
  if (type == DS_DEVICE_OPENCL_DEVICE) {
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif

    OpenclDevice::gpuEnv = *env;
    int hi_values[4];
    int retVal = OpenclDevice::ThresholdRectToPixOCL(
        input.imageData, input.numChannels, bytes_per_line, thresholds, hi_values, &input.pix,
        input.height, input.width, top, left);

#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    if (retVal == 0) {
      time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
    } else {
      time = FLT_MAX;
    }

#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
  } else {
    tesseract::ImageThresholder thresholder;
    thresholder.SetImage(input.pix);
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif
    int hi_values[4] = {};
    ThresholdRectToPix_Native(input.imageData, input.numChannels, bytes_per_line, thresholds,
                              hi_values, &input.pix);

#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
  }

  return time;
}

static double getLineMasksMorphMicroBench(GPUEnv *env, TessScoreEvaluationInputData input,
                                          ds_device_type type) {
  double time = 0;
#  if ON_WINDOWS
  LARGE_INTEGER freq, time_funct_start, time_funct_end;
  QueryPerformanceFrequency(&freq);
#  elif ON_APPLE
  mach_timebase_info_data_t info = {0, 0};
  mach_timebase_info(&info);
  long long start, stop;
#  else
  timespec time_funct_start, time_funct_end;
#  endif

  // input data
  int resolution = 300;
  int wpl = pixGetWpl(input.pix);
  int kThinLineFraction = 20;     // tess constant
  int kMinLineLengthFraction = 4; // tess constant
  int max_line_width = resolution / kThinLineFraction;
  int min_line_length = resolution / kMinLineLengthFraction;
  int closing_brick = max_line_width / 3;

  // function call
  if (type == DS_DEVICE_OPENCL_DEVICE) {
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif
    OpenclDevice::gpuEnv = *env;
    OpenclDevice::initMorphCLAllocations(wpl, input.height, input.pix);
    Image pix_vline = nullptr, pix_hline = nullptr, pix_closed = nullptr;
    OpenclDevice::pixGetLinesCL(nullptr, input.pix, &pix_vline, &pix_hline, &pix_closed, true,
                                closing_brick, closing_brick, max_line_width, max_line_width,
                                min_line_length, min_line_length);

    OpenclDevice::releaseMorphCLBuffers();

#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
  } else {
#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_start);
#  elif ON_APPLE
    start = mach_absolute_time();
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_start);
#  endif

    // native serial code
    Image src_pix = input.pix;
    Image pix_closed = pixCloseBrick(nullptr, src_pix, closing_brick, closing_brick);
    Image pix_solid = pixOpenBrick(nullptr, pix_closed, max_line_width, max_line_width);
    Image pix_hollow = pixSubtract(nullptr, pix_closed, pix_solid);
    pix_solid.destroy();
    Image pix_vline = pixOpenBrick(nullptr, pix_hollow, 1, min_line_length);
    Image pix_hline = pixOpenBrick(nullptr, pix_hollow, min_line_length, 1);
    pix_hline.destroy();
    pix_vline.destroy();
    pix_hollow.destroy();

#  if ON_WINDOWS
    QueryPerformanceCounter(&time_funct_end);
    time = (time_funct_end.QuadPart - time_funct_start.QuadPart) / (double)(freq.QuadPart);
#  elif ON_APPLE
    stop = mach_absolute_time();
    time = ((stop - start) * (double)info.numer / info.denom) / 1.0E9;
#  else
    clock_gettime(CLOCK_MONOTONIC, &time_funct_end);
    time = (time_funct_end.tv_sec - time_funct_start.tv_sec) * 1.0 +
           (time_funct_end.tv_nsec - time_funct_start.tv_nsec) / 1000000000.0;
#  endif
  }

  return time;
}

/******************************************************************************
 * Device Selection
 *****************************************************************************/

// encode score object as byte string
static ds_status serializeScore(ds_device *device, uint8_t **serializedScore,
                                unsigned int *serializedScoreSize) {
  *serializedScoreSize = sizeof(TessDeviceScore);
  *serializedScore = new uint8_t[*serializedScoreSize];
  memcpy(*serializedScore, device->score, *serializedScoreSize);
  return DS_SUCCESS;
}

// parses byte string and stores in score object
static ds_status deserializeScore(ds_device *device, const uint8_t *serializedScore,
                                  unsigned int serializedScoreSize) {
  // check that serializedScoreSize == sizeof(TessDeviceScore);
  device->score = new TessDeviceScore;
  memcpy(device->score, serializedScore, serializedScoreSize);
  return DS_SUCCESS;
}

static ds_status releaseScore(TessDeviceScore *score) {
  delete score;
  return DS_SUCCESS;
}

// evaluate devices
static ds_status evaluateScoreForDevice(ds_device *device, void *inputData) {
  // overwrite statuc gpuEnv w/ current device
  // so native opencl calls can be used; they use static gpuEnv
  tprintf("\n[DS] Device: \"%s\" (%s) evaluation...\n", device->oclDeviceName,
          device->type == DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native");
  GPUEnv *env = nullptr;
  if (device->type == DS_DEVICE_OPENCL_DEVICE) {
    env = &OpenclDevice::gpuEnv;
    memset(env, 0, sizeof(*env));
    // tprintf("[DS] populating tmp GPUEnv from device\n");
    populateGPUEnvFromDevice(env, device->oclDeviceID);
    env->mnFileCount = 0; // argc;
    env->mnKernelCount = 0UL;
    // tprintf("[DS] compiling kernels for tmp GPUEnv\n");
    OpenclDevice::CompileKernelFile(env, "");
  }

  TessScoreEvaluationInputData *input = static_cast<TessScoreEvaluationInputData *>(inputData);

  // pixReadTiff
  double composeRGBPixelTime = composeRGBPixelMicroBench(env, *input, device->type);

  // HistogramRect
  double histogramRectTime = histogramRectMicroBench(env, *input, device->type);

  // ThresholdRectToPix
  double thresholdRectToPixTime = thresholdRectToPixMicroBench(env, *input, device->type);

  // getLineMasks
  double getLineMasksMorphTime = getLineMasksMorphMicroBench(env, *input, device->type);

  // weigh times (% of cpu time)
  // these weights should be the % execution time that the native cpu code took
  float composeRGBPixelWeight = 1.2f;
  float histogramRectWeight = 2.4f;
  float thresholdRectToPixWeight = 4.5f;
  float getLineMasksMorphWeight = 5.0f;

  float weightedTime = composeRGBPixelWeight * composeRGBPixelTime +
                       histogramRectWeight * histogramRectTime +
                       thresholdRectToPixWeight * thresholdRectToPixTime +
                       getLineMasksMorphWeight * getLineMasksMorphTime;
  device->score = new TessDeviceScore;
  device->score->time = weightedTime;

  tprintf("[DS] Device: \"%s\" (%s) evaluated\n", device->oclDeviceName,
          device->type == DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native");
  tprintf("[DS]%25s: %f (w=%.1f)\n", "composeRGBPixel", composeRGBPixelTime, composeRGBPixelWeight);
  tprintf("[DS]%25s: %f (w=%.1f)\n", "HistogramRect", histogramRectTime, histogramRectWeight);
  tprintf("[DS]%25s: %f (w=%.1f)\n", "ThresholdRectToPix", thresholdRectToPixTime,
          thresholdRectToPixWeight);
  tprintf("[DS]%25s: %f (w=%.1f)\n", "getLineMasksMorph", getLineMasksMorphTime,
          getLineMasksMorphWeight);
  tprintf("[DS]%25s: %f\n", "Score", device->score->time);
  return DS_SUCCESS;
}

// initial call to select device
ds_device OpenclDevice::getDeviceSelection() {
  if (!deviceIsSelected) {
    // check if opencl is available at runtime
    if (1 == LoadOpencl()) {
      // opencl is available
      // setup devices
      ds_status status;
      ds_profile *profile;
      status = initDSProfile(&profile, "v0.1");
      // try reading scores from file
      const char *fileName = "tesseract_opencl_profile_devices.dat";
      status = readProfileFromFile(profile, deserializeScore, fileName);
      if (status != DS_SUCCESS) {
        // need to run evaluation
        tprintf("[DS] Profile file not available (%s); performing profiling.\n", fileName);

        // create input data
        TessScoreEvaluationInputData input;
        populateTessScoreEvaluationInputData(&input);
        // perform evaluations
        unsigned int numUpdates;
        status =
            profileDevices(profile, DS_EVALUATE_ALL, evaluateScoreForDevice, &input, &numUpdates);
        // write scores to file
        if (status == DS_SUCCESS) {
          status = writeProfileToFile(profile, serializeScore, fileName);
          if (status == DS_SUCCESS) {
            tprintf("[DS] Scores written to file (%s).\n", fileName);
          } else {
            tprintf(
                "[DS] Error saving scores to file (%s); scores not written to "
                "file.\n",
                fileName);
          }
        } else {
          tprintf(
              "[DS] Unable to evaluate performance; scores not written to "
              "file.\n");
        }
      } else {
        tprintf("[DS] Profile read from file (%s).\n", fileName);
      }

      // we now have device scores either from file or evaluation
      // select fastest using custom Tesseract selection algorithm
      float bestTime = FLT_MAX; // begin search with worst possible time
      int bestDeviceIdx = -1;
      for (unsigned d = 0; d < profile->numDevices; d++) {
        ds_device device = profile->devices[d];
        if (device.score == nullptr)
          continue;
        TessDeviceScore score = *device.score;

        float time = score.time;
        tprintf("[DS] Device[%u] %i:%s score is %f\n", d + 1, device.type, device.oclDeviceName,
                time);
        if (time < bestTime) {
          bestTime = time;
          bestDeviceIdx = d;
        }
      }
      if (bestDeviceIdx >= 0) {
        tprintf(
            "[DS] Selected Device[%i]: \"%s\" (%s)\n", bestDeviceIdx + 1,
            profile->devices[bestDeviceIdx].oclDeviceName,
            profile->devices[bestDeviceIdx].type == DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native");
      }
      // cleanup
      // TODO: call destructor for profile object?

      bool overridden = false;
      char *overrideDeviceStr = getenv("TESSERACT_OPENCL_DEVICE");
      if (overrideDeviceStr != nullptr) {
        int overrideDeviceIdx = atoi(overrideDeviceStr);
        if (overrideDeviceIdx > 0 && overrideDeviceIdx <= profile->numDevices) {
          tprintf(
              "[DS] Overriding Device Selection (TESSERACT_OPENCL_DEVICE=%s, "
              "%i)\n",
              overrideDeviceStr, overrideDeviceIdx);
          bestDeviceIdx = overrideDeviceIdx - 1;
          overridden = true;
        } else {
          tprintf(
              "[DS] Ignoring invalid TESSERACT_OPENCL_DEVICE=%s ([1,%i] are "
              "valid devices).\n",
              overrideDeviceStr, profile->numDevices);
        }
      }

      if (overridden) {
        tprintf(
            "[DS] Overridden Device[%i]: \"%s\" (%s)\n", bestDeviceIdx + 1,
            profile->devices[bestDeviceIdx].oclDeviceName,
            profile->devices[bestDeviceIdx].type == DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native");
      }
      selectedDevice = profile->devices[bestDeviceIdx];
      // cleanup
      releaseDSProfile(profile, releaseScore);
    } else {
      // opencl isn't available at runtime, select native cpu device
      tprintf("[DS] OpenCL runtime not available.\n");
      selectedDevice.type = DS_DEVICE_NATIVE_CPU;
      selectedDevice.oclDeviceName = "(null)";
      selectedDevice.score = nullptr;
      selectedDevice.oclDeviceID = nullptr;
      selectedDevice.oclDriverVersion = nullptr;
    }
    deviceIsSelected = true;
  }
  return selectedDevice;
}

bool OpenclDevice::selectedDeviceIsOpenCL() {
  ds_device device = getDeviceSelection();
  return (device.type == DS_DEVICE_OPENCL_DEVICE);
}

} // namespace

#endif
