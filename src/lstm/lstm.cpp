///////////////////////////////////////////////////////////////////////
// File:        lstm.cpp
// Description: Long-term-short-term-memory Recurrent neural network.
// Author:      Ray Smith
// Created:     Wed May 01 17:43:06 PST 2013
//
// (C) Copyright 2013, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#include "lstm.h"

#ifdef _OPENMP
#include <omp.h>
#endif
#include <cstdio>
#include <cstdlib>

#if !defined(__GNUC__) && defined(_MSC_VER)
#include <intrin.h>     // _BitScanReverse
#endif

#include "fullyconnected.h"
#include "functions.h"
#include "networkscratch.h"
#include "tprintf.h"

// Macros for openmp code if it is available, otherwise empty macros.
#ifdef _OPENMP
#define PARALLEL_IF_OPENMP(__num_threads)                                  \
  PRAGMA(omp parallel if (__num_threads > 1) num_threads(__num_threads)) { \
    PRAGMA(omp sections nowait) {                                          \
      PRAGMA(omp section) {
#define SECTION_IF_OPENMP \
        } \
    PRAGMA(omp section) \
        {

#define END_PARALLEL_IF_OPENMP \
        } \
      }  /* end of sections */ \
    }  /* end of parallel section */

// Define the portable PRAGMA macro.
#ifdef _MSC_VER  // Different _Pragma
#define PRAGMA(x) __pragma(x)
#else
#define PRAGMA(x) _Pragma(#x)
#endif  // _MSC_VER

#else  // _OPENMP
#define PARALLEL_IF_OPENMP(__num_threads)
#define SECTION_IF_OPENMP
#define END_PARALLEL_IF_OPENMP
#endif  // _OPENMP


namespace tesseract {

// Max absolute value of state_. It is reasonably high to enable the state
// to count things.
const double kStateClip = 100.0;
// Max absolute value of gate_errors (the gradients).
const double kErrClip = 1.0f;

// Calculate ceil(log2(n)).
static inline uint32_t ceil_log2(uint32_t n)
{
  // l2 = (unsigned)log2(n).
#if defined(__GNUC__)
  // Use fast inline assembler code for gcc or clang.
  uint32_t l2 = 31 - __builtin_clz(n);
#elif defined(_MSC_VER)
  // Use fast intrinsic function for MS compiler.
  unsigned long l2 = 0;
  _BitScanReverse(&l2, n);
#else
  if (n == 0) return UINT_MAX;
  if (n == 1) return 0;
  uint32_t val = n;
  uint32_t l2 = 0;
  while (val > 1) {
      val >>= 1;
      l2++;
  }
#endif
  // Round up if n is not a power of 2.
  return (n == (1u << l2)) ? l2 : l2 + 1;
}

LSTM::LSTM(const STRING& name, int ni, int ns, int no, bool two_dimensional,
           NetworkType type)
    : Network(type, name, ni, no),
      na_(ni + ns),
      ns_(ns),
      nf_(0),
      is_2d_(two_dimensional),
      softmax_(nullptr),
      input_width_(0) {
  if (two_dimensional) na_ += ns_;
  if (type_ == NT_LSTM || type_ == NT_LSTM_SUMMARY) {
    nf_ = 0;
    // networkbuilder ensures this is always true.
    ASSERT_HOST(no == ns);
  } else if (type_ == NT_LSTM_SOFTMAX || type_ == NT_LSTM_SOFTMAX_ENCODED) {
    nf_ = type_ == NT_LSTM_SOFTMAX ? no_ : ceil_log2(no_);
    softmax_ = new FullyConnected("LSTM Softmax", ns_, no_, NT_SOFTMAX);
  } else {
    tprintf("%d is invalid type of LSTM!\n", type);
    ASSERT_HOST(false);
  }
  na_ += nf_;
}

LSTM::~LSTM() { delete softmax_; }

// Returns the shape output from the network given an input shape (which may
// be partially unknown ie zero).
StaticShape LSTM::OutputShape(const StaticShape& input_shape) const {
  StaticShape result = input_shape;
  result.set_depth(no_);
  if (type_ == NT_LSTM_SUMMARY) result.set_width(1);
  if (softmax_ != nullptr) return softmax_->OutputShape(result);
  return result;
}

// Suspends/Enables training by setting the training_ flag. Serialize and
// DeSerialize only operate on the run-time data if state is false.
void LSTM::SetEnableTraining(TrainingState state) {
  if (state == TS_RE_ENABLE) {
    // Enable only from temp disabled.
    if (training_ == TS_TEMP_DISABLE) training_ = TS_ENABLED;
  } else if (state == TS_TEMP_DISABLE) {
    // Temp disable only from enabled.
    if (training_ == TS_ENABLED) training_ = state;
  } else {
    if (state == TS_ENABLED && training_ != TS_ENABLED) {
      for (int w = 0; w < WT_COUNT; ++w) {
        if (w == GFS && !Is2D()) continue;
        gate_weights_[w].InitBackward();
      }
    }
    training_ = state;
  }
  if (softmax_ != nullptr) softmax_->SetEnableTraining(state);
}

// Sets up the network for training. Initializes weights using weights of
// scale `range` picked according to the random number generator `randomizer`.
int LSTM::InitWeights(float range, TRand* randomizer) {
  Network::SetRandomizer(randomizer);
  num_weights_ = 0;
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    num_weights_ += gate_weights_[w].InitWeightsFloat(
        ns_, na_ + 1, TestFlag(NF_ADAM), range, randomizer);
  }
  if (softmax_ != nullptr) {
    num_weights_ += softmax_->InitWeights(range, randomizer);
  }
  return num_weights_;
}

