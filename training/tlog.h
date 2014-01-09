/**********************************************************************
 * File:        tlog.h
 * Description: Variant of printf with logging level controllable by a
 *              commandline flag.
 * Author:      Ranjith Unnikrishnan
 * Created:     Wed Nov 20 2013
 *
 * (C) Copyright 2013, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/
#ifndef TESSERACT_TRAINING_TLOG_H_
#define TESSERACT_TRAINING_TLOG_H_

#include "commandlineflags.h"
#include "errcode.h"
#include "tprintf.h"

DECLARE_INT_PARAM_FLAG(tlog_level);

// Variant guarded by the numeric logging level parameter FLAGS_tlog_level
// (default 0).  Code using ParseCommandLineFlags() can control its value using
// the --tlog_level commandline argument. Otherwise it must be specified in a
// config file like other params.
#define tlog(level, ...) {                        \
  if (FLAGS_tlog_level >= level) {                \
    tprintf_internal(__VA_ARGS__);                \
  }                                               \
}

#define TLOG_IS_ON(level) (FLAGS_tlog_level >= level)

#define TLOG_FATAL(msg...)                                              \
  {                                                                     \
    tprintf(msg);                                                       \
    ASSERT_FAILED.error("", ABORT, "in file %s, line %d",               \
                        __FILE__, __LINE__);                            \
  }


#endif  // TESSERACT_TRAINING_TLOG_H_
