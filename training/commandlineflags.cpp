#include "commandlineflags.h"

#ifdef USE_STD_NAMESPACE

namespace tesseract {
bool IntFlagExists(const char* flag_name, inT32* value) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<IntParam*> empty;
  IntParam *p = ParamUtils::FindParam<IntParam>(
      full_flag_name.string(), GlobalParams()->int_params, empty);
  if (p == NULL) return false;
  *value = (inT32)(*p);
  return true;
}

bool DoubleFlagExists(const char* flag_name, double* value) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<DoubleParam*> empty;
  DoubleParam *p = ParamUtils::FindParam<DoubleParam>(
      full_flag_name.string(), GlobalParams()->double_params, empty);
  if (p == NULL) return false;
  *value = static_cast<double>(*p);
  return true;
}

bool BoolFlagExists(const char* flag_name, bool* value) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<BoolParam*> empty;
  BoolParam *p = ParamUtils::FindParam<BoolParam>(
      full_flag_name.string(), GlobalParams()->bool_params, empty);
  if (p == NULL) return false;
  *value = (BOOL8)(*p);
  return true;
}

bool StringFlagExists(const char* flag_name, const char** value) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<StringParam*> empty;
  StringParam *p = ParamUtils::FindParam<StringParam>(
      full_flag_name.string(), GlobalParams()->string_params, empty);
  *value = (p != NULL) ? p->string() : NULL;
  return p != NULL;
}


void SetIntFlagValue(const char* flag_name, const inT32 new_val) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<IntParam*> empty;
  IntParam *p = ParamUtils::FindParam<IntParam>(
      full_flag_name.string(), GlobalParams()->int_params, empty);
  ASSERT_HOST(p != NULL);
  p->set_value(new_val);
}

void SetDoubleFlagValue(const char* flag_name, const double new_val) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<DoubleParam*> empty;
  DoubleParam *p = ParamUtils::FindParam<DoubleParam>(
      full_flag_name.string(), GlobalParams()->double_params, empty);
  ASSERT_HOST(p != NULL);
  p->set_value(new_val);
}

void SetBoolFlagValue(const char* flag_name, const bool new_val) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<BoolParam*> empty;
  BoolParam *p = ParamUtils::FindParam<BoolParam>(
      full_flag_name.string(), GlobalParams()->bool_params, empty);
  ASSERT_HOST(p != NULL);
  p->set_value(new_val);
}

void SetStringFlagValue(const char* flag_name, const char* new_val) {
  STRING full_flag_name("FLAGS_");
  full_flag_name += flag_name;
  GenericVector<StringParam*> empty;
  StringParam *p = ParamUtils::FindParam<StringParam>(
      full_flag_name.string(), GlobalParams()->string_params, empty);
  ASSERT_HOST(p != NULL);
  p->set_value(STRING(new_val));
}

bool SafeAtoi(const char* str, int* val) {
  char *endptr = NULL;
  *val = strtol(str, &endptr, 10);
  return endptr != NULL && *endptr == '\0';
}

bool SafeAtod(const char* str, double* val) {
  char *endptr = NULL;
  *val = strtod(str, &endptr);
  return endptr != NULL && *endptr == '\0';
}

void PrintCommandLineFlags() {
  const char* kFlagNamePrefix = "FLAGS_";
  const int kFlagNamePrefixLen = strlen(kFlagNamePrefix);
  for (int i = 0; i < GlobalParams()->int_params.size(); ++i) {
    if (!strncmp(GlobalParams()->int_params[i]->name_str(),
                 kFlagNamePrefix, kFlagNamePrefixLen)) {
      tprintf("  --%s  %s  (type:int default:%d)\n",
              GlobalParams()->int_params[i]->name_str() + kFlagNamePrefixLen,
              GlobalParams()->int_params[i]->info_str(),
              inT32(*(GlobalParams()->int_params[i])));
    }
  }
  for (int i = 0; i < GlobalParams()->double_params.size(); ++i) {
    if (!strncmp(GlobalParams()->double_params[i]->name_str(),
                 kFlagNamePrefix, kFlagNamePrefixLen)) {
      tprintf("  --%s  %s  (type:double default:%g)\n",
              GlobalParams()->double_params[i]->name_str() + kFlagNamePrefixLen,
              GlobalParams()->double_params[i]->info_str(),
              static_cast<double>(*(GlobalParams()->double_params[i])));
    }
  }
  for (int i = 0; i < GlobalParams()->bool_params.size(); ++i) {
    if (!strncmp(GlobalParams()->bool_params[i]->name_str(),
                 kFlagNamePrefix, kFlagNamePrefixLen)) {
      tprintf("  --%s  %s  (type:bool default:%s)\n",
              GlobalParams()->bool_params[i]->name_str() + kFlagNamePrefixLen,
              GlobalParams()->bool_params[i]->info_str(),
              (BOOL8(*(GlobalParams()->bool_params[i])) ? "true" : "false"));
    }
  }
  for (int i = 0; i < GlobalParams()->string_params.size(); ++i) {
    if (!strncmp(GlobalParams()->string_params[i]->name_str(),
                 kFlagNamePrefix, kFlagNamePrefixLen)) {
      tprintf("  --%s  %s  (type:string default:%s)\n",
              GlobalParams()->string_params[i]->name_str() + kFlagNamePrefixLen,
              GlobalParams()->string_params[i]->info_str(),
              GlobalParams()->string_params[i]->string());
    }
  }
}


