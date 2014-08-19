#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif
#include <float.h>

#include "openclwrapper.h"
#include "oclkernels.h"

// for micro-benchmark
#include "otsuthr.h"
#include "thresholder.h"

#ifdef USE_OPENCL

#include "opencl_device_selection.h"
GPUEnv OpenclDevice::gpuEnv;

#if USE_DEVICE_SELECTION
bool OpenclDevice::deviceIsSelected = false;
ds_device OpenclDevice::selectedDevice;
#endif

int OpenclDevice::isInited =0;

struct tiff_transform {
    int vflip;    /* if non-zero, image needs a vertical fip */
    int hflip;    /* if non-zero, image needs a horizontal flip */
    int rotate;   /* -1 -> counterclockwise 90-degree rotation,
                      0 -> no rotation
                      1 -> clockwise 90-degree rotation */
};

static struct tiff_transform tiff_orientation_transforms[] = {
    {0, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {1, 0, 0},
    {0, 1, -1},
    {0, 0, 1},
    {0, 1, 1},
    {0, 0, -1}
};

static const l_int32  MAX_PAGES_IN_TIFF_FILE = 3000; 

cl_mem pixsCLBuffer, pixdCLBuffer, pixdCLIntermediate; //Morph operations buffers
cl_mem pixThBuffer; //output from thresholdtopix calculation
cl_int clStatus;
KernelEnv rEnv;

// substitute invalid characters in device name with _
void legalizeFileName( char *fileName) {
    //printf("fileName: %s\n", fileName);
    char *invalidChars = "/\?:*\"><| "; // space is valid but can cause headaches
    // for each invalid char
    for (int i = 0; i < strlen(invalidChars); i++) {
        char invalidStr[4];
        invalidStr[0] = invalidChars[i];
        invalidStr[1] = NULL;
        //printf("eliminating %s\n", invalidStr);
        //char *pos = strstr(fileName, invalidStr);
        // initial ./ is valid for present directory
        //if (*pos == '.') pos++;
        //if (*pos == '/') pos++;
        for ( char *pos = strstr(fileName, invalidStr); pos != NULL; pos = strstr(pos+1, invalidStr)) {
            //printf("\tfound: %s, ", pos);
            pos[0] = '_';
            //printf("fileName: %s\n", fileName);
        }
    }
}

void populateGPUEnvFromDevice( GPUEnv *gpuInfo, cl_device_id device ) {
    //printf("[DS] populateGPUEnvFromDevice\n");
    size_t size;
    gpuInfo->mnIsUserCreated = 1;
    // device
    gpuInfo->mpDevID = device;
    gpuInfo->mpArryDevsID = new cl_device_id[1];
    gpuInfo->mpArryDevsID[0] = gpuInfo->mpDevID;
    clStatus = clGetDeviceInfo(gpuInfo->mpDevID, CL_DEVICE_TYPE       , sizeof(cl_device_type), (void *) &gpuInfo->mDevType       , &size);
    CHECK_OPENCL( clStatus, "populateGPUEnv::getDeviceInfo(TYPE)");
    // platform
    clStatus = clGetDeviceInfo(gpuInfo->mpDevID, CL_DEVICE_PLATFORM   , sizeof(cl_platform_id), (void *) &gpuInfo->mpPlatformID   , &size);
    CHECK_OPENCL( clStatus, "populateGPUEnv::getDeviceInfo(PLATFORM)");
    // context
    cl_context_properties props[3];
    props[0] = CL_CONTEXT_PLATFORM;
    props[1] = (cl_context_properties) gpuInfo->mpPlatformID;
    props[2] = 0;
    gpuInfo->mpContext = clCreateContext(props, 1, &gpuInfo->mpDevID, NULL, NULL, &clStatus);
    CHECK_OPENCL( clStatus, "populateGPUEnv::createContext");
    // queue
    cl_command_queue_properties queueProperties = 0;
    gpuInfo->mpCmdQueue = clCreateCommandQueue( gpuInfo->mpContext, gpuInfo->mpDevID, queueProperties, &clStatus );
    CHECK_OPENCL( clStatus, "populateGPUEnv::createCommandQueue");
    
}

int OpenclDevice::LoadOpencl()
{
#ifdef WIN32
    HINSTANCE HOpenclDll = NULL;
  void * OpenclDll = NULL;
    //fprintf(stderr, " LoadOpenclDllxx... \n");
    OpenclDll = static_cast<HINSTANCE>( HOpenclDll );
    OpenclDll = LoadLibrary( "openCL.dll" );
    if ( !static_cast<HINSTANCE>( OpenclDll ) )
    {
        fprintf(stderr, "[OD] Load opencl.dll failed!\n");
        FreeLibrary( static_cast<HINSTANCE>( OpenclDll ) );
        return 0;
        
    }
    fprintf(stderr, "[OD] Load opencl.dll successful!\n");
#endif
    return 1;
}
int OpenclDevice::SetKernelEnv( KernelEnv *envInfo )
{
    envInfo->mpkContext = gpuEnv.mpContext;
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
    envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];

    return 1;
}

cl_mem allocateZeroCopyBuffer(KernelEnv rEnv, l_uint32 *hostbuffer, size_t nElements, cl_mem_flags flags, cl_int *pStatus)
{
    cl_mem membuffer = clCreateBuffer( rEnv.mpkContext, (cl_mem_flags) (flags),
                                        nElements * sizeof(l_uint32), hostbuffer, pStatus);

    return membuffer;
}

PIX* mapOutputCLBuffer(KernelEnv rEnv, cl_mem clbuffer, PIX* pixd, PIX* pixs, int elements, cl_mem_flags flags, bool memcopy = false, bool sync = true)
{   
    PROCNAME("mapOutputCLBuffer");
    if (!pixd)
    {
        if (memcopy)
        {
            if ((pixd = pixCreateTemplate(pixs)) == NULL)
                (PIX *)ERROR_PTR("pixd not made", procName, NULL);
        }
        else
        {
            if ((pixd = pixCreateHeader(pixGetWidth(pixs), pixGetHeight(pixs), pixGetDepth(pixs))) == NULL)
                (PIX *)ERROR_PTR("pixd not made", procName, NULL);
        }
    }
    l_uint32 *pValues = (l_uint32 *)clEnqueueMapBuffer(rEnv.mpkCmdQueue, clbuffer, CL_TRUE, flags, 0,
                                                    elements * sizeof(l_uint32), 0, NULL, NULL, NULL );
    
    if (memcopy)
    {
        memcpy(pixGetData(pixd), pValues, elements * sizeof(l_uint32));
    }
    else
    {
        pixSetData(pixd, pValues);
    }

    clEnqueueUnmapMemObject(rEnv.mpkCmdQueue,clbuffer,pValues,0,NULL,NULL);
    
    if (sync)
    {
        clFinish( rEnv.mpkCmdQueue );
    }

    return pixd;
}

 cl_mem allocateIntBuffer( KernelEnv rEnv, const l_uint32 *_pValues, size_t nElements, cl_int *pStatus , bool sync = false)
{
    cl_mem xValues = clCreateBuffer( rEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nElements * sizeof(l_int32), NULL, pStatus);

    if (_pValues != NULL)
    {
        l_int32 *pValues = (l_int32 *)clEnqueueMapBuffer( rEnv.mpkCmdQueue, xValues, CL_TRUE, CL_MAP_WRITE, 0,
            nElements * sizeof(l_int32), 0, NULL, NULL, NULL );

        memcpy(pValues, _pValues, nElements * sizeof(l_int32));

        clEnqueueUnmapMemObject(rEnv.mpkCmdQueue,xValues,pValues,0,NULL,NULL);

        if (sync)
            clFinish( rEnv.mpkCmdQueue );
    }

    return xValues;
}

int OpenclDevice::InitOpenclRunEnv( GPUEnv *gpuInfo )
{
    size_t length;
    cl_int clStatus;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platforms;
    cl_context_properties cps[3];
    char platformName[256];
    unsigned int i;


    // Have a look at the available platforms.

    if ( !gpuInfo->mnIsUserCreated )
    {
        clStatus = clGetPlatformIDs( 0, NULL, &numPlatforms );
        if ( clStatus != CL_SUCCESS )
        {
            return 1;
        }
        gpuInfo->mpPlatformID = NULL;

        if ( 0 < numPlatforms )
        {
            platforms = (cl_platform_id*) malloc( numPlatforms * sizeof( cl_platform_id ) );
            if ( platforms == (cl_platform_id*) NULL )
            {
                return 1;
            }
            clStatus = clGetPlatformIDs( numPlatforms, platforms, NULL );

            if ( clStatus != CL_SUCCESS )
            {
                return 1;
            }

            for ( i = 0; i < numPlatforms; i++ )
            {
                clStatus = clGetPlatformInfo( platforms[i], CL_PLATFORM_VENDOR,
                    sizeof( platformName ), platformName, NULL );

                if ( clStatus != CL_SUCCESS )
                {
                    return 1;
                }
                gpuInfo->mpPlatformID = platforms[i];

                //if (!strcmp(platformName, "Intel(R) Coporation"))
                //if( !strcmp( platformName, "Advanced Micro Devices, Inc." ))
                {
                    gpuInfo->mpPlatformID = platforms[i];
                    
                    if ( getenv("SC_OPENCLCPU") )
                    {
                        clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                                  CL_DEVICE_TYPE_CPU,    // device_type for CPU device
                                                  0,                     // num_entries
                                                  NULL,                  // devices
                                                  &numDevices);
                        printf("Selecting OpenCL device: CPU (a)\n");
                    }
                    else
                    {
                          clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                                  CL_DEVICE_TYPE_GPU,      // device_type for GPU device
                                                  0,                       // num_entries
                                                  NULL,                    // devices
                                                  &numDevices);
                          printf("Selecting OpenCL device: GPU (a)\n");
                    }
                    if ( clStatus != CL_SUCCESS )
                        continue;

                    if ( numDevices )
                        break;
                }
            }
            if ( clStatus != CL_SUCCESS )
                return 1;
            free( platforms );
        }
        if ( NULL == gpuInfo->mpPlatformID )
            return 1;

        // Use available platform.
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties) gpuInfo->mpPlatformID;
        cps[2] = 0;
        // Set device type for OpenCL
        
        if ( getenv("SC_OPENCLCPU") )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
            printf("Selecting OpenCL device: CPU (b)\n");
        }
        else
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_GPU;
            printf("Selecting OpenCL device: GPU (b)\n");
        }

        gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );

        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
            gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );
            printf("Selecting OpenCL device: CPU (c)\n");
        }
        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_DEFAULT;
            gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );
            printf("Selecting OpenCL device: DEFAULT (c)\n");
        }
        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
            return 1;
        // Detect OpenCL devices.
        // First, get the size of device list data
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES, 0, NULL, &length );
        if ( ( clStatus != CL_SUCCESS ) || ( length == 0 ) )
            return 1;
        // Now allocate memory for device list based on the size we got earlier
        gpuInfo->mpArryDevsID = (cl_device_id*) malloc( length );
        if ( gpuInfo->mpArryDevsID == (cl_device_id*) NULL )
            return 1;
        // Now, get the device list data
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES, length,
                       gpuInfo->mpArryDevsID, NULL );
        if ( clStatus != CL_SUCCESS )
            return 1;

        // Create OpenCL command queue.
        gpuInfo->mpCmdQueue = clCreateCommandQueue( gpuInfo->mpContext, gpuInfo->mpArryDevsID[0], 0, &clStatus );

        if ( clStatus != CL_SUCCESS )
            return 1;
    }

    clStatus = clGetCommandQueueInfo( gpuInfo->mpCmdQueue, CL_QUEUE_THREAD_HANDLE_AMD, 0, NULL, NULL );
    // Check device extensions for double type
    size_t aDevExtInfoSize = 0;

    clStatus = clGetDeviceInfo( gpuInfo->mpArryDevsID[0], CL_DEVICE_EXTENSIONS, 0, NULL, &aDevExtInfoSize );
    CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

    char *aExtInfo = new char[aDevExtInfoSize];

    clStatus = clGetDeviceInfo( gpuInfo->mpArryDevsID[0], CL_DEVICE_EXTENSIONS,
                   sizeof(char) * aDevExtInfoSize, aExtInfo, NULL);
    CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

    gpuInfo->mnKhrFp64Flag = 0;
    gpuInfo->mnAmdFp64Flag = 0;

    if ( strstr( aExtInfo, "cl_khr_fp64" ) )
    {
        gpuInfo->mnKhrFp64Flag = 1;
    }
    else
    {
        // Check if cl_amd_fp64 extension is supported
        if ( strstr( aExtInfo, "cl_amd_fp64" ) )
            gpuInfo->mnAmdFp64Flag = 1;
    }
    delete []aExtInfo;

    return 0;
}

void OpenclDevice::releaseMorphCLBuffers()
{
    if (pixdCLIntermediate != NULL)
        clReleaseMemObject(pixdCLIntermediate);
    if (pixsCLBuffer != NULL)
        clReleaseMemObject(pixsCLBuffer);
    if (pixdCLBuffer != NULL)
        clReleaseMemObject(pixdCLBuffer);
    if (pixThBuffer != NULL)
        clReleaseMemObject(pixThBuffer);
}

int OpenclDevice::initMorphCLAllocations(l_int32 wpl, l_int32 h, PIX* pixs)
{
    SetKernelEnv( &rEnv );
    
    if (pixThBuffer != NULL)
    {
        pixsCLBuffer = allocateZeroCopyBuffer(rEnv, NULL, wpl*h, CL_MEM_ALLOC_HOST_PTR, &clStatus);
        
        //Get the output from ThresholdToPix operation
        clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixThBuffer, pixsCLBuffer, 0, 0, sizeof(l_uint32) * wpl*h, 0, NULL, NULL);
    }
    else
    {
        //Get data from the source image
        l_uint32* srcdata = (l_uint32*) malloc(wpl*h*sizeof(l_uint32));
        memcpy(srcdata, pixGetData(pixs), wpl*h*sizeof(l_uint32));
    
        pixsCLBuffer = allocateZeroCopyBuffer(rEnv, srcdata, wpl*h, CL_MEM_USE_HOST_PTR, &clStatus);
    }
    
    pixdCLBuffer = allocateZeroCopyBuffer(rEnv, NULL, wpl*h, CL_MEM_ALLOC_HOST_PTR, &clStatus);

    pixdCLIntermediate = allocateZeroCopyBuffer(rEnv, NULL, wpl*h, CL_MEM_ALLOC_HOST_PTR, &clStatus);

    return (int)clStatus;
}

