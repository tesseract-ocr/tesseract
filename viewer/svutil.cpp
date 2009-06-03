///////////////////////////////////////////////////////////////////////
// File:        svutil.cpp
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
// SVUtil contains the SVSync and SVNetwork classes, which are used for
// thread/process creation & synchronization and network connection.

#ifndef GRAPHICS_DISABLED

#include "svutil.h"

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#endif

#include <iostream>

const int kBufferSize = 65536;
const int kMaxMsgSize = 4096;

// Signals a thread to exit.
void SVSync::ExitThread() {
#ifdef WIN32
  //ExitThread(0);
#else
  pthread_exit(0);
#endif
}

// Starts a new process.
void SVSync::StartProcess(const char* executable, const char* args) {
#ifdef WIN32
  std::string proc;
  proc.append(executable);
  proc.append(" ");
  proc.append(args);
  std::cout << "Starting " << proc << std::endl;
  STARTUPINFO start_info;
  PROCESS_INFORMATION proc_info;
  GetStartupInfo(&start_info);
  if (!CreateProcess(NULL, const_cast<char*>(proc.c_str()), NULL, NULL, FALSE,
                CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL,
                &start_info, &proc_info))
    return;
#else
  int pid = fork();
  if (pid != 0) {   // The father process returns
  } else {
#ifdef __linux__
    // Make sure the java process terminates on exit, since its
    // broken socket detection seems to be useless.
    prctl(PR_SET_PDEATHSIG, 2, 0, 0, 0);
#endif
    char* mutable_args = strdup(args);
    int argc = 1;
    for (int i = 0; mutable_args[i]; ++i) {
      if (mutable_args[i] == ' ') {
        ++argc;
      }
    }
    char** argv = new char*[argc + 2];
    argv[0] = strdup(executable);
    argv[1] = mutable_args;
    argc = 2;
    bool inquote = false;
    for (int i = 0; mutable_args[i]; ++i) {
      if (!inquote && mutable_args[i] == ' ') {
        mutable_args[i] = '\0';
        argv[argc++] = mutable_args + i + 1;
      } else if (mutable_args[i] == '"') {
        inquote = !inquote;
        mutable_args[i] = ' ';
      }
    }
    argv[argc] = NULL;
    execvp(executable, argv);
  }
#endif
}

SVSemaphore::SVSemaphore() {
#ifdef WIN32
  semaphore_ = CreateSemaphore(0, 0, 10, 0);
#else
  sem_init(&semaphore_, 0, 0);
#endif
}

void SVSemaphore::Signal() {
#ifdef WIN32
  ReleaseSemaphore(semaphore_, 1, NULL);
#else
  sem_post(&semaphore_);
#endif
}

void SVSemaphore::Wait() {
#ifdef WIN32
  WaitForSingleObject(semaphore_, INFINITE);
#else
  sem_wait(&semaphore_);
#endif
}

SVMutex::SVMutex() {
#ifdef WIN32
  mutex_ = CreateMutex(0, FALSE, 0);
#else
  pthread_mutex_init(&mutex_, NULL);
#endif
}

void SVMutex::Lock() {
#ifdef WIN32
  WaitForSingleObject(mutex_, INFINITE);
#else
  pthread_mutex_lock(&mutex_);
#endif
}

void SVMutex::Unlock() {
#ifdef WIN32
  ReleaseMutex(mutex_);
#else
  pthread_mutex_unlock(&mutex_);
#endif
}

// Create new thread.

void SVSync::StartThread(void *(*func)(void*), void* arg) {
#ifdef WIN32
  LPTHREAD_START_ROUTINE f = (LPTHREAD_START_ROUTINE) func;
  DWORD threadid;
  HANDLE newthread = CreateThread(
  NULL,          // default security attributes
  0,             // use default stack size
  f,             // thread function
  arg,           // argument to thread function
  0,             // use default creation flags
  &threadid);    // returns the thread identifier
#else
  pthread_t helper;
  pthread_create(&helper, NULL, func, arg);
#endif
}

// Place a message in the message buffer (and flush it).
void SVNetwork::Send(const char* msg) {
  mutex_send_->Lock();
  msg_buffer_out_.append(msg);
  mutex_send_->Unlock();
}

// Send the whole buffer.
void SVNetwork::Flush() {
  mutex_send_->Lock();
  while (msg_buffer_out_.size() > 0) {
    int i = send(stream_, msg_buffer_out_.c_str(), msg_buffer_out_.length(), 0);
    msg_buffer_out_.erase(0, i);
  }
  mutex_send_->Unlock();
}

