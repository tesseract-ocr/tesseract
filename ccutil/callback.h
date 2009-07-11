///////////////////////////////////////////////////////////////////////
// File:        callback.h
// Description: classes and functions to replace pointer-to-functions
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#ifndef _CALLBACK_SPECIALIZATIONS_H
#define _CALLBACK_SPECIALIZATIONS_H

struct CallbackUtils_ {
  static void FailIsRepeatable(const char* name);
};


class Closure {
 public:
  virtual ~Closure() { }
  virtual void Run() = 0;
};

template <class R>
class ResultCallback {
 public:
  virtual ~ResultCallback() { }
  virtual R Run() = 0;
};

template <bool del, class R, class T>
class _ConstMemberResultCallback_0_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)() const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_0(
     const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)();
      return result;
    } else {
      R result = (object_->*member_)();
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T>
class _ConstMemberResultCallback_0_0<del, void, T>
  : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)() const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_0(
      const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)();
    } else {
      (object_->*member_)();
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _ConstMemberResultCallback_0_0<true,R,T1>::base*
NewCallback(
    const T1* obj, R (T2::*member)() const) {
  return new _ConstMemberResultCallback_0_0<true,R,T1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _ConstMemberResultCallback_0_0<false,R,T1>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)() const) {
  return new _ConstMemberResultCallback_0_0<false,R,T1>(
      obj, member);
}
#endif

