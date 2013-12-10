
#ifndef _OCL_KERNEL_H_
#define _OCL_KERNEL_H_
#ifndef USE_EXTERNAL_KERNEL
#define KERNEL( ... )# __VA_ARGS__ "\n"
// Double precision is a default of spreadsheets
// cl_khr_fp64: Khronos extension
// cl_amd_fp64: AMD extension
// use build option outside to define fp_t
/////////////////////////////////////////////
const char *kernel_src = KERNEL(
\n#ifdef KHR_DP_EXTENSION\n
\n#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n
\n#elif AMD_DP_EXTENSION\n
\n#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n
\n#else\n
\n#endif\n
__kernel void composeRGBPixel(__global uint *tiffdata, int w, int h,int wpl, __global uint *output)
{
    int i = get_global_id(1);
	int j = get_global_id(0);
	int tiffword,rval,gval,bval;

	//Ignore the excess
	if ((i >= h) || (j >= w))
		return;

	tiffword = tiffdata[i * w + j];
    rval = ((tiffword) & 0xff);
    gval = (((tiffword) >> 8) & 0xff);
    bval = (((tiffword) >> 16) & 0xff);
	output[i*wpl+j] = (rval << (8 * (sizeof(uint) - 1 - 0))) | (gval << (8 * (sizeof(uint) - 1 - 1))) | (bval << (8 * (sizeof(uint) - 1 - 2)));
}
)

KERNEL(
\n__kernel void pixSubtract_inplace(__global int *dword, __global int *sword,
							const int wpl, const int h)
{
	const unsigned int row = get_global_id(1);
	const unsigned int col = get_global_id(0);
	const unsigned int pos = row * wpl + col;

	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	*(dword + pos) &= ~(*(sword + pos));
}\n
)

KERNEL(
\n__kernel void pixSubtract(__global int *dword, __global int *sword, 
							const int wpl, const int h, __global int *outword)
{
	const unsigned int row = get_global_id(1);
	const unsigned int col = get_global_id(0);
	const unsigned int pos = row * wpl + col;

	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	*(outword + pos) = *(dword + pos) & ~(*(sword + pos));
}\n
)

KERNEL(
\n__kernel void pixAND(__global int *dword, __global int *sword, __global int *outword,
							const int wpl, const int h)
{
	const unsigned int row = get_global_id(1);
	const unsigned int col = get_global_id(0);
	const unsigned int pos = row * wpl + col;

	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	 *(outword + pos) = *(dword + pos) & (*(sword + pos));
}\n
)

KERNEL(
\n__kernel void pixOR(__global int *dword, __global int *sword, __global int *outword,
							const int wpl, const int h)
{
	const unsigned int row = get_global_id(1);
	const unsigned int col = get_global_id(0);
	const unsigned int pos = row * wpl + col;

	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	*(outword + pos) = *(dword + pos) | (*(sword + pos));
}\n
)

KERNEL(
\n__kernel void morphoDilateHor_5x5(__global int *sword,__global int *dword,
							const int wpl, const int h)
{
	const unsigned int pos = get_global_id(0);
	unsigned int prevword, nextword, currword,tempword;
	unsigned int destword;
	const int col = pos % wpl;
	
	//Ignore the execss
	if (pos >= (wpl * h))
		return;
	
	
	currword = *(sword + pos);	
	destword = currword;
	
	//Handle boundary conditions
	if(col==0)
		prevword=0;
	else
		prevword = *(sword + pos - 1);

	if(col==(wpl - 1))
		nextword=0;
	else
		nextword = *(sword + pos + 1);
	
	//Loop unrolled
	
	//1 bit to left and 1 bit to right
		//Get the max value on LHS of every pixel
		tempword = (prevword << (31)) | ((currword >> 1));
		destword |= tempword;
		//Get max value on RHS of every pixel
		tempword = (currword << 1) | (nextword >> (31));
		destword |= tempword;

	//2 bit to left and 2 bit to right
		//Get the max value on LHS of every pixel
		tempword = (prevword << (30)) | ((currword >> 2));
		destword |= tempword;
		//Get max value on RHS of every pixel
		tempword = (currword << 2) | (nextword >> (30));
		destword |= tempword;
	
    
	*(dword + pos) = destword;
	
}\n
)

