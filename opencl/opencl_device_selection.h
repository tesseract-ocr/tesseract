// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifdef USE_OPENCL
#ifndef DEVICE_SELECTION_H
#define DEVICE_SELECTION_H


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define DS_DEVICE_NAME_LENGTH 256

typedef enum {
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
} ds_status;

// device type
typedef enum {
  DS_DEVICE_NATIVE_CPU = 0,
  DS_DEVICE_OPENCL_DEVICE 
} ds_device_type;


typedef struct {
  ds_device_type  type;
  cl_device_id    oclDeviceID;
  char*           oclDeviceName;
  char*           oclDriverVersion;
  // a pointer to the score data, the content/format is application defined.
  void*           score;
} ds_device;

typedef struct {
  unsigned int  numDevices;
  ds_device*    devices;
  const char*   version;
} ds_profile;

// deallocate memory used by score
typedef ds_status (*ds_score_release)(void* score);
static ds_status releaseDSProfile(ds_profile* profile, ds_score_release sr) {
  ds_status status = DS_SUCCESS;
  if (profile != NULL) {
    if (profile->devices != NULL && sr != NULL) {
      unsigned int i;
      for (i = 0; i < profile->numDevices; i++) {
        free(profile->devices[i].oclDeviceName);
        free(profile->devices[i].oclDriverVersion);
        status = sr(profile->devices[i].score);
        if (status != DS_SUCCESS)
          break;
      }
      free(profile->devices);
    }
    free(profile);
  }
  return status;
}


static ds_status initDSProfile(ds_profile** p, const char* version) {
  int numDevices;
  cl_uint numPlatforms;
  cl_platform_id* platforms = NULL;
  cl_device_id* devices = NULL;
  ds_status status = DS_SUCCESS;
  unsigned int next;
  unsigned int i;

  if (p == NULL) return DS_INVALID_PROFILE;
  ds_profile* profile = (ds_profile*)malloc(sizeof(ds_profile));
  if (profile == NULL) return DS_MEMORY_ERROR;

  memset(profile, 0, sizeof(ds_profile));

  clGetPlatformIDs(0, NULL, &numPlatforms);
  if (numPlatforms == 0)
    goto cleanup;

  platforms = (cl_platform_id*)malloc(numPlatforms*sizeof(cl_platform_id));
  if (platforms == NULL) {
    status = DS_MEMORY_ERROR;
    goto cleanup;
  }
  clGetPlatformIDs(numPlatforms, platforms, NULL);

  numDevices = 0;
  for (i = 0; i < (unsigned int)numPlatforms; i++) {
    cl_uint num;
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num);
    numDevices+=num;
  }
  if (numDevices == 0)
    goto cleanup;
  
  devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));
  if (devices == NULL) {
    status = DS_MEMORY_ERROR;
    goto cleanup;
  }

  profile->numDevices = numDevices+1;     // +1 to numDevices to include the native CPU
  profile->devices =
      (ds_device*)malloc(profile->numDevices * sizeof(ds_device));
  if (profile->devices == NULL) {
    profile->numDevices = 0;
    status = DS_MEMORY_ERROR;
    goto cleanup;    
  }
  memset(profile->devices, 0, profile->numDevices*sizeof(ds_device));

  next = 0;
  for (i = 0; i < (unsigned int)numPlatforms; i++) {
    cl_uint num;
    unsigned j;
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices, &num);
    for (j = 0; j < num; j++, next++) {
      char buffer[DS_DEVICE_NAME_LENGTH];
      size_t length;

      profile->devices[next].type = DS_DEVICE_OPENCL_DEVICE;
      profile->devices[next].oclDeviceID = devices[j];

      clGetDeviceInfo(profile->devices[next].oclDeviceID, CL_DEVICE_NAME,
                      DS_DEVICE_NAME_LENGTH, &buffer, NULL);
      length = strlen(buffer);
      profile->devices[next].oclDeviceName = (char*)malloc(length+1);
      memcpy(profile->devices[next].oclDeviceName, buffer, length+1);

      clGetDeviceInfo(profile->devices[next].oclDeviceID, CL_DRIVER_VERSION,
                      DS_DEVICE_NAME_LENGTH, &buffer, NULL);
      length = strlen(buffer);
      profile->devices[next].oclDriverVersion = (char*)malloc(length+1);
      memcpy(profile->devices[next].oclDriverVersion, buffer, length+1);
    }
  }
  profile->devices[next].type = DS_DEVICE_NATIVE_CPU;
  profile->version = version;

cleanup:
  free(platforms);
  free(devices);
  if (status == DS_SUCCESS) {
    *p = profile;
  }
  else {
    if (profile) {
      free(profile->devices);
      free(profile);
    }
  }
  return status;
}