// Recursively searches the network for softmaxes with old_no outputs,
// and remaps their outputs according to code_map. See network.h for details.
int LSTM::RemapOutputs(int old_no, const std::vector<int>& code_map) {
  if (softmax_ != nullptr) {
    num_weights_ -= softmax_->num_weights();
    num_weights_ += softmax_->RemapOutputs(old_no, code_map);
  }
  return num_weights_;
}

// Converts a float network to an int network.
void LSTM::ConvertToInt() {
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    gate_weights_[w].ConvertToInt();
  }
  if (softmax_ != nullptr) {
    softmax_->ConvertToInt();
  }
}

// Sets up the network for training using the given weight_range.
void LSTM::DebugWeights() {
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    STRING msg = name_;
    msg.add_str_int(" Gate weights ", w);
    gate_weights_[w].Debug2D(msg.string());
  }
  if (softmax_ != nullptr) {
    softmax_->DebugWeights();
  }
}

// Writes to the given file. Returns false in case of error.
bool LSTM::Serialize(TFile* fp) const {
  if (!Network::Serialize(fp)) return false;
  if (!fp->Serialize(&na_)) return false;
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    if (!gate_weights_[w].Serialize(IsTraining(), fp)) return false;
  }
  if (softmax_ != nullptr && !softmax_->Serialize(fp)) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.

bool LSTM::DeSerialize(TFile* fp) {
  if (!fp->DeSerialize(&na_)) return false;
  if (type_ == NT_LSTM_SOFTMAX) {
    nf_ = no_;
  } else if (type_ == NT_LSTM_SOFTMAX_ENCODED) {
    nf_ = ceil_log2(no_);
  } else {
    nf_ = 0;
  }
  is_2d_ = false;
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    if (!gate_weights_[w].DeSerialize(IsTraining(), fp)) return false;
    if (w == CI) {
      ns_ = gate_weights_[CI].NumOutputs();
      is_2d_ = na_ - nf_ == ni_ + 2 * ns_;
    }
  }
  delete softmax_;
  if (type_ == NT_LSTM_SOFTMAX || type_ == NT_LSTM_SOFTMAX_ENCODED) {
    softmax_ = static_cast<FullyConnected*>(Network::CreateFromFile(fp));
    if (softmax_ == nullptr) return false;
  } else {
    softmax_ = nullptr;
  }
  return true;
}