KERNEL(
\n__kernel void morphoDilateVer_5x5(__global int *sword,__global int *dword,
							const int wpl, const int h)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int tempword;
	unsigned int destword;
	int i;

	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	destword = *(sword + pos);

	//2 words above
	i = (row - 2) < 0 ? row : (row - 2);
	tempword = *(sword + i*wpl + col);
	destword |= tempword;

	//1 word above
	i = (row - 1) < 0 ? row  : (row - 1);
	tempword = *(sword + i*wpl + col);
	destword |= tempword;

	//1 word below
	i = (row >= (h - 1)) ? row : (row + 1);
	tempword = *(sword + i*wpl + col);
	destword |= tempword;

	//2 words below
	i = (row >= (h - 2)) ? row : (row + 2);
	tempword = *(sword + i*wpl + col);
	destword |= tempword;

	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoDilateHor(__global int *sword,__global int *dword,const int xp, const int xn, const int wpl, const int h)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int parbitsxp, parbitsxn, nwords;
	unsigned int destword, tempword, lastword, currword;
	unsigned int lnextword, lprevword, rnextword, rprevword, firstword, secondword;
	int i, j, siter, eiter;
	
	//Ignore the execss
	if (pos >= (wpl*h) || (xn < 1 && xp < 1))
		return;

	currword = *(sword + pos);
	destword = currword;

	parbitsxp = xp & 31;
	parbitsxn = xn & 31;
	nwords = xp >> 5;

	if (parbitsxp > 0)
		nwords += 1;
	else
		parbitsxp = 31;

	siter = (col - nwords);
	eiter = (col + nwords);

	//Get prev word
	if (col==0)
		firstword = 0x0;
	else
		firstword = *(sword + pos - 1);
	
	//Get next word
	if (col == (wpl - 1))
		secondword = 0x0;
	else
		secondword = *(sword + pos + 1);

	//Last partial bits on either side
	for (i = 1; i <= parbitsxp; i++)
	{
		//Get the max value on LHS of every pixel
		tempword = ((i == parbitsxp) && (parbitsxp != parbitsxn)) ? 0x0 : (firstword << (32-i)) | ((currword >> i));
		
		destword |= tempword;

		//Get max value on RHS of every pixel
		tempword = (currword << i) | (secondword >> (32 - i));
		destword |= tempword;
	}

	//Return if halfwidth <= 1 word
	if (nwords == 1)
	{
		if (xn == 32)
		{
			destword |= firstword;
		}
		if (xp == 32)
		{
			destword |= secondword;
		}

		*(dword + pos) = destword;
		return;
	}

	if (siter < 0)
		firstword = 0x0;
	else
		firstword = *(sword + row*wpl + siter);

	if (eiter >= wpl)	
		lastword = 0x0;
	else
		lastword = *(sword + row*wpl + eiter);
	
	for ( i = 1; i < nwords; i++)
	{
		//Gets LHS words
		if ((siter + i) < 0)
			secondword = 0x0;
		else
			secondword = *(sword + row*wpl + siter + i);

		lprevword = firstword << (32 - parbitsxn) | secondword >> parbitsxn;
		
		firstword = secondword;

		if ((siter + i + 1) < 0)
			secondword = 0x0;
		else
			secondword = *(sword + row*wpl + siter + i + 1);
		
		lnextword = firstword << (32 - parbitsxn) | secondword >> parbitsxn;

		//Gets RHS words
		if ((eiter - i) >= wpl)
			firstword = 0x0;
		else
			firstword = *(sword + row*wpl + eiter - i);
			
		rnextword = firstword << parbitsxp | lastword >> (32 - parbitsxp);

		lastword = firstword;
		if ((eiter - i - 1) >= wpl)
			firstword = 0x0;
		else
			firstword = *(sword + row*wpl + eiter - i - 1);

		rprevword = firstword << parbitsxp | lastword >> (32 - parbitsxp);

		for (j = 1; j < 32; j++)
		{
			//OR LHS full words
			tempword = (lprevword << j) | (lnextword >> (32 - j));
			destword |= tempword;

			//OR RHS full words
			tempword = (rprevword << j) | (rnextword >> (32 - j));
			destword |= tempword;
		}

		destword |= lprevword;
		destword |= lnextword;
		destword |= rprevword;
		destword |= rnextword;

		lastword = firstword;
		firstword = secondword;
	}
	
	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoDilateHor_32word(__global int *sword,__global int *dword,
							const int halfwidth,
							const int wpl, const int h,
							const char isEven)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int prevword, nextword, currword,tempword;
	unsigned int destword;
	int i;
	
	//Ignore the execss
	if (pos >= (wpl * h))
		return;

	currword = *(sword + pos);	
	destword = currword;
	
	//Handle boundary conditions
	if(col==0)
		prevword=0;
	else
		prevword = *(sword + pos - 1);

	if(col==(wpl - 1))
		nextword=0;
	else
		nextword = *(sword + pos + 1);
	
	for (i = 1; i <= halfwidth; i++)
	{
		//Get the max value on LHS of every pixel
		if (i == halfwidth && isEven)
		{
			tempword = 0x0;
		}
		else
		{
			tempword = (prevword << (32-i)) | ((currword >> i));
		}

		destword |= tempword;

		//Get max value on RHS of every pixel
		tempword = (currword << i) | (nextword >> (32 - i));
		
		destword |= tempword;
	}

	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoDilateVer(__global int *sword,__global int *dword,
							const int yp,
							const int wpl, const int h,
							const int yn)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int tempword;
	unsigned int destword;
	int i, siter, eiter;
	
	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	destword = *(sword + pos);

	//Set start position and end position considering the boundary conditions
	siter = (row - yn) < 0 ? 0 : (row - yn);
	eiter = (row >= (h - yp)) ? (h - 1) : (row + yp);

	for (i = siter; i <= eiter; i++)
	{
		tempword = *(sword + i*wpl + col);

		destword |= tempword;
	}

	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoErodeHor_5x5(__global int *sword,__global int *dword,
							const int wpl, const int h)
{
	const unsigned int pos = get_global_id(0);
	unsigned int prevword, nextword, currword,tempword;
	unsigned int destword;
	const int col = pos % wpl;
	
	//Ignore the execss
	if (pos >= (wpl * h))
		return;
	
	currword = *(sword + pos);	
	destword = currword;
	
	//Handle boundary conditions
	if(col==0)
		prevword=0xffffffff;
	else
		prevword = *(sword + pos - 1);
	
	if(col==(wpl - 1))
		nextword=0xffffffff;
	else
		nextword = *(sword + pos + 1);
	
	//Loop unrolled
	
	//1 bit to left and 1 bit to right
		//Get the min value on LHS of every pixel
		tempword = (prevword << (31)) | ((currword >> 1));
		destword &= tempword;
		//Get min value on RHS of every pixel
		tempword = (currword << 1) | (nextword >> (31));
		destword &= tempword;

	//2 bit to left and 2 bit to right
		//Get the min value on LHS of every pixel
		tempword = (prevword << (30)) | ((currword >> 2));
		destword &= tempword;
		//Get min value on RHS of every pixel
		tempword = (currword << 2) | (nextword >> (30));
		destword &= tempword;
	
    
	*(dword + pos) = destword;
	
}\n
)