int OpenclDevice::InitEnv()
{
//PERF_COUNT_START("OD::InitEnv")
//    printf("[OD] OpenclDevice::InitEnv()\n");
#ifdef SAL_WIN32
    while( 1 )
    {
        if( 1 == LoadOpencl() )
            break;
    }
PERF_COUNT_SUB("LoadOpencl")
#endif
    // sets up environment, compiles programs
    

#if USE_DEVICE_SELECTION
    
    InitOpenclRunEnv_DeviceSelection( 0 );
//PERF_COUNT_SUB("called InitOpenclRunEnv_DS")
#else
    // init according to device
    InitOpenclRunEnv( 0 );
#endif
//PERF_COUNT_END
    return 1;
}

int OpenclDevice::ReleaseOpenclRunEnv()
{
    ReleaseOpenclEnv( &gpuEnv );
#ifdef SAL_WIN32
    FreeOpenclDll();
#endif
    return 1;
}
inline int OpenclDevice::AddKernelConfig( int kCount, const char *kName )
{
    if ( kCount < 1 )
        fprintf(stderr,"Error: ( KCount < 1 ) AddKernelConfig\n" );
    strcpy( gpuEnv.mArrykernelNames[kCount-1], kName );
    gpuEnv.mnKernelCount++;
    return 0;
}
int OpenclDevice::RegistOpenclKernel()
{
    if ( !gpuEnv.mnIsUserCreated )
        memset( &gpuEnv, 0, sizeof(gpuEnv) );

    gpuEnv.mnFileCount = 0; //argc;
    gpuEnv.mnKernelCount = 0UL;

    AddKernelConfig( 1, (const char*) "oclAverageSub1" );
    return 0;
}
int OpenclDevice::InitOpenclRunEnv( int argc )
{
    int status = 0;
    if ( MAX_CLKERNEL_NUM <= 0 )
    {
        return 1;
    }
    if ( ( argc > MAX_CLFILE_NUM ) || ( argc < 0 ) )
        return 1;

    if ( !isInited )
    {
        RegistOpenclKernel();
        //initialize devices, context, comand_queue
        status = InitOpenclRunEnv( &gpuEnv );
        if ( status )
        {
            fprintf(stderr,"init_opencl_env failed.\n");
            return 1;
        }
        fprintf(stderr,"init_opencl_env successed.\n");
        //initialize program, kernelName, kernelCount
        if( getenv( "SC_FLOAT" ) )
        {
            gpuEnv.mnKhrFp64Flag = 0;
            gpuEnv.mnAmdFp64Flag = 0;
        }
        if( gpuEnv.mnKhrFp64Flag )
        {
            fprintf(stderr,"----use khr double type in kernel----\n");
            status = CompileKernelFile( &gpuEnv, "-D KHR_DP_EXTENSION -Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16" );
        }
        else if( gpuEnv.mnAmdFp64Flag )
        {
            fprintf(stderr,"----use amd double type in kernel----\n");
            status = CompileKernelFile( &gpuEnv, "-D AMD_DP_EXTENSION -Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16" );
        }
        else
        {
            fprintf(stderr,"----use float type in kernel----\n");
            status = CompileKernelFile( &gpuEnv, "-Dfp_t=float -Dfp_t4=float4 -Dfp_t16=float16" );
        }
        if ( status == 0 || gpuEnv.mnKernelCount == 0 )
        {
            fprintf(stderr,"CompileKernelFile failed.\n");
            return 1;
        }
        fprintf(stderr,"CompileKernelFile successed.\n");
        isInited = 1;
    }
    return 0;
}

int OpenclDevice::InitOpenclRunEnv_DeviceSelection( int argc ) {
//PERF_COUNT_START("InitOpenclRunEnv_DS")
#if USE_DEVICE_SELECTION
    if (!isInited) {
        // after programs compiled, selects best device
        //printf("[DS] InitOpenclRunEnv_DS::Calling performDeviceSelection()\n");
        ds_device bestDevice_DS = getDeviceSelection( );
//PERF_COUNT_SUB("called getDeviceSelection()")
        cl_device_id bestDevice = bestDevice_DS.oclDeviceID;
        // overwrite global static GPUEnv with new device
        if (selectedDeviceIsOpenCL() ) {
            //printf("[DS] InitOpenclRunEnv_DS::Calling populateGPUEnvFromDevice() for selected device\n");
        populateGPUEnvFromDevice( &gpuEnv, bestDevice );
        gpuEnv.mnFileCount = 0; //argc;
        gpuEnv.mnKernelCount = 0UL;
//PERF_COUNT_SUB("populate gpuEnv")
        CompileKernelFile(&gpuEnv, "");
//PERF_COUNT_SUB("CompileKernelFile")
        } else {
            //printf("[DS] InitOpenclRunEnv_DS::Skipping populateGPUEnvFromDevice() b/c native cpu selected\n");
        }
        isInited = 1;
    }
#endif
//PERF_COUNT_END
    return 0;
}


OpenclDevice::OpenclDevice()
{
    //InitEnv();
}

OpenclDevice::~OpenclDevice()
{
    //ReleaseOpenclRunEnv();
}

int OpenclDevice::ReleaseOpenclEnv( GPUEnv *gpuInfo )
{
    int i = 0;
    int clStatus = 0;

    if ( !isInited )
    {
        return 1;
    }

    for ( i = 0; i < gpuEnv.mnFileCount; i++ )
    {
        if ( gpuEnv.mpArryPrograms[i] )
        {
            clStatus = clReleaseProgram( gpuEnv.mpArryPrograms[i] );
            CHECK_OPENCL( clStatus, "clReleaseProgram" );
            gpuEnv.mpArryPrograms[i] = NULL;
        }
    }
    if ( gpuEnv.mpCmdQueue )
    {
        clReleaseCommandQueue( gpuEnv.mpCmdQueue );
        gpuEnv.mpCmdQueue = NULL;
    }
    if ( gpuEnv.mpContext )
    {
        clReleaseContext( gpuEnv.mpContext );
        gpuEnv.mpContext = NULL;
    }
    isInited = 0;
    gpuInfo->mnIsUserCreated = 0;
    free( gpuInfo->mpArryDevsID );
    return 1;
}
int OpenclDevice::BinaryGenerated( const char * clFileName, FILE ** fhandle )
{
    unsigned int i = 0;
    cl_int clStatus;
    int status = 0;
    char *str = NULL;
    FILE *fd = NULL;
    cl_uint numDevices=0;
    if ( getenv("SC_OPENCLCPU") )
    {
        clStatus = clGetDeviceIDs(gpuEnv.mpPlatformID, // platform
                                  CL_DEVICE_TYPE_CPU,  // device_type for CPU device
                                  0,                   // num_entries
                                  NULL,                // devices ID
                                  &numDevices);
    }
    else
    {
        clStatus = clGetDeviceIDs(gpuEnv.mpPlatformID, // platform
                                  CL_DEVICE_TYPE_GPU,  // device_type for GPU device
                                  0,                   // num_entries
                                  NULL,                // devices ID
                                  &numDevices);
    }
    CHECK_OPENCL( clStatus, "clGetDeviceIDs" );
    for ( i = 0; i < numDevices; i++ )
    {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };
        if ( gpuEnv.mpArryDevsID[i] != 0 )
        {
            char deviceName[1024];
            clStatus = clGetDeviceInfo( gpuEnv.mpArryDevsID[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL );
            CHECK_OPENCL( clStatus, "clGetDeviceInfo" );
            str = (char*) strstr( clFileName, (char*) ".cl" );
            memcpy( cl_name, clFileName, str - clFileName );
            cl_name[str - clFileName] = '\0';
            sprintf( fileName, "%s-%s.bin", cl_name, deviceName );
            legalizeFileName(fileName);
            fd = fopen( fileName, "rb" );
            status = ( fd != NULL ) ? 1 : 0;
        }
    }
    if ( fd != NULL )
    {
        *fhandle = fd;
    }
    return status;

}
int OpenclDevice::CachedOfKernerPrg( const GPUEnv *gpuEnvCached, const char * clFileName )
{
    int i;
    for ( i = 0; i < gpuEnvCached->mnFileCount; i++ )
    {
        if ( strcasecmp( gpuEnvCached->mArryKnelSrcFile[i], clFileName ) == 0 )
        {
            if ( gpuEnvCached->mpArryPrograms[i] != NULL )
            {
                return 1;
            }
        }
    }

    return 0;
}
int OpenclDevice::WriteBinaryToFile( const char* fileName, const char* birary, size_t numBytes )
{
    FILE *output = NULL;
    output = fopen( fileName, "wb" );
    if ( output == NULL )
    {
        return 0;
    }

    fwrite( birary, sizeof(char), numBytes, output );
    fclose( output );

    return 1;

}
int OpenclDevice::GeneratBinFromKernelSource( cl_program program, const char * clFileName )
{
    unsigned int i = 0;
    cl_int clStatus;
    size_t *binarySizes, numDevices;
    cl_device_id *mpArryDevsID;
    char **binaries, *str = NULL;

    clStatus = clGetProgramInfo( program, CL_PROGRAM_NUM_DEVICES,
                   sizeof(numDevices), &numDevices, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    mpArryDevsID = (cl_device_id*) malloc( sizeof(cl_device_id) * numDevices );
    if ( mpArryDevsID == NULL )
    {
        return 0;
    }
    /* grab the handles to all of the devices in the program. */
    clStatus = clGetProgramInfo( program, CL_PROGRAM_DEVICES,
                   sizeof(cl_device_id) * numDevices, mpArryDevsID, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* figure out the sizes of each of the binaries. */
    binarySizes = (size_t*) malloc( sizeof(size_t) * numDevices );

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES,
                   sizeof(size_t) * numDevices, binarySizes, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* copy over all of the generated binaries. */
    binaries = (char**) malloc( sizeof(char *) * numDevices );
    if ( binaries == NULL )
    {
        return 0;
    }

    for ( i = 0; i < numDevices; i++ )
    {
        if ( binarySizes[i] != 0 )
        {
            binaries[i] = (char*) malloc( sizeof(char) * binarySizes[i] );
            if ( binaries[i] == NULL )
            {
                // cleanup all memory allocated so far
                for(int cleanupIndex = 0; cleanupIndex < i; ++cleanupIndex)
                {
                    free(binaries[cleanupIndex]);
                }
                // cleanup binary array
                free(binaries);

                return 0;
            }
        }
        else
        {
            binaries[i] = NULL;
        }
    }

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARIES,
                   sizeof(char *) * numDevices, binaries, NULL );
    CHECK_OPENCL(clStatus,"clGetProgramInfo");

    /* dump out each binary into its own separate file. */
    for ( i = 0; i < numDevices; i++ )
    {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };

        if ( binarySizes[i] != 0 )
        {
            char deviceName[1024];
            clStatus = clGetDeviceInfo(mpArryDevsID[i], CL_DEVICE_NAME,
                           sizeof(deviceName), deviceName, NULL);
            CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

            str = (char*) strstr( clFileName, (char*) ".cl" );
            memcpy( cl_name, clFileName, str - clFileName );
            cl_name[str - clFileName] = '\0';
            sprintf( fileName, "%s-%s.bin", cl_name, deviceName );
            legalizeFileName(fileName);
            if ( !WriteBinaryToFile( fileName, binaries[i], binarySizes[i] ) )
            {
                printf("[OD] write binary[%s] failed\n", fileName);
                return 0;
            } //else
            printf("[OD] write binary[%s] succesfully\n", fileName);
        }
    }

    // Release all resouces and memory
    for ( i = 0; i < numDevices; i++ )
    {
        if ( binaries[i] != NULL )
        {
            free( binaries[i] );
            binaries[i] = NULL;
        }
    }

    if ( binaries != NULL )
    {
        free( binaries );
        binaries = NULL;
    }

    if ( binarySizes != NULL )
    {
        free( binarySizes );
        binarySizes = NULL;
    }

    if ( mpArryDevsID != NULL )
    {
        free( mpArryDevsID );
        mpArryDevsID = NULL;
    }
    return 1;
}

void copyIntBuffer( KernelEnv rEnv, cl_mem xValues, const l_uint32 *_pValues, size_t nElements, cl_int *pStatus )
{
    l_int32 *pValues = (l_int32 *)clEnqueueMapBuffer( rEnv.mpkCmdQueue, xValues, CL_TRUE, CL_MAP_WRITE, 0,
        nElements * sizeof(l_int32), 0, NULL, NULL, NULL );
    clFinish( rEnv.mpkCmdQueue );
    if (_pValues != NULL)
    {
        for ( int i = 0; i < (int)nElements; i++ )
            pValues[i] = (l_int32)_pValues[i];
    }

    clEnqueueUnmapMemObject(rEnv.mpkCmdQueue,xValues,pValues,0,NULL,NULL);
    //clFinish( rEnv.mpkCmdQueue );
    return;
}