// Runs forward propagation of activations on the input line.
// See NetworkCpp for a detailed discussion of the arguments.
void LSTM::Forward(bool debug, const NetworkIO& input,
                   const TransposedArray* input_transpose,
                   NetworkScratch* scratch, NetworkIO* output) {
  input_map_ = input.stride_map();
  input_width_ = input.Width();
  if (softmax_ != nullptr)
    output->ResizeFloat(input, no_);
  else if (type_ == NT_LSTM_SUMMARY)
    output->ResizeXTo1(input, no_);
  else
    output->Resize(input, no_);
  ResizeForward(input);
  // Temporary storage of forward computation for each gate.
  NetworkScratch::FloatVec temp_lines[WT_COUNT];
  for (int i = 0; i < WT_COUNT; ++i) temp_lines[i].Init(ns_, scratch);
  // Single timestep buffers for the current/recurrent output and state.
  NetworkScratch::FloatVec curr_state, curr_output;
  curr_state.Init(ns_, scratch);
  ZeroVector<double>(ns_, curr_state);
  curr_output.Init(ns_, scratch);
  ZeroVector<double>(ns_, curr_output);
  // Rotating buffers of width buf_width allow storage of the state and output
  // for the other dimension, used only when working in true 2D mode. The width
  // is enough to hold an entire strip of the major direction.
  int buf_width = Is2D() ? input_map_.Size(FD_WIDTH) : 1;
  GenericVector<NetworkScratch::FloatVec> states, outputs;
  if (Is2D()) {
    states.init_to_size(buf_width, NetworkScratch::FloatVec());
    outputs.init_to_size(buf_width, NetworkScratch::FloatVec());
    for (int i = 0; i < buf_width; ++i) {
      states[i].Init(ns_, scratch);
      ZeroVector<double>(ns_, states[i]);
      outputs[i].Init(ns_, scratch);
      ZeroVector<double>(ns_, outputs[i]);
    }
  }
  // Used only if a softmax LSTM.
  NetworkScratch::FloatVec softmax_output;
  NetworkScratch::IO int_output;
  if (softmax_ != nullptr) {
    softmax_output.Init(no_, scratch);
    ZeroVector<double>(no_, softmax_output);
    int rounded_softmax_inputs = gate_weights_[CI].RoundInputs(ns_);
    if (input.int_mode())
      int_output.Resize2d(true, 1, rounded_softmax_inputs, scratch);
    softmax_->SetupForward(input, nullptr);
  }
  NetworkScratch::FloatVec curr_input;
  curr_input.Init(na_, scratch);
  StrideMap::Index src_index(input_map_);
  // Used only by NT_LSTM_SUMMARY.
  StrideMap::Index dest_index(output->stride_map());
  do {
    int t = src_index.t();
    // True if there is a valid old state for the 2nd dimension.
    bool valid_2d = Is2D();
    if (valid_2d) {
      StrideMap::Index dim_index(src_index);
      if (!dim_index.AddOffset(-1, FD_HEIGHT)) valid_2d = false;
    }
    // Index of the 2-D revolving buffers (outputs, states).
    int mod_t = Modulo(t, buf_width);      // Current timestep.
    // Setup the padded input in source.
    source_.CopyTimeStepGeneral(t, 0, ni_, input, t, 0);
    if (softmax_ != nullptr) {
      source_.WriteTimeStepPart(t, ni_, nf_, softmax_output);
    }
    source_.WriteTimeStepPart(t, ni_ + nf_, ns_, curr_output);
    if (Is2D())
      source_.WriteTimeStepPart(t, ni_ + nf_ + ns_, ns_, outputs[mod_t]);
    if (!source_.int_mode()) source_.ReadTimeStep(t, curr_input);
    // Matrix multiply the inputs with the source.
    PARALLEL_IF_OPENMP(GFS)
    // It looks inefficient to create the threads on each t iteration, but the
    // alternative of putting the parallel outside the t loop, a single around
    // the t-loop and then tasks in place of the sections is a *lot* slower.
    // Cell inputs.
    if (source_.int_mode())
      gate_weights_[CI].MatrixDotVector(source_.i(t), temp_lines[CI]);
    else
      gate_weights_[CI].MatrixDotVector(curr_input, temp_lines[CI]);
    FuncInplace<GFunc>(ns_, temp_lines[CI]);

    SECTION_IF_OPENMP
    // Input Gates.
    if (source_.int_mode())
      gate_weights_[GI].MatrixDotVector(source_.i(t), temp_lines[GI]);
    else
      gate_weights_[GI].MatrixDotVector(curr_input, temp_lines[GI]);
    FuncInplace<FFunc>(ns_, temp_lines[GI]);

    SECTION_IF_OPENMP
    // 1-D forget gates.
    if (source_.int_mode())
      gate_weights_[GF1].MatrixDotVector(source_.i(t), temp_lines[GF1]);
    else
      gate_weights_[GF1].MatrixDotVector(curr_input, temp_lines[GF1]);
    FuncInplace<FFunc>(ns_, temp_lines[GF1]);

    // 2-D forget gates.
    if (Is2D()) {
      if (source_.int_mode())
        gate_weights_[GFS].MatrixDotVector(source_.i(t), temp_lines[GFS]);
      else
        gate_weights_[GFS].MatrixDotVector(curr_input, temp_lines[GFS]);
      FuncInplace<FFunc>(ns_, temp_lines[GFS]);
    }

    SECTION_IF_OPENMP
    // Output gates.
    if (source_.int_mode())
      gate_weights_[GO].MatrixDotVector(source_.i(t), temp_lines[GO]);
    else
      gate_weights_[GO].MatrixDotVector(curr_input, temp_lines[GO]);
    FuncInplace<FFunc>(ns_, temp_lines[GO]);
    END_PARALLEL_IF_OPENMP

    // Apply forget gate to state.
    MultiplyVectorsInPlace(ns_, temp_lines[GF1], curr_state);
    if (Is2D()) {
      // Max-pool the forget gates (in 2-d) instead of blindly adding.
      int8_t* which_fg_col = which_fg_[t];
      memset(which_fg_col, 1, ns_ * sizeof(which_fg_col[0]));
      if (valid_2d) {
        const double* stepped_state = states[mod_t];
        for (int i = 0; i < ns_; ++i) {
          if (temp_lines[GF1][i] < temp_lines[GFS][i]) {
            curr_state[i] = temp_lines[GFS][i] * stepped_state[i];
            which_fg_col[i] = 2;
          }
        }
      }
    }
    MultiplyAccumulate(ns_, temp_lines[CI], temp_lines[GI], curr_state);
    // Clip curr_state to a sane range.
    ClipVector<double>(ns_, -kStateClip, kStateClip, curr_state);
    if (IsTraining()) {
      // Save the gate node values.
      node_values_[CI].WriteTimeStep(t, temp_lines[CI]);
      node_values_[GI].WriteTimeStep(t, temp_lines[GI]);
      node_values_[GF1].WriteTimeStep(t, temp_lines[GF1]);
      node_values_[GO].WriteTimeStep(t, temp_lines[GO]);
      if (Is2D()) node_values_[GFS].WriteTimeStep(t, temp_lines[GFS]);
    }
    FuncMultiply<HFunc>(curr_state, temp_lines[GO], ns_, curr_output);
    if (IsTraining()) state_.WriteTimeStep(t, curr_state);
    if (softmax_ != nullptr) {
      if (input.int_mode()) {
        int_output->WriteTimeStepPart(0, 0, ns_, curr_output);
        softmax_->ForwardTimeStep(int_output->i(0), t, softmax_output);
      } else {
        softmax_->ForwardTimeStep(curr_output, t, softmax_output);
      }
      output->WriteTimeStep(t, softmax_output);
      if (type_ == NT_LSTM_SOFTMAX_ENCODED) {
        CodeInBinary(no_, nf_, softmax_output);
      }
    } else if (type_ == NT_LSTM_SUMMARY) {
      // Output only at the end of a row.
      if (src_index.IsLast(FD_WIDTH)) {
        output->WriteTimeStep(dest_index.t(), curr_output);
        dest_index.Increment();
      }
    } else {
      output->WriteTimeStep(t, curr_output);
    }
    // Save states for use by the 2nd dimension only if needed.
    if (Is2D()) {
      CopyVector(ns_, curr_state, states[mod_t]);
      CopyVector(ns_, curr_output, outputs[mod_t]);
    }
    // Always zero the states at the end of every row, but only for the major
    // direction. The 2-D state remains intact.
    if (src_index.IsLast(FD_WIDTH)) {
      ZeroVector<double>(ns_, curr_state);
      ZeroVector<double>(ns_, curr_output);
    }
  } while (src_index.Increment());
#if DEBUG_DETAIL > 0
  tprintf("Source:%s\n", name_.string());
  source_.Print(10);
  tprintf("State:%s\n", name_.string());
  state_.Print(10);
  tprintf("Output:%s\n", name_.string());
  output->Print(10);
#endif
  if (debug) DisplayForward(*output);
}

