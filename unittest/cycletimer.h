// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// Portability include to match the Google test environment.

#ifndef TESSERACT_UNITTEST_CYCLETIMER_H
#define TESSERACT_UNITTEST_CYCLETIMER_H

#include "absl/time/clock.h" // for GetCurrentTimeNanos

// See https://github.com/google/or-tools/blob/master/ortools/base/timer.h
class CycleTimer {
public:
  CycleTimer() {
    Reset();
  }

  void Reset() {
    running_ = false;
    sum_ = 0;
    start_ = 0;
  }

  // When Start() is called multiple times, only the most recent is used.
  void Start() {
    running_ = true;
    start_ = absl::GetCurrentTimeNanos();
  }

  void Restart() {
    sum_ = 0;
    Start();
  }

  void Stop() {
    if (running_) {
      sum_ += absl::GetCurrentTimeNanos() - start_;
      running_ = false;
    }
  }
  int64_t GetInMs() const { return GetNanos() / 1000000; }

 protected:
  int64_t GetNanos() const {
    return running_ ? absl::GetCurrentTimeNanos() - start_ + sum_ : sum_;
  }

 private:
  bool running_;
  int64_t start_;
  int64_t sum_;
};

#endif  // TESSERACT_UNITTEST_CYCLETIMER_H