KERNEL(
\n__kernel void morphoErodeVer_5x5(__global int *sword,__global int *dword,
							const int wpl, const int h,
							const int fwmask, const int lwmask)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int tempword;
	unsigned int destword;
	int i;

	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	destword = *(sword + pos);

	if (row < 2 || row >= (h - 2))
	{
		destword = 0x0;
	}	
	else
	{
		//2 words above
		//i = (row - 2) < 0 ? row : (row - 2);
		i = (row - 2);
		tempword = *(sword + i*wpl + col);
		destword &= tempword;

		//1 word above
		//i = (row - 1) < 0 ? row  : (row - 1);
		i = (row - 1);
		tempword = *(sword + i*wpl + col);
		destword &= tempword;

		//1 word below
		//i = (row >= (h - 1)) ? row : (row + 1);
		i = (row + 1);
		tempword = *(sword + i*wpl + col);
		destword &= tempword;

		//2 words below
		//i = (row >= (h - 2)) ? row : (row + 2);
		i = (row + 2);
		tempword = *(sword + i*wpl + col);
		destword &= tempword;

		if (col == 0) 
		{
			destword &= fwmask;
		}
		if (col == (wpl - 1))
		{
			destword &= lwmask;
		}
	}


	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoErodeHor(__global int *sword,__global int *dword,	const int xp, const int xn, const int wpl, 
								const int h, const char isAsymmetric, const int rwmask, const int lwmask)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int parbitsxp, parbitsxn, nwords;
	unsigned int destword, tempword, lastword, currword;
	unsigned int lnextword, lprevword, rnextword, rprevword, firstword, secondword;
	int i, j, siter, eiter;

	//Ignore the execss
	if (pos >= (wpl*h) || (xn < 1 && xp < 1))
		return;

	currword = *(sword + pos);
	destword = currword;

	parbitsxp = xp & 31;
	parbitsxn = xn & 31;
	nwords = xp >> 5;

	if (parbitsxp > 0)
		nwords += 1;
	else
		parbitsxp = 31;

	siter = (col - nwords);
	eiter = (col + nwords);

	//Get prev word
	if (col==0)
		firstword = 0xffffffff;
	else
		firstword = *(sword + pos - 1);
	
	//Get next word
	if (col == (wpl - 1))
		secondword = 0xffffffff;
	else
		secondword = *(sword + pos + 1);

	//Last partial bits on either side
	for (i = 1; i <= parbitsxp; i++)
	{
		//Get the max value on LHS of every pixel
		tempword = (firstword << (32-i)) | ((currword >> i));
		destword &= tempword;

		//Get max value on RHS of every pixel
		tempword = ((i == parbitsxp) && (parbitsxp != parbitsxn)) ? 0xffffffff : (currword << i) | (secondword >> (32 - i));
		
		//tempword = (currword << i) | (secondword >> (32 - i));
		destword &= tempword;
	}

	//Return if halfwidth <= 1 word
	if (nwords == 1)
	{
		if (xp == 32)
		{
			destword &= firstword;
		}
		if (xn == 32)
		{
			destword &= secondword;
		}

		//Clear boundary pixels
		if (isAsymmetric)
		{
			if (col == 0)
				destword &= rwmask;
			if (col == (wpl - 1))
				destword &= lwmask;
		}

		*(dword + pos) = destword;
		return;
	}
	
	if (siter < 0)
		firstword = 0xffffffff;
	else
		firstword = *(sword + row*wpl + siter);

	if (eiter >= wpl)	
		lastword = 0xffffffff;
	else
		lastword = *(sword + row*wpl + eiter);
	
	
	for ( i = 1; i < nwords; i++)
	{
		//Gets LHS words
		if ((siter + i) < 0)
			secondword = 0xffffffff;
		else
			secondword = *(sword + row*wpl + siter + i);

		lprevword = firstword << (32 - parbitsxp) | secondword >> (parbitsxp);
		
		firstword = secondword;

		if ((siter + i + 1) < 0)
			secondword = 0xffffffff;
		else
			secondword = *(sword + row*wpl + siter + i + 1);
		
		lnextword = firstword << (32 - parbitsxp) | secondword >> (parbitsxp);

		//Gets RHS words
		if ((eiter - i) >= wpl)
			firstword = 0xffffffff;
		else
			firstword = *(sword + row*wpl + eiter - i);
			
		rnextword = firstword << parbitsxn | lastword >> (32 - parbitsxn);

		lastword = firstword;
		if ((eiter - i - 1) >= wpl)
			firstword = 0xffffffff;
		else
			firstword = *(sword + row*wpl + eiter - i - 1);

		rprevword = firstword << parbitsxn | lastword >> (32 - parbitsxn);

		for (j = 0; j < 32; j++)
		{
			//OR LHS full words
			tempword = (lprevword << j) | (lnextword >> (32 - j));
			destword &= tempword;

			//OR RHS full words
			tempword = (rprevword << j) | (rnextword >> (32 - j));
			destword &= tempword;
		}

		destword &= lprevword;
		destword &= lnextword;
		destword &= rprevword;
		destword &= rnextword;

		lastword = firstword;
		firstword = secondword;
	}
	
	if (isAsymmetric)
	{
		//Clear boundary pixels
		if (col < (nwords - 1))
			destword = 0x0;
		else if (col == (nwords - 1))
			destword &= rwmask;
		else if (col > (wpl - nwords))
			destword = 0x0;
		else if (col == (wpl - nwords))
			destword &= lwmask;
	}

	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoErodeHor_32word(__global int *sword,__global int *dword,
							const int halfwidth, const int wpl, 
							const int h, const char clearBoundPixH, 
							const int rwmask, const int lwmask,
							const char isEven)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int prevword, nextword, currword,tempword, destword;
	int i;

	//Ignore the execss
	if (pos >= (wpl * h))
		return;

	currword = *(sword + pos);	
	destword = currword;
	
	//Handle boundary conditions
	if(col==0)
		prevword=0xffffffff;
	else
		prevword = *(sword + pos - 1);
	
	if(col==(wpl - 1))
		nextword=0xffffffff;
	else
		nextword = *(sword + pos + 1);
	
	for (i = 1; i <= halfwidth; i++)
	{
		//Get the min value on LHS of every pixel
		tempword = (prevword << (32-i)) | ((currword >> i));
		
		destword &= tempword;

		//Get min value on RHS of every pixel
		if (i == halfwidth && isEven)
		{
			tempword = 0xffffffff;
		}
		else
		{
			tempword = (currword << i) | (nextword >> (32 - i));
		}

		destword &= tempword;
	}

	if (clearBoundPixH)
	{
		if (col == 0) 
		{
			destword &= rwmask;
		}
		else if (col == (wpl - 1))
		{
			destword &= lwmask;
		}
	}

	*(dword + pos) = destword;
}\n
)