int OpenclDevice::CompileKernelFile( GPUEnv *gpuInfo, const char *buildOption )
{
//PERF_COUNT_START("CompileKernelFile")
    cl_int clStatus = 0;
    size_t length;
    char *buildLog = NULL, *binary;
    const char *source;
    size_t source_size[1];
    int b_error, binary_status, binaryExisted, idx;
    size_t numDevices;
    cl_device_id *mpArryDevsID;
    FILE *fd, *fd1;
    const char* filename = "kernel.cl";
    //fprintf(stderr, "[OD] CompileKernelFile ... \n");
    if ( CachedOfKernerPrg(gpuInfo, filename) == 1 )
    {
        return 1;
    }

    idx = gpuInfo->mnFileCount;

    source = kernel_src;

    source_size[0] = strlen( source );
    binaryExisted = 0;
        binaryExisted = BinaryGenerated( filename, &fd ); // don't check for binary during microbenchmark
//PERF_COUNT_SUB("BinaryGenerated")
    if ( binaryExisted == 1 )
    {
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_NUM_DEVICES,
                       sizeof(numDevices), &numDevices, NULL );
        CHECK_OPENCL( clStatus, "clGetContextInfo" );

        mpArryDevsID = (cl_device_id*) malloc( sizeof(cl_device_id) * numDevices );
        if ( mpArryDevsID == NULL )
        {
            return 0;
        }
//PERF_COUNT_SUB("get numDevices")
        b_error = 0;
        length = 0;
        b_error |= fseek( fd, 0, SEEK_END ) < 0;
        b_error |= ( length = ftell(fd) ) <= 0;
        b_error |= fseek( fd, 0, SEEK_SET ) < 0;
        if ( b_error )
        {
            return 0;
        }

        binary = (char*) malloc( length + 2 );
        if ( !binary )
        {
            return 0;
        }

        memset( binary, 0, length + 2 );
        b_error |= fread( binary, 1, length, fd ) != length;


        fclose( fd );
//PERF_COUNT_SUB("read file")
        fd = NULL;
        // grab the handles to all of the devices in the context.
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                       sizeof( cl_device_id ) * numDevices, mpArryDevsID, NULL );
        CHECK_OPENCL( clStatus, "clGetContextInfo" );
//PERF_COUNT_SUB("get devices")
        //fprintf(stderr, "[OD] Create kernel from binary\n");
        gpuInfo->mpArryPrograms[idx] = clCreateProgramWithBinary( gpuInfo->mpContext,numDevices,
                                           mpArryDevsID, &length, (const unsigned char**) &binary,
                                           &binary_status, &clStatus );
        CHECK_OPENCL( clStatus, "clCreateProgramWithBinary" );
//PERF_COUNT_SUB("clCreateProgramWithBinary")
        free( binary );
        free( mpArryDevsID );
        mpArryDevsID = NULL;
//PERF_COUNT_SUB("binaryExisted")
    }
    else
    {
        // create a CL program using the kernel source
        //fprintf(stderr, "[OD] Create kernel from source\n");
        gpuInfo->mpArryPrograms[idx] = clCreateProgramWithSource( gpuInfo->mpContext, 1, &source,
                                         source_size, &clStatus);
        CHECK_OPENCL( clStatus, "clCreateProgramWithSource" );
//PERF_COUNT_SUB("!binaryExisted")
    }

    if ( gpuInfo->mpArryPrograms[idx] == (cl_program) NULL )
    {
        return 0;
    }

    //char options[512];
    // create a cl program executable for all the devices specified
    //printf("[OD] BuildProgram.\n");
PERF_COUNT_START("OD::CompileKernel::clBuildProgram")
    if (!gpuInfo->mnIsUserCreated)
    {
        clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, gpuInfo->mpArryDevsID,
                       buildOption, NULL, NULL);
//PERF_COUNT_SUB("clBuildProgram notUserCreated")
    }
    else
    {
        clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, &(gpuInfo->mpDevID),
                       buildOption, NULL, NULL);
//PERF_COUNT_SUB("clBuildProgram isUserCreated")
    }
PERF_COUNT_END
    if ( clStatus != CL_SUCCESS )
    {
        printf ("BuildProgram error!\n");
        if ( !gpuInfo->mnIsUserCreated )
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                           CL_PROGRAM_BUILD_LOG, 0, NULL, &length );
        }
        else
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                           CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if ( clStatus != CL_SUCCESS )
        {
            printf("opencl create build log fail\n");
            return 0;
        }
        buildLog = (char*) malloc( length );
        if ( buildLog == (char*) NULL )
        {
            return 0;
        }
        if ( !gpuInfo->mnIsUserCreated )
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                           CL_PROGRAM_BUILD_LOG, length, buildLog, &length );
        }
        else
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                           CL_PROGRAM_BUILD_LOG, length, buildLog, &length );
        }
        if ( clStatus != CL_SUCCESS )
        {
            printf("opencl program build info fail\n");
            return 0;
        }

        fd1 = fopen( "kernel-build.log", "w+" );
        if ( fd1 != NULL )
        {
            fwrite( buildLog, sizeof(char), length, fd1 );
            fclose( fd1 );
        }

        free( buildLog );
//PERF_COUNT_SUB("build error log")
        return 0;
    }

    strcpy( gpuInfo->mArryKnelSrcFile[idx], filename );
//PERF_COUNT_SUB("strcpy")
    if ( binaryExisted == 0 ) {
        GeneratBinFromKernelSource( gpuInfo->mpArryPrograms[idx], filename );
        PERF_COUNT_SUB("GenerateBinFromKernelSource")
    }

    gpuInfo->mnFileCount += 1;
//PERF_COUNT_END
    return 1;
}

l_uint32* OpenclDevice::pixReadFromTiffKernel(l_uint32 *tiffdata,l_int32 w,l_int32 h,l_int32 wpl,l_uint32 *line)
{
PERF_COUNT_START("pixReadFromTiffKernel")
    cl_int clStatus;
    KernelEnv rEnv;
    size_t globalThreads[2];
    size_t localThreads[2];
    int gsize;
    cl_mem valuesCl; 
    cl_mem outputCl;

    //global and local work dimensions for Horizontal pass
    gsize = (w + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;
    localThreads[0] = GROUPSIZE_X;
    localThreads[1] = GROUPSIZE_Y;

    SetKernelEnv( &rEnv );
    
    l_uint32 *pResult = (l_uint32 *)malloc(w*h * sizeof(l_uint32));
    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "composeRGBPixel", &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel");
    
    //Allocate input and output OCL buffers
    valuesCl = allocateZeroCopyBuffer(rEnv, tiffdata, w*h, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &clStatus);
    outputCl = allocateZeroCopyBuffer(rEnv, pResult, w*h, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, &clStatus);

    //Kernel arguments
    clStatus = clSetKernelArg( rEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&valuesCl );
    CHECK_OPENCL( clStatus, "clSetKernelArg");
    clStatus = clSetKernelArg( rEnv.mpkKernel, 1, sizeof(w), (void *)&w );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 2, sizeof(h), (void *)&h );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 3, sizeof(wpl), (void *)&wpl );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 4, sizeof(cl_mem), (void *)&outputCl );
    CHECK_OPENCL( clStatus, "clSetKernelArg");
    
    //Kernel enqueue