template <bool del, class R, class T>
class _MemberResultCallback_0_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)() ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_0(
      T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)();
      return result;
    } else {
      R result = (object_->*member_)();
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T>
class _MemberResultCallback_0_0<del, void, T>
  : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)() ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_0(
       T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)();
    } else {
      (object_->*member_)();
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _MemberResultCallback_0_0<true,R,T1>::base*
NewCallback(
     T1* obj, R (T2::*member)() ) {
  return new _MemberResultCallback_0_0<true,R,T1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _MemberResultCallback_0_0<false,R,T1>::base*
NewPermanentCallback(
     T1* obj, R (T2::*member)() ) {
  return new _MemberResultCallback_0_0<false,R,T1>(
      obj, member);
}
#endif

template <bool del, class R>
class _FunctionResultCallback_0_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)();

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_0(
      FunctionSignature function)
    : function_(function) {
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)();
      return result;
    } else {
      R result = (*function_)();
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del>
class _FunctionResultCallback_0_0<del, void>
  : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)();

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_0(
      FunctionSignature function)
    : function_(function) {
  }

  virtual void Run() {
    if (!del) {
      (*function_)();
    } else {
      (*function_)();
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R>
inline typename _FunctionResultCallback_0_0<true,R>::base*
NewCallback(R (*function)()) {
  return new _FunctionResultCallback_0_0<true,R>(function);
}

template <class R>
inline typename _FunctionResultCallback_0_0<false,R>::base*
NewPermanentCallback(R (*function)()) {
  return new _FunctionResultCallback_0_0<false,R>(function);
}

template <class A1>
class Callback1 {
 public:
  virtual ~Callback1() { }
  virtual void Run(A1) = 0;
};

template <class R, class A1>
class ResultCallback1 {
 public:
  virtual ~ResultCallback1() { }
  virtual R Run(A1) = 0;
};

template <bool del, class R, class T, class A1>
class _ConstMemberResultCallback_0_1 : public ResultCallback1<R,A1> {
 public:
  typedef ResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(A1) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_1(
     const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(a1);
      return result;
    } else {
      R result = (object_->*member_)(a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1>
class _ConstMemberResultCallback_0_1<del, void, T, A1>
  : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(A1) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_1(
      const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(a1);
    } else {
      (object_->*member_)(a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _ConstMemberResultCallback_0_1<true,R,T1,A1>::base*
NewCallback(
    const T1* obj, R (T2::*member)(A1) const) {
  return new _ConstMemberResultCallback_0_1<true,R,T1,A1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _ConstMemberResultCallback_0_1<false,R,T1,A1>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(A1) const) {
  return new _ConstMemberResultCallback_0_1<false,R,T1,A1>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1>
class _MemberResultCallback_0_1 : public ResultCallback1<R,A1> {
 public:
  typedef ResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(A1) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_1(
      T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(a1);
      return result;
    } else {
      R result = (object_->*member_)(a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1>
class _MemberResultCallback_0_1<del, void, T, A1>
  : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(A1) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_1(
       T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(a1);
    } else {
      (object_->*member_)(a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _MemberResultCallback_0_1<true,R,T1,A1>::base*
NewCallback(
     T1* obj, R (T2::*member)(A1) ) {
  return new _MemberResultCallback_0_1<true,R,T1,A1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _MemberResultCallback_0_1<false,R,T1,A1>::base*
NewPermanentCallback(
     T1* obj, R (T2::*member)(A1) ) {
  return new _MemberResultCallback_0_1<false,R,T1,A1>(
      obj, member);
}
#endif

template <bool del, class R, class A1>
class _FunctionResultCallback_0_1 : public ResultCallback1<R,A1> {
 public:
  typedef ResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(A1);

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_1(
      FunctionSignature function)
    : function_(function) {
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(a1);
      return result;
    } else {
      R result = (*function_)(a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1>
class _FunctionResultCallback_0_1<del, void, A1>
  : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(A1);

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_1(
      FunctionSignature function)
    : function_(function) {
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(a1);
    } else {
      (*function_)(a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1>
inline typename _FunctionResultCallback_0_1<true,R,A1>::base*
NewCallback(R (*function)(A1)) {
  return new _FunctionResultCallback_0_1<true,R,A1>(function);
}

template <class R, class A1>
inline typename _FunctionResultCallback_0_1<false,R,A1>::base*
NewPermanentCallback(R (*function)(A1)) {
  return new _FunctionResultCallback_0_1<false,R,A1>(function);
}

template <class A1,class A2>
class Callback2 {
 public:
  virtual ~Callback2() { }
  virtual void Run(A1,A2) = 0;
};

template <class R, class A1,class A2>
class ResultCallback2 {
 public:
  virtual ~ResultCallback2() { }
  virtual R Run(A1,A2) = 0;
};

template <bool del, class R, class T, class A1, class A2>
class _ConstMemberResultCallback_0_2 : public ResultCallback2<R,A1,A2> {
 public:
  typedef ResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_2(
     const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2>
class _ConstMemberResultCallback_0_2<del, void, T, A1, A2>
  : public Callback2<A1,A2> {
 public:
  typedef Callback2<A1,A2> base;
  typedef void (T::*MemberSignature)(A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_2(
      const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(a1,a2);
    } else {
      (object_->*member_)(a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _ConstMemberResultCallback_0_2<true,R,T1,A1,A2>::base*
NewCallback(
    const T1* obj, R (T2::*member)(A1,A2) const) {
  return new _ConstMemberResultCallback_0_2<true,R,T1,A1,A2>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _ConstMemberResultCallback_0_2<false,R,T1,A1,A2>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(A1,A2) const) {
  return new _ConstMemberResultCallback_0_2<false,R,T1,A1,A2>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2>
class _MemberResultCallback_0_2 : public ResultCallback2<R,A1,A2> {
 public:
  typedef ResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_2(
      T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2>
class _MemberResultCallback_0_2<del, void, T, A1, A2>
  : public Callback2<A1,A2> {
 public:
  typedef Callback2<A1,A2> base;
  typedef void (T::*MemberSignature)(A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_2(
       T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(a1,a2);
    } else {
      (object_->*member_)(a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _MemberResultCallback_0_2<true,R,T1,A1,A2>::base*
NewCallback(
     T1* obj, R (T2::*member)(A1,A2) ) {
  return new _MemberResultCallback_0_2<true,R,T1,A1,A2>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _MemberResultCallback_0_2<false,R,T1,A1,A2>::base*
NewPermanentCallback(
     T1* obj, R (T2::*member)(A1,A2) ) {
  return new _MemberResultCallback_0_2<false,R,T1,A1,A2>(
      obj, member);
}
#endif

template <bool del, class R, class A1, class A2>
class _FunctionResultCallback_0_2 : public ResultCallback2<R,A1,A2> {
 public:
  typedef ResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(A1,A2);

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_2(
      FunctionSignature function)
    : function_(function) {
  }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(a1,a2);
      return result;
    } else {
      R result = (*function_)(a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2>
class _FunctionResultCallback_0_2<del, void, A1, A2>
  : public Callback2<A1,A2> {
 public:
  typedef Callback2<A1,A2> base;
  typedef void (*FunctionSignature)(A1,A2);

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_2(
      FunctionSignature function)
    : function_(function) {
  }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(a1,a2);
    } else {
      (*function_)(a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1, class A2>
inline typename _FunctionResultCallback_0_2<true,R,A1,A2>::base*
NewCallback(R (*function)(A1,A2)) {
  return new _FunctionResultCallback_0_2<true,R,A1,A2>(function);
}

template <class R, class A1, class A2>
inline typename _FunctionResultCallback_0_2<false,R,A1,A2>::base*
NewPermanentCallback(R (*function)(A1,A2)) {
  return new _FunctionResultCallback_0_2<false,R,A1,A2>(function);
}

template <class A1,class A2,class A3>
class Callback3 {
 public:
  virtual ~Callback3() { }
  virtual void Run(A1,A2,A3) = 0;
};

template <class R, class A1,class A2,class A3>
class ResultCallback3 {
 public:
  virtual ~ResultCallback3() { }
  virtual R Run(A1,A2,A3) = 0;
};

template <bool del, class R, class T, class A1, class A2, class A3>
class _ConstMemberResultCallback_0_3 : public ResultCallback3<R,A1,A2,A3> {
 public:
  typedef ResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_3(
     const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3>
class _ConstMemberResultCallback_0_3<del, void, T, A1, A2, A3>
  : public Callback3<A1,A2,A3> {
 public:
  typedef Callback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_3(
      const T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(a1,a2,a3);
    } else {
      (object_->*member_)(a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_0_3<true,R,T1,A1,A2,A3>::base*
NewCallback(
    const T1* obj, R (T2::*member)(A1,A2,A3) const) {
  return new _ConstMemberResultCallback_0_3<true,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_0_3<false,R,T1,A1,A2,A3>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(A1,A2,A3) const) {
  return new _ConstMemberResultCallback_0_3<false,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3>
class _MemberResultCallback_0_3 : public ResultCallback3<R,A1,A2,A3> {
 public:
  typedef ResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_3(
      T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3>
class _MemberResultCallback_0_3<del, void, T, A1, A2, A3>
  : public Callback3<A1,A2,A3> {
 public:
  typedef Callback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_3(
       T* object, MemberSignature member)
    : object_(object),
      member_(member) {
  }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(a1,a2,a3);
    } else {
      (object_->*member_)(a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _MemberResultCallback_0_3<true,R,T1,A1,A2,A3>::base*
NewCallback(
     T1* obj, R (T2::*member)(A1,A2,A3) ) {
  return new _MemberResultCallback_0_3<true,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _MemberResultCallback_0_3<false,R,T1,A1,A2,A3>::base*
NewPermanentCallback(
     T1* obj, R (T2::*member)(A1,A2,A3) ) {
  return new _MemberResultCallback_0_3<false,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

template <bool del, class R, class A1, class A2, class A3>
class _FunctionResultCallback_0_3 : public ResultCallback3<R,A1,A2,A3> {
 public:
  typedef ResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(A1,A2,A3);

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_3(
      FunctionSignature function)
    : function_(function) {
  }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (*function_)(a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2, class A3>
class _FunctionResultCallback_0_3<del, void, A1, A2, A3>
  : public Callback3<A1,A2,A3> {
 public:
  typedef Callback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(A1,A2,A3);

 private:
  FunctionSignature function_;

 public:
  inline _FunctionResultCallback_0_3(
      FunctionSignature function)
    : function_(function) {
  }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (*function_)(a1,a2,a3);
    } else {
      (*function_)(a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1, class A2, class A3>
inline typename _FunctionResultCallback_0_3<true,R,A1,A2,A3>::base*
NewCallback(R (*function)(A1,A2,A3)) {
  return new _FunctionResultCallback_0_3<true,R,A1,A2,A3>(function);
}

template <class R, class A1, class A2, class A3>
inline typename _FunctionResultCallback_0_3<false,R,A1,A2,A3>::base*
NewPermanentCallback(R (*function)(A1,A2,A3)) {
  return new _FunctionResultCallback_0_3<false,R,A1,A2,A3>(function);
}

#endif /* _CALLBACK_SPECIALIZATIONS_H */
