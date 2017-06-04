// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains simple macros and functions to output log messages, and
// to perform assertions. Some of these can be enable/disabled at compile
// time using definitions.
//
#ifndef HELIUM_DEBUGGING_H__
#define HELIUM_DEBUGGING_H__

namespace helium {
  // Function to do an assertion. If the passed boolean expression is false,
  // This function will output an error message containing the assertion that
  // failed, and the file and line number where it appears in the code. The
  // program is than terminated with an exit code of -1.
  // You should never have to invoke this directly. Use the ASSERT macros
  // instead.
  void AssertionFunction(bool assertion, 
                         const char* assertion_string, 
                         unsigned line, 
                         const char* file);
  
  // Function to log a message. Includes the file name and line number where
  // the logging appears in the code.
  // You should never have to invoke this directly. Use the LOG_MSG macros
  // instead.
  void LogMessageFunction(const char* message, 
                          unsigned line, 
                          const char* file);
                          
  // Function to write an error message to stderr. Includes the file name and 
  // line number where the error was printed.
  // You should never have to invoke this directly. Use the ERROR macro
  // instead.
  void ErrorMessageFunction(const char* message, 
                            unsigned line, 
                            const char* file);
} // namespace

// ASSERT Macro
#define ASSERT(ASSERTION) \
  AssertionFunction(ASSERTION, #ASSERTION, __LINE__, __FILE__)

// ASSERT_IN_DEBUG Macro, which only calls AssertionFunction(...), if 
// DEBUG is defined. 
#ifdef DEBUG
#define ASSERT_IN_DEBUG_MODE(ASSERTION) ASSERT(ASSERTION)
#else
#define ASSERT_IN_DEBUG_MODE(ASSERTION)
#endif // DEBUG

// LOG_MSG Macro, which calls LogMessageFunction(...), if LOGGING_DISABLED is
// not defined
#ifndef LOGGING_DISABLED
#define LOG_MSG(MESSAGE) LogMessageFunction(MESSAGE, __LINE__, __FILE__)
#else
#define LOG_MSG(MESSAGE)
#endif // LOGGING_DISABLED

// ERROR prints the message to stderr.
#define ERROR(MESSAGE) ErrorMessageFunction(MESSAGE, __LINE__, __FILE__)

#endif  // TEXTDETECTOR_DEBUGGING_H__
