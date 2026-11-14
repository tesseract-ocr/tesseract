# Benchmarks for the LSTM code

Tools to measure the speed of LSTM training (and of the matrix products
which dominate training and recognition), and the results which were
measured with them.

## Files

- `bench.sh` times `lstmtraining` for a fixed number of training lines.
- `sample-summary.py` summarizes a profile which was made with the macOS tool
  `sample`.
- `neon-kernel-variants.cpp` is a microbenchmark for the float matrix products
  on ARM64 (variants of `RowDotProductsNEON` and `CrossDotProductsNEON`).
- `neon-int-kernel-variants.cpp` is a microbenchmark for the `int8_t` matrix
  product which is used for the recognition with integer (fast) models
  (variants of `PartialMatrixDotVector8`).

- `x86-kernel-variants.cpp` is the same microbenchmark for the float matrix
  products on x86_64 (variants of `RowDotProductsFMA`, `CrossDotProductsFMA`,
  `RowDotProductsAVX512F` and `CrossDotProductsAVX512F`).

The microbenchmarks are standalone programs for ARM64 or x86_64. How to
build them is described at the beginning of each file.

## Timing the training

`bench.sh` trains the default network of
[tesstrain](https://github.com/tesseract-ocr/tesstrain)
(`[1,36,0,1 Ct3,3,16 Mp3,3 Lfys48 Lfx96 Lrx96 Lfx192 O1c111]`) from scratch
on the 1036 lines of `test/testdata/eng.Arial.exp0.lstmf`. It needs a build
directory with the training tools, the `test` submodule and the sibling
directory `../langdata_lstm`.

    benchmark/lstm/bench.sh build
    NAME=generic benchmark/lstm/bench.sh build env DOTPRODUCT=generic
    NAME=omp1 benchmark/lstm/bench.sh build-openmp env OMP_THREAD_LIMIT=1
    NAME=x925 benchmark/lstm/bench.sh build taskset -c 5-9,15-19

Each run prints one line with the times in seconds and the last progress
message of the training, which shows whether two runs calculated the same
results:

    build:   real 15.83 user 15.41 sys 0.27 | 999/1000/1000, mean rms=5.173%, ...

The number of iterations, the network and the working directory can be set
with environment variables (see the comment in the script).

Run only one benchmark at a time (several runs disturb each other, and
tests which run at the same time do so, too) and repeat a measurement if
the machine was busy.

## Profiling

On macOS, `sample` works without special rights:

    ITER=2500 benchmark/lstm/bench.sh build &
    sleep 4
    sample $(pgrep -n lstmtraining) 20 1 -file sample.txt
    benchmark/lstm/sample-summary.py sample.txt

On Linux, `perf record` / `perf report` can be used if
`/proc/sys/kernel/perf_event_paranoid` allows it (a value of 2 or less).

## Results

Measured in November 2026 with 1000 training lines, FAST_FLOAT, a Release
build with CMake, and a single thread unless noted otherwise.

- M1: Apple M1 Pro, macOS, Apple clang 21, performance cores.
- X925: NVIDIA DGX Spark (10 Cortex-X925 and 10 Cortex-A725 cores),
  Ubuntu 24.04, g++ 13, pinned to the Cortex-X925 cores.

"After" is the commit "lstm: calculate several dot products at once with
NEON", "before" is its parent.

### Training time

| Build and settings                          | M1 before | M1 after | X925 before | X925 after |
| ------------------------------------------- | --------: | -------: | ----------: | ---------: |
| Without OpenMP                              |    22.2 s |   15.9 s |      14.4 s |     10.2 s |
| Without OpenMP, Cortex-A725 cores           |           |          |      31.3 s |     24.3 s |
| Without OpenMP, `DOTPRODUCT=generic`        |           |   85.2 s |      51.5 s |     50.6 s |
| OpenMP, `OMP_THREAD_LIMIT=1`                |    23.0 s |          |      15.5 s |            |
| OpenMP, default (4 threads)                 | 56 - 98 s |          |      15.5 s |     12.8 s |
| OpenMP, `OMP_PROC_BIND=true OMP_PLACES=cores` |         |          |      11.6 s |      9.6 s |
| OpenMP, `OMP_WAIT_POLICY=passive`           |    79.4 s |          |      62.4 s |            |

- The new functions make the training 1.4 times faster on both machines.
- The results for `DOTPRODUCT=generic` are the same before and after.
- OpenMP does not pay off. The LSTM code starts a parallel region for each
  timestep, which costs more than the small matrices gain. On macOS (libomp)
  it is several times slower. On Linux (libgomp) it is slower by default, and
  with bound threads it is 1.06 (after) to 1.24 (before) times faster for
  more than twice the CPU time.
- The results with and without OpenMP are identical.
- A process which is not pinned runs on the Cortex-X925 cores by itself.

### Profile of the training (M1, share of the samples of the main thread)

| Function (with everything it calls)           | Before | After |
| --------------------------------------------- | -----: | ----: |
| `WeightMatrix::SumOuterTransposed` (gradients) |  25 %  | 12 %  |
| `WeightMatrix::MatrixDotVector` (forward)      |  22 %  | 24 %  |
| `WeightMatrix::VectorDotMatrix` (backward)     |  21 %  | 20 %  |
| `Plumbing::Update` (weight update)             | 3.5 %  | 4.9 % |
| `LSTMTrainer::MaintainCheckpoints`             | 2.6 %  | 3.8 % |
| `CTC::ComputeCTCTargets`                       | 1.8 %  | 2.4 % |
| `ImageData::PreScale` (decode and scale image) | 0.8 %  |       |

The shares after the change refer to a total which is 1.4 times smaller.
`VectorDotMatrix` is inlined after the change, its share is the one of
`RowDotProductsNEON` called from `LSTM::Backward`.

### Float matrix products (`neon-kernel-variants.cpp`)

Time for the largest layer (Lfx192, 192 x 289 weights for each of the four
gates), single dot products compared with the implemented variants:

| Product                                  | M1 single | M1 new | X925 single | X925 new |
| ---------------------------------------- | --------: | -----: | ----------: | -------: |
| Forward, 4 gates, per timestep (4x2)     |   17.9 us | 14.1 us |    11.0 us |   9.0 us |
| Backward, 4 gates, per timestep (4x2)    |   17.0 us | 13.3 us |     9.5 us |   7.5 us |
| Gradients, 1 gate, 300 timesteps (4x4)   |   1.39 ms | 0.39 ms |    0.78 ms |  0.25 ms |

- The smaller layers gain more for the products with a vector (up to 1.9
  times).
- More accumulators for a single dot product gain much less in the context
  of a matrix product (about 7 % on M1) than for a single call.
- Variants which keep the accumulators in an array of vectors instead of
  named variables were two to four times slower with g++ 13.

### Integer matrix product (`neon-int-kernel-variants.cpp`)

Recognition of three test images with `tessdata_fast/eng` on M1 spent 73 %
of the time in `LSTMRecognizer::RecognizeLine` and 30 % in the integer
`matrixDotVector` of `src/arch/intsimdmatrixneon.cpp` with the old code.

Time for the four gates of a layer per timestep in microseconds. "old" is
the old code, "mul8" the new code for any ARM CPU with NEON
(`intSimdMatrixNEON`) and "dot8" the new code for CPUs with the dot product
instructions of ARMv8.2 (`intSimdMatrixNEONDotProd`). A725 is a Cortex-A725
core of the same NVIDIA DGX Spark.

| Layer  | M1 old | M1 mul8 | M1 dot8 | X925 old | X925 mul8 | X925 dot8 | A725 old | A725 mul8 | A725 dot8 |
| ------ | -----: | ------: | ------: | -------: | --------: | --------: | -------: | --------: | --------: |
| Lfys48 |   0.60 |    0.32 |    0.14 |     0.46 |      0.29 |      0.12 |     1.67 |      0.77 |      0.34 |
| Lfx96  |   2.48 |    1.25 |    0.53 |     2.02 |      1.12 |      0.46 |     7.44 |      2.94 |      1.05 |
| Lrx96  |   3.33 |    1.61 |    0.66 |     2.69 |      1.43 |      0.55 |     9.92 |      3.77 |      1.41 |
| Lfx192 |   9.93 |    4.75 |    2.85 |     8.04 |      4.12 |      1.78 |     29.7 |      10.8 |      3.88 |
| Lfx512 |   56.0 |    25.9 |    15.7 |     45.4 |      22.5 |      10.3 |      167 |      58.1 |      21.6 |

- All variants give identical results, also for the extreme values of
  weights and inputs.
- 8 rows per pass are a bit faster than 4 rows.
- A variant of "mul" which adds two products with 16 bits before they are
  widened was about 1.5 times faster, but it overflows if weights and inputs
  are -128, so it is not used.

### Recognition time

Time for 36 pages (12 times `phototest.tif`, `eurotext.tif` and
`8087_054.3B.tif` from `test/testing`, listed in a text file) with
`tesseract pages.txt out --tessdata-dir ../MODELS -l eng --oem 1`, before
and after the new NEON functions for the integer matrix product:

| Models          | M1 before | M1 after | X925 before | X925 after | A725 before | A725 after |
| --------------- | --------: | -------: | ----------: | ---------: | ----------: | ---------: |
| `tessdata_fast` |    22.2 s |   17.4 s |      18.7 s |     14.8 s |      43.1 s |     27.8 s |
| `tessdata_best` |    48.7 s |   48.6 s |      36.3 s |     36.2 s |             |            |

- Recognition with the integer models is about 1.27 times faster on M1 and
  X925 and 1.55 times faster on A725. The text results are identical.
- The float models do not use the integer matrix product.

### Whole-vector functions

"Before" is the commit "lstm: calculate several dot products at once with
NEON", "after" the commit which adds the NEON implementations of the
functions for tanh, the logistic function and the conversion to `int8_t`
for whole vectors (`src/arch/functionsneon.cpp`).

Time for 36 pages (see above, `tessdata_fast`) and for the first 12 of them
(`tessdata_best`) on M1 and on the Cortex-X925 cores of the DGX Spark:

| Models                     | M1 before | M1 after | X925 before | X925 after |
| -------------------------- | --------: | -------: | ----------: | ---------: |
| `tessdata_fast` (36 pages) |    17.1 s |   13.8 s |     14.7 s  |    11.6 s  |
| `tessdata_best` (12 pages) |    16.1 s |   14.4 s |     12.1 s  |    11.1 s  |

- Recognition is 1.23 times faster with the integer models on M1 and 1.27
  times on the X925, and 1.11 times faster with the float models on M1 and
  1.09 times on the X925. The text results are identical.

Training time (1000 training lines) on M1 and the X925:

| Settings   | M1 before | M1 after | X925 before | X925 after |
| ---------- | --------: | -------: | ----------: | ---------: |
| Default    |   15.83 s |  15.38 s |     10.4 s  |    10.1 s  |

- The training is 1.03 times faster on both machines. The before and the
  after runs give exactly the same results (`mean rms=5.173%`,
  `delta=35.870%` on M1 and `delta=35.879%` on the X925), so the NEON
  functions give exactly the same results as the scalar functions, also
  with g++ 13.

Time for the whole-vector functions with 192 values (best of several runs,
`-O3`, Apple clang 21) compared to the generic implementations, which use
the scalar functions:

| Function            | Generic | NEON  |
| ------------------- | ------: | ----: |
| Tanh                |  175 ns | 98 ns |
| Logistic            |  162 ns | 138 ns |
| Conversion to `int8_t` | 41 ns | 37 ns |

- Tanh is about 1.8 times faster (the other layer widths give 1.7 to
  1.8 times), the conversion to `int8_t` about 1.1 times faster and the
  logistic function a bit faster.

## Results for x86_64

Measured in November 2026 on an Intel Core i3-1115G4 (Tiger Lake, AVX512),
Debian 13, g++ 14, FAST_FLOAT, a Release build with CMake without OpenMP.
"Before" is the commit "arch: speed up the integer matrix product with
NEON". "Products" adds the functions for several dot products at once for
FMA and AVX512F, "functions" also adds the functions for tanh, the logistic
function and the conversion to `int8_t` for whole vectors with AVX2
(`src/arch/functionsavx2.cpp`).

### Training time on x86_64 (1000 training lines)

| Settings                       | Before | Products | Functions |
| ------------------------------ | -----: | -------: | --------: |
| Default (AVX512F on this CPU)  | 24.3 s |   16.0 s |    15.3 s |
| `DOTPRODUCT=fma`               | 24.3 s |   15.7 s |    15.0 s |
| `DOTPRODUCT=avx2`              | 28.1 s |   27.2 s |           |

- The training is 1.6 times faster.
- A CPU with AVX2 and FMA, but without AVX512F, now uses the functions for
  FMA by default. Before it used `DotProductAVX` (the time for
  `DOTPRODUCT=avx2`), so the gain is larger there.
- `DOTPRODUCT=avx2` still calculates single dot products with the same
  results as before.
- The functions for whole vectors give exactly the same results as the
  scalar functions, so they do not change the results of the training.

### Recognition time on x86_64

Time for the 36 pages which are described above (`tessdata_fast`) and for
the first 12 of them (`tessdata_best`):

| Models                     | Before | Products | Functions |
| -------------------------- | -----: | -------: | --------: |
| `tessdata_fast` (36 pages) | 23.0 s |   23.0 s |    17.0 s |
| `tessdata_best` (12 pages) | 22.7 s |   18.6 s |    16.4 s |

- Recognition is 1.35 times faster with the integer models and 1.4 times
  faster with the float models. The text results are identical.

### Float matrix products (`x86-kernel-variants.cpp`)

Time for the largest layer (Lfx192), single dot products compared with the
implemented variants (g++ 14):

| Product                                 | FMA single | FMA new | AVX512F single | AVX512F new |
| --------------------------------------- | ---------: | ------: | -------------: | ----------: |
| Forward, 4 gates, per timestep (4x2)    |    12.1 us |  9.1 us |        17.6 us |      9.5 us |
| Backward, 4 gates, per timestep (4x2)   |    12.6 us |  9.0 us |        18.2 us |      9.7 us |
| Gradients, 1 gate, 300 timesteps        |    1.24 ms | 0.37 ms |        1.17 ms |     0.35 ms |

- The gradients use blocks of 4 x 2 dot products for FMA (16 registers;
  4 x 4 is 1.3 times slower) and of 4 x 4 for AVX512F (32 registers).
- The smaller layers gain more for the products with a vector (2 to 3
  times).
- The rest of a row (less than 8 or 16 elements) is loaded with a mask
  instead of a scalar loop.
- Registers with 512 bits are not faster than those with 256 bits on this
  CPU.
- The four sums of a pass are reduced together (`Sum4`). A profile showed
  that single calls of `_mm512_reduce_add_ps` took 12 % of the training
  time.

### Tanh, logistic function and conversion to `int8_t`

A profile of the recognition with `tessdata_fast` (see below) showed that
the scalar table lookups for tanh and the logistic function took 17 % of
the time and the conversion of the results to `int8_t` 10 %, more than the
integer matrix product (12 %).

Time for tanh of 192 random values:

| Variant                                              |   Time |
| ---------------------------------------------------- | -----: |
| Scalar (`Tanh`, with branches)                       | 707 ns |
| Scalar without branches                              | 434 ns |
| AVX2 with the gather instruction                     | 442 ns |
| AVX2, table entries loaded one by one (implemented)  | 158 ns |

- The gather instruction is slow on this CPU because of the microcode
  mitigation for "gather data sampling".
- All variants give the same results.

### Integer matrix product

A variant with the dot product instruction of AVX512VNNI (`VPDPBUSD`, 256
bits) was only 1.1 times faster than the code for AVX2 for the integer
matrix product on this CPU, which is not measurable for the whole
recognition. Therefore it was not added.

### Profiling on Linux without perf

The CPU profiler of gperftools works with an unchanged binary (build with
`-g` to get source lines):

    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so.0 \
      CPUPROFILE=/tmp/tess.prof CPUPROFILE_FREQUENCY=1000 \
      build/bin/tesseract pages.txt out --tessdata-dir ../tessdata_fast -l eng
    google-pprof --text build/bin/tesseract /tmp/tess.prof | head -30
    google-pprof --lines --text build/bin/tesseract /tmp/tess.prof | head -40

Profile of the recognition with `tessdata_fast` (12 pages, share of the
samples with everything a function calls):

| Function                                   | Products | Functions |
| ------------------------------------------ | -------: | --------: |
| `LSTM::Forward`                            |     37 % |      22 % |
| Tanh and logistic function                 |     17 % |      10 % |
| Integer `matrixDotVector`                  |     12 % |      14 % |
| Conversion to `int8_t`                     |     10 % |     2.4 % |
| `Convolve::Forward`                        |     12 % |      17 % |
| Page layout analysis (`SegmentPage`)       |     21 % |      28 % |

The shares in the last column refer to a total which is 1.35 times smaller.

## Results for x86_64 on AMD EPYC 9554

Measured in November 2026 on a virtual machine with an AMD EPYC 9554
(Zen 4, AVX512), Debian 13, g++ 14, FAST_FLOAT, built with autotools
(`configure --disable-shared --disable-openmp`, which uses `-g -O2`),
pinned to one core. "Before", "products" and "functions" are the same
commits as for the Intel CPU above.

### Training time on AMD EPYC 9554 (1000 training lines)

| Settings                       | Before | Products | Functions |
| ------------------------------ | -----: | -------: | --------: |
| Default (AVX512F on this CPU)  | 22.2 s |   16.9 s |    16.0 s |
| `DOTPRODUCT=fma`               | 23.2 s |   16.8 s |    16.0 s |
| `DOTPRODUCT=avx2`              | 23.5 s |   23.6 s |    22.8 s |

- The training is 1.4 times faster.

### Recognition time on AMD EPYC 9554

| Models                     | Before | Products | Functions |
| -------------------------- | -----: | -------: | --------: |
| `tessdata_fast` (36 pages) | 24.0 s |   24.0 s |    17.1 s |
| `tessdata_best` (12 pages) | 18.0 s |   15.2 s |    12.9 s |

- Recognition is 1.4 times faster with the integer models and with the
  float models. The text results are identical, and those for
  `tessdata_fast` are also identical to the results on ARM64.
- With the new code, `tessdata_fast` needs 17.1 s with `DOTPRODUCT=fma` or
  `DOTPRODUCT=avx2`, 30.5 s with `DOTPRODUCT=sse` and 70.4 s with
  `DOTPRODUCT=generic`, always with the same text results.

### Profiling with perf

If `/proc/sys/kernel/perf_event_paranoid` is 2, a user can only profile the
user space code of own processes. That needs these options (`cpu-clock`
also works in a virtual machine without hardware counters):

    perf record -e cpu-clock:u --user-callchains --call-graph dwarf,16384 \
      -F 499 build/tesseract pages.txt out --tessdata-dir ../tessdata_fast \
      -l eng
    perf report --children --stdio --sort symbol -g none | head -80

Profile of the recognition with `tessdata_fast` (12 pages) with the new
code, share of the samples with everything a function calls:

| Function                                               | Share |
| ------------------------------------------------------ | ----: |
| Page layout analysis (`SegmentPage`)                   |  24 % |
| `Series::Forward` (the whole network)                  |  55 % |
| `Convolve::Forward`                                    |  22 % |
| `LSTM::Forward`                                        |  22 % |
| Integer `matrixDotVector`                              |  15 % |
| `StrideMap::Index::AddOffset` and `IsValid`            |  15 % |
| Tanh and logistic function (`TanhVectorAVX2`, ...)     | 6.6 % |
| `RecodeBeamSearch::Decode`                             | 5.8 % |
| PNG compression of the line images (`ImageData::SetPix`) | 4.8 % |
| Conversion to `int8_t` (`FloatToInt8VectorAVX2`)       | 1.5 % |

Profile of the training with the new code:

| Function                                               | Share |
| ------------------------------------------------------ | ----: |
| `RowDotProductsAVX512F` (forward and backward)         |  32 % |
| `Plumbing::Update` (weight update)                     |  15 % |
| `LSTM::Backward` without the functions which it calls  |  11 % |
| `Convolve::Forward`                                    | 8.7 % |
| `CrossDotProductsAVX512F` (gradients)                  | 7.9 % |
| `StrideMap::Index::AddOffset` and `IsValid`            | 5.8 % |
| `CTC::ComputeCTCTargets`                               | 2.5 % |

- The weight update takes a much larger share than on ARM64 with clang
  (4.9 %). Most of it is `GENERIC_2D_ARRAY::AdamUpdate` (9.5 %).
- The index calculations of `StrideMap::Index` take a large share of the
  recognition with the integer models. Most calls of `AddOffset` come from
  `Convolve::Forward`.