// Pointer to a function that calculates the score of a device (ex:
// device->score) update the data size of score. The encoding and the format
// of the score data is implementation defined. The function should return
// DS_SUCCESS if there's no error to be reported.
typedef ds_status (*ds_perf_evaluator)(ds_device* device, void* data);

typedef enum {
  DS_EVALUATE_ALL
  ,DS_EVALUATE_NEW_ONLY
} ds_evaluation_type;

static ds_status profileDevices(ds_profile* profile,
                                const ds_evaluation_type type,
                                ds_perf_evaluator evaluator,
                                void* evaluatorData, unsigned int* numUpdates) {
  ds_status status = DS_SUCCESS;
  unsigned int i;
  unsigned int updates = 0;

  if (profile == NULL) {
    return DS_INVALID_PROFILE;
  }
  if (evaluator == NULL) {
    return DS_INVALID_PERF_EVALUATOR;
  }

  for (i = 0; i < profile->numDevices; i++) {
    ds_status evaluatorStatus;
    
    switch (type) {
    case DS_EVALUATE_NEW_ONLY:
      if (profile->devices[i].score != NULL) break;
      //  else fall through
    case DS_EVALUATE_ALL:
      evaluatorStatus = evaluator(profile->devices+i, evaluatorData);
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


#define DS_TAG_VERSION                      "<version>"
#define DS_TAG_VERSION_END                  "</version>"
#define DS_TAG_DEVICE                       "<device>"
#define DS_TAG_DEVICE_END                   "</device>"
#define DS_TAG_SCORE                        "<score>"
#define DS_TAG_SCORE_END                    "</score>"
#define DS_TAG_DEVICE_TYPE                  "<type>"
#define DS_TAG_DEVICE_TYPE_END              "</type>"
#define DS_TAG_DEVICE_NAME                  "<name>"
#define DS_TAG_DEVICE_NAME_END              "</name>"
#define DS_TAG_DEVICE_DRIVER_VERSION        "<driver>"
#define DS_TAG_DEVICE_DRIVER_VERSION_END    "</driver>"

#define DS_DEVICE_NATIVE_CPU_STRING  "native_cpu"



typedef ds_status (*ds_score_serializer)(ds_device* device,
                                         void** serializedScore,
                                         unsigned int* serializedScoreSize);
static ds_status writeProfileToFile(ds_profile* profile,
                                    ds_score_serializer serializer,
                                    const char* file) {
  ds_status status = DS_SUCCESS;

  if (profile == NULL) return DS_INVALID_PROFILE;

  FILE* profileFile = fopen(file, "wb");
  if (profileFile == NULL) {
    status = DS_FILE_ERROR;
  }
  else {
    unsigned int i;

    // write version string
    fwrite(DS_TAG_VERSION, sizeof(char), strlen(DS_TAG_VERSION), profileFile);
    fwrite(profile->version, sizeof(char), strlen(profile->version), profileFile);
    fwrite(DS_TAG_VERSION_END, sizeof(char), strlen(DS_TAG_VERSION_END), profileFile);
    fwrite("\n", sizeof(char), 1, profileFile);

    for (i = 0; i < profile->numDevices && status == DS_SUCCESS; i++) {
      void* serializedScore;
      unsigned int serializedScoreSize;

      fwrite(DS_TAG_DEVICE, sizeof(char), strlen(DS_TAG_DEVICE), profileFile);

      fwrite(DS_TAG_DEVICE_TYPE, sizeof(char), strlen(DS_TAG_DEVICE_TYPE),
             profileFile);
      fwrite(&profile->devices[i].type,sizeof(ds_device_type),1, profileFile);
      fwrite(DS_TAG_DEVICE_TYPE_END, sizeof(char),
             strlen(DS_TAG_DEVICE_TYPE_END), profileFile);

      switch(profile->devices[i].type) {
      case DS_DEVICE_NATIVE_CPU:
        { 
          // There's no need to emit a device name for the native CPU device.
          /*
          fwrite(DS_TAG_DEVICE_NAME, sizeof(char), strlen(DS_TAG_DEVICE_NAME),
                 profileFile);
          fwrite(DS_DEVICE_NATIVE_CPU_STRING,sizeof(char),
                 strlen(DS_DEVICE_NATIVE_CPU_STRING), profileFile);
          fwrite(DS_TAG_DEVICE_NAME_END, sizeof(char),
                 strlen(DS_TAG_DEVICE_NAME_END), profileFile);
          */
        }
        break;
      case DS_DEVICE_OPENCL_DEVICE: 
        {
          fwrite(DS_TAG_DEVICE_NAME, sizeof(char), strlen(DS_TAG_DEVICE_NAME),
                 profileFile);
          fwrite(profile->devices[i].oclDeviceName,
                 sizeof(char),strlen(profile->devices[i].oclDeviceName), profileFile);
          fwrite(DS_TAG_DEVICE_NAME_END, sizeof(char),
                 strlen(DS_TAG_DEVICE_NAME_END), profileFile);

          fwrite(DS_TAG_DEVICE_DRIVER_VERSION, sizeof(char),
                 strlen(DS_TAG_DEVICE_DRIVER_VERSION), profileFile);
          fwrite(profile->devices[i].oclDriverVersion, sizeof(char),
                 strlen(profile->devices[i].oclDriverVersion), profileFile);
          fwrite(DS_TAG_DEVICE_DRIVER_VERSION_END, sizeof(char),
                 strlen(DS_TAG_DEVICE_DRIVER_VERSION_END), profileFile);
        }
        break;
      default:
        status = DS_UNKNOWN_DEVICE_TYPE;
        break;
      };

      fwrite(DS_TAG_SCORE, sizeof(char), strlen(DS_TAG_SCORE), profileFile);
      status = serializer(profile->devices+i, &serializedScore,
                          &serializedScoreSize);
      if (status == DS_SUCCESS && serializedScore != NULL &&
          serializedScoreSize > 0) {
        fwrite(serializedScore, sizeof(char), serializedScoreSize, profileFile);
        free(serializedScore);
      }
      fwrite(DS_TAG_SCORE_END, sizeof(char), strlen(DS_TAG_SCORE_END), profileFile);
      fwrite(DS_TAG_DEVICE_END, sizeof(char), strlen(DS_TAG_DEVICE_END), profileFile);
      fwrite("\n",sizeof(char),1,profileFile);
    }
    fclose(profileFile);
  }
  return status;
}


static ds_status readProFile(const char* fileName, char** content,
                             size_t* contentSize) {
  size_t size = 0;

  *contentSize = 0;
  *content = NULL;

  FILE* input = fopen(fileName, "rb");
  if (input == NULL) {
    return DS_FILE_ERROR;
  }

  fseek(input, 0L, SEEK_END); 
  size = ftell(input);
  rewind(input);
  char* binary = (char*)malloc(size);
  if (binary == NULL) {
    fclose(input);
    return DS_FILE_ERROR;
  }
  fread(binary, sizeof(char), size, input);
  fclose(input);

  *contentSize = size;
  *content = binary;
  return DS_SUCCESS;
}


static const char* findString(const char* contentStart, const char* contentEnd,
                              const char* string) {
  size_t stringLength;
  const char* currentPosition;
  const char* found;
  found = NULL;
  stringLength = strlen(string);
  currentPosition = contentStart;
  for(currentPosition = contentStart; currentPosition < contentEnd; currentPosition++) {
    if (*currentPosition == string[0]) {
      if (currentPosition+stringLength < contentEnd) {
        if (strncmp(currentPosition, string, stringLength) == 0) {
          found = currentPosition;
          break;
        }
      }
    }
  }
  return found;
}


typedef ds_status (*ds_score_deserializer)(ds_device* device,
                                           const unsigned char* serializedScore,
                                           unsigned int serializedScoreSize); 
static ds_status readProfileFromFile(ds_profile* profile,
                                     ds_score_deserializer deserializer,
                                     const char* file) {

  ds_status status = DS_SUCCESS;
  char* contentStart = NULL;
  const char* contentEnd = NULL;
  size_t contentSize;

  if (profile == NULL) return DS_INVALID_PROFILE;

  status = readProFile(file, &contentStart, &contentSize);
  if (status == DS_SUCCESS) {
    const char* currentPosition;
    const char* dataStart;
    const char* dataEnd;
    size_t versionStringLength;

    contentEnd = contentStart + contentSize;
    currentPosition = contentStart;


    // parse the version string
    dataStart = findString(currentPosition, contentEnd, DS_TAG_VERSION);
    if (dataStart == NULL) {
      status = DS_PROFILE_FILE_ERROR;
      goto cleanup;
    }
    dataStart += strlen(DS_TAG_VERSION);

    dataEnd = findString(dataStart, contentEnd, DS_TAG_VERSION_END);
    if (dataEnd == NULL) {
      status = DS_PROFILE_FILE_ERROR;
      goto cleanup;
    }

    versionStringLength = strlen(profile->version);
    if (versionStringLength!=(dataEnd-dataStart)   
        || strncmp(profile->version, dataStart, versionStringLength)!=0) {
      // version mismatch
      status = DS_PROFILE_FILE_ERROR;
      goto cleanup;
    }
    currentPosition = dataEnd+strlen(DS_TAG_VERSION_END);

    // parse the device information
    while (1) {
      unsigned int i;

      const char* deviceTypeStart;
      const char* deviceTypeEnd;
      ds_device_type deviceType;

      const char* deviceNameStart;
      const char* deviceNameEnd;

      const char* deviceScoreStart;
      const char* deviceScoreEnd;

      const char* deviceDriverStart;
      const char* deviceDriverEnd;

      dataStart = findString(currentPosition, contentEnd, DS_TAG_DEVICE);
      if (dataStart == NULL) {
        // nothing useful remain, quit...
        break;
      }
      dataStart+=strlen(DS_TAG_DEVICE);
      dataEnd = findString(dataStart, contentEnd, DS_TAG_DEVICE_END);
      if (dataEnd == NULL) {
        status = DS_PROFILE_FILE_ERROR;
        goto cleanup;
      }

      // parse the device type
      deviceTypeStart = findString(dataStart, contentEnd, DS_TAG_DEVICE_TYPE);
      if (deviceTypeStart == NULL) {
        status = DS_PROFILE_FILE_ERROR;
        goto cleanup;       
      }
      deviceTypeStart+=strlen(DS_TAG_DEVICE_TYPE);
      deviceTypeEnd = findString(deviceTypeStart, contentEnd,
                                 DS_TAG_DEVICE_TYPE_END);
      if (deviceTypeEnd == NULL) {
        status = DS_PROFILE_FILE_ERROR;
        goto cleanup;
      }
      memcpy(&deviceType, deviceTypeStart, sizeof(ds_device_type));


      // parse the device name
      if (deviceType == DS_DEVICE_OPENCL_DEVICE) {

        deviceNameStart = findString(dataStart, contentEnd, DS_TAG_DEVICE_NAME);
        if (deviceNameStart == NULL) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;       
        }
        deviceNameStart+=strlen(DS_TAG_DEVICE_NAME);
        deviceNameEnd = findString(deviceNameStart, contentEnd,
                                   DS_TAG_DEVICE_NAME_END);
        if (deviceNameEnd == NULL) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;       
        }


        deviceDriverStart = findString(dataStart, contentEnd,
                                       DS_TAG_DEVICE_DRIVER_VERSION);
        if (deviceDriverStart == NULL) {
          status = DS_PROFILE_FILE_ERROR;
          goto cleanup;       
        }
        deviceDriverStart+=strlen(DS_TAG_DEVICE_DRIVER_VERSION);
        deviceDriverEnd = findString(deviceDriverStart, contentEnd,
                                     DS_TAG_DEVICE_DRIVER_VERSION_END);
        if (deviceDriverEnd == NULL) {
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
            if (actualDeviceNameLength == (deviceNameEnd - deviceNameStart)
               && driverVersionLength == (deviceDriverEnd - deviceDriverStart)
               && strncmp(profile->devices[i].oclDeviceName, deviceNameStart,
                          actualDeviceNameLength)==0
               && strncmp(profile->devices[i].oclDriverVersion, deviceDriverStart,
                          driverVersionLength)==0) {
              deviceScoreStart = findString(dataStart, contentEnd, DS_TAG_SCORE);
              if (deviceNameStart == NULL) {
                status = DS_PROFILE_FILE_ERROR;
                goto cleanup;       
              }
              deviceScoreStart+=strlen(DS_TAG_SCORE);
              deviceScoreEnd = findString(deviceScoreStart, contentEnd,
                                          DS_TAG_SCORE_END);
              status = deserializer(profile->devices+i,
                                    (const unsigned char*)deviceScoreStart,
                                    deviceScoreEnd-deviceScoreStart);
              if (status != DS_SUCCESS) {
                goto cleanup;
              }
            }
          }
        }

      }
      else if (deviceType == DS_DEVICE_NATIVE_CPU) {
        for (i = 0; i < profile->numDevices; i++) {
          if (profile->devices[i].type == DS_DEVICE_NATIVE_CPU) {
            deviceScoreStart = findString(dataStart, contentEnd, DS_TAG_SCORE);
            if (deviceScoreStart == NULL) {
              status = DS_PROFILE_FILE_ERROR;
              goto cleanup;       
            }
            deviceScoreStart+=strlen(DS_TAG_SCORE);
            deviceScoreEnd = findString(deviceScoreStart, contentEnd,
                                        DS_TAG_SCORE_END);
            status = deserializer(profile->devices+i,
                                  (const unsigned char*)deviceScoreStart,
                                  deviceScoreEnd-deviceScoreStart);
            if (status != DS_SUCCESS) {
              goto cleanup;
            }
          }
        }
      }

      // skip over the current one to find the next device
      currentPosition = dataEnd+strlen(DS_TAG_DEVICE_END);
    }
  }
cleanup:
  free(contentStart);
  return status;
}

#endif
#endif
