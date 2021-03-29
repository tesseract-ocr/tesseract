/**********************************************************************
 * File:        params.cpp
 * Description: Initialization and setting of Tesseract parameters.
 * Author:      Ray Smith
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#include "params.h"

#include "helpers.h"  // for chomp_string
#include "host.h"     // tesseract/export.h, windows.h for MAX_PATH
#include "serialis.h" // for TFile
#include "tprintf.h"

#include <climits> // for INT_MIN, INT_MAX
#include <cmath>   // for NAN, std::isnan
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>  // for std::locale::classic
#include <sstream> // for std::stringstream

namespace tesseract {

tesseract::ParamsVectors *GlobalParams() {
  static tesseract::ParamsVectors global_params = tesseract::ParamsVectors();
  return &global_params;
}

bool ParamUtils::ReadParamsFile(const char *file, SetParamConstraint constraint,
                                ParamsVectors *member_params) {
  TFile fp;
  if (!fp.Open(file, nullptr)) {
    tprintf("read_params_file: Can't open %s\n", file);
    return true;
  }
  return ReadParamsFromFp(constraint, &fp, member_params);
}

bool ParamUtils::ReadParamsFromFp(SetParamConstraint constraint, TFile *fp,
                                  ParamsVectors *member_params) {
  char line[MAX_PATH]; // input line
  bool anyerr = false; // true if any error
  bool foundit;        // found parameter
  char *valptr;        // value field

  while (fp->FGets(line, MAX_PATH) != nullptr) {
    if (line[0] != '\r' && line[0] != '\n' && line[0] != '#') {
      chomp_string(line); // remove newline
      for (valptr = line; *valptr && *valptr != ' ' && *valptr != '\t'; valptr++) {
        ;
      }
      if (*valptr) {    // found blank
        *valptr = '\0'; // make name a string
        do {
          valptr++; // find end of blanks
        } while (*valptr == ' ' || *valptr == '\t');
      }
      foundit = SetParam(line, valptr, constraint, member_params);

      if (!foundit) {
        anyerr = true; // had an error
        tprintf("Warning: Parameter not found: %s\n", line);
      }
    }
  }
  return anyerr;
}

bool ParamUtils::SetParam(const char *name, const char *value, SetParamConstraint constraint,
                          ParamsVectors *member_params) {
  // Look for the parameter among string parameters.
  auto *sp =
      FindParam<StringParam>(name, GlobalParams()->string_params, member_params->string_params);
  if (sp != nullptr && sp->constraint_ok(constraint)) {
    sp->set_value(value);
  }
  if (*value == '\0') {
    return (sp != nullptr);
  }

  // Look for the parameter among int parameters.
  auto *ip = FindParam<IntParam>(name, GlobalParams()->int_params, member_params->int_params);
  if (ip && ip->constraint_ok(constraint)) {
    int intval = INT_MIN;
    std::stringstream stream(value);
    stream.imbue(std::locale::classic());
    stream >> intval;
    if (intval != INT_MIN) {
      ip->set_value(intval);
    }
  }

  // Look for the parameter among bool parameters.
  auto *bp = FindParam<BoolParam>(name, GlobalParams()->bool_params, member_params->bool_params);
  if (bp != nullptr && bp->constraint_ok(constraint)) {
    if (*value == 'T' || *value == 't' || *value == 'Y' || *value == 'y' || *value == '1') {
      bp->set_value(true);
    } else if (*value == 'F' || *value == 'f' || *value == 'N' || *value == 'n' || *value == '0') {
      bp->set_value(false);
    }
  }

  // Look for the parameter among double parameters.
  auto *dp =
      FindParam<DoubleParam>(name, GlobalParams()->double_params, member_params->double_params);
  if (dp != nullptr && dp->constraint_ok(constraint)) {
    double doubleval = NAN;
    std::stringstream stream(value);
    stream.imbue(std::locale::classic());
    stream >> doubleval;
    if (!std::isnan(doubleval)) {
      dp->set_value(doubleval);
    }
  }
  return (sp || ip || bp || dp);
}

bool ParamUtils::GetParamAsString(const char *name, const ParamsVectors *member_params,
                                  std::string *value) {
  // Look for the parameter among string parameters.
  auto *sp =
      FindParam<StringParam>(name, GlobalParams()->string_params, member_params->string_params);
  if (sp) {
    *value = sp->c_str();
    return true;
  }
  // Look for the parameter among int parameters.
  auto *ip = FindParam<IntParam>(name, GlobalParams()->int_params, member_params->int_params);
  if (ip) {
    *value = std::to_string(int32_t(*ip));
    return true;
  }
  // Look for the parameter among bool parameters.
  auto *bp = FindParam<BoolParam>(name, GlobalParams()->bool_params, member_params->bool_params);
  if (bp != nullptr) {
    *value = bool(*bp) ? "1" : "0";
    return true;
  }
  // Look for the parameter among double parameters.
  auto *dp =
      FindParam<DoubleParam>(name, GlobalParams()->double_params, member_params->double_params);
  if (dp != nullptr) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << double(*dp);
    *value = stream.str();
    return true;
  }
  return false;
}

void ParamUtils::PrintParams(FILE *fp, const ParamsVectors *member_params) {
  int num_iterations = (member_params == nullptr) ? 1 : 2;
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  for (int v = 0; v < num_iterations; ++v) {
    const ParamsVectors *vec = (v == 0) ? GlobalParams() : member_params;
    for (auto int_param : vec->int_params) {
      stream << int_param->name_str() << '\t' << (int32_t)(*int_param) << '\t'
             << int_param->info_str() << '\n';
    }
    for (auto bool_param : vec->bool_params) {
      stream << bool_param->name_str() << '\t' << bool(*bool_param) << '\t'
             << bool_param->info_str() << '\n';
    }
    for (auto string_param : vec->string_params) {
      stream << string_param->name_str() << '\t' << string_param->c_str() << '\t'
             << string_param->info_str() << '\n';
    }
    for (auto double_param : vec->double_params) {
      stream << double_param->name_str() << '\t' << (double)(*double_param) << '\t'
             << double_param->info_str() << '\n';
    }
  }
  fprintf(fp, "%s", stream.str().c_str());
}

// Resets all parameters back to default values;
void ParamUtils::ResetToDefaults(ParamsVectors *member_params) {
  int num_iterations = (member_params == nullptr) ? 1 : 2;
  for (int v = 0; v < num_iterations; ++v) {
    ParamsVectors *vec = (v == 0) ? GlobalParams() : member_params;
    for (auto &param : vec->int_params) {
      param->ResetToDefault();
    }
    for (auto &param : vec->bool_params) {
      param->ResetToDefault();
    }
    for (auto &param : vec->string_params) {
      param->ResetToDefault();
    }
    for (auto &param : vec->double_params) {
      param->ResetToDefault();
    }
  }
}

} // namespace tesseract
