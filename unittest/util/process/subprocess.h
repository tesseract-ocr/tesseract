// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef UTIL_PROCESS_SUBPROCESS_H__
#define UTIL_PROCESS_SUBPROCESS_H__

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "base/integral_types.h"

enum Channel {
  CHAN_STDIN = STDIN_FILENO,
  CHAN_STDOUT = STDOUT_FILENO,
  CHAN_STDERR = STDERR_FILENO,
};

// How the channel is handled.
enum ChannelAction {
  ACTION_CLOSE,
  ACTION_DUPPARENT,
  ACTION_PIPE
};

// Utility for running other processes.
//
// Class is thread-compatible.
class SubProcess {
 public:
  SubProcess();
  virtual ~SubProcess();

  virtual inline pid_t pid() const { return pid_; }
  virtual bool running() const { return running_; }

  // Sets the child as a group leader.
  virtual void SetUseSession();

  // Returns a error message that describes the exit status of the child
  // process.
  virtual ::std::string error_text() const;

  // Returns the exit code returned by the child process.
  virtual int exit_code() const;

  // Whether to inherit the parent's "higher" (not stdin, stdout, stderr) file
  // descriptors.
  void SetInheritHigherFDs(bool value);

  // How to handle standard input/output channels in the new process.
  virtual void SetChannelAction(Channel chan, ChannelAction action);

  // Set up a program and argument list for execution. The first argument is the
  // program that will be executed.
  virtual void SetArgv(const ::std::vector<::std::string> &argv);

  // Starts process.
  virtual bool Start();

  // Waits for the subprocess to exit and reaps it.
  virtual bool Wait();

  // This is similar to Python's subprocess.Popen.communicate(). This
  // asynchronously reads from stdout and stderr until all output pipes have
  // closed, then waits for the process to exit. 'stdout_output' and
  // 'stderr_output' may be nullptr.
  //
  // Returns:
  //   int: the command's exit status.
  virtual int Communicate(::std::string* stdout_output,
                          ::std::string* stderr_output);

 protected:
  // Actually exec the child process.
  virtual void ExecChild();

 private:
  struct CommBuf;

  void BlockSignals();
  void UnblockSignals();
  void CloseNonChannelFds();
  void ChildFork();
  bool SetupChildToParentFds();
  int SendMessageToParent();
  bool ReceiveMessageFromChild();
  void SendFatalError(int error_no, const ::std::string &error_msg);
  void ShareFdsWithParent();
  void CloseAllPipeFds();
  void CloseChildPipeFds();
  static int NumOfChannels();
  bool SetupPipesForChannels();
  void MaybeAddFD(Channel channel, ::std::string* output,
                  ::std::string** io_strings, Channel* channels,
                  struct pollfd* fds, int* descriptors_to_poll,
                  int* descriptors_left, int16 events);
  void Close(Channel chan);

  bool running_;
  bool use_session_;
  bool inherit_higher_fds_;

  pid_t pid_;
  ::std::vector<::std::string> argv_;
  sigset_t old_signals_;
  ::std::vector<ChannelAction> actions_;

  ::std::string error_text_;
  int exit_status_;

  // fd to use for receiving errors, debug info and file handles from child.
  int parent_to_child_fd_;
  // fd to use for reporting errors, debug info and file handles to parent.
  int child_to_parent_fd_;

  ::std::unique_ptr<int[]> child_pipe_fds_;
  ::std::unique_ptr<int[]> parent_pipe_fds_;

  ::std::unique_ptr<CommBuf> comm_buf_;
};

#endif  // UTIL_PROCESS_SUBPROCESS_H__