// Receive a message from the server.
// This will always return one line of char* (denoted by \n).
char* SVNetwork::Receive() {
  char* result = NULL;
#ifdef WIN32
  if (has_content) { result = strtok (NULL, "\n"); }
#else
  if (buffer_ptr_ != NULL) { result = strtok_r(NULL, "\n", &buffer_ptr_); }
#endif

  // This means there is something left in the buffer and we return it.
  if (result != NULL) { return result;
  // Otherwise, we read from the stream_.
  } else {
    buffer_ptr_ = NULL;
    has_content = false;

    // The timeout length is not really important since we are looping anyway
    // until a new message is delivered.
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    // Set the flags to return when the stream_ is ready to be read.
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(stream_, &readfds);

    int i = select(stream_+1, &readfds, NULL, NULL, &tv);

    // The stream_ died.
    if (i == 0) { return NULL; }

    // Read the message buffer.
    i = recv(stream_, msg_buffer_in_, kMaxMsgSize, 0);

    // Server quit (0) or error (-1).
    if (i <= 0) { return NULL; }
    msg_buffer_in_[i] = '\0';
    has_content = true;
#ifdef WIN32
    return strtok(msg_buffer_in_,"\n");
#else
    // Setup a new string tokenizer.
    return strtok_r(msg_buffer_in_, "\n", &buffer_ptr_);
#endif
  }
}

// Close the connection to the server.
void SVNetwork::Close() {
#ifdef WIN32
  closesocket(stream_);
#else
  close(stream_);
#endif
}

// Set up a connection to hostname on port.
SVNetwork::SVNetwork(const char* hostname, int port) {
  mutex_send_ = new SVMutex();
  struct sockaddr_in address;
  struct hostent *name;

  msg_buffer_in_ = new char[kMaxMsgSize + 1];
  msg_buffer_in_[0] = '\0';

  has_content = false;

  buffer_ptr_ = NULL;

// Get the host data depending on the OS.
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(1, 1), &wsaData);
  name = gethostbyname(hostname);
#elif defined(__linux__)
  struct hostent hp;
  int herr;
  char *buffer = new char[kBufferSize];
  gethostbyname_r(hostname, &hp, buffer, kBufferSize, &name, &herr);
  delete[] buffer;
#else
  name = gethostbyname(hostname);
#endif

  // Fill in the appropriate variables to be able to connect to the server.
  address.sin_family = name->h_addrtype;
  memcpy((char *) &address.sin_addr.s_addr,
         name->h_addr_list[0], name->h_length);
  address.sin_port = htons(port);

  stream_ = socket(AF_INET, SOCK_STREAM, 0);

  // If server is not there, we will start a new server as local child process.
  if (connect(stream_, (struct sockaddr *) &address, sizeof(address)) < 0) {
    const char* scrollview_path = getenv("SCROLLVIEW_PATH");
    if (scrollview_path == NULL) {
#ifdef SCROLLVIEW_PATH
#define _STR(a) #a
#define _XSTR(a) _STR(a)
      scrollview_path = _XSTR(SCROLLVIEW_PATH);
#undef _XSTR
#undef _STR
#else
      scrollview_path = ".";
#endif
    }
    // The following ugly ifdef is to enable the output of the java runtime
    // to be sent down a black hole on non-windows to ignore all the
    // exceptions in piccolo. Ideally piccolo would be debugged to make
    // this unnecessary.
    // Also the path has to be separated by ; on windows and : otherwise.
#ifdef WIN32
    const char* prog = "java -Xms512m -Xmx1024m";
    const char* cmd_template = "-Djava.library.path=%s -cp %s/ScrollView.jar;"
        "%s/piccolo-1.2.jar;%s/piccolox-1.2.jar"
        " com.google.scrollview.ScrollView";
#else
    const char* prog = "sh";
    const char* cmd_template = "-c \"trap 'kill %1' 0 1 2 ; java "
        "-Xms1024m -Xmx2048m -Djava.library.path=%s -cp %s/ScrollView.jar:"
        "%s/piccolo-1.2.jar:%s/piccolox-1.2.jar"
        " com.google.scrollview.ScrollView"
        " >/dev/null 2>&1 & wait\"";
#endif
    int cmdlen = strlen(cmd_template) + 4*strlen(scrollview_path) + 1;
    char* cmd = new char[cmdlen];
    snprintf(cmd, cmdlen, cmd_template, scrollview_path, scrollview_path,
             scrollview_path, scrollview_path);

    SVSync::StartProcess(prog, cmd);
    delete [] cmd;

    // Wait for server to show up.
    // Note: There is no exception handling in case the server never turns up.
    while (connect(stream_, (struct sockaddr *) &address,
                   sizeof(address)) < 0) {
      std::cout << "ScrollView: Waiting for server...\n";
#ifdef WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
    }
  }
}

SVNetwork::~SVNetwork() {
  delete[] msg_buffer_in_;
  delete mutex_send_;
}

#endif  // GRAPHICS_DISABLED
