#include <stdio.h>
#include "allheaders.h"
#include "pix.h"
#include "tiff.h"
#include "tiffio.h"
#include "tprintf.h"
// including CL/cl.h doesn't occur until USE_OPENCL defined below


/**************************************************************************
 * enable/disable use of OpenCL
 **************************************************************************/

#ifdef USE_OPENCL

#define USE_DEVICE_SELECTION 0

#include "CL/cl.h"
#ifndef strcasecmp
#define strcasecmp strcmp
#endif

#define MAX_KERNEL_STRING_LEN 64
#define MAX_CLFILE_NUM 50
#define MAX_CLKERNEL_NUM 200
#define MAX_KERNEL_NAME_LEN 64
#define CL_QUEUE_THREAD_HANDLE_AMD 0x403E
#define GROUPSIZE_X 16
#define GROUPSIZE_Y 16
#define GROUPSIZE_HMORX 256
#define GROUPSIZE_HMORY 1

typedef struct _KernelEnv
{
    cl_context mpkContext;
    cl_command_queue mpkCmdQueue;
    cl_program mpkProgram;
    cl_kernel mpkKernel;
    char mckKernelName[150];
} KernelEnv;

typedef struct _OpenCLEnv
{
    cl_platform_id mpOclPlatformID;
    cl_context mpOclContext;
    cl_device_id mpOclDevsID;
    cl_command_queue mpOclCmdQueue;
} OpenCLEnv;
typedef int ( *cl_kernel_function )( void **userdata, KernelEnv *kenv );


static l_int32 MORPH_BC = ASYMMETRIC_MORPH_BC;

static const l_uint32 lmask32[] = {0x0,
    0x80000000, 0xc0000000, 0xe0000000, 0xf0000000,
    0xf8000000, 0xfc000000, 0xfe000000, 0xff000000,
    0xff800000, 0xffc00000, 0xffe00000, 0xfff00000,
    0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000,
    0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
    0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00,
    0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0,
    0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff};

static const l_uint32 rmask32[] = {0x0,
    0x00000001, 0x00000003, 0x00000007, 0x0000000f,
    0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
    0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
    0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
    0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
    0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
    0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff};

#define CHECK_OPENCL(status,name)    \
if( status != CL_SUCCESS )    \
{    \
    printf ("OpenCL error code is %d at   when %s .\n", status, name);    \
}


typedef struct _GPUEnv
{
    //share vb in all modules in hb library
    cl_platform_id mpPlatformID;
    cl_device_type mDevType;
    cl_context mpContext;
    cl_device_id *mpArryDevsID;
    cl_device_id mpDevID;
    cl_command_queue mpCmdQueue;
    cl_kernel mpArryKernels[MAX_CLFILE_NUM];
    cl_program mpArryPrograms[MAX_CLFILE_NUM]; //one program object maps one kernel source file
    char mArryKnelSrcFile[MAX_CLFILE_NUM][256], //the max len of kernel file name is 256
         mArrykernelNames[MAX_CLKERNEL_NUM][MAX_KERNEL_STRING_LEN + 1];
         cl_kernel_function mpArryKnelFuncs[MAX_CLKERNEL_NUM];
    int mnKernelCount, mnFileCount, // only one kernel file
        mnIsUserCreated; // 1: created , 0:no create and needed to create by opencl wrapper
    int mnKhrFp64Flag;
    int mnAmdFp64Flag;

} GPUEnv;


class OpenclDevice
{

public:
    static GPUEnv gpuEnv;
    static int isInited;
    OpenclDevice();
    ~OpenclDevice();
    static int InitEnv(); // load dll, call InitOpenclRunEnv(0)
    static int InitOpenclRunEnv( int argc ); // RegistOpenclKernel, double flags, compile kernels
    static int InitOpenclRunEnv_DeviceSelection( int argc ); // RegistOpenclKernel, double flags, compile kernels
    static int InitOpenclRunEnv( GPUEnv *gpu ); // select device by env_CPU or selector
    static int RegistOpenclKernel();
    static int ReleaseOpenclRunEnv();
    static int ReleaseOpenclEnv( GPUEnv *gpuInfo );
    static int CompileKernelFile( GPUEnv *gpuInfo, const char *buildOption );
    static int CachedOfKernerPrg( const GPUEnv *gpuEnvCached, const char * clFileName );
    static int GeneratBinFromKernelSource( cl_program program, const char * clFileName );
    static int WriteBinaryToFile( const char* fileName, const char* birary, size_t numBytes );
    static int BinaryGenerated( const char * clFileName, FILE ** fhandle );
    //static int CompileKernelFile( const char *filename, GPUEnv *gpuInfo, const char *buildOption );
	static l_uint32* pixReadFromTiffKernel(l_uint32 *tiffdata,l_int32 w,l_int32 h,l_int32 wpl, l_uint32 *line);
	static Pix* pixReadTiffCl( const char *filename, l_int32 n );
	static PIX * pixReadStreamTiffCl ( FILE *fp, l_int32 n );
	static PIX* pixReadFromTiffStreamCl(TIFF  *tif);
	static int composeRGBPixelCl(int *tiffdata,int *line,int h,int w);
	static l_int32 getTiffStreamResolutionCl(TIFF *tif,l_int32  *pxres,l_int32  *pyres);
	static TIFF* fopenTiffCl(FILE *fp,const char  *modestring);

/* OpenCL implementations of Morphological operations*/
	