PERF_COUNT_SUB("before")
    clStatus = clEnqueueNDRangeKernel( rEnv.mpkCmdQueue, rEnv.mpkKernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    
     /* map results back from gpu */
    void *ptr = clEnqueueMapBuffer(rEnv.mpkCmdQueue, outputCl, CL_TRUE, CL_MAP_READ, 0, w*h * sizeof(l_uint32), 0, NULL, NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer outputCl");
    clEnqueueUnmapMemObject(rEnv.mpkCmdQueue, outputCl, ptr, 0, NULL, NULL);

    //Sync
    clFinish( rEnv.mpkCmdQueue );
PERF_COUNT_SUB("kernel & map")
PERF_COUNT_END
    return pResult;
}


PIX * OpenclDevice::pixReadTiffCl ( const char *filename, l_int32 n )
{
PERF_COUNT_START("pixReadTiffCL")
    FILE  *fp;
PIX   *pix;

    //printf("pixReadTiffCl file");
    PROCNAME("pixReadTiff");

    if (!filename)
        return (PIX *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (PIX *)ERROR_PTR("image file not found", procName, NULL);
    if ((pix = pixReadStreamTiffCl(fp, n)) == NULL) {
        fclose(fp);
        return (PIX *)ERROR_PTR("pix not read", procName, NULL);
    }
    fclose(fp);
PERF_COUNT_END
    return pix;
    
}
TIFF *
OpenclDevice::fopenTiffCl(FILE        *fp,
          const char  *modestring)
{
l_int32  fd;

    PROCNAME("fopenTiff");

    if (!fp)
        return (TIFF *)ERROR_PTR("stream not opened", procName, NULL);
    if (!modestring)
        return (TIFF *)ERROR_PTR("modestring not defined", procName, NULL);

    if ((fd = fileno(fp)) < 0)
        return (TIFF *)ERROR_PTR("invalid file descriptor", procName, NULL);
    lseek(fd, 0, SEEK_SET);

    return TIFFFdOpen(fd, "TIFFstream", modestring);
}
l_int32 OpenclDevice::getTiffStreamResolutionCl(TIFF     *tif,
                        l_int32  *pxres,
                        l_int32  *pyres)
{
l_uint16   resunit;
l_int32    foundxres, foundyres;
l_float32  fxres, fyres;

    PROCNAME("getTiffStreamResolution");

    if (!tif)
        return ERROR_INT("tif not opened", procName, 1);
    if (!pxres || !pyres)
        return ERROR_INT("&xres and &yres not both defined", procName, 1);
    *pxres = *pyres = 0;

    TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &resunit);
    foundxres = TIFFGetField(tif, TIFFTAG_XRESOLUTION, &fxres);
    foundyres = TIFFGetField(tif, TIFFTAG_YRESOLUTION, &fyres);
    if (!foundxres && !foundyres) return 1;
    if (!foundxres && foundyres)
        fxres = fyres;
    else if (foundxres && !foundyres)
        fyres = fxres;

    if (resunit == RESUNIT_CENTIMETER) {  /* convert to ppi */
        *pxres = (l_int32)(2.54 * fxres + 0.5);
        *pyres = (l_int32)(2.54 * fyres + 0.5);
    }
    else {
        *pxres = (l_int32)fxres;
        *pyres = (l_int32)fyres;
    }

    return 0;
}

struct L_Memstream
{
l_uint8   *buffer;    /* expands to hold data when written to;         */
                        /* fixed size when read from.                    */
size_t     bufsize;   /* current size allocated when written to;       */
                        /* fixed size of input data when read from.      */
size_t     offset;    /* byte offset from beginning of buffer.         */
size_t     hw;        /* high-water mark; max bytes in buffer.         */
l_uint8  **poutdata;  /* input param for writing; data goes here.      */
size_t    *poutsize;  /* input param for writing; data size goes here. */
};
typedef struct L_Memstream  L_MEMSTREAM;

/* These are static functions for memory I/O */
static L_MEMSTREAM *memstreamCreateForRead(l_uint8 *indata, size_t pinsize);
static L_MEMSTREAM *memstreamCreateForWrite(l_uint8 **poutdata,
	size_t *poutsize);
static tsize_t tiffReadCallback(thandle_t handle, tdata_t data, tsize_t length);
static tsize_t tiffWriteCallback(thandle_t handle, tdata_t data,
	tsize_t length);
static toff_t tiffSeekCallback(thandle_t handle, toff_t offset, l_int32 whence);
static l_int32 tiffCloseCallback(thandle_t handle);
static toff_t tiffSizeCallback(thandle_t handle);
static l_int32 tiffMapCallback(thandle_t handle, tdata_t *data, toff_t *length);
static void tiffUnmapCallback(thandle_t handle, tdata_t data, toff_t length);


static L_MEMSTREAM *
memstreamCreateForRead(l_uint8  *indata,
size_t    insize)
{
	L_MEMSTREAM  *mstream;

	mstream = (L_MEMSTREAM *)CALLOC(1, sizeof(L_MEMSTREAM));
	mstream->buffer = indata;   /* handle to input data array */
	mstream->bufsize = insize;  /* amount of input data */
	mstream->hw = insize;       /* high-water mark fixed at input data size */
	mstream->offset = 0;        /* offset always starts at 0 */
	return mstream;
}


static L_MEMSTREAM *
memstreamCreateForWrite(l_uint8  **poutdata,
size_t    *poutsize)
{
	L_MEMSTREAM  *mstream;

	mstream = (L_MEMSTREAM *)CALLOC(1, sizeof(L_MEMSTREAM));
	mstream->buffer = (l_uint8 *)CALLOC(8 * 1024, 1);
	mstream->bufsize = 8 * 1024;
	mstream->poutdata = poutdata;  /* used only at end of write */
	mstream->poutsize = poutsize;  /* ditto  */
	mstream->hw = mstream->offset = 0;
	return mstream;
}


static tsize_t
tiffReadCallback(thandle_t  handle,
tdata_t    data,
tsize_t    length)
{
	L_MEMSTREAM  *mstream;
	size_t        amount;

	mstream = (L_MEMSTREAM *)handle;
	amount = L_MIN((size_t)length, mstream->hw - mstream->offset);
	memcpy(data, mstream->buffer + mstream->offset, amount);
	mstream->offset += amount;
	return amount;
}


static tsize_t
tiffWriteCallback(thandle_t  handle,
tdata_t    data,
tsize_t    length)
{
	L_MEMSTREAM  *mstream;
	size_t        newsize;

	/* reallocNew() uses calloc to initialize the array.
	* If malloc is used instead, for some of the encoding methods,
	* not all the data in 'bufsize' bytes in the buffer will
	* have been initialized by the end of the compression. */
	mstream = (L_MEMSTREAM *)handle;
	if (mstream->offset + length > mstream->bufsize) {
		newsize = 2 * (mstream->offset + length);
		mstream->buffer = (l_uint8 *)reallocNew((void **)&mstream->buffer,
			mstream->offset, newsize);
		mstream->bufsize = newsize;
	}

	memcpy(mstream->buffer + mstream->offset, data, length);
	mstream->offset += length;
	mstream->hw = L_MAX(mstream->offset, mstream->hw);
	return length;
}


static toff_t
tiffSeekCallback(thandle_t  handle,
toff_t     offset,
l_int32    whence)
{
	L_MEMSTREAM  *mstream;

	PROCNAME("tiffSeekCallback");
	mstream = (L_MEMSTREAM *)handle;
	switch (whence) {
	case SEEK_SET:
		/*            fprintf(stderr, "seek_set: offset = %d\n", offset); */
		mstream->offset = offset;
		break;
	case SEEK_CUR:
		/*            fprintf(stderr, "seek_cur: offset = %d\n", offset); */
		mstream->offset += offset;
		break;
	case SEEK_END:
		/*            fprintf(stderr, "seek end: hw = %d, offset = %d\n",
		mstream->hw, offset); */
		mstream->offset = mstream->hw - offset;  /* offset >= 0 */
		break;
	default:
		return (toff_t)ERROR_INT("bad whence value", procName,
			mstream->offset);
	}

	return mstream->offset;
}


static l_int32
tiffCloseCallback(thandle_t  handle)
{
	L_MEMSTREAM  *mstream;

	mstream = (L_MEMSTREAM *)handle;
	if (mstream->poutdata) {   /* writing: save the output data */
		*mstream->poutdata = mstream->buffer;
		*mstream->poutsize = mstream->hw;
	}
	FREE(mstream);  /* never free the buffer! */
	return 0;
}


static toff_t
tiffSizeCallback(thandle_t  handle)
{
	L_MEMSTREAM  *mstream;

	mstream = (L_MEMSTREAM *)handle;
	return mstream->hw;
}


static l_int32
tiffMapCallback(thandle_t  handle,
tdata_t   *data,
toff_t    *length)
{
	L_MEMSTREAM  *mstream;

	mstream = (L_MEMSTREAM *)handle;
	*data = mstream->buffer;
	*length = mstream->hw;
	return 0;
}


static void
tiffUnmapCallback(thandle_t  handle,
tdata_t    data,
toff_t     length)
{
	return;
}


/*!
*  fopenTiffMemstream()
*
*      Input:  filename (for error output; can be "")
*              operation ("w" for write, "r" for read)
*              &data (<return> written data)
*              &datasize (<return> size of written data)
*      Return: tiff (data structure, opened for write to memory)
*
*  Notes:
*      (1) This wraps up a number of callbacks for either:
*            * reading from tiff in memory buffer --> pix
*            * writing from pix --> tiff in memory buffer
*      (2) After use, the memstream is automatically destroyed when
*          TIFFClose() is called.  TIFFCleanup() doesn't free the memstream.
*/
static TIFF *
fopenTiffMemstream(const char  *filename,
const char  *operation,
l_uint8    **pdata,
size_t      *pdatasize)
{
	L_MEMSTREAM  *mstream;

	PROCNAME("fopenTiffMemstream");

	if (!filename)
		return (TIFF *)ERROR_PTR("filename not defined", procName, NULL);
	if (!operation)
		return (TIFF *)ERROR_PTR("operation not defined", procName, NULL);
	if (!pdata)
		return (TIFF *)ERROR_PTR("&data not defined", procName, NULL);
	if (!pdatasize)
		return (TIFF *)ERROR_PTR("&datasize not defined", procName, NULL);
	if (!strcmp(operation, "r") && !strcmp(operation, "w"))
		return (TIFF *)ERROR_PTR("operation not 'r' or 'w'}", procName, NULL);

	if (!strcmp(operation, "r"))
		mstream = memstreamCreateForRead(*pdata, *pdatasize);
	else
		mstream = memstreamCreateForWrite(pdata, pdatasize);

	return TIFFClientOpen(filename, operation, mstream,
		tiffReadCallback, tiffWriteCallback,
		tiffSeekCallback, tiffCloseCallback,
		tiffSizeCallback, tiffMapCallback,
		tiffUnmapCallback);
}



PIX *
OpenclDevice::pixReadMemTiffCl(const l_uint8 *data,size_t size,l_int32  n)
{
	l_int32  i, pagefound;
	PIX     *pix;
	TIFF    *tif;
	L_MEMSTREAM *memStream;
	PROCNAME("pixReadMemTiffCl");

	if (!data)
		return (PIX *)ERROR_PTR("data pointer is NULL", procName, NULL);

	if ((tif = fopenTiffMemstream("", "r", (l_uint8 **)&data, &size)) == NULL)
		return (PIX *)ERROR_PTR("tif not opened", procName, NULL);

	pagefound = FALSE;
	pix = NULL;
	for (i = 0; i < MAX_PAGES_IN_TIFF_FILE; i++) {
		if (i == n) {
			pagefound = TRUE;
			if ((pix = pixReadFromTiffStreamCl(tif)) == NULL) {
				TIFFCleanup(tif);
				return (PIX *)ERROR_PTR("pix not read", procName, NULL);
			}
			break;
		}
		if (TIFFReadDirectory(tif) == 0)
			break;
	}

	if (pagefound == FALSE) {
		L_WARNING("tiff page %d not found", procName);
		TIFFCleanup(tif);
		return NULL;
	}

	TIFFCleanup(tif);
	return pix;
}

PIX *
OpenclDevice::pixReadStreamTiffCl(FILE    *fp,
                  l_int32  n)
{
l_int32  i, pagefound;
PIX     *pix;
TIFF    *tif;

    PROCNAME("pixReadStreamTiff");

    if (!fp)
        return (PIX *)ERROR_PTR("stream not defined", procName, NULL);

    if ((tif = fopenTiffCl(fp, "rb")) == NULL)
        return (PIX *)ERROR_PTR("tif not opened", procName, NULL);

    pagefound = FALSE;
    pix = NULL;
    for (i = 0; i < MAX_PAGES_IN_TIFF_FILE; i++) {
        if (i == n) {
            pagefound = TRUE;
            if ((pix = pixReadFromTiffStreamCl(tif)) == NULL) {
                TIFFCleanup(tif);
                return (PIX *)ERROR_PTR("pix not read", procName, NULL);
            }
            break;
        }
        if (TIFFReadDirectory(tif) == 0)
            break;
    }

    if (pagefound == FALSE) {
        L_WARNING("tiff page %d not found", procName, n);
        TIFFCleanup(tif);
        return NULL;
    }

    TIFFCleanup(tif);
    return pix;
}

static l_int32
getTiffCompressedFormat(l_uint16  tiffcomp)
{
l_int32  comptype;

    switch (tiffcomp)
    {
    case COMPRESSION_CCITTFAX4:
        comptype = IFF_TIFF_G4;
        break;
    case COMPRESSION_CCITTFAX3:
        comptype = IFF_TIFF_G3;
        break;
    case COMPRESSION_CCITTRLE:
        comptype = IFF_TIFF_RLE;
        break;
    case COMPRESSION_PACKBITS:
        comptype = IFF_TIFF_PACKBITS;
        break;
    case COMPRESSION_LZW:
        comptype = IFF_TIFF_LZW;
        break;
    case COMPRESSION_ADOBE_DEFLATE:
        comptype = IFF_TIFF_ZIP;
        break;
    default:
        comptype = IFF_TIFF;
        break;
    }
    return comptype;
}

void compare(l_uint32  *cpu, l_uint32  *gpu,int size)
{
    for(int i=0;i<size;i++)
    {
        if(cpu[i]!=gpu[i])
        {
            printf("\ndoesnot match\n");
            return;
        }
    }
    printf("\nit matches\n");
    
}

//OpenCL implementation of pixReadFromTiffStream.
//Similar to the CPU implentation of pixReadFromTiffStream
PIX *
OpenclDevice::pixReadFromTiffStreamCl(TIFF  *tif)
{
l_uint8   *linebuf, *data;
l_uint16   spp, bps, bpp, tiffbpl, photometry, tiffcomp, orientation;
l_uint16  *redmap, *greenmap, *bluemap;
l_int32    d, wpl, bpl, comptype, i, ncolors;
l_int32    xres, yres;
l_uint32   w, h;
l_uint32  *line, *tiffdata;
PIX       *pix;
PIXCMAP   *cmap;

    PROCNAME("pixReadFromTiffStream");

    if (!tif)
        return (PIX *)ERROR_PTR("tif not defined", procName, NULL);

    
    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    bpp = bps * spp;
    if (bpp > 32)
        return (PIX *)ERROR_PTR("can't handle bpp > 32", procName, NULL);
    if (spp == 1)
        d = bps;
    else if (spp == 3 || spp == 4)
        d = 32;
    else
        return (PIX *)ERROR_PTR("spp not in set {1,3,4}", procName, NULL);

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    tiffbpl = TIFFScanlineSize(tif);

    if ((pix = pixCreate(w, h, d)) == NULL)
        return (PIX *)ERROR_PTR("pix not made", procName, NULL);
    data = (l_uint8 *)pixGetData(pix);
    wpl = pixGetWpl(pix);
    bpl = 4 * wpl;

   
    if (spp == 1) {
        if ((linebuf = (l_uint8 *)CALLOC(tiffbpl + 1, sizeof(l_uint8))) == NULL)
            return (PIX *)ERROR_PTR("calloc fail for linebuf", procName, NULL);
        
        for (i = 0 ; i < h ; i++) {
            if (TIFFReadScanline(tif, linebuf, i, 0) < 0) {
                FREE(linebuf);
                pixDestroy(&pix);
                return (PIX *)ERROR_PTR("line read fail", procName, NULL);
            }
            memcpy((char *)data, (char *)linebuf, tiffbpl);
            data += bpl;
        }
        if (bps <= 8)
            pixEndianByteSwap(pix);
        else   
            pixEndianTwoByteSwap(pix);
        FREE(linebuf);
    }
    else {  
        if ((tiffdata = (l_uint32 *)CALLOC(w * h, sizeof(l_uint32))) == NULL) {
            pixDestroy(&pix);
            return (PIX *)ERROR_PTR("calloc fail for tiffdata", procName, NULL);
        }
        if (!TIFFReadRGBAImageOriented(tif, w, h, (uint32 *)tiffdata,
                                       ORIENTATION_TOPLEFT, 0)) {
            FREE(tiffdata);
            pixDestroy(&pix);
            return (PIX *)ERROR_PTR("failed to read tiffdata", procName, NULL);
        }
        line = pixGetData(pix);

        //Invoke the OpenCL kernel for pixReadFromTiff
        l_uint32* output_gpu=pixReadFromTiffKernel(tiffdata,w,h,wpl,line);
        pixSetData(pix, output_gpu);
        
        FREE(tiffdata);
    }

    if (getTiffStreamResolutionCl(tif, &xres, &yres) == 0) {
        pixSetXRes(pix, xres);
        pixSetYRes(pix, yres);
    }


    TIFFGetFieldDefaulted(tif, TIFFTAG_COMPRESSION, &tiffcomp);
    comptype = getTiffCompressedFormat(tiffcomp);
    pixSetInputFormat(pix, comptype);

    if (TIFFGetField(tif, TIFFTAG_COLORMAP, &redmap, &greenmap, &bluemap)) {
           
        if ((cmap = pixcmapCreate(bps)) == NULL) {
            pixDestroy(&pix);
            return (PIX *)ERROR_PTR("cmap not made", procName, NULL);
        }
        ncolors = 1 << bps;
        for (i = 0; i < ncolors; i++)
            pixcmapAddColor(cmap, redmap[i] >> 8, greenmap[i] >> 8,
                            bluemap[i] >> 8);
        pixSetColormap(pix, cmap);
    }
    else {  
        if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometry)) {
       
            if (tiffcomp == COMPRESSION_CCITTFAX3 ||
                tiffcomp == COMPRESSION_CCITTFAX4 ||
                tiffcomp == COMPRESSION_CCITTRLE ||
                tiffcomp == COMPRESSION_CCITTRLEW) {
                photometry = PHOTOMETRIC_MINISWHITE;
            }
            else
                photometry = PHOTOMETRIC_MINISBLACK;
        }
        if ((d == 1 && photometry == PHOTOMETRIC_MINISBLACK) ||
            (d == 8 && photometry == PHOTOMETRIC_MINISWHITE))
            pixInvert(pix, pix);
    }

    if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation)) {
        if (orientation >= 1 && orientation <= 8) {
            struct tiff_transform *transform =
              &tiff_orientation_transforms[orientation - 1];
            if (transform->vflip) pixFlipTB(pix, pix);
            if (transform->hflip) pixFlipLR(pix, pix);
            if (transform->rotate) {
                PIX *oldpix = pix;
                pix = pixRotate90(oldpix, transform->rotate);
                pixDestroy(&oldpix);
           }
        }
    }

    return pix;
}

//Morphology Dilate operation for 5x5 structuring element. Invokes the relevant OpenCL kernels
cl_int
pixDilateCL_55(l_int32  wpl, l_int32  h)
{
    size_t globalThreads[2];
    cl_mem pixtemp;
    cl_int status;
    int gsize;
    size_t localThreads[2];

    //Horizontal pass
    gsize = (wpl*h + GROUPSIZE_HMORX - 1)/ GROUPSIZE_HMORX * GROUPSIZE_HMORX;
    globalThreads[0] = gsize;
    globalThreads[1] = GROUPSIZE_HMORY;
    localThreads[0] = GROUPSIZE_HMORX;
    localThreads[1] = GROUPSIZE_HMORY;

    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoDilateHor_5x5", &status );
    
    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        2,
        sizeof(wpl),
        (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
        3,
        sizeof(h),
        (const void *)&h);

    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);
    
    //Swap source and dest buffers
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    //Vertical
    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;
    localThreads[0] = GROUPSIZE_X;
    localThreads[1] = GROUPSIZE_Y;

    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoDilateVer_5x5", &status );
    
    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        2,
        sizeof(wpl),
        (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
        3,
        sizeof(h),
        (const void *)&h);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);

    return status;
}