// Runs backward propagation of errors on the deltas line.
// See NetworkCpp for a detailed discussion of the arguments.
bool LSTM::Backward(bool debug, const NetworkIO& fwd_deltas,
                    NetworkScratch* scratch,
                    NetworkIO* back_deltas) {
  if (debug) DisplayBackward(fwd_deltas);
  back_deltas->ResizeToMap(fwd_deltas.int_mode(), input_map_, ni_);
  // ======Scratch space.======
  // Output errors from deltas with recurrence from sourceerr.
  NetworkScratch::FloatVec outputerr;
  outputerr.Init(ns_, scratch);
  // Recurrent error in the state/source.
  NetworkScratch::FloatVec curr_stateerr, curr_sourceerr;
  curr_stateerr.Init(ns_, scratch);
  curr_sourceerr.Init(na_, scratch);
  ZeroVector<double>(ns_, curr_stateerr);
  ZeroVector<double>(na_, curr_sourceerr);
  // Errors in the gates.
  NetworkScratch::FloatVec gate_errors[WT_COUNT];
  for (int g = 0; g < WT_COUNT; ++g) gate_errors[g].Init(ns_, scratch);
  // Rotating buffers of width buf_width allow storage of the recurrent time-
  // steps used only for true 2-D. Stores one full strip of the major direction.
  int buf_width = Is2D() ? input_map_.Size(FD_WIDTH) : 1;
  GenericVector<NetworkScratch::FloatVec> stateerr, sourceerr;
  if (Is2D()) {
    stateerr.init_to_size(buf_width, NetworkScratch::FloatVec());
    sourceerr.init_to_size(buf_width, NetworkScratch::FloatVec());
    for (int t = 0; t < buf_width; ++t) {
      stateerr[t].Init(ns_, scratch);
      sourceerr[t].Init(na_, scratch);
      ZeroVector<double>(ns_, stateerr[t]);
      ZeroVector<double>(na_, sourceerr[t]);
    }
  }
  // Parallel-generated sourceerr from each of the gates.
  NetworkScratch::FloatVec sourceerr_temps[WT_COUNT];
  for (int w = 0; w < WT_COUNT; ++w)
    sourceerr_temps[w].Init(na_, scratch);
  int width = input_width_;
  // Transposed gate errors stored over all timesteps for sum outer.
  NetworkScratch::GradientStore gate_errors_t[WT_COUNT];
  for (int w = 0; w < WT_COUNT; ++w) {
    gate_errors_t[w].Init(ns_, width, scratch);
  }
  // Used only if softmax_ != nullptr.
  NetworkScratch::FloatVec softmax_errors;
  NetworkScratch::GradientStore softmax_errors_t;
  if (softmax_ != nullptr) {
    softmax_errors.Init(no_, scratch);
    softmax_errors_t.Init(no_, width, scratch);
  }
  double state_clip = Is2D() ? 9.0 : 4.0;
#if DEBUG_DETAIL > 1
  tprintf("fwd_deltas:%s\n", name_.string());
  fwd_deltas.Print(10);
#endif
  StrideMap::Index dest_index(input_map_);
  dest_index.InitToLast();
  // Used only by NT_LSTM_SUMMARY.
  StrideMap::Index src_index(fwd_deltas.stride_map());
  src_index.InitToLast();
  do {
    int t = dest_index.t();
    bool at_last_x = dest_index.IsLast(FD_WIDTH);
    // up_pos is the 2-D back step, down_pos is the 2-D fwd step, and are only
    // valid if >= 0, which is true if 2d and not on the top/bottom.
    int up_pos = -1;
    int down_pos = -1;
    if (Is2D()) {
      if (dest_index.index(FD_HEIGHT) > 0) {
        StrideMap::Index up_index(dest_index);
        if (up_index.AddOffset(-1, FD_HEIGHT)) up_pos = up_index.t();
      }
      if (!dest_index.IsLast(FD_HEIGHT)) {
        StrideMap::Index down_index(dest_index);
        if (down_index.AddOffset(1, FD_HEIGHT)) down_pos = down_index.t();
      }
    }
    // Index of the 2-D revolving buffers (sourceerr, stateerr).
    int mod_t = Modulo(t, buf_width);      // Current timestep.
    // Zero the state in the major direction only at the end of every row.
    if (at_last_x) {
      ZeroVector<double>(na_, curr_sourceerr);
      ZeroVector<double>(ns_, curr_stateerr);
    }
    // Setup the outputerr.
    if (type_ == NT_LSTM_SUMMARY) {
      if (dest_index.IsLast(FD_WIDTH)) {
        fwd_deltas.ReadTimeStep(src_index.t(), outputerr);
        src_index.Decrement();
      } else {
        ZeroVector<double>(ns_, outputerr);
      }
    } else if (softmax_ == nullptr) {
      fwd_deltas.ReadTimeStep(t, outputerr);
    } else {
      softmax_->BackwardTimeStep(fwd_deltas, t, softmax_errors,
                                 softmax_errors_t.get(), outputerr);
    }
    if (!at_last_x)
      AccumulateVector(ns_, curr_sourceerr + ni_ + nf_, outputerr);
    if (down_pos >= 0)
      AccumulateVector(ns_, sourceerr[mod_t] + ni_ + nf_ + ns_, outputerr);
    // Apply the 1-d forget gates.
    if (!at_last_x) {
      const float* next_node_gf1 = node_values_[GF1].f(t + 1);
      for (int i = 0; i < ns_; ++i) {
        curr_stateerr[i] *= next_node_gf1[i];
      }
    }
    if (Is2D() && t + 1 < width) {
      for (int i = 0; i < ns_; ++i) {
        if (which_fg_[t + 1][i] != 1) curr_stateerr[i] = 0.0;
      }
      if (down_pos >= 0) {
        const float* right_node_gfs = node_values_[GFS].f(down_pos);
        const double* right_stateerr = stateerr[mod_t];
        for (int i = 0; i < ns_; ++i) {
          if (which_fg_[down_pos][i] == 2) {
            curr_stateerr[i] += right_stateerr[i] * right_node_gfs[i];
          }
        }
      }
    }
    state_.FuncMultiply3Add<HPrime>(node_values_[GO], t, outputerr,
                                    curr_stateerr);
    // Clip stateerr_ to a sane range.
    ClipVector<double>(ns_, -state_clip, state_clip, curr_stateerr);
#if DEBUG_DETAIL > 1
    if (t + 10 > width) {
      tprintf("t=%d, stateerr=", t);
      for (int i = 0; i < ns_; ++i)
        tprintf(" %g,%g,%g", curr_stateerr[i], outputerr[i],
                curr_sourceerr[ni_ + nf_ + i]);
      tprintf("\n");
    }
#endif
    // Matrix multiply to get the source errors.
    PARALLEL_IF_OPENMP(GFS)

    // Cell inputs.
    node_values_[CI].FuncMultiply3<GPrime>(t, node_values_[GI], t,
                                           curr_stateerr, gate_errors[CI]);
    ClipVector(ns_, -kErrClip, kErrClip, gate_errors[CI].get());
    gate_weights_[CI].VectorDotMatrix(gate_errors[CI], sourceerr_temps[CI]);
    gate_errors_t[CI].get()->WriteStrided(t, gate_errors[CI]);

    SECTION_IF_OPENMP
    // Input Gates.
    node_values_[GI].FuncMultiply3<FPrime>(t, node_values_[CI], t,
                                           curr_stateerr, gate_errors[GI]);
    ClipVector(ns_, -kErrClip, kErrClip, gate_errors[GI].get());
    gate_weights_[GI].VectorDotMatrix(gate_errors[GI], sourceerr_temps[GI]);
    gate_errors_t[GI].get()->WriteStrided(t, gate_errors[GI]);

    SECTION_IF_OPENMP
    // 1-D forget Gates.
    if (t > 0) {
      node_values_[GF1].FuncMultiply3<FPrime>(t, state_, t - 1, curr_stateerr,
                                              gate_errors[GF1]);
      ClipVector(ns_, -kErrClip, kErrClip, gate_errors[GF1].get());
      gate_weights_[GF1].VectorDotMatrix(gate_errors[GF1],
                                         sourceerr_temps[GF1]);
    } else {
      memset(gate_errors[GF1], 0, ns_ * sizeof(gate_errors[GF1][0]));
      memset(sourceerr_temps[GF1], 0, na_ * sizeof(*sourceerr_temps[GF1]));
    }
    gate_errors_t[GF1].get()->WriteStrided(t, gate_errors[GF1]);

    // 2-D forget Gates.
    if (up_pos >= 0) {
      node_values_[GFS].FuncMultiply3<FPrime>(t, state_, up_pos, curr_stateerr,
                                              gate_errors[GFS]);
      ClipVector(ns_, -kErrClip, kErrClip, gate_errors[GFS].get());
      gate_weights_[GFS].VectorDotMatrix(gate_errors[GFS],
                                         sourceerr_temps[GFS]);
    } else {
      memset(gate_errors[GFS], 0, ns_ * sizeof(gate_errors[GFS][0]));
      memset(sourceerr_temps[GFS], 0, na_ * sizeof(*sourceerr_temps[GFS]));
    }
    if (Is2D()) gate_errors_t[GFS].get()->WriteStrided(t, gate_errors[GFS]);

    SECTION_IF_OPENMP
    // Output gates.
    state_.Func2Multiply3<HFunc, FPrime>(node_values_[GO], t, outputerr,
                                         gate_errors[GO]);
    ClipVector(ns_, -kErrClip, kErrClip, gate_errors[GO].get());
    gate_weights_[GO].VectorDotMatrix(gate_errors[GO], sourceerr_temps[GO]);
    gate_errors_t[GO].get()->WriteStrided(t, gate_errors[GO]);
    END_PARALLEL_IF_OPENMP

    SumVectors(na_, sourceerr_temps[CI], sourceerr_temps[GI],
               sourceerr_temps[GF1], sourceerr_temps[GO], sourceerr_temps[GFS],
               curr_sourceerr);
    back_deltas->WriteTimeStep(t, curr_sourceerr);
    // Save states for use by the 2nd dimension only if needed.
    if (Is2D()) {
      CopyVector(ns_, curr_stateerr, stateerr[mod_t]);
      CopyVector(na_, curr_sourceerr, sourceerr[mod_t]);
    }
  } while (dest_index.Decrement());
#if DEBUG_DETAIL > 2
  for (int w = 0; w < WT_COUNT; ++w) {
    tprintf("%s gate errors[%d]\n", name_.string(), w);
    gate_errors_t[w].get()->PrintUnTransposed(10);
  }
#endif
  // Transposed source_ used to speed-up SumOuter.
  NetworkScratch::GradientStore source_t, state_t;
  source_t.Init(na_, width, scratch);
  source_.Transpose(source_t.get());
  state_t.Init(ns_, width, scratch);
  state_.Transpose(state_t.get());
#ifdef _OPENMP
#pragma omp parallel for num_threads(GFS) if (!Is2D())
#endif
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    gate_weights_[w].SumOuterTransposed(*gate_errors_t[w], *source_t, false);
  }
  if (softmax_ != nullptr) {
    softmax_->FinishBackward(*softmax_errors_t);
  }
  return needs_to_backprop_;
}

