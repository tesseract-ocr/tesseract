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

#include <chrono> // for std::chrono

// See https://github.com/google/or-tools/blob/master/ortools/base/timer.h
class CycleTimer {
private:
  static int64_t now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
  }

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
    start_ = now();
  }

  void Restart() {
    sum_ = 0;
    Start();
  }

  void Stop() {
    if (running_) {
      sum_ += now() - start_;
      running_ = false;
    }
  }
  int64_t GetInMs() const {
    return running_ ? now() - start_ + sum_ : sum_;
  }

private:
  bool running_;
  int64_t start_;
  int64_t sum_;
};

#endif // TESSERACT_UNITTEST_CYCLETIMER_H
