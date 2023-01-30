/**********************************************************************
 * File:        params.h
 * Description: Class definitions of the *_VAR classes for tunable constants.
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

#ifndef PARAMS_H
#define PARAMS_H

#include <tesseract/export.h> // for TESS_API

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace tesseract {

class IntParam;
class BoolParam;
class StringParam;
class DoubleParam;
class TFile;

// Enum for constraints on what kind of params should be set by SetParam().
enum SetParamConstraint {
  SET_PARAM_CONSTRAINT_NONE,
  SET_PARAM_CONSTRAINT_DEBUG_ONLY,
  SET_PARAM_CONSTRAINT_NON_DEBUG_ONLY,
  SET_PARAM_CONSTRAINT_NON_INIT_ONLY,
};

struct ParamsVectors {
  std::vector<IntParam *> int_params;
  std::vector<BoolParam *> bool_params;
  std::vector<StringParam *> string_params;
  std::vector<DoubleParam *> double_params;
};

// Utility functions for working with Tesseract parameters.
class TESS_API ParamUtils {
public:
  // Reads a file of parameter definitions and set/modify the values therein.
  // If the filename begins with a + or -, the BoolVariables will be
  // ORed or ANDed with any current values.
  // Blank lines and lines beginning # are ignored.
  // Values may have any whitespace after the name and are the rest of line.
  static bool ReadParamsFile(const char *file, // filename to read
                             SetParamConstraint constraint, ParamsVectors *member_params);

  // Read parameters from the given file pointer.
  static bool ReadParamsFromFp(SetParamConstraint constraint, TFile *fp,
                               ParamsVectors *member_params);

  // Set a parameters to have the given value.
  static bool SetParam(const char *name, const char *value, SetParamConstraint constraint,
                       ParamsVectors *member_params);

  // Returns the pointer to the parameter with the given name (of the
  // appropriate type) if it was found in the vector obtained from
  // GlobalParams() or in the given member_params.
  template <class T>
  static T *FindParam(const char *name, const std::vector<T *> &global_vec,
                      const std::vector<T *> &member_vec) {
    for (auto *param : global_vec) {
      if (strcmp(param->name_str(), name) == 0) {
        return param;
      }
    }
    for (auto *param : member_vec) {
      if (strcmp(param->name_str(), name) == 0) {
        return param;
      }
    }
    return nullptr;
  }
  // Removes the given pointer to the param from the given vector.
  template <class T>
  static void RemoveParam(T *param_ptr, std::vector<T *> *vec) {
    for (auto it = vec->begin(); it != vec->end(); ++it) {
      if (*it == param_ptr) {
        vec->erase(it);
        break;
      }
    }
  }
  // Fetches the value of the named param as a string. Returns false if not
  // found.
  static bool GetParamAsString(const char *name, const ParamsVectors *member_params,
                               std::string *value);

  // Print parameters to the given file.
  static void PrintParams(FILE *fp, const ParamsVectors *member_params);

  // Resets all parameters back to default values;
  static void ResetToDefaults(ParamsVectors *member_params);
};

// Definition of various parameter types.
class Param {
public:
  ~Param() = default;

  const char *name_str() const {
    return name_;
  }
  const char *info_str() const {
    return info_;
  }
  bool is_init() const {
    return init_;
  }
  bool is_debug() const {
    return debug_;
  }
  bool constraint_ok(SetParamConstraint constraint) const {
    return (constraint == SET_PARAM_CONSTRAINT_NONE ||
            (constraint == SET_PARAM_CONSTRAINT_DEBUG_ONLY && this->is_debug()) ||
            (constraint == SET_PARAM_CONSTRAINT_NON_DEBUG_ONLY && !this->is_debug()) ||
            (constraint == SET_PARAM_CONSTRAINT_NON_INIT_ONLY && !this->is_init()));
  }

protected:
  Param(const char *name, const char *comment, bool init)
      : name_(name), info_(comment), init_(init) {
    debug_ = (strstr(name, "debug") != nullptr) || (strstr(name, "display"));
  }

  const char *name_; // name of this parameter
  const char *info_; // for menus
  bool init_;        // needs to be set before init
  bool debug_;
};

class IntParam : public Param {
public:
  IntParam(int32_t value, const char *name, const char *comment, bool init, ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
    params_vec_ = &(vec->int_params);
    vec->int_params.push_back(this);
  }
  ~IntParam() {
    ParamUtils::RemoveParam<IntParam>(this, params_vec_);
  }
  operator int32_t() const {
    return value_;
  }
  void operator=(int32_t value) {
    value_ = value;
  }
  void set_value(int32_t value) {
    value_ = value;
  }
  void ResetToDefault() {
    value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->int_params) {
      if (strcmp(param->name_str(), name_) == 0) {
        // printf("overriding param %s=%d by =%d\n", name_, value_,
        // param);
        value_ = *param;
        break;
      }
    }
  }

private:
  int32_t value_;
  int32_t default_;
  // Pointer to the vector that contains this param (not owned by this class).
  std::vector<IntParam *> *params_vec_;
};

class BoolParam : public Param {
public:
  BoolParam(bool value, const char *name, const char *comment, bool init, ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
    params_vec_ = &(vec->bool_params);
    vec->bool_params.push_back(this);
  }
  ~BoolParam() {
    ParamUtils::RemoveParam<BoolParam>(this, params_vec_);
  }
  operator bool() const {
    return value_;
  }
  void operator=(bool value) {
    value_ = value;
  }
  void set_value(bool value) {
    value_ = value;
  }
  void ResetToDefault() {
    value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->bool_params) {
      if (strcmp(param->name_str(), name_) == 0) {
        // printf("overriding param %s=%s by =%s\n", name_, value_ ? "true" :
        // "false", *param ? "true" : "false");
        value_ = *param;
        break;
      }
    }
  }

private:
  bool value_;
  bool default_;
  // Pointer to the vector that contains this param (not owned by this class).
  std::vector<BoolParam *> *params_vec_;
};

class StringParam : public Param {
public:
  StringParam(const char *value, const char *name, const char *comment, bool init,
              ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
    params_vec_ = &(vec->string_params);
    vec->string_params.push_back(this);
  }
  ~StringParam() {
    ParamUtils::RemoveParam<StringParam>(this, params_vec_);
  }
  operator std::string &() {
    return value_;
  }
  const char *c_str() const {
    return value_.c_str();
  }
  bool contains(char c) const {
    return value_.find(c) != std::string::npos;
  }
  bool empty() const {
    return value_.empty();
  }
  bool operator==(const std::string &other) const {
    return value_ == other;
  }
  void operator=(const std::string &value) {
    value_ = value;
  }
  void set_value(const std::string &value) {
    value_ = value;
  }
  void ResetToDefault() {
    value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->string_params) {
      if (strcmp(param->name_str(), name_) == 0) {
        // printf("overriding param %s=%s by =%s\n", name_, value_,
        // param->c_str());
        value_ = *param;
        break;
      }
    }
  }

private:
  std::string value_;
  std::string default_;
  // Pointer to the vector that contains this param (not owned by this class).
  std::vector<StringParam *> *params_vec_;
};

class DoubleParam : public Param {
public:
  DoubleParam(double value, const char *name, const char *comment, bool init, ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
    params_vec_ = &(vec->double_params);
    vec->double_params.push_back(this);
  }
  ~DoubleParam() {
    ParamUtils::RemoveParam<DoubleParam>(this, params_vec_);
  }
  operator double() const {
    return value_;
  }
  void operator=(double value) {
    value_ = value;
  }
  void set_value(double value) {
    value_ = value;
  }
  void ResetToDefault() {
    value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->double_params) {
      if (strcmp(param->name_str(), name_) == 0) {
        // printf("overriding param %s=%f by =%f\n", name_, value_,
        // *param);
        value_ = *param;
        break;
      }
    }
  }

private:
  double value_;
  double default_;
  // Pointer to the vector that contains this param (not owned by this class).
  std::vector<DoubleParam *> *params_vec_;
};

// Global parameter lists.
//
// To avoid the problem of undetermined order of static initialization
// global_params are accessed through the GlobalParams function that
// initializes the static pointer to global_params only on the first time
// GlobalParams() is called.
//
// TODO(daria): remove GlobalParams() when all global Tesseract
// parameters are converted to members.
TESS_API
ParamsVectors *GlobalParams();

/*************************************************************************
 * Note on defining parameters.
 *
 * The values of the parameters defined with *_INIT_* macros are guaranteed
 * to be loaded from config files before Tesseract initialization is done
 * (there is no such guarantee for parameters defined with the other macros).
 *************************************************************************/