// Updates the weights using the given learning rate, momentum and adam_beta.
// num_samples is used in the adam computation iff use_adam_ is true.
void LSTM::Update(float learning_rate, float momentum, float adam_beta,
                  int num_samples) {
#if DEBUG_DETAIL > 3
  PrintW();
#endif
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    gate_weights_[w].Update(learning_rate, momentum, adam_beta, num_samples);
  }
  if (softmax_ != nullptr) {
    softmax_->Update(learning_rate, momentum, adam_beta, num_samples);
  }
#if DEBUG_DETAIL > 3
  PrintDW();
#endif
}

// Sums the products of weight updates in *this and other, splitting into
// positive (same direction) in *same and negative (different direction) in
// *changed.
void LSTM::CountAlternators(const Network& other, double* same,
                            double* changed) const {
  ASSERT_HOST(other.type() == type_);
  const LSTM* lstm = static_cast<const LSTM*>(&other);
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    gate_weights_[w].CountAlternators(lstm->gate_weights_[w], same, changed);
  }
  if (softmax_ != nullptr) {
    softmax_->CountAlternators(*lstm->softmax_, same, changed);
  }
}

// Prints the weights for debug purposes.
void LSTM::PrintW() {
  tprintf("Weight state:%s\n", name_.string());
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    tprintf("Gate %d, inputs\n", w);
    for (int i = 0; i < ni_; ++i) {
      tprintf("Row %d:", i);
      for (int s = 0; s < ns_; ++s)
        tprintf(" %g", gate_weights_[w].GetWeights(s)[i]);
      tprintf("\n");
    }
    tprintf("Gate %d, outputs\n", w);
    for (int i = ni_; i < ni_ + ns_; ++i) {
      tprintf("Row %d:", i - ni_);
      for (int s = 0; s < ns_; ++s)
        tprintf(" %g", gate_weights_[w].GetWeights(s)[i]);
      tprintf("\n");
    }
    tprintf("Gate %d, bias\n", w);
    for (int s = 0; s < ns_; ++s)
      tprintf(" %g", gate_weights_[w].GetWeights(s)[na_]);
    tprintf("\n");
  }
}