	//Initialiation of OCL buffers used in Morph operations
	static int initMorphCLAllocations(l_int32  wpl, l_int32  h, PIX* pixs);
	static void releaseMorphCLBuffers();

	// OpenCL implementation of Morphology Dilate
	static PIX* pixDilateBrickCL(PIX  *pixd, PIX  *pixs, l_int32  hsize, l_int32  vsize, bool reqDataCopy);
	
	// OpenCL implementation of Morphology Erode
	static PIX* pixErodeBrickCL(PIX  *pixd, PIX  *pixs, l_int32  hsize, l_int32  vsize, bool reqDataCopy);
	
	// OpenCL implementation of Morphology Close
	static PIX* pixCloseBrickCL(PIX  *pixd, PIX  *pixs, l_int32  hsize, l_int32  vsize, bool reqDataCopy);

	// OpenCL implementation of Morphology Open
	static PIX* pixOpenBrickCL(PIX  *pixd, PIX  *pixs, l_int32  hsize, l_int32  vsize, bool reqDataCopy);
    
	// OpenCL implementation of Morphology Open
	static PIX* pixSubtractCL(PIX  *pixd, PIX  *pixs1, PIX  *pixs2, bool reqDataCopy);

	// OpenCL implementation of Morphology (Hollow = Closed - Open)
	static PIX* pixHollowCL(PIX  *pixd, PIX  *pixs, l_int32  close_hsize, l_int32  close_vsize, l_int32  open_hsize, l_int32  open_vsize, bool reqDataCopy);

	static void pixGetLinesCL(PIX  *pixd, PIX  *pixs, 
											PIX** pix_vline, PIX** pix_hline, 
											PIX** pixClosed, bool  getpixClosed,
											l_int32  close_hsize, l_int32  close_vsize, 
											l_int32  open_hsize, l_int32  open_vsize,
											l_int32  line_hsize, l_int32  line_vsize);

	//int InitOpenclAttr( OpenCLEnv * env );
    //int ReleaseKernel( KernelEnv * env );
    static int SetKernelEnv( KernelEnv *envInfo );
    //int CreateKernel( char * kernelname, KernelEnv * env );
    //int RunKernel( const char *kernelName, void **userdata );
    //int ConvertToString( const char *filename, char **source );
    //int CheckKernelName( KernelEnv *envInfo, const char *kernelName );
    //int RegisterKernelWrapper( const char *kernelName, cl_kernel_function function );
    //int RunKernelWrapper( cl_kernel_function function, const char * kernelName, void **usrdata );
    //int GetKernelEnvAndFunc( const char *kernelName, KernelEnv *env, cl_kernel_function *function );
	// static cl_device_id performDeviceSelection( );
    //static bool thresholdRectToPixMicroBench( TessScoreEvaluationInputData input, ds_device_type type);

#ifdef WIN32
    static int LoadOpencl();
    //static int OpenclInite();
    static void FreeOpenclDll();
#endif

    //int GetOpenclState();
    //void SetOpenclState( int state );
    inline static int AddKernelConfig( int kCount, const char *kName );

    /* for binarization */
    static void HistogramRectOCL(
        const unsigned char *imagedata,
        int bytes_per_pixel,
        int bytes_per_line,
        int left,
        int top,
        int width,
        int height,
        int kHistogramSize,
        int *histogramAllChannels);
    static void ThresholdRectToPixOCL(
        const unsigned char* imagedata,
        int bytes_per_pixel,
        int bytes_per_line,
        const int* thresholds,
        const int* hi_values,
        Pix** pix,
        int rect_height,
        int rect_width,
        int rect_top,
        int rect_left);
};


#endif