#define INT_VAR_H(name) ::tesseract::IntParam name

#define BOOL_VAR_H(name) ::tesseract::BoolParam name

#define STRING_VAR_H(name) ::tesseract::StringParam name

#define double_VAR_H(name) ::tesseract::DoubleParam name

#define INT_VAR(name, val, comment) \
  ::tesseract::IntParam name(val, #name, comment, false, ::tesseract::GlobalParams())

#define BOOL_VAR(name, val, comment) \
  ::tesseract::BoolParam name(val, #name, comment, false, ::tesseract::GlobalParams())

#define STRING_VAR(name, val, comment) \
  ::tesseract::StringParam name(val, #name, comment, false, ::tesseract::GlobalParams())

#define double_VAR(name, val, comment) \
  ::tesseract::DoubleParam name(val, #name, comment, false, ::tesseract::GlobalParams())

#define INT_MEMBER(name, val, comment, vec) name(val, #name, comment, false, vec)

#define BOOL_MEMBER(name, val, comment, vec) name(val, #name, comment, false, vec)

#define STRING_MEMBER(name, val, comment, vec) name(val, #name, comment, false, vec)

#define double_MEMBER(name, val, comment, vec) name(val, #name, comment, false, vec)

#define INT_INIT_MEMBER(name, val, comment, vec) name(val, #name, comment, true, vec)

#define BOOL_INIT_MEMBER(name, val, comment, vec) name(val, #name, comment, true, vec)

#define STRING_INIT_MEMBER(name, val, comment, vec) name(val, #name, comment, true, vec)

#define double_INIT_MEMBER(name, val, comment, vec) name(val, #name, comment, true, vec)

} // namespace tesseract

#endif
