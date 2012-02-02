///////////////////////////////////////////////////////////////////////
// File:        tesscallback.h
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

#ifndef _TESS_CALLBACK_SPECIALIZATIONS_H
#define _TESS_CALLBACK_SPECIALIZATIONS_H

#include "host.h"  // For NULL.

struct TessCallbackUtils_ {
  static void FailIsRepeatable(const char* name);
};


class TessClosure {
 public:
  virtual ~TessClosure() { }
  virtual void Run() = 0;
};

template <class R>
class TessResultCallback {
 public:
  virtual ~TessResultCallback() { }
  virtual R Run() = 0;
};

template <bool del, class R, class T>
class _ConstTessMemberResultCallback_0_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)() const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_0(
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
class _ConstTessMemberResultCallback_0_0<del, void, T>
  : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)() const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_0(
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
inline typename _ConstTessMemberResultCallback_0_0<true,R,T1>::base*
NewTessCallback(
    const T1* obj, R (T2::*member)() const) {
  return new _ConstTessMemberResultCallback_0_0<true,R,T1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _ConstTessMemberResultCallback_0_0<false,R,T1>::base*
NewPermanentTessCallback(
    const T1* obj, R (T2::*member)() const) {
  return new _ConstTessMemberResultCallback_0_0<false,R,T1>(
      obj, member);
}
#endif

template <bool del, class R, class T>
class _TessMemberResultCallback_0_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)() ;

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_0(
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
class _TessMemberResultCallback_0_0<del, void, T>
  : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)() ;

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_0(
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
inline typename _TessMemberResultCallback_0_0<true,R,T1>::base*
NewTessCallback(
     T1* obj, R (T2::*member)() ) {
  return new _TessMemberResultCallback_0_0<true,R,T1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _TessMemberResultCallback_0_0<false,R,T1>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)() ) {
  return new _TessMemberResultCallback_0_0<false,R,T1>(
      obj, member);
}
#endif

template <bool del, class R>
class _TessFunctionResultCallback_0_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)();

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_0(
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
class _TessFunctionResultCallback_0_0<del, void>
  : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)();

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_0(
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
inline typename _TessFunctionResultCallback_0_0<true,R>::base*
NewTessCallback(R (*function)()) {
  return new _TessFunctionResultCallback_0_0<true,R>(function);
}

template <class R>
inline typename _TessFunctionResultCallback_0_0<false,R>::base*
NewPermanentTessCallback(R (*function)()) {
  return new _TessFunctionResultCallback_0_0<false,R>(function);
}

template <class A1>
class TessCallback1 {
 public:
  virtual ~TessCallback1() { }
  virtual void Run(A1) = 0;
};

template <class R, class A1>
class TessResultCallback1 {
 public:
  virtual ~TessResultCallback1() { }
  virtual R Run(A1) = 0;
};

template <bool del, class R, class T, class A1>
class _ConstTessMemberResultCallback_0_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(A1) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_1(
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
class _ConstTessMemberResultCallback_0_1<del, void, T, A1>
  : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(A1) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_1(
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
inline typename _ConstTessMemberResultCallback_0_1<true,R,T1,A1>::base*
NewTessCallback(
    const T1* obj, R (T2::*member)(A1) const) {
  return new _ConstTessMemberResultCallback_0_1<true,R,T1,A1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _ConstTessMemberResultCallback_0_1<false,R,T1,A1>::base*
NewPermanentTessCallback(
    const T1* obj, R (T2::*member)(A1) const) {
  return new _ConstTessMemberResultCallback_0_1<false,R,T1,A1>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1>
class _TessMemberResultCallback_0_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(A1) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_1(
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
class _TessMemberResultCallback_0_1<del, void, T, A1>
  : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(A1) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_1(
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
inline typename _TessMemberResultCallback_0_1<true,R,T1,A1>::base*
NewTessCallback(
     T1* obj, R (T2::*member)(A1) ) {
  return new _TessMemberResultCallback_0_1<true,R,T1,A1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _TessMemberResultCallback_0_1<false,R,T1,A1>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)(A1) ) {
  return new _TessMemberResultCallback_0_1<false,R,T1,A1>(
      obj, member);
}
#endif

template <bool del, class R, class A1>
class _TessFunctionResultCallback_0_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(A1);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_1(
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
class _TessFunctionResultCallback_0_1<del, void, A1>
  : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(A1);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_1(
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
inline typename _TessFunctionResultCallback_0_1<true,R,A1>::base*
NewTessCallback(R (*function)(A1)) {
  return new _TessFunctionResultCallback_0_1<true,R,A1>(function);
}

template <class R, class A1>
inline typename _TessFunctionResultCallback_0_1<false,R,A1>::base*
NewPermanentTessCallback(R (*function)(A1)) {
  return new _TessFunctionResultCallback_0_1<false,R,A1>(function);
}

template <class A1,class A2>
class TessCallback2 {
 public:
  virtual ~TessCallback2() { }
  virtual void Run(A1,A2) = 0;
};

template <class R, class A1,class A2>
class TessResultCallback2 {
 public:
  virtual ~TessResultCallback2() { }
  virtual R Run(A1,A2) = 0;
};

template <bool del, class R, class T, class A1, class A2>
class _ConstTessMemberResultCallback_0_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_2(
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
class _ConstTessMemberResultCallback_0_2<del, void, T, A1, A2>
  : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_2(
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
inline typename _ConstTessMemberResultCallback_0_2<true,R,T1,A1,A2>::base*
NewTessCallback(
    const T1* obj, R (T2::*member)(A1,A2) const) {
  return new _ConstTessMemberResultCallback_0_2<true,R,T1,A1,A2>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _ConstTessMemberResultCallback_0_2<false,R,T1,A1,A2>::base*
NewPermanentTessCallback(
    const T1* obj, R (T2::*member)(A1,A2) const) {
  return new _ConstTessMemberResultCallback_0_2<false,R,T1,A1,A2>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2>
class _TessMemberResultCallback_0_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_2(
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
class _TessMemberResultCallback_0_2<del, void, T, A1, A2>
  : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_2(
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
inline typename _TessMemberResultCallback_0_2<true,R,T1,A1,A2>::base*
NewTessCallback(
     T1* obj, R (T2::*member)(A1,A2) ) {
  return new _TessMemberResultCallback_0_2<true,R,T1,A1,A2>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _TessMemberResultCallback_0_2<false,R,T1,A1,A2>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)(A1,A2) ) {
  return new _TessMemberResultCallback_0_2<false,R,T1,A1,A2>(
      obj, member);
}
#endif

template <bool del, class R, class A1, class A2>
class _TessFunctionResultCallback_0_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(A1,A2);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_2(
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
class _TessFunctionResultCallback_0_2<del, void, A1, A2>
  : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(A1,A2);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_2(
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
inline typename _TessFunctionResultCallback_0_2<true,R,A1,A2>::base*
NewTessCallback(R (*function)(A1,A2)) {
  return new _TessFunctionResultCallback_0_2<true,R,A1,A2>(function);
}

template <class R, class A1, class A2>
inline typename _TessFunctionResultCallback_0_2<false,R,A1,A2>::base*
NewPermanentTessCallback(R (*function)(A1,A2)) {
  return new _TessFunctionResultCallback_0_2<false,R,A1,A2>(function);
}

template <class A1,class A2,class A3>
class TessCallback3 {
 public:
  virtual ~TessCallback3() { }
  virtual void Run(A1,A2,A3) = 0;
};

template <class R, class A1,class A2,class A3>
class TessResultCallback3 {
 public:
  virtual ~TessResultCallback3() { }
  virtual R Run(A1,A2,A3) = 0;
};

template <bool del, class R, class T, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_0_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_3(
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
class _ConstTessMemberResultCallback_0_3<del, void, T, A1, A2, A3>
  : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_3(
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
inline typename _ConstTessMemberResultCallback_0_3<true,R,T1,A1,A2,A3>::base*
NewTessCallback(
    const T1* obj, R (T2::*member)(A1,A2,A3) const) {
  return new _ConstTessMemberResultCallback_0_3<true,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_0_3<false,R,T1,A1,A2,A3>::base*
NewPermanentTessCallback(
    const T1* obj, R (T2::*member)(A1,A2,A3) const) {
  return new _ConstTessMemberResultCallback_0_3<false,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3>
class _TessMemberResultCallback_0_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_3(
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
class _TessMemberResultCallback_0_3<del, void, T, A1, A2, A3>
  : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_3(
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
inline typename _TessMemberResultCallback_0_3<true,R,T1,A1,A2,A3>::base*
NewTessCallback(
     T1* obj, R (T2::*member)(A1,A2,A3) ) {
  return new _TessMemberResultCallback_0_3<true,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_0_3<false,R,T1,A1,A2,A3>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)(A1,A2,A3) ) {
  return new _TessMemberResultCallback_0_3<false,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

template <bool del, class R, class A1, class A2, class A3>
class _TessFunctionResultCallback_0_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(A1,A2,A3);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_3(
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
class _TessFunctionResultCallback_0_3<del, void, A1, A2, A3>
  : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(A1,A2,A3);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_3(
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
inline typename _TessFunctionResultCallback_0_3<true,R,A1,A2,A3>::base*
NewTessCallback(R (*function)(A1,A2,A3)) {
  return new _TessFunctionResultCallback_0_3<true,R,A1,A2,A3>(function);
}

template <class R, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_0_3<false,R,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(A1,A2,A3)) {
  return new _TessFunctionResultCallback_0_3<false,R,A1,A2,A3>(function);
}

// Specified by TR1 [4.7.2] Reference modifications.
template <class T> struct remove_reference;
template<typename T> struct remove_reference { typedef T type; };
template<typename T> struct remove_reference<T&> { typedef T type; };

// Identity<T>::type is a typedef of T. Useful for preventing the
// compiler from inferring the type of an argument in templates.
template <typename T>
struct Identity {
  typedef T type;
};

template <bool del, class R, class T, class P1, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_1_3
  : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,A1,A2,A3) const;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_3(T* object,
                                            MemberSignature member, P1 p1)
    : object_(object), member_(member), p1_(p1) { }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_1_3<del, void, T, P1, A1, A2, A3>
  : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,A1,A2,A3) const;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_3(T* object,
                                            MemberSignature member, P1 p1)
    : object_(object), member_(member), p1_(p1) { }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>::base*
NewTessCallback( T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_1_3<false,R,T1,P1,A1,A2,A3>::base*
NewPermanentTessCallback( T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_3<false,R,T1,P1,A1,A2,A3>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1, class A2, class A3>
class _TessMemberResultCallback_1_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_3(T* object,
                                        MemberSignature member, P1 p1)
    : object_(object), member_(member), p1_(p1) { }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3>
class _TessMemberResultCallback_1_3<del, void, T, P1, A1, A2, A3>
  : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_3(T* object,
                                        MemberSignature member, P1 p1)
    : object_(object), member_(member), p1_(p1) { }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>::base*
NewTessCallback( T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_1_3<false,R,T1,P1,A1,A2,A3>::base*
NewPermanentTessCallback( T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_3<false,R,T1,P1,A1,A2,A3>(obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2, class A3>
class _TessFunctionResultCallback_1_3 : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef R (*FunctionSignature)(P1,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_3(FunctionSignature function, P1 p1)
    : function_(function), p1_(p1) { }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_,a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(p1_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2, class A3>
class _TessFunctionResultCallback_1_3<del, void, P1, A1, A2, A3>
  : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(P1,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_3(FunctionSignature function, P1 p1)
    : function_(function), p1_(p1) { }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_,a1,a2,a3);
    } else {
      (*function_)(p1_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_1_3<true,R,P1,A1,A2,A3>::base*
NewTessCallback(R (*function)(P1,A1,A2,A3), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_3<true,R,P1,A1,A2,A3>(function, p1);
}

template <class R, class P1, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_1_3<false,R,P1,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(P1,A1,A2,A3), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_3<false,R,P1,A1,A2,A3>(function, p1);
}

#endif /* _TESS_CALLBACK_SPECIALIZATIONS_H */
