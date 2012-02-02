/**********************************************************************
 * File:        params.h
 * Description: Class definitions of the *_VAR classes for tunable constants.
 * Author:      Ray Smith
 * Created:     Fri Feb 22 11:26:25 GMT 1991
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

#ifndef           PARAMS_H
#define           PARAMS_H

#include          <stdio.h>

#include          "genericvector.h"
#include          "strngs.h"

namespace tesseract {

class IntParam;
class BoolParam;
class StringParam;
class DoubleParam;

// Enum for constraints on what kind of params should be set by SetParam().
enum SetParamConstraint {
  SET_PARAM_CONSTRAINT_NONE,
  SET_PARAM_CONSTRAINT_DEBUG_ONLY,
  SET_PARAM_CONSTRAINT_NON_DEBUG_ONLY,
  SET_PARAM_CONSTRAINT_NON_INIT_ONLY,
};

struct ParamsVectors {
  GenericVector<IntParam *> int_params;
  GenericVector<BoolParam *> bool_params;
  GenericVector<StringParam *> string_params;
  GenericVector<DoubleParam *> double_params;
};

// Utility functions for working with Tesseract parameters.
class ParamUtils {
 public:
  // Reads a file of parameter definitions and set/modify the values therein.
  // If the filename begins with a + or -, the BoolVariables will be
  // ORed or ANDed with any current values.
  // Blank lines and lines beginning # are ignored.
  // Values may have any whitespace after the name and are the rest of line.
  static bool ReadParamsFile(
      const char *file,   // filename to read
      SetParamConstraint constraint,
      ParamsVectors *member_params);

  // Read parameters from the given file pointer (stop at end_offset).
  static bool ReadParamsFromFp(FILE *fp, inT64 end_offset,
                               SetParamConstraint constraint,
                               ParamsVectors *member_params);

  // Set a parameters to have the given value.
  static bool SetParam(const char *name, const char* value,
                       SetParamConstraint constraint,
                       ParamsVectors *member_params);

  // Returns the pointer to the parameter with the given name (of the
  // appropriate type) if it was found in the vector obtained from
  // GlobalParams() or in the given member_params.
  template<class T>
  static T *FindParam(const char *name,
                      const GenericVector<T *> &global_vec,
                      const GenericVector<T *> &member_vec) {
    int i;
    for (i = 0; i < global_vec.size(); ++i) {
      if (strcmp(global_vec[i]->name_str(), name) == 0) return global_vec[i];
    }
    for (i = 0; i < member_vec.size(); ++i) {
      if (strcmp(member_vec[i]->name_str(), name) == 0) return member_vec[i];
    }
    return NULL;
  }
  // Removes the given pointer to the param from the given vector.
  template<class T>
  static void RemoveParam(T *param_ptr, GenericVector<T *> *vec) {
    for (int i = 0; i < vec->size(); ++i) {
      if ((*vec)[i] == param_ptr) {
        vec->remove(i);
        return;
      }
    }
  }
  // Fetches the value of the named param as a STRING. Returns false if not
  // found.
  static bool GetParamAsString(const char *name,
                               const ParamsVectors* member_params,
                               STRING *value);

  // Print parameters to the given file.
  static void PrintParams(FILE *fp, const ParamsVectors *member_params);
};

// Definition of various parameter types.
class Param {
 public:
  ~Param() {}

  const char *name_str() const { return name_; }
  const char *info_str() const { return info_; }
  bool is_init() const { return init_; }
  bool is_debug() const { return debug_; }
  bool constraint_ok(SetParamConstraint constraint) const {
    return (constraint == SET_PARAM_CONSTRAINT_NONE ||
            (constraint == SET_PARAM_CONSTRAINT_DEBUG_ONLY &&
             this->is_debug()) ||
            (constraint == SET_PARAM_CONSTRAINT_NON_DEBUG_ONLY &&
             !this->is_debug()) ||
            (constraint == SET_PARAM_CONSTRAINT_NON_INIT_ONLY &&
             !this->is_init()));
  }

 protected:
  Param(const char *name, const char *comment, bool init) :
    name_(name), info_(comment), init_(init) {
    debug_ = (strstr(name, "debug") != NULL) || (strstr(name, "display"));
  }

  const char *name_;      // name of this parameter
  const char *info_;      // for menus
  bool init_;             // needs to be set before init
  bool debug_;
};

class IntParam : public Param {
  public:
   IntParam(inT32 value, const char *name, const char *comment, bool init,
            ParamsVectors *vec) : Param(name, comment, init) {
    value_ = value;
    params_vec_ = &(vec->int_params);
    vec->int_params.push_back(this);
  }
  ~IntParam() { ParamUtils::RemoveParam<IntParam>(this, params_vec_); }
  operator inT32() const { return value_; }
  void set_value(inT32 value) { value_ = value; }

 private:
  inT32 value_;
  // Pointer to the vector that contains this param (not owened by this class).
  GenericVector<IntParam *> *params_vec_;
};

class BoolParam : public Param {
 public:
  BoolParam(bool value, const char *name, const char *comment, bool init,
            ParamsVectors *vec) : Param(name, comment, init) {
    value_ = value;
    params_vec_ = &(vec->bool_params);
    vec->bool_params.push_back(this);
  }
  ~BoolParam() { ParamUtils::RemoveParam<BoolParam>(this, params_vec_); }
  operator BOOL8() const { return value_; }
  void set_value(BOOL8 value) { value_ = value; }

 private:
  BOOL8 value_;
  // Pointer to the vector that contains this param (not owned by this class).
  GenericVector<BoolParam *> *params_vec_;
};

class StringParam : public Param {
 public:
  StringParam(const char *value, const char *name,
              const char *comment, bool init,
              ParamsVectors *vec) : Param(name, comment, init) {
    value_ = value;
    params_vec_ = &(vec->string_params);
    vec->string_params.push_back(this);
  }
  ~StringParam() { ParamUtils::RemoveParam<StringParam>(this, params_vec_); }
  operator STRING &() { return value_; }
  const char *string() const { return value_.string(); }
  bool empty() { return value_.length() <= 0; }
  void set_value(const STRING &value) { value_ = value; }

 private:
  STRING value_;
  // Pointer to the vector that contains this param (not owened by this class).
  GenericVector<StringParam *> *params_vec_;
};

class DoubleParam : public Param {
 public:
  DoubleParam(double value, const char *name, const char *comment,
              bool init, ParamsVectors *vec) : Param(name, comment, init) {
    value_ = value;
    params_vec_ = &(vec->double_params);
    vec->double_params.push_back(this);
  }
  ~DoubleParam() { ParamUtils::RemoveParam<DoubleParam>(this, params_vec_); }
  operator double() const { return value_; }
  void set_value(double value) { value_ = value; }

 private:
  double value_;
  // Pointer to the vector that contains this param (not owned by this class).
  GenericVector<DoubleParam *> *params_vec_;
};

}  // namespace tesseract

// Global parameter lists.
//
// To avoid the problem of undetermined order of static initialization
// global_params are accessed through the GlobalParams function that
// initializes the static pointer to global_params only on the first
// first time GlobalParams() is called.
//
// TODO(daria): remove GlobalParams() when all global Tesseract
// parameters are converted to members.
tesseract::ParamsVectors *GlobalParams();

/*************************************************************************
 * Note on defining parameters.
 *
 * The values of the parameters defined with *_INIT_* macros are guaranteed
 * to be loaded from config files before Tesseract initialization is done
 * (there is no such guarantee for parameters defined with the other macros).
 *************************************************************************/