void ParseCommandLineFlags(const char* usage,
                           int* argc, char*** argv,
                           const bool remove_flags) {
  unsigned int i = 1;
  for (i = 1; i < *argc; ++i) {
    const char* current_arg = (*argv)[i];
    // If argument does not start with a hyphen then break.
    if (current_arg[0] != '-') {
      break;
    }
    // Position current_arg after startings hyphens. We treat a sequence of
    // consecutive hyphens of any length identically.
    while (*current_arg == '-') {
      ++current_arg;
    }
    // If this is asking for usage, print the help message and abort.
    if (!strcmp(current_arg, "help") ||
        !strcmp(current_arg, "helpshort")) {
      tprintf("USAGE: %s\n", usage);
      PrintCommandLineFlags();
      exit(0);
    }
    // Find the starting position of the value if it was specified in this
    // string.
    const char* equals_position = strchr(current_arg, '=');
    const char* rhs = NULL;
    if (equals_position != NULL) {
      rhs = equals_position + 1;
    }
    // Extract the flag name.
    STRING lhs;
    if (equals_position == NULL) {
      lhs = current_arg;
    } else {
      lhs.assign(current_arg, equals_position - current_arg);
    }
    if (!lhs.length()) {
      tprintf("ERROR: Bad argument: %s\n", (*argv)[i]);
      exit(1);
    }

    // Find the flag name in the list of global flags.
    // inT32 flag
    inT32 int_val;
    if (IntFlagExists(lhs.string(), &int_val)) {
      if (rhs != NULL) {
        if (!strlen(rhs)) {
          // Bad input of the format --int_flag=
          tprintf("ERROR: Bad argument: %s\n", (*argv)[i]);
          exit(1);
        }
        if (!SafeAtoi(rhs, &int_val)) {
          tprintf("ERROR: Could not parse int from %s in flag %s\n",
                  rhs, (*argv)[i]);
          exit(1);
        }
      } else {
        // We need to parse the next argument
        if (i + 1 >= *argc) {
          tprintf("ERROR: Could not find value argument for flag %s\n",
                  lhs.string());
          exit(1);
        } else {
          ++i;
          if (!SafeAtoi((*argv)[i], &int_val)) {
            tprintf("ERROR: Could not parse inT32 from %s\n", (*argv)[i]);
            exit(1);
          }
        }
      }
      SetIntFlagValue(lhs.string(), int_val);
      continue;
    }

    // double flag
    double double_val;
    if (DoubleFlagExists(lhs.string(), &double_val)) {
      if (rhs != NULL) {
        if (!strlen(rhs)) {
          // Bad input of the format --double_flag=
          tprintf("ERROR: Bad argument: %s\n", (*argv)[i]);
          exit(1);
        }
        if (!SafeAtod(rhs, &double_val)) {
          tprintf("ERROR: Could not parse double from %s in flag %s\n",
                  rhs, (*argv)[i]);
          exit(1);
        }
      } else {
        // We need to parse the next argument
        if (i + 1 >= *argc) {
          tprintf("ERROR: Could not find value argument for flag %s\n",
                  lhs.string());
          exit(1);
        } else {
          ++i;
          if (!SafeAtod((*argv)[i], &double_val)) {
            tprintf("ERROR: Could not parse double from %s\n", (*argv)[i]);
            exit(1);
          }
        }
      }
      SetDoubleFlagValue(lhs.string(), double_val);
      continue;
    }

    // Bool flag. Allow input forms --flag (equivalent to --flag=true),
    // --flag=false, --flag=true, --flag=0 and --flag=1
    bool bool_val;
    if (BoolFlagExists(lhs.string(), &bool_val)) {
      if (rhs == NULL) {
        // --flag form
        bool_val = true;
      } else {
        if (!strlen(rhs)) {
          // Bad input of the format --bool_flag=
          tprintf("ERROR: Bad argument: %s\n", (*argv)[i]);
          exit(1);
        }
        if (!strcmp(rhs, "false") || !strcmp(rhs, "0")) {
          bool_val = false;
        } else if (!strcmp(rhs, "true") || !strcmp(rhs, "1")) {
          bool_val = true;
        } else {
          tprintf("ERROR: Could not parse bool from flag %s\n", (*argv)[i]);
          exit(1);
        }
      }
      SetBoolFlagValue(lhs.string(), bool_val);
      continue;
    }

    // string flag
    const char* string_val;
    if (StringFlagExists(lhs.string(), &string_val)) {
      if (rhs != NULL) {
        string_val = rhs;
      } else {
        // Pick the next argument
        if (i + 1 >= *argc) {
          tprintf("ERROR: Could not find string value for flag %s\n",
                  lhs.string());
          exit(1);
        } else {
          string_val = (*argv)[++i];
        }
      }
      SetStringFlagValue(lhs.string(), string_val);
      continue;
    }

    // Flag was not found. Exit with an error message.
    tprintf("ERROR: Non-existent flag %s\n", (*argv)[i]);
    exit(1);
  }  // for each argv
  if (remove_flags) {
    (*argv)[i - 1] = (*argv)[0];
    (*argv) += (i - 1);
    (*argc) -= (i - 1);
  }
}
}  // namespace tesseract

#else

#include "base/init_google.h"

namespace tesseract {
void ParseCommandLineFlags(const char* usage,
                           int* argc, char*** argv,
                           const bool remove_flags) {
  InitGoogle(usage, argc, argv, remove_flags);
}
}  // namespace tesseract

#endif