KERNEL(
\n__kernel void morphoErodeVer(__global int *sword,__global int *dword,
							const int yp, 
							const int wpl, const int h,
							const char clearBoundPixV, const int yn)
{
	const int col = get_global_id(0);
	const int row = get_global_id(1);
	const unsigned int pos = row * wpl + col;
	unsigned int tempword, destword;
	int i, siter, eiter;
	
	//Ignore the execss
	if (row >= h || col >= wpl)
		return;

	destword = *(sword + pos);

	//Set start position and end position considering the boundary conditions
	siter = (row - yp) < 0 ? 0 : (row - yp);
	eiter = (row >= (h - yn)) ? (h - 1) : (row + yn);

	for (i = siter; i <= eiter; i++)
	{
		tempword = *(sword + i*wpl + col);

		destword &= tempword;
	}

	//Clear boundary pixels
	if (clearBoundPixV && ((row < yp) || ((h - row) <= yn)))
	{	
		destword = 0x0;
	}

	*(dword + pos) = destword;
}\n
)

// HistogramRect Kernel: Accumulate
// assumes 4 channels, i.e., bytes_per_pixel = 4
// assumes number of pixels is multiple of 8
// data is layed out as
// ch0                                           ch1 ...
// bin0          bin1            bin2...         bin0...
// rpt0,1,2...256  rpt0,1,2...
KERNEL(
\n#define HIST_REDUNDANCY 256\n
\n#define GROUP_SIZE 256\n
\n#define HIST_SIZE 256\n
\n#define NUM_CHANNELS 4\n
\n#define HR_UNROLL_SIZE 8 \n
\n#define HR_UNROLL_TYPE uchar8 \n

__attribute__((reqd_work_group_size(256, 1, 1)))
__kernel
void kernel_HistogramRectAllChannels(
    __global const uchar8 *data,
    uint numPixels,
	__global uint *histBuffer) {

    // declare variables
    uchar8 pixels;
    int threadOffset = get_global_id(0)%HIST_REDUNDANCY;

    // for each pixel/channel, accumulate in global memory
    for ( uint pc = get_global_id(0); pc < numPixels*NUM_CHANNELS/HR_UNROLL_SIZE; pc += get_global_size(0) ) {
        pixels = data[pc];
        //                       channel                        bin                         thread
        atomic_inc( &histBuffer[ 0*HIST_SIZE*HIST_REDUNDANCY + pixels.s0*HIST_REDUNDANCY + threadOffset ]); // ch0
        atomic_inc( &histBuffer[ 0*HIST_SIZE*HIST_REDUNDANCY + pixels.s4*HIST_REDUNDANCY + threadOffset ]); // ch0
        atomic_inc( &histBuffer[ 1*HIST_SIZE*HIST_REDUNDANCY + pixels.s1*HIST_REDUNDANCY + threadOffset ]); // ch1
        atomic_inc( &histBuffer[ 1*HIST_SIZE*HIST_REDUNDANCY + pixels.s5*HIST_REDUNDANCY + threadOffset ]); // ch1
        atomic_inc( &histBuffer[ 2*HIST_SIZE*HIST_REDUNDANCY + pixels.s2*HIST_REDUNDANCY + threadOffset ]); // ch2
        atomic_inc( &histBuffer[ 2*HIST_SIZE*HIST_REDUNDANCY + pixels.s6*HIST_REDUNDANCY + threadOffset ]); // ch2
        atomic_inc( &histBuffer[ 3*HIST_SIZE*HIST_REDUNDANCY + pixels.s3*HIST_REDUNDANCY + threadOffset ]); // ch3
        atomic_inc( &histBuffer[ 3*HIST_SIZE*HIST_REDUNDANCY + pixels.s7*HIST_REDUNDANCY + threadOffset ]); // ch3
    }
}
)

