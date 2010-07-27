///////////////////////////////////////////////////////////////////////
// File:        svutil.h
// Description: ScrollView Utilities
// Author:      Joern Wanke
// Created:     Thu Nov 29 2007
//
// (C) Copyright 2007, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////
//
// SVUtil contains the SVSync, SVSemaphore, SVMutex and SVNetwork
// classes, which are used for thread/process creation & synchronization
// and network connection.

#ifndef TESSERACT_VIEWER_SVUTIL_H__
#define TESSERACT_VIEWER_SVUTIL_H__

#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf
#if (_MSC_VER <= 1400)
#define vsnprintf _vsnprintf
#endif
#pragma warning(disable:4786)
#else
#include <pthread.h>
#include <semaphore.h>
#endif

#include <string>

#ifndef MAX
#define MAX(a, b)  ((a > b) ? a : b)
#endif

#ifndef MIN
#define MIN(a, b)  ((a < b) ? a : b)
#endif

/// The SVSync class provides functionality for Thread & Process Creation
class SVSync {
 public:
  /// Create new thread.
  static void StartThread(void *(*func)(void*), void* arg);
  /// Signals a thread to exit.
  static void ExitThread();
  /// Starts a new process.
  static void StartProcess(const char* executable, const char* args);
};

/// A semaphore class which encapsulates the main signalling
/// and wait abilities of semaphores for windows and unix.
class SVSemaphore {
 public:
  /// Sets up a semaphore.
  SVSemaphore();
  /// Signal a semaphore.
  void Signal();
  /// Wait on a semaphore.
  void Wait();
 private:
#ifdef WIN32
  HANDLE semaphore_;
#else
  sem_t semaphore_;
#endif
};

/// A mutex which encapsulates the main locking and unlocking
/// abilites of mutexes for windows and unix.
class SVMutex {
 public:
  /// Sets up a new mutex.
  SVMutex();
  /// Locks on a mutex.
  void Lock();
  /// Unlocks on a mutex.
  void Unlock();
 private:
#ifdef WIN32
  HANDLE mutex_;
#else
  pthread_mutex_t mutex_;
#endif
};

/// The SVNetwork class takes care of the remote connection for ScrollView
/// This means setting up and maintaining a remote connection, sending and
/// receiving messages and closing the connection.
/// It is designed to work on both Linux and Windows.
class SVNetwork {
 public:
  /// Set up a connection to hostname on port.
  SVNetwork(const char* hostname, int port);

  /// Destructor.
  ~SVNetwork();

  /// Put a message in the messagebuffer to the server and try to send it.
  void Send(const char* msg);

  /// Receive a message from the server.
  /// This will always return one line of char* (denoted by \n).
  char* Receive();

  /// Close the connection to the server.
  void Close();

  /// Flush the buffer.
  void Flush();

 private:
  /// The mutex for access to Send() and Flush().
  SVMutex* mutex_send_;
  /// The actual stream_ to the server.
  int stream_;
  /// Stores the last received message-chunk from the server.
  char* msg_buffer_in_;

  /// Stores the messages which are supposed to go out.
  std::string msg_buffer_out_;

  bool has_content; // Win32 (strtok)
  /// Where we are at in our msg_buffer_in_
  char* buffer_ptr_;  // Unix (strtok_r)
};

#endif  // TESSERACT_VIEWER_SVUTIL_H__
