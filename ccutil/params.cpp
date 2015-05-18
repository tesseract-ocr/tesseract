/**********************************************************************
 * File:        params.cpp
 * Description: Initialization and setting of Tesseract parameters.
 * Author:      Ray Smith
 * Created:     Fri Feb 22 16:22:34 GMT 1991
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

#include          <stdio.h>
#include          <string.h>
#include          <stdlib.h>

#include          "genericvector.h"
#include          "scanutils.h"
#include          "tprintf.h"
#include          "params.h"

#define PLUS          '+'        //flag states
#define MINUS         '-'
#define EQUAL         '='

tesseract::ParamsVectors *GlobalParams() {
  static tesseract::ParamsVectors *global_params =
    new tesseract::ParamsVectors();
  return global_params;
}

namespace tesseract {

bool ParamUtils::ReadParamsFile(const char *file,
                                SetParamConstraint constraint,
                                ParamsVectors *member_params) {
  inT16 nameoffset;              // offset for real name
  FILE *fp;                      // file pointer
                                 // iterators

  if (*file == PLUS) {
    nameoffset = 1;
  } else if (*file == MINUS) {
    nameoffset = 1;
  } else {
    nameoffset = 0;
  }

  fp = fopen(file + nameoffset, "rb");
  if (fp == NULL) {
    tprintf("read_params_file: Can't open %s\n", file + nameoffset);
    return true;
  }
  const bool anyerr = ReadParamsFromFp(fp, -1, constraint, member_params);
  fclose(fp);
  return anyerr;
}

bool ParamUtils::ReadParamsFromFp(FILE *fp, inT64 end_offset,
                                  SetParamConstraint constraint,
                                  ParamsVectors *member_params) {
  char line[MAX_PATH];           // input line
  bool anyerr = false;           // true if any error
  bool foundit;                  // found parameter
  char *valptr;                  // value field

  while ((end_offset < 0 || ftell(fp) < end_offset) &&
         fgets(line, MAX_PATH, fp)) {
    if (line[0] != '\n' && line[0] != '#') {
      chomp_string(line);  // remove newline
      for (valptr = line; *valptr && *valptr != ' ' && *valptr != '\t';
        valptr++);
      if (*valptr) {             // found blank
        *valptr = '\0';          // make name a string
        do
          valptr++;              // find end of blanks
        while (*valptr == ' ' || *valptr == '\t');
      }
      foundit = SetParam(line, valptr, constraint, member_params);

      if (!foundit) {
        anyerr = true;         // had an error
        tprintf("read_params_file: parameter not found: %s\n", line);
        exit(1);
      }
    }
  }
  return anyerr;
}

bool ParamUtils::SetParam(const char *name, const char* value,
                          SetParamConstraint constraint,
                          ParamsVectors *member_params) {
  // Look for the parameter among string parameters.
  StringParam *sp = FindParam<StringParam>(name, GlobalParams()->string_params,
                                           member_params->string_params);
  if (sp != NULL && sp->constraint_ok(constraint)) sp->set_value(value);
  if (*value == '\0') return (sp != NULL);

  // Look for the parameter among int parameters.
  int intval;
  IntParam *ip = FindParam<IntParam>(name, GlobalParams()->int_params,
                                     member_params->int_params);
  if (ip && ip->constraint_ok(constraint) &&
      sscanf(value, INT32FORMAT, &intval) == 1) ip->set_value(intval);

  // Look for the parameter among bool parameters.
  BoolParam *bp = FindParam<BoolParam>(name, GlobalParams()->bool_params,
                                       member_params->bool_params);
  if (bp != NULL && bp->constraint_ok(constraint)) {
    if (*value == 'T' || *value == 't' ||
        *value == 'Y' || *value == 'y' || *value == '1') {
      bp->set_value(true);
    } else if (*value == 'F' || *value == 'f' ||
                *value == 'N' || *value == 'n' || *value == '0') {
      bp->set_value(false);
    }
  }

  // Look for the parameter among double parameters.
  double doubleval;
  DoubleParam *dp = FindParam<DoubleParam>(name, GlobalParams()->double_params,
                                           member_params->double_params);
  if (dp != NULL && dp->constraint_ok(constraint)) {
#ifdef EMBEDDED
      doubleval = strtofloat(value);
#else
      if (sscanf(value, "%lf", &doubleval) == 1)
#endif
      dp->set_value(doubleval);
  }
  return (sp || ip || bp || dp);
}

bool ParamUtils::GetParamAsString(const char *name,
                                  const ParamsVectors* member_params,
                                  STRING *value) {
  // Look for the parameter among string parameters.
  StringParam *sp = FindParam<StringParam>(name, GlobalParams()->string_params,
                                           member_params->string_params);
  if (sp) {
    *value = sp->string();
    return true;
  }
  // Look for the parameter among int parameters.
  IntParam *ip = FindParam<IntParam>(name, GlobalParams()->int_params,
                                     member_params->int_params);
  if (ip) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%d", inT32(*ip));
    *value = buf;
    return true;
  }
  // Look for the parameter among bool parameters.
  BoolParam *bp = FindParam<BoolParam>(name, GlobalParams()->bool_params,
                                       member_params->bool_params);
  if (bp != NULL) {
    *value = BOOL8(*bp) ? "1": "0";
    return true;
  }
  // Look for the parameter among double parameters.
  DoubleParam *dp = FindParam<DoubleParam>(name, GlobalParams()->double_params,
                                           member_params->double_params);
  if (dp != NULL) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%g", double(*dp));
    *value = buf;
    return true;
  }
  return false;
}

void ParamUtils::PrintParams(FILE *fp, const ParamsVectors *member_params) {
  int v, i;
  int num_iterations = (member_params == NULL) ? 1 : 2;
  for (v = 0; v < num_iterations; ++v) {
    const ParamsVectors *vec = (v == 0) ? GlobalParams() : member_params;
    for (i = 0; i < vec->int_params.size(); ++i) {
      fprintf(fp, "%s\t%d\t%s\n", vec->int_params[i]->name_str(),
              (inT32)(*vec->int_params[i]), vec->int_params[i]->info_str());
    }
    for (i = 0; i < vec->bool_params.size(); ++i) {
      fprintf(fp, "%s\t%d\t%s\n", vec->bool_params[i]->name_str(),
              (BOOL8)(*vec->bool_params[i]), vec->bool_params[i]->info_str());
    }
    for (int i = 0; i < vec->string_params.size(); ++i) {
      fprintf(fp, "%s\t%s\t%s\n", vec->string_params[i]->name_str(),
              vec->string_params[i]->string(), vec->string_params[i]->info_str());
    }
    for (int i = 0; i < vec->double_params.size(); ++i) {
      fprintf(fp, "%s\t%g\t%s\n", vec->double_params[i]->name_str(),
              (double)(*vec->double_params[i]), vec->double_params[i]->info_str());
    }
  }
}

// Resets all parameters back to default values;
void ParamUtils::ResetToDefaults(ParamsVectors* member_params) {
  int v, i;
  int num_iterations = (member_params == NULL) ? 1 : 2;
  for (v = 0; v < num_iterations; ++v) {
    ParamsVectors *vec = (v == 0) ? GlobalParams() : member_params;
    for (i = 0; i < vec->int_params.size(); ++i) {
      vec->int_params[i]->ResetToDefault();
    }
    for (i = 0; i < vec->bool_params.size(); ++i) {
      vec->bool_params[i]->ResetToDefault();
    }
    for (int i = 0; i < vec->string_params.size(); ++i) {
      vec->string_params[i]->ResetToDefault();
    }
    for (int i = 0; i < vec->double_params.size(); ++i) {
      vec->double_params[i]->ResetToDefault();
    }
  }
}

}  // namespace tesseract
