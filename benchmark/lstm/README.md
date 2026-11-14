# Benchmarks for the LSTM code

Tools to measure the speed of LSTM training (and of the matrix products
which dominate training and recognition), and the results which were
measured with them.

## Files

- `bench.sh` times `lstmtraining` for a fixed number of training lines.
- `sample-summary.py` summarizes a profile which was made with the macOS tool
  `sample`.
- `neon-kernel-variants.cpp` is a microbenchmark for the float matrix products
  (variants of `RowDotProductsNEON` and `CrossDotProductsNEON`).
- `neon-int-kernel-variants.cpp` is a microbenchmark for the `int8_t` matrix
  product which is used for the recognition with integer (fast) models
  (variants of `PartialMatrixDotVector8`).

The two microbenchmarks are standalone programs for ARM64. How to build
them is described at the beginning of each file.

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
