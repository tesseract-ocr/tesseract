///////////////////////////////////////////////////////////////////////
// File:        svutil.cpp
// Description: ScrollView Utilities
// Author:      Joern Wanke
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "svutil.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>        // for std::this_thread
#include <vector>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#  include <winsock2.h>  // for fd_set, send, ..
#  include <ws2tcpip.h>  // for addrinfo
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <csignal>
#include <sys/select.h>
#include <sys/socket.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <unistd.h>
#endif

#if defined(_WIN32) && !defined(__GNUC__)
#define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif /* _WIN32 && !__GNUC__ */

#ifndef GRAPHICS_DISABLED

const int kMaxMsgSize = 4096;

// Starts a new process.
void SVSync::StartProcess(const char* executable, const char* args) {
  std::string proc;
  proc.append(executable);
  proc.append(" ");
  proc.append(args);
  std::cout << "Starting " << proc << std::endl;
#ifdef _WIN32
  STARTUPINFO start_info;
  PROCESS_INFORMATION proc_info;
  GetStartupInfo(&start_info);
  if (!CreateProcess(nullptr, const_cast<char*>(proc.c_str()), nullptr, nullptr, FALSE,
                CREATE_NO_WINDOW | DETACHED_PROCESS, nullptr, nullptr,
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
    std::unique_ptr<char*[]> argv(new char*[argc + 2]);
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
    argv[argc] = nullptr;
    execvp(executable, argv.get());
    free(argv[0]);
    free(argv[1]);
  }
#endif
}

SVSemaphore::SVSemaphore() {
#ifdef _WIN32
  semaphore_ = CreateSemaphore(0, 0, 10, 0);
#elif defined(__APPLE__)
  char name[50];
  snprintf(name, sizeof(name), "%ld", random());
  sem_unlink(name);
  semaphore_ = sem_open(name, O_CREAT , S_IWUSR, 0);
  if (semaphore_ == SEM_FAILED) {
    perror("sem_open");
  }
#else
  sem_init(&semaphore_, 0, 0);
#endif
}

void SVSemaphore::Signal() {
#ifdef _WIN32
  ReleaseSemaphore(semaphore_, 1, nullptr);
#elif defined(__APPLE__)
  sem_post(semaphore_);
#else
  sem_post(&semaphore_);
#endif
}

void SVSemaphore::Wait() {
#ifdef _WIN32
  WaitForSingleObject(semaphore_, INFINITE);
#elif defined(__APPLE__)
  sem_wait(semaphore_);
#else
  sem_wait(&semaphore_);
#endif
}

// Place a message in the message buffer (and flush it).
void SVNetwork::Send(const char* msg) {
  std::lock_guard<std::mutex> guard(mutex_send_);
  msg_buffer_out_.append(msg);
}

// Send the whole buffer.
void SVNetwork::Flush() {
  std::lock_guard<std::mutex> guard(mutex_send_);
  while (!msg_buffer_out_.empty()) {
    int i = send(stream_, msg_buffer_out_.c_str(), msg_buffer_out_.length(), 0);
    msg_buffer_out_.erase(0, i);
  }
}

// Receive a message from the server.
// This will always return one line of char* (denoted by \n).
char* SVNetwork::Receive() {
  char* result = nullptr;
  if (buffer_ptr_ != nullptr) {
    result = strtok_r(nullptr, "\n", &buffer_ptr_);
  }

  // This means there is something left in the buffer and we return it.
  if (result != nullptr) { return result;
  // Otherwise, we read from the stream_.
  } else {
    buffer_ptr_ = nullptr;

    // The timeout length is not really important since we are looping anyway
    // until a new message is delivered.
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    // Set the flags to return when the stream_ is ready to be read.
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(stream_, &readfds);

    int i = select(stream_+1, &readfds, nullptr, nullptr, &tv);

    // The stream_ died.
    if (i == 0) { return nullptr; }

    // Read the message buffer.
    i = recv(stream_, msg_buffer_in_, kMaxMsgSize, 0);

    // Server quit (0) or error (-1).
    if (i <= 0) { return nullptr; }
    msg_buffer_in_[i] = '\0';
    // Setup a new string tokenizer.
    return strtok_r(msg_buffer_in_, "\n", &buffer_ptr_);
  }
}

// Close the connection to the server.
void SVNetwork::Close() {
#ifdef _WIN32
  closesocket(stream_);
#else
  close(stream_);
#endif
  // Mark stream_ as invalid.
  stream_ = -1;
}


// The program to invoke to start ScrollView
static const char* ScrollViewProg() {
#ifdef _WIN32
  const char* prog = "java -Xms512m -Xmx1024m";
#else
  const char* prog = "sh";
#endif
  return prog;
}


// The arguments to the program to invoke to start ScrollView
static std::string ScrollViewCommand(std::string scrollview_path) {
  // The following ugly ifdef is to enable the output of the java runtime
  // to be sent down a black hole on non-windows to ignore all the
  // exceptions in piccolo. Ideally piccolo would be debugged to make
  // this unnecessary.
  // Also the path has to be separated by ; on windows and : otherwise.
#ifdef _WIN32
  const char cmd_template[] = "-Djava.library.path=\"%s\" -jar \"%s/ScrollView.jar\"";

#else
  const char cmd_template[] =
      "-c \"trap 'kill %%1' 0 1 2 ; java "
      "-Xms1024m -Xmx2048m -jar %s/ScrollView.jar"
      " & wait\"";
#endif
  size_t cmdlen = sizeof(cmd_template) + 2 * scrollview_path.size() + 1;
  std::vector<char> cmd(cmdlen);
  const char* sv_path = scrollview_path.c_str();
#ifdef _WIN32
  snprintf(&cmd[0], cmdlen, cmd_template, sv_path, sv_path);
#else
  snprintf(&cmd[0], cmdlen, cmd_template, sv_path);
#endif
  std::string command(&cmd[0]);
  return command;
}

// Set up a connection to a ScrollView on hostname:port.
SVNetwork::SVNetwork(const char* hostname, int port) {
  msg_buffer_in_ = new char[kMaxMsgSize + 1];
  msg_buffer_in_[0] = '\0';

  buffer_ptr_ = nullptr;

  struct addrinfo *addr_info = nullptr;
  char port_str[40];
  snprintf(port_str, 40, "%d", port);
#ifdef _WIN32
  // Initialize Winsock
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    std::cerr << "WSAStartup failed: " << iResult << std::endl;
  }
#endif  // _WIN32

  if (getaddrinfo(hostname, port_str, nullptr, &addr_info) != 0) {
    std::cerr << "Error resolving name for ScrollView host "
              << std::string(hostname) << ":" << port << std::endl;
#ifdef _WIN32
    WSACleanup();
#endif  // _WIN32
  }

  stream_ = socket(addr_info->ai_family, addr_info->ai_socktype,
                   addr_info->ai_protocol);

  if (stream_ < 0) {
    std::cerr << "Failed to open socket" << std::endl;
  } else if (connect(stream_, addr_info->ai_addr, addr_info->ai_addrlen) < 0) {
    // If server is not there, we will start a new server as local child process.
    const char* scrollview_path = getenv("SCROLLVIEW_PATH");
    if (scrollview_path == nullptr) {
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
    const char *prog = ScrollViewProg();
    std::string command = ScrollViewCommand(scrollview_path);
    SVSync::StartProcess(prog, command.c_str());

    // Wait for server to show up.
    // Note: There is no exception handling in case the server never turns up.

    Close();
    for (;;) {
      stream_ = socket(addr_info->ai_family, addr_info->ai_socktype,
                       addr_info->ai_protocol);
      if (stream_ >= 0) {
        if (connect(stream_, addr_info->ai_addr, addr_info->ai_addrlen) == 0) {
          break;
        }

        Close();

        std::cout << "ScrollView: Waiting for server...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  }
#ifdef _WIN32
  // WSACleanup();  // This cause ScrollView windows is not displayed
#endif  // _WIN32
  freeaddrinfo(addr_info);
}

SVNetwork::~SVNetwork() {
  Close();
  delete[] msg_buffer_in_;
}

#endif // !GRAPHICS_DISABLED
