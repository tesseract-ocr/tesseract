/**********************************************************************
 * File:        params.h
 * Description: Class definitions of the *_VAR classes for tunable constants.
 * Author:      Ray Smith
 * 
 * UTF8 detect helper statement: «bloody MSVC»
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

class ParamsVectors {
  std::vector<IntParam *> _int_params;
  std::vector<BoolParam *> _bool_params;
  std::vector<StringParam *> _string_params;
  std::vector<DoubleParam *> _double_params;

public:
	ParamsVectors() {
	}
	~ParamsVectors() {
	}

	std::vector<IntParam *> &int_params() {
		return _int_params;
	}
	std::vector<BoolParam *> &bool_params() {
		return _bool_params;
	}
	std::vector<StringParam *> &string_params() {
		return _string_params;
	}
	std::vector<DoubleParam *> &double_params() {
		return _double_params;
	}

	const std::vector<IntParam *> &int_params_c() const {
		return _int_params;
	}
	const std::vector<BoolParam *> &bool_params_c() const {
		return _bool_params;
	}
	const std::vector<StringParam *> &string_params_c() const {
		return _string_params;
	}
	const std::vector<DoubleParam *> &double_params_c() const {
		return _double_params;
	}
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

  // accept both - and _ in key names, e.g. user-specified 'debug-all' would match 'debug_all'
  // in the database.
  static inline bool CompareKeys(const char *db_key, const char *user_key)
  {
	  // if (0 == strcmp(db_key, user_key))
	  //   return true;

	  for (; *db_key && *user_key; db_key++, user_key++)
	  {
		  if (*db_key != *user_key)
		  {
			  if (*db_key == '_' && *user_key == '-')
				  continue;
			  return false;
		  }
	  }
	  return true;
  }

  // Returns the pointer to the parameter with the given name (of the
  // appropriate type) if it was found in the vector obtained from
  // GlobalParams() or in the given member_params.
  template <class T>
  static T *FindParam(const char *name, const std::vector<T *> &global_vec,
                      const std::vector<T *> &member_vec) {
    for (auto *param : global_vec) {
      if (CompareKeys(param->name_str(), name)) {
        return param;
      }
    }
    for (auto *param : member_vec) {
      if (CompareKeys(param->name_str(), name)) {
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

  // Report parameters' usage statistics, i.e. report which params have been
  // set, modified and read/checked until now during this run-time's lifetime.
  //
  // Use this method for run-time 'discovery' about which tesseract parameters
  // are actually *used* during your particular usage of the library, ergo
  // answering the question:
  // "Which of all those parameters are actually *relevant* to my use case today?"
  static void ReportParamsUsageStatistics(const ParamsVectors *member_params);

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

  typedef struct access_counts {
	  int reading;
	  int writing;
  } access_counts_t;

  access_counts_t access_counts() const {
	  return access_counts_;
  }

  virtual std::string formatted_value_str() const = 0;

protected:
  Param(const char *name, const char *comment, bool init)
      : name_(name), info_(comment), init_(init) {
    debug_ = (strstr(name, "debug") != nullptr) || (strstr(name, "display"));
	access_counts_ = {0,0};
  }

  const char *name_; // name of this parameter
  const char *info_; // for menus
  bool init_;        // needs to be set before init
  bool debug_;
  mutable access_counts_t access_counts_;
};

class IntParam : public Param {
public:
  IntParam(int32_t value, const char *name, const char *comment, bool init, ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
	access_counts_.writing++;
    params_vec_ = vec;
    vec->int_params().push_back(this);
  }
  ~IntParam() {
    ParamUtils::RemoveParam<IntParam>(this, &params_vec_->int_params());
  }
  operator int32_t() const {
	  access_counts_.reading++;
	  return value_;
  }
  void operator=(int32_t value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void set_value(int32_t value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void ResetToDefault() {
      access_counts_.writing++;
      value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->int_params_c()) {
      if (strcmp(param->name_str(), name_) == 0) {
        // tprintf("overriding param {}={} by ={}\n", name_, value_,
        // *param);
          access_counts_.writing++;
          value_ = *param;
        break;
      }
    }
  }

  virtual std::string formatted_value_str() const override {
      return std::to_string(value_);
  }

private:
  int32_t value_;
  int32_t default_;

  // Pointer to the vector that contains this param (not owned by this class). Used by the destructor.
  ParamsVectors *params_vec_;
};

class BoolParam : public Param {
public:
  BoolParam(bool value, const char *name, const char *comment, bool init, ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
	access_counts_.writing++;
	params_vec_ = vec;
    vec->bool_params().push_back(this);
  }
  ~BoolParam() {
    ParamUtils::RemoveParam<BoolParam>(this, &params_vec_->bool_params());
  }
  operator bool() const {
	  access_counts_.reading++;
	  return value_;
  }
  void operator=(bool value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void set_value(bool value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void ResetToDefault() {
      access_counts_.writing++;
      value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->bool_params_c()) {
      if (strcmp(param->name_str(), name_) == 0) {
        // tprintf("overriding param {}={} by ={}\n", name_, value_ ? "true" :
        // "false", *param ? "true" : "false");
          access_counts_.writing++;
          value_ = *param;
        break;
      }
    }
  }

  virtual std::string formatted_value_str() const override {
      return std::to_string(value_);
  }

private:
  bool value_;
  bool default_;

  // Pointer to the vector that contains this param (not owned by this class).
  ParamsVectors *params_vec_;
};

class StringParam : public Param {
public:
  StringParam(const char *value, const char *name, const char *comment, bool init,
              ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
	access_counts_.writing++;
	params_vec_ = vec;
    vec->string_params().push_back(this);
  }
  ~StringParam() {
    ParamUtils::RemoveParam<StringParam>(this, &params_vec_->string_params());
  }
  operator std::string &() {
	  access_counts_.reading++;
	  return value_;
  }
  const char *c_str() const {
	  access_counts_.reading++;
	  return value_.c_str();
  }
  bool contains(char c) const {
	  access_counts_.reading++;
	  return value_.find(c) != std::string::npos;
  }
  bool empty() const {
	  access_counts_.reading++;
	  return value_.empty();
  }
  bool operator==(const std::string &other) const {
	  access_counts_.reading++;
	  return value_ == other;
  }
  void operator=(const std::string &value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void set_value(const std::string &value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  const std::string &value() {
	  access_counts_.reading++;
	  return value_;
  }
  void ResetToDefault() {
      access_counts_.writing++;
      value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->string_params_c()) {
      if (strcmp(param->name_str(), name_) == 0) {
        // tprintf("overriding param {}={} by ={}\n", name_, value_,
        // param->c_str());
          access_counts_.writing++;
          value_ = *param;
        break;
      }
    }
  }

  virtual std::string formatted_value_str() const override {
      std::string rv = (const char *)u8"«";
      rv += value_;
      rv += (const char *)u8"»";
      return rv;
  }

private:
  std::string value_;
  std::string default_;

  // Pointer to the vector that contains this param (not owned by this class).
  ParamsVectors *params_vec_;
};

class DoubleParam : public Param {
public:
  DoubleParam(double value, const char *name, const char *comment, bool init, ParamsVectors *vec)
      : Param(name, comment, init) {
    value_ = value;
    default_ = value;
	access_counts_.writing++;
	params_vec_ = vec;
    vec->double_params().push_back(this);
  }
  ~DoubleParam() {
    ParamUtils::RemoveParam<DoubleParam>(this, &params_vec_->double_params());
  }
  operator double() const {
	  access_counts_.reading++;
	  return value_;
  }
  void operator=(double value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void set_value(double value) {
	  access_counts_.writing++;
	  value_ = value;
  }
  void ResetToDefault() {
      access_counts_.writing++;
      value_ = default_;
  }
  void ResetFrom(const ParamsVectors *vec) {
    for (auto *param : vec->double_params_c()) {
      if (strcmp(param->name_str(), name_) == 0) {
        // tprintf("overriding param {}={} by ={}\n", name_, value_,
        // *param);
          access_counts_.writing++;
          value_ = *param;
        break;
      }
    }
  }

  virtual std::string formatted_value_str() const override {
#if 0
      return std::to_string(value_);   // always outputs %.6f format style values
#else
      char sbuf[40];
      snprintf(sbuf, sizeof(sbuf), "%1.f", value_);
      return sbuf;
#endif
  }

private:
  double value_;
  double default_;

  // Pointer to the vector that contains this param (not owned by this class).
  ParamsVectors *params_vec_;
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

// ------------------------------------

extern BOOL_VAR_H(debug_all);
extern STRING_VAR_H(vars_report_file);
extern BOOL_VAR_H(report_all_variables);

} // namespace tesseract

#endif