// Prints the weight deltas for debug purposes.
void LSTM::PrintDW() {
  tprintf("Delta state:%s\n", name_.string());
  for (int w = 0; w < WT_COUNT; ++w) {
    if (w == GFS && !Is2D()) continue;
    tprintf("Gate %d, inputs\n", w);
    for (int i = 0; i < ni_; ++i) {
      tprintf("Row %d:", i);
      for (int s = 0; s < ns_; ++s)
        tprintf(" %g", gate_weights_[w].GetDW(s, i));
      tprintf("\n");
    }
    tprintf("Gate %d, outputs\n", w);
    for (int i = ni_; i < ni_ + ns_; ++i) {
      tprintf("Row %d:", i - ni_);
      for (int s = 0; s < ns_; ++s)
        tprintf(" %g", gate_weights_[w].GetDW(s, i));
      tprintf("\n");
    }
    tprintf("Gate %d, bias\n", w);
    for (int s = 0; s < ns_; ++s)
      tprintf(" %g", gate_weights_[w].GetDW(s, na_));
    tprintf("\n");
  }
}

// Resizes forward data to cope with an input image of the given width.
void LSTM::ResizeForward(const NetworkIO& input) {
  int rounded_inputs = gate_weights_[CI].RoundInputs(na_);
  source_.Resize(input, rounded_inputs);
  which_fg_.ResizeNoInit(input.Width(), ns_);
  if (IsTraining()) {
    state_.ResizeFloat(input, ns_);
    for (int w = 0; w < WT_COUNT; ++w) {
      if (w == GFS && !Is2D()) continue;
      node_values_[w].ResizeFloat(input, ns_);
    }
  }
}


}  // namespace tesseract.