KERNEL(
// NUM_CHANNELS = 1
__attribute__((reqd_work_group_size(256, 1, 1)))
__kernel
void kernel_HistogramRectOneChannel(
    __global const uchar8 *data,
    uint numPixels,
    __global uint *histBuffer) {

    // declare variables
    uchar8 pixels;
    int threadOffset = get_global_id(0)%HIST_REDUNDANCY;

    // for each pixel/channel, accumulate in global memory
    for ( uint pc = get_global_id(0); pc < numPixels/HR_UNROLL_SIZE; pc += get_global_size(0) ) {
        pixels = data[pc];
        //                        bin                         thread
        atomic_inc( &histBuffer[ pixels.s0*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s1*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s2*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s3*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s4*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s5*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s6*HIST_REDUNDANCY + threadOffset ]);
        atomic_inc( &histBuffer[ pixels.s7*HIST_REDUNDANCY + threadOffset ]);
    }
}
)


KERNEL(
// unused
\n  __attribute__((reqd_work_group_size(256, 1, 1)))
\n  __kernel
\n  void kernel_HistogramRectAllChannels_Grey(
\n      __global const uchar* data,
\n      uint numPixels,
\n        __global uint *histBuffer) { // each wg will write HIST_SIZE*NUM_CHANNELS into this result; cpu will accumulate across wg's
\n  
\n      /* declare variables */
\n  
\n      // work indices
\n      size_t groupId = get_group_id(0);
\n      size_t localId = get_local_id(0); // 0 -> 256-1
\n      size_t globalId = get_global_id(0); // 0 -> 8*10*256-1=20480-1
\n      uint numThreads = get_global_size(0);
\n  
\n      /* accumulate in global memory */
\n      for ( uint pc = get_global_id(0); pc < numPixels; pc += get_global_size(0) ) {
\n          uchar value = data[ pc ];
\n          int idx = value * get_global_size(0) + get_global_id(0);
\n           histBuffer[ idx ]++;
\n          
\n      }
\n      
\n  } // kernel_HistogramRectAllChannels_Grey

)