//Morphology Erode operation for 5x5 structuring element. Invokes the relevant OpenCL kernels
cl_int
pixErodeCL_55(l_int32  wpl, l_int32  h)
{
    size_t globalThreads[2];
    cl_mem pixtemp;
    cl_int status;
    int gsize;
    l_uint32 fwmask, lwmask;
    size_t localThreads[2];

    lwmask = lmask32[32 - 2];
    fwmask = rmask32[32 - 2];

    //Horizontal pass
    gsize = (wpl*h + GROUPSIZE_HMORX - 1)/ GROUPSIZE_HMORX * GROUPSIZE_HMORX;
    globalThreads[0] = gsize;
    globalThreads[1] = GROUPSIZE_HMORY;
    localThreads[0] = GROUPSIZE_HMORX;
    localThreads[1] = GROUPSIZE_HMORY;

    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoErodeHor_5x5", &status );
    
    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        2,
        sizeof(wpl),
        (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
        3,
        sizeof(h),
        (const void *)&h);

    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);
    
    //Swap source and dest buffers
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    //Vertical
    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;
    localThreads[0] = GROUPSIZE_X;
    localThreads[1] = GROUPSIZE_Y;

    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoErodeVer_5x5", &status );
    
    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &pixsCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &pixdCLBuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
        2,
        sizeof(wpl),
        (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
        3,
        sizeof(h),
        (const void *)&h);
    status = clSetKernelArg(rEnv.mpkKernel,
        4,
        sizeof(fwmask),
        (const void *)&fwmask);
    status = clSetKernelArg(rEnv.mpkKernel,
        5,
        sizeof(lwmask),
        (const void *)&lwmask);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);

    return status;
}

//Morphology Dilate operation. Invokes the relevant OpenCL kernels
cl_int
pixDilateCL(l_int32  hsize, l_int32  vsize, l_int32  wpl, l_int32  h)
{
    l_int32  xp, yp, xn, yn;
    SEL* sel;
    size_t globalThreads[2];
    cl_mem pixtemp;
    cl_int status;
    int gsize;
    size_t localThreads[2];
    char isEven;

    OpenclDevice::SetKernelEnv( &rEnv );
    
    if (hsize == 5 && vsize == 5)
    {
        //Specific case for 5x5
        status = pixDilateCL_55(wpl, h);
        return status;
    }
    
    sel = selCreateBrick(vsize, hsize, vsize / 2, hsize / 2, SEL_HIT);
    
    selFindMaxTranslations(sel, &xp, &yp, &xn, &yn);

    //global and local work dimensions for Horizontal pass
    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;
    localThreads[0] = GROUPSIZE_X;
    localThreads[1] = GROUPSIZE_Y;

    if (xp > 31 || xn > 31)
    {
        //Generic case. 
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoDilateHor", &status );
    
        status = clSetKernelArg(rEnv.mpkKernel,
            0,
            sizeof(cl_mem),
            &pixsCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
            1,
            sizeof(cl_mem),
            &pixdCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
                2,
                sizeof(xp),
                (const void *)&xp);
        status = clSetKernelArg(rEnv.mpkKernel,
                3,
                sizeof(xn),
                (const void *)&xn);
        status = clSetKernelArg(rEnv.mpkKernel,
                4,
                sizeof(wpl),
                (const void *)&wpl);
        status = clSetKernelArg(rEnv.mpkKernel,
                5,
                sizeof(h),
                (const void *)&h);
        status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                                rEnv.mpkKernel,
                                2,
                                NULL,
                                globalThreads,
                                localThreads,
                                0,
                                NULL,
                                NULL);

        if (yp > 0 || yn > 0)
        {
            pixtemp = pixsCLBuffer;
            pixsCLBuffer = pixdCLBuffer;
            pixdCLBuffer = pixtemp;
        }
    }
    else if (xp > 0 || xn > 0 )
    {
        //Specfic Horizontal pass kernel for half width < 32
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoDilateHor_32word", &status );
        isEven = (xp != xn);
        
        status = clSetKernelArg(rEnv.mpkKernel,
            0,
            sizeof(cl_mem),
            &pixsCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
            1,
            sizeof(cl_mem),
            &pixdCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
                2,
                sizeof(xp),
                (const void *)&xp);
        status = clSetKernelArg(rEnv.mpkKernel,
                3,
                sizeof(wpl),
                (const void *)&wpl);
        status = clSetKernelArg(rEnv.mpkKernel,
                4,
                sizeof(h),
                (const void *)&h);
        status = clSetKernelArg(rEnv.mpkKernel,
                5,
                sizeof(isEven),
                (const void *)&isEven);
        status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                                rEnv.mpkKernel,
                                2,
                                NULL,
                                globalThreads,
                                localThreads,
                                0,
                                NULL,
                                NULL);

        if (yp > 0 || yn > 0)
        {
            pixtemp = pixsCLBuffer;
            pixsCLBuffer = pixdCLBuffer;
            pixdCLBuffer = pixtemp;
        }
    } 

    if (yp > 0 || yn > 0)
    {
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoDilateVer", &status );
        
        status = clSetKernelArg(rEnv.mpkKernel,
            0,
            sizeof(cl_mem),
            &pixsCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
            1,
            sizeof(cl_mem),
            &pixdCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
                2,
                sizeof(yp),
                (const void *)&yp);
        status = clSetKernelArg(rEnv.mpkKernel,
                3,
                sizeof(wpl),
                (const void *)&wpl);
        status = clSetKernelArg(rEnv.mpkKernel,
                4,
                sizeof(h),
                (const void *)&h);
        status = clSetKernelArg(rEnv.mpkKernel,
                5,
                sizeof(yn),
                (const void *)&yn);
        status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                                rEnv.mpkKernel,
                                2,
                                NULL,
                                globalThreads,
                                localThreads,
                                0,
                                NULL,
                                NULL);
    }
    

    return status;
}

//Morphology Erode operation. Invokes the relevant OpenCL kernels
cl_int 
pixErodeCL(l_int32  hsize, l_int32  vsize, l_uint32 wpl, l_uint32 h)
{

    l_int32  xp, yp, xn, yn;
    SEL* sel;
    size_t globalThreads[2];
    size_t localThreads[2];
    cl_mem pixtemp;
    cl_int status;
    int gsize;
    char isAsymmetric = (MORPH_BC == ASYMMETRIC_MORPH_BC);
    l_uint32 rwmask, lwmask;
    char isEven;

    sel = selCreateBrick(vsize, hsize, vsize / 2, hsize / 2, SEL_HIT);
    
    selFindMaxTranslations(sel, &xp, &yp, &xn, &yn);
    
    OpenclDevice::SetKernelEnv( &rEnv );

    if (hsize == 5 && vsize == 5 && isAsymmetric)
    {
        //Specific kernel for 5x5
        status = pixErodeCL_55(wpl, h);
        return status;
    }

    rwmask = rmask32[32 - (xp & 31)];
    lwmask = lmask32[32 - (xn & 31)];

    //global and local work dimensions for Horizontal pass
    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;
    localThreads[0] = GROUPSIZE_X;
    localThreads[1] = GROUPSIZE_Y;
    
    //Horizontal Pass
    if (xp > 31 || xn > 31 )
    {
        //Generic case. 
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoErodeHor", &status );
        
        status = clSetKernelArg(rEnv.mpkKernel,
            0,
            sizeof(cl_mem),
            &pixsCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
            1,
            sizeof(cl_mem),
            &pixdCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
                2,
                sizeof(xp),
                (const void *)&xp);
        status = clSetKernelArg(rEnv.mpkKernel,
                3,
                sizeof(xn),
                (const void *)&xn);
        status = clSetKernelArg(rEnv.mpkKernel,
                4,
                sizeof(wpl),
                (const void *)&wpl);
        status = clSetKernelArg(rEnv.mpkKernel,
                5,
                sizeof(h),
                (const void *)&h);
        status = clSetKernelArg(rEnv.mpkKernel,
                6,
                sizeof(isAsymmetric),
                (const void *)&isAsymmetric);
        status = clSetKernelArg(rEnv.mpkKernel,
                7,
                sizeof(rwmask),
                (const void *)&rwmask);
        status = clSetKernelArg(rEnv.mpkKernel,
                8,
                sizeof(lwmask),
                (const void *)&lwmask);
        status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                                rEnv.mpkKernel,
                                2,
                                NULL,
                                globalThreads,
                                localThreads,
                                0,
                                NULL,
                                NULL);

        if (yp > 0 || yn > 0)
        {
            pixtemp = pixsCLBuffer;
            pixsCLBuffer = pixdCLBuffer;
            pixdCLBuffer = pixtemp;
        }
    }
    else if (xp > 0 || xn > 0)
    {
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoErodeHor_32word", &status );
        isEven = (xp != xn);

        status = clSetKernelArg(rEnv.mpkKernel,
            0,
            sizeof(cl_mem),
            &pixsCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
            1,
            sizeof(cl_mem),
            &pixdCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
                2,
                sizeof(xp),
                (const void *)&xp);
        status = clSetKernelArg(rEnv.mpkKernel,
                3,
                sizeof(wpl),
                (const void *)&wpl);
        status = clSetKernelArg(rEnv.mpkKernel,
                4,
                sizeof(h),
                (const void *)&h);
        status = clSetKernelArg(rEnv.mpkKernel,
                5,
                sizeof(isAsymmetric),
                (const void *)&isAsymmetric);
        status = clSetKernelArg(rEnv.mpkKernel,
                6,
                sizeof(rwmask),
                (const void *)&rwmask);
        status = clSetKernelArg(rEnv.mpkKernel,
                7,
                sizeof(lwmask),
                (const void *)&lwmask);
        status = clSetKernelArg(rEnv.mpkKernel,
                8,
                sizeof(isEven),
                (const void *)&isEven);
        status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                                rEnv.mpkKernel,
                                2,
                                NULL,
                                globalThreads,
                                localThreads,
                                0,
                                NULL,
                                NULL);
    
        if (yp > 0 || yn > 0)
        {
            pixtemp = pixsCLBuffer;
            pixsCLBuffer = pixdCLBuffer;
            pixdCLBuffer = pixtemp;
        }
    }

    //Vertical Pass
    if (yp > 0 || yn > 0)
    {
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "morphoErodeVer", &status );
        
        status = clSetKernelArg(rEnv.mpkKernel,
            0,
            sizeof(cl_mem),
            &pixsCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
            1,
            sizeof(cl_mem),
            &pixdCLBuffer);
        status = clSetKernelArg(rEnv.mpkKernel,
                2,
                sizeof(yp),
                (const void *)&yp);
        status = clSetKernelArg(rEnv.mpkKernel,
                3,
                sizeof(wpl),
                (const void *)&wpl);
        status = clSetKernelArg(rEnv.mpkKernel,
                4,
                sizeof(h),
                (const void *)&h);
        status = clSetKernelArg(rEnv.mpkKernel,
                5,
                sizeof(isAsymmetric),
                (const void *)&isAsymmetric);
        status = clSetKernelArg(rEnv.mpkKernel,
                6,
                sizeof(yn),
                (const void *)&yn);
        status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                                rEnv.mpkKernel,
                                2,
                                NULL,
                                globalThreads,
                                localThreads,
                                0,
                                NULL,
                                NULL);
    }

    return status;
}

// OpenCL implementation of Morphology Dilate
//Note: Assumes the source and dest opencl buffer are initialized. No check done
PIX* 
OpenclDevice::pixDilateBrickCL(PIX  *pixd, PIX  *pixs, l_int32  hsize, l_int32  vsize, bool reqDataCopy = false)
{
    l_uint32 wpl, h;

    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);
    
    clStatus = pixDilateCL(hsize, vsize, wpl, h);

    if (reqDataCopy)
    {
        pixd = mapOutputCLBuffer(rEnv, pixdCLBuffer, pixd, pixs, wpl*h, CL_MAP_READ, false);
    }

    return pixd;
}

// OpenCL implementation of Morphology Erode
//Note: Assumes the source and dest opencl buffer are initialized. No check done
PIX* 
OpenclDevice::pixErodeBrickCL(PIX  *pixd, PIX  *pixs, l_int32  hsize, l_int32  vsize, bool reqDataCopy = false)
{
    l_uint32 wpl, h;
    
    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);

    clStatus = pixErodeCL(hsize, vsize, wpl, h);
    
    if (reqDataCopy)
    {
        pixd = mapOutputCLBuffer(rEnv, pixdCLBuffer, pixd, pixs, wpl*h, CL_MAP_READ);
    }

    return pixd;
}

//Morphology Open operation. Invokes the relevant OpenCL kernels
cl_int
pixOpenCL(l_int32  hsize, l_int32  vsize, l_int32  wpl, l_int32  h)
{
    cl_int status;
    cl_mem pixtemp;
    
    //Erode followed by Dilate
    status = pixErodeCL(hsize, vsize, wpl, h);
    
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    status = pixDilateCL(hsize, vsize, wpl, h);

    return status;
}

//Morphology Close operation. Invokes the relevant OpenCL kernels
cl_int
pixCloseCL(l_int32  hsize, l_int32  vsize, l_int32  wpl, l_int32  h)
{
    cl_int status;
    cl_mem pixtemp;
    
    //Dilate followed by Erode
    status = pixDilateCL(hsize, vsize, wpl, h);
    
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    status = pixErodeCL(hsize, vsize, wpl, h);

    return status;
}

// OpenCL implementation of Morphology Close
//Note: Assumes the source and dest opencl buffer are initialized. No check done
PIX* 
OpenclDevice::pixCloseBrickCL(PIX  *pixd, 
                              PIX  *pixs, 
                              l_int32  hsize, 
                              l_int32  vsize, 
                              bool reqDataCopy = false)
{
    l_uint32 wpl, h;
    
    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);

    clStatus = pixCloseCL(hsize, vsize, wpl, h);

    if (reqDataCopy)
    {
        pixd = mapOutputCLBuffer(rEnv, pixdCLBuffer, pixd, pixs, wpl*h, CL_MAP_READ);
    }

    return pixd;
}

// OpenCL implementation of Morphology Open
//Note: Assumes the source and dest opencl buffer are initialized. No check done
PIX* 
OpenclDevice::pixOpenBrickCL(PIX  *pixd, 
                              PIX  *pixs, 
                              l_int32  hsize, 
                              l_int32  vsize, 
                              bool reqDataCopy = false)
{
    l_uint32 wpl, h;
    
    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);

    clStatus = pixOpenCL(hsize, vsize, wpl, h);

    if (reqDataCopy)
    {
        pixd = mapOutputCLBuffer(rEnv, pixdCLBuffer, pixd, pixs, wpl*h, CL_MAP_READ);
    }

    return pixd;
}