#define INT_VAR_H(name,val,comment)\
  tesseract::IntParam      name

#define BOOL_VAR_H(name,val,comment)\
  tesseract::BoolParam     name

#define STRING_VAR_H(name,val,comment)\
  tesseract::StringParam     name

#define double_VAR_H(name,val,comment)\
  tesseract::DoubleParam     name

#define INT_VAR(name,val,comment)\
  tesseract::IntParam      name(val,#name,comment,false,GlobalParams())

#define BOOL_VAR(name,val,comment)\
  tesseract::BoolParam     name(val,#name,comment,false,GlobalParams())

#define STRING_VAR(name,val,comment)\
  tesseract::StringParam     name(val,#name,comment,false,GlobalParams())

#define double_VAR(name,val,comment)\
  tesseract::DoubleParam     name(val,#name,comment,false,GlobalParams())

#define INT_INIT_VAR(name,val,comment)\
  tesseract::IntParam      name(val,#name,comment,true,GlobalParams())

#define BOOL_INIT_VAR(name,val,comment)\
  tesseract::BoolParam     name(val,#name,comment,true,GlobalParams())

#define STRING_INIT_VAR(name,val,comment)\
  tesseract::StringParam     name(val,#name,comment,true,GlobalParams())

#define double_INIT_VAR(name,val,comment)\
  tesseract::DoubleParam     name(val,#name,comment,true,GlobalParams())

#define INT_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, false, vec)

#define BOOL_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, false, vec)

#define STRING_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, false, vec)

#define double_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, false, vec)

#define INT_INIT_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, true, vec)

#define BOOL_INIT_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, true, vec)

#define STRING_INIT_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, true, vec)

#define double_INIT_MEMBER(name, val, comment, vec)\
  name(val, #name, comment, true, vec)

#endif