// HistogramRect Kernel: Reduction
// only supports 4 channels
// each work group handles a single channel of a single histogram bin
KERNEL(
__attribute__((reqd_work_group_size(256, 1, 1)))
__kernel
void kernel_HistogramRectAllChannelsReduction(
    int n, // unused pixel redundancy
	__global uint *histBuffer,
    __global int* histResult) {

    // declare variables
    int channel = get_group_id(0)/HIST_SIZE;
    int bin     = get_group_id(0)%HIST_SIZE;
    int value = 0;

    // accumulate in register
    for ( uint i = get_local_id(0); i < HIST_REDUNDANCY; i+=GROUP_SIZE) {
        value += histBuffer[ channel*HIST_SIZE*HIST_REDUNDANCY+bin*HIST_REDUNDANCY+i];
    }

    // reduction in local memory
    __local int localHist[GROUP_SIZE];
    localHist[get_local_id(0)] = value;
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int stride = GROUP_SIZE/2; stride >= 1; stride /= 2) {
        if (get_local_id(0) < stride) {
            value = localHist[ get_local_id(0)+stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        if (get_local_id(0) < stride) {
            localHist[ get_local_id(0)] += value;
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // write reduction to final result
    if (get_local_id(0) == 0) {
        histResult[get_group_id(0)] = localHist[0];
    }
} // kernel_HistogramRectAllChannels
)


KERNEL(
// NUM_CHANNELS = 1
__attribute__((reqd_work_group_size(256, 1, 1)))
__kernel
void kernel_HistogramRectOneChannelReduction(
    int n, // unused pixel redundancy
    __global uint *histBuffer,
    __global int* histResult) {

    // declare variables
    // int channel = get_group_id(0)/HIST_SIZE;
    int bin     = get_group_id(0)%HIST_SIZE;
    int value = 0;

    // accumulate in register
    for ( int i = get_local_id(0); i < HIST_REDUNDANCY; i+=GROUP_SIZE) {
        value += histBuffer[ bin*HIST_REDUNDANCY+i];
    }

    // reduction in local memory
    __local int localHist[GROUP_SIZE];
    localHist[get_local_id(0)] = value;
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int stride = GROUP_SIZE/2; stride >= 1; stride /= 2) {
        if (get_local_id(0) < stride) {
            value = localHist[ get_local_id(0)+stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        if (get_local_id(0) < stride) {
            localHist[ get_local_id(0)] += value;
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // write reduction to final result
    if (get_local_id(0) == 0) {
        histResult[get_group_id(0)] = localHist[0];
    }
} // kernel_HistogramRectOneChannelReduction
)


KERNEL(
// unused
  // each work group (x256) handles a histogram bin 
\n  __attribute__((reqd_work_group_size(256, 1, 1)))
\n  __kernel
\n  void kernel_HistogramRectAllChannelsReduction_Grey(
\n      int n, // pixel redundancy that needs to be accumulated
\n      __global uint *histBuffer,
\n      __global uint* histResult) { // each wg accumulates 1 bin
\n  
\n      /* declare variables */
\n  
\n      // work indices
\n      size_t groupId = get_group_id(0);
\n      size_t localId = get_local_id(0); // 0 -> 256-1
\n      size_t globalId = get_global_id(0); // 0 -> 8*10*256-1=20480-1
\n      uint numThreads = get_global_size(0);
\n        unsigned int hist = 0;
\n  
\n      /* accumulate in global memory */
\n      for ( uint p = 0; p < n; p+=GROUP_SIZE) {
\n            hist += histBuffer[ (get_group_id(0)*n + p)];
\n      }
\n  
\n      /* reduction in local memory */
\n      // populate local memory
\n      __local unsigned int localHist[GROUP_SIZE];

\n      localHist[localId] = hist;
\n      barrier(CLK_LOCAL_MEM_FENCE);
\n  
\n      for (int stride = GROUP_SIZE/2; stride >= 1; stride /= 2) {
\n          if (localId < stride) {
\n              hist = localHist[ (localId+stride)];
\n          }
\n          barrier(CLK_LOCAL_MEM_FENCE);
\n          if (localId < stride) {
\n              localHist[ localId] += hist;
\n          }
\n          barrier(CLK_LOCAL_MEM_FENCE);
\n      }
\n  
\n      if (localId == 0)
\n          histResult[get_group_id(0)] = localHist[0];
\n  
\n  } // kernel_HistogramRectAllChannelsReduction_Grey

)

// ThresholdRectToPix Kernel
// only supports 4 channels
// imageData is input image (24-bits/pixel)
// pix is output image (1-bit/pixel)
KERNEL(
\n#define CHAR_VEC_WIDTH 8 \n
\n#define PIXELS_PER_WORD 32 \n
\n#define PIXELS_PER_BURST 8 \n
\n#define BURSTS_PER_WORD (PIXELS_PER_WORD/PIXELS_PER_BURST) \n
 typedef union {
  uchar s[PIXELS_PER_BURST*NUM_CHANNELS];
  uchar8 v[(PIXELS_PER_BURST*NUM_CHANNELS)/CHAR_VEC_WIDTH];
 } charVec;

__attribute__((reqd_work_group_size(256, 1, 1)))
__kernel
void kernel_ThresholdRectToPix(
    __global const uchar8 *imageData,
    int height,
    int width,
    int wpl, // words per line
    __global int *thresholds,
    __global int *hi_values,
    __global int *pix) {

    // declare variables
    int pThresholds[NUM_CHANNELS];
    int pHi_Values[NUM_CHANNELS];
    for ( int i = 0; i < NUM_CHANNELS; i++) {
        pThresholds[i] = thresholds[i];
        pHi_Values[i] = hi_values[i];
    }

    // for each word (32 pixels) in output image
    for ( uint w = get_global_id(0); w < wpl*height; w += get_global_size(0) ) {
        unsigned int word = 0; // all bits start at zero

        // for each burst in word
        for ( int b = 0; b < BURSTS_PER_WORD; b++) {

            // load burst
            charVec pixels;
            for ( int i = 0; i < (PIXELS_PER_BURST*NUM_CHANNELS)/CHAR_VEC_WIDTH; i++ ) {
                pixels.v[i] = imageData[w*(BURSTS_PER_WORD*(PIXELS_PER_BURST*NUM_CHANNELS)/CHAR_VEC_WIDTH) + b*((PIXELS_PER_BURST*NUM_CHANNELS)/CHAR_VEC_WIDTH)  + i];
            }

            // for each pixel in burst
            for ( int p = 0; p < PIXELS_PER_BURST; p++) {
                for ( int c = 0; c < NUM_CHANNELS; c++) {
                    unsigned char pixChan = pixels.s[p*NUM_CHANNELS + c];
                    if (pHi_Values[c] >= 0 && (pixChan > pThresholds[c]) == (pHi_Values[c] == 0)) {
                        word |=  (0x80000000 >> ((b*PIXELS_PER_BURST+p)&31));
                    }
                }
            }
        }
        pix[w] = word;
    }
}

// only supports 1 channel
 typedef union {
  uchar s[PIXELS_PER_BURST];
  uchar8 v[(PIXELS_PER_BURST)/CHAR_VEC_WIDTH];
 } charVec1;

__attribute__((reqd_work_group_size(256, 1, 1)))
__kernel
void kernel_ThresholdRectToPix_OneChan(
    __global const uchar8 *imageData,
    int height,
    int width,
    int wpl, // words per line
    __global int *thresholds,
    __global int *hi_values,
    __global int *pix) {

    // declare variables
    int pThresholds[1];
    int pHi_Values[1];
    for ( int i = 0; i < 1; i++) {
        pThresholds[i] = thresholds[i];
        pHi_Values[i] = hi_values[i];
    }

    // for each word (32 pixels) in output image
    for ( uint w = get_global_id(0); w < wpl*height; w += get_global_size(0) ) {
        unsigned int word = 0; // all bits start at zero

        // for each burst in word
        for ( int b = 0; b < BURSTS_PER_WORD; b++) {

            // load burst
            charVec1 pixels;
            for ( int i = 0; i < (PIXELS_PER_BURST)/CHAR_VEC_WIDTH; i++ ) {
                pixels.v[i] = imageData[w*(BURSTS_PER_WORD*(PIXELS_PER_BURST)/CHAR_VEC_WIDTH) + b*((PIXELS_PER_BURST)/CHAR_VEC_WIDTH)  + i];
            }

            // for each pixel in burst
            for ( int p = 0; p < PIXELS_PER_BURST; p++) {
                for ( int c = 0; c < 1; c++) {
                    unsigned char pixChan = pixels.s[p + c];
                    if (pHi_Values[c] >= 0 && (pixChan > pThresholds[c]) == (pHi_Values[c] == 0)) {
                        word |=  (0x80000000 >> ((b*PIXELS_PER_BURST+p)&31));
                    }
                }
            }
        }
        pix[w] = word;
    }
}
)

 ; // close char*

#endif // USE_EXTERNAL_KERNEL
#endif //_OCL_KERNEL_H_
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

// Alternative histogram kernel written to use uchar and different global memory scattered write
// was a little better for intel platforms but still not faster then native serial code
#if 0
/*  data layed out as
    bin0                                        bin1                            bin2...
    r,g,b,a,r,g,b,a,r,g,b,a nthreads/4 copies
*/
\n__attribute__((reqd_work_group_size(256, 1, 1)))
\n  __kernel
\n  void kernel_HistogramRectAllChannels_uchar(
\n      volatile __global const uchar  *data,
\n                              uint   numPixels,
\n  	volatile __global       uint   *histBuffer) {
\n      
\n      // for each pixel/channel, accumulate in global memory
\n      for ( uint pc = get_global_id(0); pc < numPixels*NUM_CHANNELS; pc += get_global_size(0) ) {
\n          uchar value = data[pc];
\n          int idx = value*get_global_size(0) + get_global_id(0);
\n          histBuffer[ idx ]++; // coalesced if same value
\n      }
\n  } // kernel_HistogramRectAllChannels
\n
\n  __attribute__((reqd_work_group_size(256, 1, 1)))
\n  __kernel
\n  void kernel_HistogramRectAllChannelsReduction_uchar(
\n      int n, // pixel redundancy that needs to be accumulated = nthreads/4
\n  	__global uint4 *histBuffer,
\n      __global uint* histResult) { // each wg accumulates 1 bin (all channels within it
\n  
\n      // declare variables
\n      int binIdx     = get_group_id(0);
\n      size_t groupId = get_group_id(0);
\n      size_t localId = get_local_id(0); // 0 -> 256-1
\n      size_t globalId = get_global_id(0); // 0 -> 8*10*256-1=20480-1
\n      uint numThreads = get_global_size(0);
\n      uint4 hist = {0, 0, 0, 0};
\n
\n      // accumulate in register
\n      for ( uint p = get_local_id(0); p < n; p+=GROUP_SIZE) {
\n          hist += histBuffer[binIdx*n+p];
\n      }
\n  
\n      // reduction in local memory
\n      __local uint4 localHist[GROUP_SIZE];
\n      localHist[localId] = hist;
\n      barrier(CLK_LOCAL_MEM_FENCE);
\n  
\n      for (int stride = GROUP_SIZE/2; stride >= 1; stride /= 2) {
\n          if (localId < stride) {
\n              hist = localHist[ localId+stride];
\n          }
\n          barrier(CLK_LOCAL_MEM_FENCE);
\n          if (localId < stride) {
\n              localHist[ localId] += hist;
\n          }
\n          barrier(CLK_LOCAL_MEM_FENCE);
\n      }
\n
\n      // write reduction to final result
\n      if (localId == 0) {
\n          histResult[0*HIST_SIZE+binIdx] = localHist[0].s0;
\n          histResult[1*HIST_SIZE+binIdx] = localHist[0].s1;
\n          histResult[2*HIST_SIZE+binIdx] = localHist[0].s2;
\n          histResult[3*HIST_SIZE+binIdx] = localHist[0].s3;
\n      }
\n  
\n  } // kernel_HistogramRectAllChannels
#endif