//pix OR operation: outbuffer = buffer1 | buffer2
cl_int
pixORCL_work(l_uint32 wpl, l_uint32 h, cl_mem buffer1, cl_mem buffer2, cl_mem outbuffer)
{
    cl_int status;
    size_t globalThreads[2];
    int gsize;
    size_t localThreads[] = {GROUPSIZE_X, GROUPSIZE_Y};

    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;

    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "pixOR", &status );

    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &buffer1);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &buffer2);
    status = clSetKernelArg(rEnv.mpkKernel,
        2,
        sizeof(cl_mem),
        &outbuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
            3,
            sizeof(wpl),
            (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
            4,
            sizeof(h),
            (const void *)&h);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);

    return status;
}

//pix AND operation: outbuffer = buffer1 & buffer2
cl_int
pixANDCL_work(l_uint32 wpl, l_uint32 h, cl_mem buffer1, cl_mem buffer2, cl_mem outbuffer)
{
    cl_int status;
    size_t globalThreads[2];
    int gsize;
    size_t localThreads[] = {GROUPSIZE_X, GROUPSIZE_Y};

    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;

    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "pixAND", &status );
                
    // Enqueue a kernel run call.
    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &buffer1);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &buffer2);
    status = clSetKernelArg(rEnv.mpkKernel,
        2,
        sizeof(cl_mem),
        &outbuffer);
    status = clSetKernelArg(rEnv.mpkKernel,
            3,
            sizeof(wpl),
            (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
            4,
            sizeof(h),
            (const void *)&h);
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);

    return status;
}

//output = buffer1 & ~(buffer2)
cl_int
pixSubtractCL_work(l_uint32 wpl, l_uint32 h, cl_mem buffer1, cl_mem buffer2, cl_mem outBuffer = NULL)
{
    cl_int status;
    size_t globalThreads[2];
    int gsize;
    size_t localThreads[] = {GROUPSIZE_X, GROUPSIZE_Y};

    gsize = (wpl + GROUPSIZE_X - 1)/ GROUPSIZE_X * GROUPSIZE_X;
    globalThreads[0] = gsize;
    gsize = (h + GROUPSIZE_Y - 1)/ GROUPSIZE_Y * GROUPSIZE_Y;
    globalThreads[1] = gsize;

    if (outBuffer != NULL)
    {
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "pixSubtract", &status );
    }
    else
    {
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "pixSubtract_inplace", &status );
    }

    // Enqueue a kernel run call.
    status = clSetKernelArg(rEnv.mpkKernel,
        0,
        sizeof(cl_mem),
        &buffer1);
    status = clSetKernelArg(rEnv.mpkKernel,
        1,
        sizeof(cl_mem),
        &buffer2);
    status = clSetKernelArg(rEnv.mpkKernel,
            2,
            sizeof(wpl),
            (const void *)&wpl);
    status = clSetKernelArg(rEnv.mpkKernel,
            3,
            sizeof(h),
            (const void *)&h);
    if (outBuffer != NULL)
    {
        status = clSetKernelArg(rEnv.mpkKernel,
            4,
            sizeof(cl_mem),
            (const void *)&outBuffer);
    }
    status = clEnqueueNDRangeKernel(rEnv.mpkCmdQueue,
                            rEnv.mpkKernel,
                            2,
                            NULL,
                            globalThreads,
                            localThreads,
                            0,
                            NULL,
                            NULL);

    return status;
}

// OpenCL implementation of Subtract pix
//Note: Assumes the source and dest opencl buffer are initialized. No check done
PIX* 
OpenclDevice::pixSubtractCL(PIX  *pixd, PIX  *pixs1, PIX  *pixs2, bool reqDataCopy = false)
{
    l_uint32 wpl, h;
    
    PROCNAME("pixSubtractCL");

    if (!pixs1)
        return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixd);
    if (!pixs2)
        return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixd);
    if (pixGetDepth(pixs1) != pixGetDepth(pixs2))
        return (PIX *)ERROR_PTR("depths of pixs* unequal", procName, pixd);

#if  EQUAL_SIZE_WARNING
    if (!pixSizesEqual(pixs1, pixs2))
        L_WARNING("pixs1 and pixs2 not equal sizes", procName);
#endif  /* EQUAL_SIZE_WARNING */

    wpl = pixGetWpl(pixs1);
    h = pixGetHeight(pixs1);

    clStatus = pixSubtractCL_work(wpl, h, pixdCLBuffer, pixsCLBuffer);

    if (reqDataCopy)
    {
        //Read back output data from OCL buffer to cpu
        pixd = mapOutputCLBuffer(rEnv, pixdCLBuffer, pixd, pixs1, wpl*h, CL_MAP_READ);
    }

    return pixd;
}

// OpenCL implementation of Hollow pix
//Note: Assumes the source and dest opencl buffer are initialized. No check done
PIX* 
OpenclDevice::pixHollowCL(PIX  *pixd, 
                        PIX  *pixs, 
                        l_int32  close_hsize, 
                        l_int32  close_vsize, 
                        l_int32  open_hsize, 
                        l_int32  open_vsize,
                        bool reqDataCopy = false)
{
    l_uint32 wpl, h;
    cl_mem pixtemp;

    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);

    //First step : Close Morph operation: Dilate followed by Erode
    clStatus = pixCloseCL(close_hsize, close_vsize, wpl, h);

    //Store the output of close operation in an intermediate buffer
    //this will be later used for pixsubtract
    clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixdCLBuffer, pixdCLIntermediate, 0, 0, sizeof(int) * wpl*h, 0, NULL, NULL);

    //Second step: Open Operation - Erode followed by Dilate
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    clStatus = pixOpenCL(open_hsize, open_vsize, wpl, h);

    //Third step: Subtract : (Close - Open)
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixdCLIntermediate;
    pixdCLIntermediate = pixtemp;

    clStatus = pixSubtractCL_work(wpl, h, pixdCLBuffer, pixsCLBuffer);

    if (reqDataCopy)
    {
        //Read back output data from OCL buffer to cpu
        pixd = mapOutputCLBuffer(rEnv, pixdCLBuffer, pixd, pixs, wpl*h, CL_MAP_READ);
    }
    return pixd;
}

// OpenCL implementation of Get Lines from pix function
//Note: Assumes the source and dest opencl buffer are initialized. No check done
void 
OpenclDevice::pixGetLinesCL(PIX  *pixd, 
                            PIX  *pixs, 
                            PIX** pix_vline, 
                            PIX** pix_hline, 
                            PIX** pixClosed,
                            bool  getpixClosed,
                            l_int32  close_hsize, l_int32  close_vsize, 
                            l_int32  open_hsize, l_int32  open_vsize,
                            l_int32  line_hsize, l_int32  line_vsize)
{
    l_uint32 wpl, h;
    cl_mem pixtemp;

    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);

    //First step : Close Morph operation: Dilate followed by Erode
    clStatus = pixCloseCL(close_hsize, close_vsize, wpl, h);

    //Copy the Close output to CPU buffer
    if (getpixClosed)
    {
        *pixClosed = mapOutputCLBuffer(rEnv, pixdCLBuffer, *pixClosed, pixs, wpl*h, CL_MAP_READ, true, false);
    }

    //Store the output of close operation in an intermediate buffer
    //this will be later used for pixsubtract
    clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixdCLBuffer, pixdCLIntermediate, 0, 0, sizeof(int) * wpl*h, 0, NULL, NULL);
    
    //Second step: Open Operation - Erode followed by Dilate
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    clStatus = pixOpenCL(open_hsize, open_vsize, wpl, h);

    //Third step: Subtract : (Close - Open)
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixdCLIntermediate;
    pixdCLIntermediate = pixtemp;

    clStatus = pixSubtractCL_work(wpl, h, pixdCLBuffer, pixsCLBuffer);

    //Store the output of Hollow operation in an intermediate buffer
    //this will be later used 
    clStatus = clEnqueueCopyBuffer(rEnv.mpkCmdQueue, pixdCLBuffer, pixdCLIntermediate, 0, 0, sizeof(int) * wpl*h, 0, NULL, NULL);

    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLBuffer;
    pixdCLBuffer = pixtemp;

    //Fourth step: Get vertical line
    //pixOpenBrick(NULL, pix_hollow, 1, min_line_length);
    clStatus = pixOpenCL(1, line_vsize, wpl, h);

    //Copy the vertical line output to CPU buffer
    *pix_vline = mapOutputCLBuffer(rEnv, pixdCLBuffer, *pix_vline, pixs, wpl*h, CL_MAP_READ, true, false);
    
    pixtemp = pixsCLBuffer;
    pixsCLBuffer = pixdCLIntermediate;
    pixdCLIntermediate = pixtemp;

    //Fifth step: Get horizontal line
    //pixOpenBrick(NULL, pix_hollow, min_line_length, 1);
    clStatus = pixOpenCL(line_hsize, 1, wpl, h);
        
    //Copy the horizontal line output to CPU buffer
    *pix_hline = mapOutputCLBuffer(rEnv, pixdCLBuffer, *pix_hline, pixs, wpl*h, CL_MAP_READ, true, true);

    return;
}


/*************************************************************************
 *  HistogramRect
 *  Otsu Thresholding Operations
 *  histogramAllChannels is layed out as all channel 0, then all channel 1...
 *  only supports 1 or 4 channels (bytes_per_pixel)
 ************************************************************************/
void OpenclDevice::HistogramRectOCL(
    const unsigned char* imageData,
    int bytes_per_pixel,
    int bytes_per_line,
    int left, // always 0
    int top, // always 0
    int width,
    int height,
    int kHistogramSize,
    int* histogramAllChannels)
{
PERF_COUNT_START("HistogramRectOCL")
    cl_int clStatus;
    KernelEnv histKern;
    SetKernelEnv( &histKern );
    KernelEnv histRedKern;
    SetKernelEnv( &histRedKern );
    /* map imagedata to device as read only */
    // USE_HOST_PTR uses onion+ bus which is slowest option; also happens to be coherent which we don't need.
    // faster option would be to allocate initial image buffer
    // using a garlic bus memory type
    cl_mem imageBuffer = clCreateBuffer( histKern.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, width*height*bytes_per_pixel*sizeof(char), (void *)imageData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer imageBuffer");

    /* setup work group size parameters */
    int block_size = 256;
    cl_uint numCUs;
    clStatus = clGetDeviceInfo( gpuEnv.mpDevID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numCUs), &numCUs, NULL);
    CHECK_OPENCL( clStatus, "clCreateBuffer imageBuffer");

    int requestedOccupancy = 10;
    int numWorkGroups = numCUs * requestedOccupancy;
    int numThreads = block_size*numWorkGroups;
    size_t local_work_size[] = {block_size};
    size_t global_work_size[] = {numThreads};
    size_t red_global_work_size[] = {block_size*kHistogramSize*bytes_per_pixel}; 

    /* map histogramAllChannels as write only */
    int numBins = kHistogramSize*bytes_per_pixel*numWorkGroups;
    
    cl_mem histogramBuffer = clCreateBuffer( histKern.mpkContext, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, kHistogramSize*bytes_per_pixel*sizeof(int), (void *)histogramAllChannels, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer histogramBuffer");

    /* intermediate histogram buffer */
    int histRed = 256;
    int tmpHistogramBins =  kHistogramSize*bytes_per_pixel*histRed; 

    cl_mem tmpHistogramBuffer = clCreateBuffer( histKern.mpkContext, CL_MEM_READ_WRITE, tmpHistogramBins*sizeof(cl_uint), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer tmpHistogramBuffer");

    /* atomic sync buffer */
    int *zeroBuffer = new int[1];
    zeroBuffer[0] = 0;
    cl_mem atomicSyncBuffer = clCreateBuffer( histKern.mpkContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), (void *)zeroBuffer, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer atomicSyncBuffer");

    //Create kernel objects based on bytes_per_pixel
    if (bytes_per_pixel == 1)
    {
        histKern.mpkKernel = clCreateKernel( histKern.mpkProgram, "kernel_HistogramRectOneChannel", &clStatus );
        CHECK_OPENCL( clStatus, "clCreateKernel kernel_HistogramRectOneChannel");

        histRedKern.mpkKernel = clCreateKernel( histRedKern.mpkProgram, "kernel_HistogramRectOneChannelReduction", &clStatus );
        CHECK_OPENCL( clStatus, "clCreateKernel kernel_HistogramRectOneChannelReduction");
    } else {
    histKern.mpkKernel = clCreateKernel( histKern.mpkProgram, "kernel_HistogramRectAllChannels", &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel kernel_HistogramRectAllChannels");

    histRedKern.mpkKernel = clCreateKernel( histRedKern.mpkProgram, "kernel_HistogramRectAllChannelsReduction", &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel kernel_HistogramRectAllChannelsReduction");
    }

    void *ptr;
    
    //Initialize tmpHistogramBuffer buffer
    ptr = clEnqueueMapBuffer(histKern.mpkCmdQueue, tmpHistogramBuffer, CL_TRUE, CL_MAP_WRITE, 0, tmpHistogramBins*sizeof(cl_uint), 0, NULL, NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer tmpHistogramBuffer");
    
    memset(ptr, 0, tmpHistogramBins*sizeof(cl_uint));
    clEnqueueUnmapMemObject(histKern.mpkCmdQueue, tmpHistogramBuffer, ptr, 0, NULL, NULL);

    /* set kernel 1 arguments */
    clStatus = clSetKernelArg( histKern.mpkKernel, 0, sizeof(cl_mem), (void *)&imageBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg imageBuffer");
    cl_uint numPixels = width*height;
    clStatus = clSetKernelArg( histKern.mpkKernel, 1, sizeof(cl_uint), (void *)&numPixels );
    CHECK_OPENCL( clStatus, "clSetKernelArg numPixels" );
    clStatus = clSetKernelArg( histKern.mpkKernel, 2, sizeof(cl_mem), (void *)&tmpHistogramBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg tmpHistogramBuffer");

    /* set kernel 2 arguments */
    int n = numThreads/bytes_per_pixel;
    clStatus = clSetKernelArg( histRedKern.mpkKernel, 0, sizeof(cl_int), (void *)&n );
    CHECK_OPENCL( clStatus, "clSetKernelArg imageBuffer");
    clStatus = clSetKernelArg( histRedKern.mpkKernel, 1, sizeof(cl_mem), (void *)&tmpHistogramBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg tmpHistogramBuffer");
    clStatus = clSetKernelArg( histRedKern.mpkKernel, 2, sizeof(cl_mem), (void *)&histogramBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg histogramBuffer");

    /* launch histogram */
PERF_COUNT_SUB("before")
    clStatus = clEnqueueNDRangeKernel(
        histKern.mpkCmdQueue,
        histKern.mpkKernel,
        1, NULL, global_work_size, local_work_size,
        0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel kernel_HistogramRectAllChannels" );
    clFinish( histKern.mpkCmdQueue );

    /* launch histogram */
    clStatus = clEnqueueNDRangeKernel(
        histRedKern.mpkCmdQueue,
        histRedKern.mpkKernel,
        1, NULL, red_global_work_size, local_work_size,
        0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel kernel_HistogramRectAllChannelsReduction" );
    clFinish( histRedKern.mpkCmdQueue );

PERF_COUNT_SUB("redKernel")

    /* map results back from gpu */
    ptr = clEnqueueMapBuffer(histRedKern.mpkCmdQueue, histogramBuffer, CL_TRUE, CL_MAP_READ, 0, kHistogramSize*bytes_per_pixel*sizeof(int), 0, NULL, NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer histogramBuffer");
    
    clEnqueueUnmapMemObject(histRedKern.mpkCmdQueue, histogramBuffer, ptr, 0, NULL, NULL);
   
    clReleaseMemObject(histogramBuffer);
    clReleaseMemObject(imageBuffer);
PERF_COUNT_SUB("after")
PERF_COUNT_END

}

/*************************************************************************
 * Threshold the rectangle, taking everything except the image buffer pointer
 * from the class, using thresholds/hi_values to the output IMAGE.
 * only supports 1 or 4 channels
 ************************************************************************/
void OpenclDevice::ThresholdRectToPixOCL(
    const unsigned char* imageData,
    int bytes_per_pixel,
    int bytes_per_line,
    const int* thresholds,
    const int* hi_values,
    Pix** pix,
    int height,
    int width,
    int top,
    int left) {
PERF_COUNT_START("ThresholdRectToPixOCL")

    /* create pix result buffer */                                 
    *pix = pixCreate(width, height, 1);
    uinT32* pixData = pixGetData(*pix);
    int wpl = pixGetWpl(*pix);
    int pixSize = wpl*height*sizeof(uinT32);

    cl_int clStatus;
    KernelEnv rEnv;
    SetKernelEnv( &rEnv );

    /* setup work group size parameters */
    int block_size = 256;
    cl_uint numCUs = 6;
     clStatus = clGetDeviceInfo( gpuEnv.mpDevID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numCUs), &numCUs, NULL);
    CHECK_OPENCL( clStatus, "clCreateBuffer imageBuffer");

    int requestedOccupancy = 10;
    int numWorkGroups = numCUs * requestedOccupancy;
    int numThreads = block_size*numWorkGroups;
    size_t local_work_size[] = {(size_t) block_size};
    size_t global_work_size[] = {(size_t) numThreads};

    /* map imagedata to device as read only */
    // USE_HOST_PTR uses onion+ bus which is slowest option; also happens to be coherent which we don't need.
    // faster option would be to allocate initial image buffer
    // using a garlic bus memory type
    cl_mem imageBuffer = clCreateBuffer( rEnv.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, width*height*bytes_per_pixel*sizeof(char), (void *)imageData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer imageBuffer");

    /* map pix as write only */
    pixThBuffer = clCreateBuffer( rEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, pixSize, (void *)pixData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer pix");

    /* map thresholds and hi_values */
    cl_mem thresholdsBuffer = clCreateBuffer( rEnv.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bytes_per_pixel*sizeof(int), (void *)thresholds, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer thresholdBuffer");
    cl_mem hiValuesBuffer   = clCreateBuffer( rEnv.mpkContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bytes_per_pixel*sizeof(int), (void *)hi_values, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer hiValuesBuffer");

    /* compile kernel */
    if (bytes_per_pixel == 4) {
    rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "kernel_ThresholdRectToPix", &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel kernel_ThresholdRectToPix");
    } else {
        rEnv.mpkKernel = clCreateKernel( rEnv.mpkProgram, "kernel_ThresholdRectToPix_OneChan", &clStatus );
        CHECK_OPENCL( clStatus, "clCreateKernel kernel_ThresholdRectToPix_OneChan");
    }

    /* set kernel arguments */
    clStatus = clSetKernelArg( rEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&imageBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg imageBuffer");
    cl_uint numPixels = width*height;
    clStatus = clSetKernelArg( rEnv.mpkKernel, 1, sizeof(int), (void *)&height );
    CHECK_OPENCL( clStatus, "clSetKernelArg height" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 2, sizeof(int), (void *)&width );
    CHECK_OPENCL( clStatus, "clSetKernelArg width" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 3, sizeof(int), (void *)&wpl );
    CHECK_OPENCL( clStatus, "clSetKernelArg wpl" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 4, sizeof(cl_mem), (void *)&thresholdsBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg thresholdsBuffer" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 5, sizeof(cl_mem), (void *)&hiValuesBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg hiValuesBuffer" );
    clStatus = clSetKernelArg( rEnv.mpkKernel, 6, sizeof(cl_mem), (void *)&pixThBuffer );
    CHECK_OPENCL( clStatus, "clSetKernelArg pixThBuffer");

    /* launch kernel & wait */
PERF_COUNT_SUB("before")
    clStatus = clEnqueueNDRangeKernel(
        rEnv.mpkCmdQueue,
        rEnv.mpkKernel,
        1, NULL, global_work_size, local_work_size,
        0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel kernel_ThresholdRectToPix" );
    clFinish( rEnv.mpkCmdQueue );
PERF_COUNT_SUB("kernel")
    
    /* map results back from gpu */
    void *ptr = clEnqueueMapBuffer(rEnv.mpkCmdQueue, pixThBuffer, CL_TRUE, CL_MAP_READ, 0, pixSize, 0, NULL, NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer histogramBuffer");
    clEnqueueUnmapMemObject(rEnv.mpkCmdQueue, pixThBuffer, ptr, 0, NULL, NULL);
    
    clReleaseMemObject(imageBuffer);
    clReleaseMemObject(thresholdsBuffer);
    clReleaseMemObject(hiValuesBuffer);

PERF_COUNT_SUB("after")
PERF_COUNT_END
}


#if USE_DEVICE_SELECTION

/******************************************************************************
 * Data Types for Device Selection
 *****************************************************************************/

typedef struct _TessScoreEvaluationInputData {
    int height;
    int width;
    int numChannels;
    unsigned char *imageData;
    Pix *pix;
} TessScoreEvaluationInputData;

void populateTessScoreEvaluationInputData( TessScoreEvaluationInputData *input ) {
    srand(1);
    // 8.5x11 inches @ 300dpi rounded to clean multiples
    int height = 3328; // %256
    int width = 2560; // %512
    int numChannels = 4;
    input->height = height;
    input->width = width;
    input->numChannels = numChannels;
    unsigned char (*imageData4)[4] = (unsigned char (*)[4]) malloc(height*width*numChannels*sizeof(unsigned char)); // new unsigned char[4][height*width];
    input->imageData = (unsigned char *) &imageData4[0];
    
    // zero out image
    unsigned char pixelWhite[4] = {  0,   0,   0, 255};
    unsigned char pixelBlack[4] = {255, 255, 255, 255};
    for (int p = 0; p < height*width; p++) {
        //unsigned char tmp[4] = imageData4[0];
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
        int lineWidth = rand()%maxLineWidth;
        int vertLinePos = lineWidth + rand()%(width-2*lineWidth);
        //printf("[PI] VerticalLine @ %i (w=%i)\n", vertLinePos, lineWidth);
        for (int row = vertLinePos-lineWidth/2; row < vertLinePos+lineWidth/2; row++) {
            for (int col = 0; col < height; col++) {
                //imageData4[row*width+col] = pixelBlack;
                imageData4[row*width+col][0] = pixelBlack[0];
                imageData4[row*width+col][1] = pixelBlack[1];
                imageData4[row*width+col][2] = pixelBlack[2];
                imageData4[row*width+col][3] = pixelBlack[3];
            }
        }
    }
    // horizontal lines
    for (int i = 0; i < numLines; i++) {
        int lineWidth = rand()%maxLineWidth;
        int horLinePos = lineWidth + rand()%(height-2*lineWidth);
        //printf("[PI] HorizontalLine @ %i (w=%i)\n", horLinePos, lineWidth);
        for (int row = 0; row < width; row++) {
            for (int col = horLinePos-lineWidth/2; col < horLinePos+lineWidth/2; col++) { // for (int row = vertLinePos-lineWidth/2; row < vertLinePos+lineWidth/2; row++) {
                //printf("[PI] HoizLine pix @ (%3i, %3i)\n", row, col);
                //imageData4[row*width+col] = pixelBlack;
                imageData4[row*width+col][0] = pixelBlack[0];
                imageData4[row*width+col][1] = pixelBlack[1];
                imageData4[row*width+col][2] = pixelBlack[2];
                imageData4[row*width+col][3] = pixelBlack[3];
            }
        }
    }
    // spots (noise, squares)
    float fractionBlack = 0.1; // how much of the image should be blackened
    int numSpots = (height*width)*fractionBlack/(maxLineWidth*maxLineWidth/2/2);
    for (int i = 0; i < numSpots; i++) {
        
        int lineWidth = rand()%maxLineWidth;
        int col = lineWidth + rand()%(width-2*lineWidth);
        int row = lineWidth + rand()%(height-2*lineWidth);
        //printf("[PI] Spot[%i/%i] @ (%3i, %3i)\n", i, numSpots, row, col );
        for (int r = row-lineWidth/2; r < row+lineWidth/2; r++) {
            for (int c = col-lineWidth/2; c < col+lineWidth/2; c++) {
                //printf("[PI] \tSpot[%i/%i] @ (%3i, %3i)\n", i, numSpots, r, c );
                //imageData4[row*width+col] = pixelBlack;
                imageData4[r*width+c][0] = pixelBlack[0];
                imageData4[r*width+c][1] = pixelBlack[1];
                imageData4[r*width+c][2] = pixelBlack[2];
                imageData4[r*width+c][3] = pixelBlack[3];
            }
        }
    }

    input->pix = pixCreate(input->width, input->height, 1);
}

typedef struct _TessDeviceScore {
    float time; // small time means faster device
    bool clError; // were there any opencl errors
    bool valid; // was the correct response generated
} TessDeviceScore;

/******************************************************************************
 * Micro Benchmarks for Device Selection
 *****************************************************************************/

double composeRGBPixelMicroBench( GPUEnv *env, TessScoreEvaluationInputData input, ds_device_type type ) {
    
    double time = 0;
#if ON_WINDOWS
    LARGE_INTEGER freq, time_funct_start, time_funct_end;
    QueryPerformanceFrequency(&freq);
#else
    timespec time_funct_start, time_funct_end;
#endif
    // input data
    l_uint32 *tiffdata = (l_uint32 *)input.imageData;// same size and random data; data doesn't change workload

    // function call
    if (type == DS_DEVICE_OPENCL_DEVICE) {
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif

        OpenclDevice::gpuEnv = *env;
        int wpl = pixGetWpl(input.pix);
        OpenclDevice::pixReadFromTiffKernel(tiffdata, input.width, input.height, wpl, NULL);
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif

    } else {
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif
        Pix *pix = pixCreate(input.width, input.height, 32);
        l_uint32 *pixData = pixGetData(pix);
        int wpl = pixGetWpl(pix);
        //l_uint32* output_gpu=pixReadFromTiffKernel(tiffdata,w,h,wpl,line);
        //pixSetData(pix, output_gpu);
        int i, j;
        int idx = 0;
        for (i = 0; i < input.height ; i++) {
            for (j = 0; j < input.width; j++) {
                
                l_uint32 tiffword = tiffdata[i * input.width + j];
                l_int32 rval = ((tiffword) & 0xff);
                l_int32 gval = (((tiffword) >> 8) & 0xff);
                l_int32 bval = (((tiffword) >> 16) & 0xff);
                l_uint32 value = (rval << 24) | (gval << 16) | (bval << 8);
                pixData[idx] = value;
                idx++;
            }
        }
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
        pixDestroy(&pix);
    }


    // cleanup

    return time;
}

double histogramRectMicroBench( GPUEnv *env, TessScoreEvaluationInputData input, ds_device_type type ) {
    
    double time;
#if ON_WINDOWS
    LARGE_INTEGER freq, time_funct_start, time_funct_end;
    QueryPerformanceFrequency(&freq);
#else
    timespec time_funct_start, time_funct_end;
#endif
    
    unsigned char pixelHi = (unsigned char)255;
    
    int left = 0;
    int top = 0;
    int kHistogramSize = 256;
    int bytes_per_line = input.width*input.numChannels;
    int *histogramAllChannels = new int[kHistogramSize*input.numChannels];

    // function call
    if (type == DS_DEVICE_OPENCL_DEVICE) {
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif

        OpenclDevice::gpuEnv = *env;
        int wpl = pixGetWpl(input.pix);
        OpenclDevice::HistogramRectOCL(input.imageData, input.numChannels, bytes_per_line, top, left, input.width, input.height, kHistogramSize, histogramAllChannels);

#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
    } else {
    
        int *histogram = new int[kHistogramSize];
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif
        for (int ch = 0; ch < input.numChannels; ++ch) { 
            tesseract::HistogramRect(input.pix, input.numChannels,
                  left, top, input.width, input.height, histogram);
        }
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
        delete[] histogram;
    }

    // cleanup
    //delete[] imageData;
    delete[] histogramAllChannels;
    return time;
}

//Reproducing the ThresholdRectToPix native version
void ThresholdRectToPix_Native(const unsigned char* imagedata,
                                          int bytes_per_pixel,
                                          int bytes_per_line,
                                          const int* thresholds,
                                          const int* hi_values,
                                          Pix** pix) {
    int top = 0;
    int left = 0;
    int width = pixGetWidth(*pix);
    int height = pixGetHeight(*pix);

  *pix = pixCreate(width, height, 1);
  uinT32* pixdata = pixGetData(*pix);
  int wpl = pixGetWpl(*pix);
  const unsigned char* srcdata = imagedata + top * bytes_per_line +
                                 left * bytes_per_pixel;
  for (int y = 0; y < height; ++y) {
    const uinT8* linedata = srcdata;
    uinT32* pixline = pixdata + y * wpl;
    for (int x = 0; x < width; ++x, linedata += bytes_per_pixel) {
      bool white_result = true;
      for (int ch = 0; ch < bytes_per_pixel; ++ch) {
        if (hi_values[ch] >= 0 &&
            (linedata[ch] > thresholds[ch]) == (hi_values[ch] == 0)) {
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

double thresholdRectToPixMicroBench( GPUEnv *env, TessScoreEvaluationInputData input, ds_device_type type ) {
    
    double time;
#if ON_WINDOWS
    LARGE_INTEGER freq, time_funct_start, time_funct_end;
    QueryPerformanceFrequency(&freq);
#else
    timespec time_funct_start, time_funct_end;
#endif
    
    // input data
    unsigned char pixelHi = (unsigned char)255;
    int* thresholds = new int[4];
    thresholds[0] = pixelHi/2;
    thresholds[1] = pixelHi/2;
    thresholds[2] = pixelHi/2;
    thresholds[3] = pixelHi/2;
    int *hi_values = new int[4];
    thresholds[0] = pixelHi;
    thresholds[1] = pixelHi;
    thresholds[2] = pixelHi;
    thresholds[3] = pixelHi;
    //Pix* pix = pixCreate(width, height, 1);
    int top = 0;
    int left = 0;
    int bytes_per_line = input.width*input.numChannels;

    // function call
    if (type == DS_DEVICE_OPENCL_DEVICE) {
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif

        OpenclDevice::gpuEnv = *env;
        int wpl = pixGetWpl(input.pix);
        OpenclDevice::ThresholdRectToPixOCL(input.imageData, input.numChannels, bytes_per_line, thresholds, hi_values, &input.pix, input.height, input.width, top, left);

#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
    } else {


        tesseract::ImageThresholder thresholder;
        thresholder.SetImage( input.pix );
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif
        ThresholdRectToPix_Native( input.imageData, input.numChannels, bytes_per_line,
            thresholds, hi_values, &input.pix );

#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
    }

    // cleanup
    delete[] thresholds;
    delete[] hi_values;
    return time;
}

double getLineMasksMorphMicroBench( GPUEnv *env, TessScoreEvaluationInputData input, ds_device_type type ) {

    double time = 0;
#if ON_WINDOWS
    LARGE_INTEGER freq, time_funct_start, time_funct_end;
    QueryPerformanceFrequency(&freq);
#else
    timespec time_funct_start, time_funct_end;
#endif

    // input data
    int resolution = 300;
    int wpl = pixGetWpl(input.pix);
    int kThinLineFraction = 20; // tess constant
    int kMinLineLengthFraction = 4; // tess constant
    int max_line_width = resolution / kThinLineFraction;
    int min_line_length = resolution / kMinLineLengthFraction;
    int closing_brick = max_line_width / 3;
   
    // function call
    if (type == DS_DEVICE_OPENCL_DEVICE) {
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif
        Pix *src_pix = input.pix;
        OpenclDevice::gpuEnv = *env;
        OpenclDevice::initMorphCLAllocations(wpl, input.height, input.pix);
        Pix *pix_vline = NULL, *pix_hline = NULL, *pix_closed = NULL;
        OpenclDevice::pixGetLinesCL(NULL, input.pix, &pix_vline, &pix_hline, &pix_closed, true, closing_brick, closing_brick, max_line_width, max_line_width, min_line_length, min_line_length);

        OpenclDevice::releaseMorphCLBuffers();

#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
    } else {
#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_start);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_start );
#endif

        // native serial code
        Pix *src_pix = input.pix;
        Pix *pix_closed = pixCloseBrick(NULL, src_pix, closing_brick, closing_brick);
        Pix *pix_solid  = pixOpenBrick(NULL, pix_closed, max_line_width, max_line_width);
        Pix *pix_hollow = pixSubtract(NULL, pix_closed, pix_solid);
        pixDestroy(&pix_solid);
        Pix *pix_vline = pixOpenBrick(NULL, pix_hollow, 1, min_line_length);
        Pix *pix_hline = pixOpenBrick(NULL, pix_hollow, min_line_length, 1);
        pixDestroy(&pix_hollow);

#if ON_WINDOWS
        QueryPerformanceCounter(&time_funct_end);
        time = (time_funct_end.QuadPart-time_funct_start.QuadPart)/(double)(freq.QuadPart);
#else
        clock_gettime( CLOCK_MONOTONIC, &time_funct_end );
        time = (time_funct_end.tv_sec - time_funct_start.tv_sec)*1.0 + (time_funct_end.tv_nsec - time_funct_start.tv_nsec)/1000000000.0;
#endif
    }

    return time;
}



/******************************************************************************
 * Device Selection
 *****************************************************************************/

#include "stdlib.h"

 
// encode score object as byte string
ds_status serializeScore( ds_device* device, void **serializedScore, unsigned int* serializedScoreSize ) {
    *serializedScoreSize = sizeof(TessDeviceScore);
    *serializedScore = (void *) new unsigned char[*serializedScoreSize];
    memcpy(*serializedScore, device->score, *serializedScoreSize);
    return DS_SUCCESS;
}

// parses byte string and stores in score object
ds_status deserializeScore( ds_device* device, const unsigned char* serializedScore, unsigned int serializedScoreSize ) {
    // check that serializedScoreSize == sizeof(TessDeviceScore);
    device->score = new TessDeviceScore;
    memcpy(device->score, serializedScore, serializedScoreSize);
    return DS_SUCCESS;
}



// evaluate devices
ds_status evaluateScoreForDevice( ds_device *device, void *inputData) {
    
    // overwrite statuc gpuEnv w/ current device
    // so native opencl calls can be used; they use static gpuEnv
    printf("\n[DS] Device: \"%s\" (%s) evaluation...\n", device->oclDeviceName, device->type==DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native" );
    GPUEnv *env = NULL;
    if (device->type == DS_DEVICE_OPENCL_DEVICE) {
        env = new GPUEnv;
        //printf("[DS] populating tmp GPUEnv from device\n");
        populateGPUEnvFromDevice( env, device->oclDeviceID);
        env->mnFileCount = 0; //argc;
        env->mnKernelCount = 0UL;
        //printf("[DS] compiling kernels for tmp GPUEnv\n");
        OpenclDevice::gpuEnv = *env;
        OpenclDevice::CompileKernelFile(env, "");
    }
    

    TessScoreEvaluationInputData *input = (TessScoreEvaluationInputData *)inputData;
    
    // pixReadTiff
    double composeRGBPixelTime = composeRGBPixelMicroBench( env, *input, device->type );

    // HistogramRect
    double histogramRectTime = histogramRectMicroBench( env, *input, device->type );

    // ThresholdRectToPix
    double thresholdRectToPixTime = thresholdRectToPixMicroBench( env, *input, device->type );

    // getLineMasks
    double getLineMasksMorphTime = getLineMasksMorphMicroBench( env, *input, device->type );


    // weigh times (% of cpu time)
    // these weights should be the % execution time that the native cpu code took
    float composeRGBPixelWeight     = 1.2f;
    float histogramRectWeight       = 2.4f;
    float thresholdRectToPixWeight  = 4.5f;
    float getLineMasksMorphWeight        = 5.0f;
    
    float weightedTime = 
        composeRGBPixelWeight       * composeRGBPixelTime +
        histogramRectWeight         * histogramRectTime +
        thresholdRectToPixWeight    * thresholdRectToPixTime +
        getLineMasksMorphWeight     * getLineMasksMorphTime
        ;
    device->score = (void *)new TessDeviceScore;
    ((TessDeviceScore *)device->score)->time = weightedTime;
    
    printf("[DS] Device: \"%s\" (%s) evaluated\n", device->oclDeviceName, device->type==DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native" );
    printf("[DS]%25s: %f (w=%.1f)\n", "composeRGBPixel", composeRGBPixelTime, composeRGBPixelWeight );
    printf("[DS]%25s: %f (w=%.1f)\n", "HistogramRect", histogramRectTime, histogramRectWeight );
    printf("[DS]%25s: %f (w=%.1f)\n", "ThresholdRectToPix", thresholdRectToPixTime, thresholdRectToPixWeight );
    printf("[DS]%25s: %f (w=%.1f)\n", "getLineMasksMorph", getLineMasksMorphTime, getLineMasksMorphWeight );
    printf("[DS]%25s: %f\n", "Score", ((TessDeviceScore *)device->score)->time );
    return DS_SUCCESS;
}

// initial call to select device
ds_device OpenclDevice::getDeviceSelection( ) {
//PERF_COUNT_START("getDeviceSelection")
    if (!deviceIsSelected) {
PERF_COUNT_START("getDeviceSelection")
        // check if opencl is available at runtime
        if( 1 == LoadOpencl() ) {
            // opencl is available
//PERF_COUNT_SUB("LoadOpencl")
    // setup devices
    ds_status status;
    ds_profile *profile;
    status = initDSProfile( &profile, "v0.1" );
PERF_COUNT_SUB("initDSProfile")
    // try reading scores from file
    char *fileName = "tesseract_opencl_profile_devices.dat";
    status = readProfileFromFile( profile, deserializeScore, fileName);
    if (status != DS_SUCCESS) {
        // need to run evaluation
                printf("[DS] Profile file not available (%s); performing profiling.\n", fileName);

        // create input data
        TessScoreEvaluationInputData input;
                populateTessScoreEvaluationInputData( &input );
//PERF_COUNT_SUB("populateTessScoreEvaluationInputData")
        // perform evaluations
        unsigned int numUpdates;
        status =  profileDevices( profile, DS_EVALUATE_ALL, evaluateScoreForDevice, (void *)&input, &numUpdates );
PERF_COUNT_SUB("profileDevices")
        // write scores to file
        if ( status == DS_SUCCESS ) {
            status = writeProfileToFile( profile, serializeScore, fileName);
PERF_COUNT_SUB("writeProfileToFile")
            if ( status == DS_SUCCESS ) {
                        printf("[DS] Scores written to file (%s).\n", fileName);
            } else {
                        printf("[DS] Error saving scores to file (%s); scores not written to file.\n", fileName);
            }
        } else {
                    printf("[DS] Unable to evaluate performance; scores not written to file.\n");
        }

    } else {

PERF_COUNT_SUB("readProfileFromFile")
                printf("[DS] Profile read from file (%s).\n", fileName);
    }

    // we now have device scores either from file or evaluation
    // select fastest using custom Tesseract selection algorithm
    float bestTime = FLT_MAX; // begin search with worst possible time
    int bestDeviceIdx = -1;
    for (int d = 0; d < profile->numDevices; d++) {
        //((TessDeviceScore *)device->score)->time
        ds_device device = profile->devices[d];
        TessDeviceScore score = *(TessDeviceScore *)device.score;
        
        float time = score.time;
                printf("[DS] Device[%i] %i:%s score is %f\n", d+1, device.type, device.oclDeviceName, time);
        if (time < bestTime) {
                    bestTime = time;
            bestDeviceIdx = d;
        }
    }
            printf("[DS] Selected Device[%i]: \"%s\" (%s)\n", bestDeviceIdx+1, profile->devices[bestDeviceIdx].oclDeviceName, profile->devices[bestDeviceIdx].type==DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native");
    // cleanup
            // TODO: call destructor for profile object?

            bool overrided = false;
            char *overrideDeviceStr = getenv("TESSERACT_OPENCL_DEVICE");
            if (overrideDeviceStr != NULL) {
                int overrideDeviceIdx = atoi(overrideDeviceStr);
                if (overrideDeviceIdx > 0 && overrideDeviceIdx <= profile->numDevices ) {
                    printf("[DS] Overriding Device Selection (TESSERACT_OPENCL_DEVICE=%s, %i)\n", overrideDeviceStr, overrideDeviceIdx);
                    bestDeviceIdx = overrideDeviceIdx - 1;
                    overrided = true;
                } else {
                    printf("[DS] Ignoring invalid TESSERACT_OPENCL_DEVICE=%s ([1,%i] are valid devices).\n", overrideDeviceStr, profile->numDevices);
                }
}

            if (overrided) {
                printf("[DS] Overridden Device[%i]: \"%s\" (%s)\n", bestDeviceIdx+1, profile->devices[bestDeviceIdx].oclDeviceName, profile->devices[bestDeviceIdx].type==DS_DEVICE_OPENCL_DEVICE ? "OpenCL" : "Native");
            }
            selectedDevice = profile->devices[bestDeviceIdx];

        } else {
            // opencl isn't available at runtime, select native cpu device
            printf("[DS] OpenCL runtime not available.\n");
            selectedDevice.type = DS_DEVICE_NATIVE_CPU;
            selectedDevice.oclDeviceName = "(null)";
            selectedDevice.score = NULL;
            selectedDevice.oclDeviceID = NULL;
            selectedDevice.oclDriverVersion = NULL;
        }
        deviceIsSelected = true;
PERF_COUNT_SUB("select from Profile")
PERF_COUNT_END
    }
//PERF_COUNT_END
    return selectedDevice;
}

#endif

bool OpenclDevice::selectedDeviceIsOpenCL() {
#if USE_DEVICE_SELECTION
    ds_device device = getDeviceSelection();
    return (device.type == DS_DEVICE_OPENCL_DEVICE);
#else
    return true;
#endif
}

bool OpenclDevice::selectedDeviceIsNativeCPU() {
#if USE_DEVICE_SELECTION
    ds_device device = getDeviceSelection();
    return (device.type == DS_DEVICE_NATIVE_CPU);
#else
    return false;
#endif
}



#endif
