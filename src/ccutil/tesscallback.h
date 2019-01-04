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

#ifndef TESS_CALLBACK_SPECIALIZATIONS_H_
#define TESS_CALLBACK_SPECIALIZATIONS_H_

#include "host.h"  // For nullptr.

struct TessCallbackUtils_ {
  static void FailIsRepeatable(const char* name);
};


class TessClosure {
 public:
  virtual ~TessClosure();
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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _TessMemberResultCallback_0_0<true,R,T1>::base*
NewTessCallback(
     T1* obj, R (T2::*member)()) {
  return new _TessMemberResultCallback_0_0<true,R,T1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _TessMemberResultCallback_0_0<false,R,T1>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)()) {
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
      function_ = nullptr;
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
      function_ = nullptr;
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

template <bool del, class R, class T, class P1>
class _ConstTessMemberResultCallback_1_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_0(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_);
      return result;
    } else {
      R result = (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1>
class _ConstTessMemberResultCallback_1_0<del, void, T, P1> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_0(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_);
    } else {
      (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _ConstTessMemberResultCallback_1_0<true,R,T1,P1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_0<true,R,T1,P1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _ConstTessMemberResultCallback_1_0<false,R,T1,P1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_0<false,R,T1,P1>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1>
class _TessMemberResultCallback_1_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_0(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_);
      return result;
    } else {
      R result = (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1>
class _TessMemberResultCallback_1_0<del, void, T, P1> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_0(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_);
    } else {
      (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _TessMemberResultCallback_1_0<true,R,T1,P1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_0<true,R,T1,P1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _TessMemberResultCallback_1_0<false,R,T1,P1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_0<false,R,T1,P1>(obj, member, p1);
}
#endif

template <bool del, class R, class P1>
class _TessFunctionResultCallback_1_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)(P1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_0(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_);
      return result;
    } else {
      R result = (*function_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1>
class _TessFunctionResultCallback_1_0<del, void, P1> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)(P1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_0(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_);
    } else {
      (*function_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1>
inline typename _TessFunctionResultCallback_1_0<true,R,P1>::base*
NewTessCallback(R (*function)(P1), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_0<true,R,P1>(function, p1);
}

template <class R, class P1>
inline typename _TessFunctionResultCallback_1_0<false,R,P1>::base*
NewPermanentTessCallback(R (*function)(P1), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_0<false,R,P1>(function, p1);
}

template <bool del, class R, class T, class P1, class P2>
class _ConstTessMemberResultCallback_2_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_0(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2>
class _ConstTessMemberResultCallback_2_0<del, void, T, P1, P2> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_0(const T* object, MemberSignature member, P1 p1, P2 p2)
    :
      object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_);
    } else {
      (object_->*member_)(p1_,p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _ConstTessMemberResultCallback_2_0<true,R,T1,P1,P2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_0<true,R,T1,P1,P2>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _ConstTessMemberResultCallback_2_0<false,R,T1,P1,P2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_0<false,R,T1,P1,P2>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2>
class _TessMemberResultCallback_2_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_0(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2>
class _TessMemberResultCallback_2_0<del, void, T, P1, P2> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_0(T* object, MemberSignature member, P1 p1, P2 p2)
    :
      object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_);
    } else {
      (object_->*member_)(p1_,p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _TessMemberResultCallback_2_0<true,R,T1,P1,P2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_0<true,R,T1,P1,P2>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _TessMemberResultCallback_2_0<false,R,T1,P1,P2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_0<false,R,T1,P1,P2>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2>
class _TessFunctionResultCallback_2_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)(P1,P2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_0(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_,p2_);
      return result;
    } else {
      R result = (*function_)(p1_,p2_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2>
class _TessFunctionResultCallback_2_0<del, void, P1, P2> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)(P1,P2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_0(FunctionSignature function, P1 p1, P2 p2)
    :
      function_(function),      p1_(p1),      p2_(p2) { }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_,p2_);
    } else {
      (*function_)(p1_,p2_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2>
inline typename _TessFunctionResultCallback_2_0<true,R,P1,P2>::base*
NewTessCallback(R (*function)(P1,P2), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_0<true,R,P1,P2>(function, p1, p2);
}

template <class R, class P1, class P2>
inline typename _TessFunctionResultCallback_2_0<false,R,P1,P2>::base*
NewPermanentTessCallback(R (*function)(P1,P2), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_0<false,R,P1,P2>(function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3>
class _ConstTessMemberResultCallback_3_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    :
      object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3>
class _ConstTessMemberResultCallback_3_0<del, void, T, P1, P2, P3> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_);
    } else {
      (object_->*member_)(p1_,p2_,p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _ConstTessMemberResultCallback_3_0<true,R,T1,P1,P2,P3>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_0<true,R,T1,P1,P2,P3>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _ConstTessMemberResultCallback_3_0<false,R,T1,P1,P2,P3>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_0<false,R,T1,P1,P2,P3>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3>
class _TessMemberResultCallback_3_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3>
class _TessMemberResultCallback_3_0<del, void, T, P1, P2, P3> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_);
    } else {
      (object_->*member_)(p1_,p2_,p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _TessMemberResultCallback_3_0<true,R,T1,P1,P2,P3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_0<true,R,T1,P1,P2,P3>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _TessMemberResultCallback_3_0<false,R,T1,P1,P2,P3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_0<false,R,T1,P1,P2,P3>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3>
class _TessFunctionResultCallback_3_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)(P1,P2,P3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_0(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3>
class _TessFunctionResultCallback_3_0<del, void, P1, P2, P3> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)(P1,P2,P3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_0(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_,p2_,p3_);
    } else {
      (*function_)(p1_,p2_,p3_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3>
inline typename _TessFunctionResultCallback_3_0<true,R,P1,P2,P3>::base*
NewTessCallback(R (*function)(P1,P2,P3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_0<true,R,P1,P2,P3>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3>
inline typename _TessFunctionResultCallback_3_0<false,R,P1,P2,P3>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_0<false,R,P1,P2,P3>(function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4>
class _ConstTessMemberResultCallback_4_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4>
class _ConstTessMemberResultCallback_4_0<del, void, T, P1, P2, P3, P4> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _ConstTessMemberResultCallback_4_0<true,R,T1,P1,P2,P3,P4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_0<true,R,T1,P1,P2,P3,P4>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _ConstTessMemberResultCallback_4_0<false,R,T1,P1,P2,P3,P4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_0<false,R,T1,P1,P2,P3,P4>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4>
class _TessMemberResultCallback_4_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4>
class _TessMemberResultCallback_4_0<del, void, T, P1, P2, P3, P4> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _TessMemberResultCallback_4_0<true,R,T1,P1,P2,P3,P4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_0<true,R,T1,P1,P2,P3,P4>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _TessMemberResultCallback_4_0<false,R,T1,P1,P2,P3,P4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_0<false,R,T1,P1,P2,P3,P4>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4>
class _TessFunctionResultCallback_4_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_0(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4>
class _TessFunctionResultCallback_4_0<del, void, P1, P2, P3, P4> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_0(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_);
    } else {
      (*function_)(p1_,p2_,p3_,p4_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4>
inline typename _TessFunctionResultCallback_4_0<true,R,P1,P2,P3,P4>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_0<true,R,P1,P2,P3,P4>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4>
inline typename _TessFunctionResultCallback_4_0<false,R,P1,P2,P3,P4>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_0<false,R,P1,P2,P3,P4>(function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5>
class _ConstTessMemberResultCallback_5_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5>
class _ConstTessMemberResultCallback_5_0<del, void, T, P1, P2, P3, P4, P5> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5>
inline typename _ConstTessMemberResultCallback_5_0<true,R,T1,P1,P2,P3,P4,P5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_0<true,R,T1,P1,P2,P3,P4,P5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5>
inline typename _ConstTessMemberResultCallback_5_0<false,R,T1,P1,P2,P3,P4,P5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_0<false,R,T1,P1,P2,P3,P4,P5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5>
class _TessMemberResultCallback_5_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5>
class _TessMemberResultCallback_5_0<del, void, T, P1, P2, P3, P4, P5> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5>
inline typename _TessMemberResultCallback_5_0<true,R,T1,P1,P2,P3,P4,P5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_0<true,R,T1,P1,P2,P3,P4,P5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5>
inline typename _TessMemberResultCallback_5_0<false,R,T1,P1,P2,P3,P4,P5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_0<false,R,T1,P1,P2,P3,P4,P5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5>
class _TessFunctionResultCallback_5_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_0(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5>
class _TessFunctionResultCallback_5_0<del, void, P1, P2, P3, P4, P5> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_0(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5>
inline typename _TessFunctionResultCallback_5_0<true,R,P1,P2,P3,P4,P5>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_0<true,R,P1,P2,P3,P4,P5>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5>
inline typename _TessFunctionResultCallback_5_0<false,R,P1,P2,P3,P4,P5>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_0<false,R,P1,P2,P3,P4,P5>(function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6>
class _ConstTessMemberResultCallback_6_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6>
class _ConstTessMemberResultCallback_6_0<del, void, T, P1, P2, P3, P4, P5, P6> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_0(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _ConstTessMemberResultCallback_6_0<true,R,T1,P1,P2,P3,P4,P5,P6>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_0<true,R,T1,P1,P2,P3,P4,P5,P6>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _ConstTessMemberResultCallback_6_0<false,R,T1,P1,P2,P3,P4,P5,P6>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_0<false,R,T1,P1,P2,P3,P4,P5,P6>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6>
class _TessMemberResultCallback_6_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6>
class _TessMemberResultCallback_6_0<del, void, T, P1, P2, P3, P4, P5, P6> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_0(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _TessMemberResultCallback_6_0<true,R,T1,P1,P2,P3,P4,P5,P6>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_0<true,R,T1,P1,P2,P3,P4,P5,P6>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _TessMemberResultCallback_6_0<false,R,T1,P1,P2,P3,P4,P5,P6>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_0<false,R,T1,P1,P2,P3,P4,P5,P6>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class P6>
class _TessFunctionResultCallback_6_0 : public TessResultCallback<R> {
 public:
  typedef TessResultCallback<R> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,P6);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_0(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6>
class _TessFunctionResultCallback_6_0<del, void, P1, P2, P3, P4, P5, P6> : public TessClosure {
 public:
  typedef TessClosure base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,P6);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_0(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _TessFunctionResultCallback_6_0<true,R,P1,P2,P3,P4,P5,P6>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,P6), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_0<true,R,P1,P2,P3,P4,P5,P6>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _TessFunctionResultCallback_6_0<false,R,P1,P2,P3,P4,P5,P6>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,P6), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_0<false,R,P1,P2,P3,P4,P5,P6>(function, p1, p2, p3, p4, p5, p6);
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

template <class A1,class A2,class A3,class A4>
class TessCallback4 {
 public:
  virtual ~TessCallback4() { }
  virtual void Run(A1,A2,A3,A4) = 0;
};

template <class R, class A1,class A2,class A3,class A4>
class TessResultCallback4 {
 public:
  virtual ~TessResultCallback4() { }
  virtual R Run(A1,A2,A3,A4) = 0;
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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _TessMemberResultCallback_0_1<true,R,T1,A1>::base*
NewTessCallback(
     T1* obj, R (T2::*member)(A1)) {
  return new _TessMemberResultCallback_0_1<true,R,T1,A1>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _TessMemberResultCallback_0_1<false,R,T1,A1>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)(A1)) {
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
      function_ = nullptr;
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
      function_ = nullptr;
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

template <bool del, class R, class T, class P1, class A1>
class _ConstTessMemberResultCallback_1_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_1(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1>
class _ConstTessMemberResultCallback_1_1<del, void, T, P1, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_1(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,a1);
    } else {
      (object_->*member_)(p1_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _ConstTessMemberResultCallback_1_1<true,R,T1,P1,A1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,A1) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_1<true,R,T1,P1,A1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _ConstTessMemberResultCallback_1_1<false,R,T1,P1,A1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,A1) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_1<false,R,T1,P1,A1>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1>
class _TessMemberResultCallback_1_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_1(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1>
class _TessMemberResultCallback_1_1<del, void, T, P1, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_1(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,a1);
    } else {
      (object_->*member_)(p1_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _TessMemberResultCallback_1_1<true,R,T1,P1,A1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,A1) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_1<true,R,T1,P1,A1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _TessMemberResultCallback_1_1<false,R,T1,P1,A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,A1) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_1<false,R,T1,P1,A1>(obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1>
class _TessFunctionResultCallback_1_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(P1,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_1(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_,a1);
      return result;
    } else {
      R result = (*function_)(p1_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1>
class _TessFunctionResultCallback_1_1<del, void, P1, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(P1,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_1(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_,a1);
    } else {
      (*function_)(p1_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class A1>
inline typename _TessFunctionResultCallback_1_1<true,R,P1,A1>::base*
NewTessCallback(R (*function)(P1,A1), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_1<true,R,P1,A1>(function, p1);
}

template <class R, class P1, class A1>
inline typename _TessFunctionResultCallback_1_1<false,R,P1,A1>::base*
NewPermanentTessCallback(R (*function)(P1,A1), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_1<false,R,P1,A1>(function, p1);
}

template <bool del, class R, class T, class P1, class P2, class A1>
class _ConstTessMemberResultCallback_2_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_1(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1>
class _ConstTessMemberResultCallback_2_1<del, void, T, P1, P2, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_1(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1);
    } else {
      (object_->*member_)(p1_,p2_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _ConstTessMemberResultCallback_2_1<true,R,T1,P1,P2,A1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_1<true,R,T1,P1,P2,A1>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _ConstTessMemberResultCallback_2_1<false,R,T1,P1,P2,A1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_1<false,R,T1,P1,P2,A1>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1>
class _TessMemberResultCallback_2_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_1(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1>
class _TessMemberResultCallback_2_1<del, void, T, P1, P2, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_1(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1);
    } else {
      (object_->*member_)(p1_,p2_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _TessMemberResultCallback_2_1<true,R,T1,P1,P2,A1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_1<true,R,T1,P1,P2,A1>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _TessMemberResultCallback_2_1<false,R,T1,P1,P2,A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_1<false,R,T1,P1,P2,A1>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1>
class _TessFunctionResultCallback_2_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(P1,P2,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_1(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_,p2_,a1);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1>
class _TessFunctionResultCallback_2_1<del, void, P1, P2, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(P1,P2,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_1(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_,p2_,a1);
    } else {
      (*function_)(p1_,p2_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1>
inline typename _TessFunctionResultCallback_2_1<true,R,P1,P2,A1>::base*
NewTessCallback(R (*function)(P1,P2,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_1<true,R,P1,P2,A1>(function, p1, p2);
}

template <class R, class P1, class P2, class A1>
inline typename _TessFunctionResultCallback_2_1<false,R,P1,P2,A1>::base*
NewPermanentTessCallback(R (*function)(P1,P2,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_1<false,R,P1,P2,A1>(function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1>
class _ConstTessMemberResultCallback_3_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1>
class _ConstTessMemberResultCallback_3_1<del, void, T, P1, P2, P3, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _ConstTessMemberResultCallback_3_1<true,R,T1,P1,P2,P3,A1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_1<true,R,T1,P1,P2,P3,A1>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _ConstTessMemberResultCallback_3_1<false,R,T1,P1,P2,P3,A1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_1<false,R,T1,P1,P2,P3,A1>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1>
class _TessMemberResultCallback_3_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1>
class _TessMemberResultCallback_3_1<del, void, T, P1, P2, P3, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _TessMemberResultCallback_3_1<true,R,T1,P1,P2,P3,A1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_1<true,R,T1,P1,P2,P3,A1>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _TessMemberResultCallback_3_1<false,R,T1,P1,P2,P3,A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_1<false,R,T1,P1,P2,P3,A1>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1>
class _TessFunctionResultCallback_3_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(P1,P2,P3,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_1(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,a1);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1>
class _TessFunctionResultCallback_3_1<del, void, P1, P2, P3, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(P1,P2,P3,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_1(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,a1);
    } else {
      (*function_)(p1_,p2_,p3_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1>
inline typename _TessFunctionResultCallback_3_1<true,R,P1,P2,P3,A1>::base*
NewTessCallback(R (*function)(P1,P2,P3,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_1<true,R,P1,P2,P3,A1>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1>
inline typename _TessFunctionResultCallback_3_1<false,R,P1,P2,P3,A1>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_1<false,R,P1,P2,P3,A1>(function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1>
class _ConstTessMemberResultCallback_4_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1>
class _ConstTessMemberResultCallback_4_1<del, void, T, P1, P2, P3, P4, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1>
inline typename _ConstTessMemberResultCallback_4_1<true,R,T1,P1,P2,P3,P4,A1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_1<true,R,T1,P1,P2,P3,P4,A1>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1>
inline typename _ConstTessMemberResultCallback_4_1<false,R,T1,P1,P2,P3,P4,A1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_1<false,R,T1,P1,P2,P3,P4,A1>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1>
class _TessMemberResultCallback_4_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1>
class _TessMemberResultCallback_4_1<del, void, T, P1, P2, P3, P4, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1>
inline typename _TessMemberResultCallback_4_1<true,R,T1,P1,P2,P3,P4,A1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_1<true,R,T1,P1,P2,P3,P4,A1>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1>
inline typename _TessMemberResultCallback_4_1<false,R,T1,P1,P2,P3,P4,A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_1<false,R,T1,P1,P2,P3,P4,A1>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1>
class _TessFunctionResultCallback_4_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_1(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1>
class _TessFunctionResultCallback_4_1<del, void, P1, P2, P3, P4, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_1(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,a1);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1>
inline typename _TessFunctionResultCallback_4_1<true,R,P1,P2,P3,P4,A1>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_1<true,R,P1,P2,P3,P4,A1>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1>
inline typename _TessFunctionResultCallback_4_1<false,R,P1,P2,P3,P4,A1>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_1<false,R,P1,P2,P3,P4,A1>(function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1>
class _ConstTessMemberResultCallback_5_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1>
class _ConstTessMemberResultCallback_5_1<del, void, T, P1, P2, P3, P4, P5, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _ConstTessMemberResultCallback_5_1<true,R,T1,P1,P2,P3,P4,P5,A1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_1<true,R,T1,P1,P2,P3,P4,P5,A1>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _ConstTessMemberResultCallback_5_1<false,R,T1,P1,P2,P3,P4,P5,A1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_1<false,R,T1,P1,P2,P3,P4,P5,A1>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1>
class _TessMemberResultCallback_5_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1>
class _TessMemberResultCallback_5_1<del, void, T, P1, P2, P3, P4, P5, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _TessMemberResultCallback_5_1<true,R,T1,P1,P2,P3,P4,P5,A1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_1<true,R,T1,P1,P2,P3,P4,P5,A1>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _TessMemberResultCallback_5_1<false,R,T1,P1,P2,P3,P4,P5,A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_1<false,R,T1,P1,P2,P3,P4,P5,A1>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class A1>
class _TessFunctionResultCallback_5_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_1(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1>
class _TessFunctionResultCallback_5_1<del, void, P1, P2, P3, P4, P5, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_1(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _TessFunctionResultCallback_5_1<true,R,P1,P2,P3,P4,P5,A1>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_1<true,R,P1,P2,P3,P4,P5,A1>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _TessFunctionResultCallback_5_1<false,R,P1,P2,P3,P4,P5,A1>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_1<false,R,P1,P2,P3,P4,P5,A1>(function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
class _ConstTessMemberResultCallback_6_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
class _ConstTessMemberResultCallback_6_1<del, void, T, P1, P2, P3, P4, P5, P6, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_1(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
inline typename _ConstTessMemberResultCallback_6_1<true,R,T1,P1,P2,P3,P4,P5,P6,A1>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_1<true,R,T1,P1,P2,P3,P4,P5,P6,A1>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
inline typename _ConstTessMemberResultCallback_6_1<false,R,T1,P1,P2,P3,P4,P5,P6,A1>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_1<false,R,T1,P1,P2,P3,P4,P5,P6,A1>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
class _TessMemberResultCallback_6_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
class _TessMemberResultCallback_6_1<del, void, T, P1, P2, P3, P4, P5, P6, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_1(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
inline typename _TessMemberResultCallback_6_1<true,R,T1,P1,P2,P3,P4,P5,P6,A1>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_1<true,R,T1,P1,P2,P3,P4,P5,P6,A1>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
inline typename _TessMemberResultCallback_6_1<false,R,T1,P1,P2,P3,P4,P5,P6,A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_1<false,R,T1,P1,P2,P3,P4,P5,P6,A1>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
class _TessFunctionResultCallback_6_1 : public TessResultCallback1<R,A1> {
 public:
  typedef TessResultCallback1<R,A1> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_1(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
class _TessFunctionResultCallback_6_1<del, void, P1, P2, P3, P4, P5, P6, A1> : public TessCallback1<A1> {
 public:
  typedef TessCallback1<A1> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_1(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
inline typename _TessFunctionResultCallback_6_1<true,R,P1,P2,P3,P4,P5,P6,A1>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_1<true,R,P1,P2,P3,P4,P5,P6,A1>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1>
inline typename _TessFunctionResultCallback_6_1<false,R,P1,P2,P3,P4,P5,P6,A1>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_1<false,R,P1,P2,P3,P4,P5,P6,A1>(function, p1, p2, p3, p4, p5, p6);
}

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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _TessMemberResultCallback_0_2<true,R,T1,A1,A2>::base*
NewTessCallback(
     T1* obj, R (T2::*member)(A1,A2)) {
  return new _TessMemberResultCallback_0_2<true,R,T1,A1,A2>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _TessMemberResultCallback_0_2<false,R,T1,A1,A2>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)(A1,A2)) {
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
      function_ = nullptr;
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
      function_ = nullptr;
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

template <bool del, class R, class T, class P1, class A1, class A2>
class _ConstTessMemberResultCallback_1_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_2(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2>
class _ConstTessMemberResultCallback_1_2<del, void, T, P1, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_2(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2);
    } else {
      (object_->*member_)(p1_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _ConstTessMemberResultCallback_1_2<true,R,T1,P1,A1,A2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,A1,A2) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_2<true,R,T1,P1,A1,A2>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _ConstTessMemberResultCallback_1_2<false,R,T1,P1,A1,A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,A1,A2) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_2<false,R,T1,P1,A1,A2>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1, class A2>
class _TessMemberResultCallback_1_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_2(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2>
class _TessMemberResultCallback_1_2<del, void, T, P1, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_2(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2);
    } else {
      (object_->*member_)(p1_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _TessMemberResultCallback_1_2<true,R,T1,P1,A1,A2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,A1,A2) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_2<true,R,T1,P1,A1,A2>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _TessMemberResultCallback_1_2<false,R,T1,P1,A1,A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,A1,A2) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_2<false,R,T1,P1,A1,A2>(obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2>
class _TessFunctionResultCallback_1_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(P1,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_2(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(p1_,a1,a2);
      return result;
    } else {
      R result = (*function_)(p1_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2>
class _TessFunctionResultCallback_1_2<del, void, P1, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(P1,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_2(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(p1_,a1,a2);
    } else {
      (*function_)(p1_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2>
inline typename _TessFunctionResultCallback_1_2<true,R,P1,A1,A2>::base*
NewTessCallback(R (*function)(P1,A1,A2), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_2<true,R,P1,A1,A2>(function, p1);
}

template <class R, class P1, class A1, class A2>
inline typename _TessFunctionResultCallback_1_2<false,R,P1,A1,A2>::base*
NewPermanentTessCallback(R (*function)(P1,A1,A2), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_2<false,R,P1,A1,A2>(function, p1);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2>
class _ConstTessMemberResultCallback_2_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_2(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2>
class _ConstTessMemberResultCallback_2_2<del, void, T, P1, P2, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_2(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _ConstTessMemberResultCallback_2_2<true,R,T1,P1,P2,A1,A2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_2<true,R,T1,P1,P2,A1,A2>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _ConstTessMemberResultCallback_2_2<false,R,T1,P1,P2,A1,A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_2<false,R,T1,P1,P2,A1,A2>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2>
class _TessMemberResultCallback_2_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_2(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2>
class _TessMemberResultCallback_2_2<del, void, T, P1, P2, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_2(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _TessMemberResultCallback_2_2<true,R,T1,P1,P2,A1,A2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_2<true,R,T1,P1,P2,A1,A2>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _TessMemberResultCallback_2_2<false,R,T1,P1,P2,A1,A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_2<false,R,T1,P1,P2,A1,A2>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2>
class _TessFunctionResultCallback_2_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(P1,P2,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_2(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(p1_,p2_,a1,a2);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2>
class _TessFunctionResultCallback_2_2<del, void, P1, P2, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(P1,P2,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_2(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(p1_,p2_,a1,a2);
    } else {
      (*function_)(p1_,p2_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2>
inline typename _TessFunctionResultCallback_2_2<true,R,P1,P2,A1,A2>::base*
NewTessCallback(R (*function)(P1,P2,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_2<true,R,P1,P2,A1,A2>(function, p1, p2);
}

template <class R, class P1, class P2, class A1, class A2>
inline typename _TessFunctionResultCallback_2_2<false,R,P1,P2,A1,A2>::base*
NewPermanentTessCallback(R (*function)(P1,P2,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_2<false,R,P1,P2,A1,A2>(function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2>
class _ConstTessMemberResultCallback_3_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2>
class _ConstTessMemberResultCallback_3_2<del, void, T, P1, P2, P3, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2>
inline typename _ConstTessMemberResultCallback_3_2<true,R,T1,P1,P2,P3,A1,A2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_2<true,R,T1,P1,P2,P3,A1,A2>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2>
inline typename _ConstTessMemberResultCallback_3_2<false,R,T1,P1,P2,P3,A1,A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_2<false,R,T1,P1,P2,P3,A1,A2>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2>
class _TessMemberResultCallback_3_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2>
class _TessMemberResultCallback_3_2<del, void, T, P1, P2, P3, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2>
inline typename _TessMemberResultCallback_3_2<true,R,T1,P1,P2,P3,A1,A2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_2<true,R,T1,P1,P2,P3,A1,A2>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2>
inline typename _TessMemberResultCallback_3_2<false,R,T1,P1,P2,P3,A1,A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_2<false,R,T1,P1,P2,P3,A1,A2>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2>
class _TessFunctionResultCallback_3_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(P1,P2,P3,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_2(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,a1,a2);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2>
class _TessFunctionResultCallback_3_2<del, void, P1, P2, P3, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(P1,P2,P3,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_2(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,a1,a2);
    } else {
      (*function_)(p1_,p2_,p3_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2>
inline typename _TessFunctionResultCallback_3_2<true,R,P1,P2,P3,A1,A2>::base*
NewTessCallback(R (*function)(P1,P2,P3,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_2<true,R,P1,P2,P3,A1,A2>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2>
inline typename _TessFunctionResultCallback_3_2<false,R,P1,P2,P3,A1,A2>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_2<false,R,P1,P2,P3,A1,A2>(function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2>
class _ConstTessMemberResultCallback_4_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2>
class _ConstTessMemberResultCallback_4_2<del, void, T, P1, P2, P3, P4, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _ConstTessMemberResultCallback_4_2<true,R,T1,P1,P2,P3,P4,A1,A2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_2<true,R,T1,P1,P2,P3,P4,A1,A2>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _ConstTessMemberResultCallback_4_2<false,R,T1,P1,P2,P3,P4,A1,A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_2<false,R,T1,P1,P2,P3,P4,A1,A2>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2>
class _TessMemberResultCallback_4_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2>
class _TessMemberResultCallback_4_2<del, void, T, P1, P2, P3, P4, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _TessMemberResultCallback_4_2<true,R,T1,P1,P2,P3,P4,A1,A2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_2<true,R,T1,P1,P2,P3,P4,A1,A2>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _TessMemberResultCallback_4_2<false,R,T1,P1,P2,P3,P4,A1,A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_2<false,R,T1,P1,P2,P3,P4,A1,A2>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1, class A2>
class _TessFunctionResultCallback_4_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_2(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2>
class _TessFunctionResultCallback_4_2<del, void, P1, P2, P3, P4, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_2(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _TessFunctionResultCallback_4_2<true,R,P1,P2,P3,P4,A1,A2>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_2<true,R,P1,P2,P3,P4,A1,A2>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _TessFunctionResultCallback_4_2<false,R,P1,P2,P3,P4,A1,A2>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_2<false,R,P1,P2,P3,P4,A1,A2>(function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
class _ConstTessMemberResultCallback_5_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
class _ConstTessMemberResultCallback_5_2<del, void, T, P1, P2, P3, P4, P5, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
inline typename _ConstTessMemberResultCallback_5_2<true,R,T1,P1,P2,P3,P4,P5,A1,A2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_2<true,R,T1,P1,P2,P3,P4,P5,A1,A2>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
inline typename _ConstTessMemberResultCallback_5_2<false,R,T1,P1,P2,P3,P4,P5,A1,A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_2<false,R,T1,P1,P2,P3,P4,P5,A1,A2>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
class _TessMemberResultCallback_5_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
class _TessMemberResultCallback_5_2<del, void, T, P1, P2, P3, P4, P5, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
inline typename _TessMemberResultCallback_5_2<true,R,T1,P1,P2,P3,P4,P5,A1,A2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_2<true,R,T1,P1,P2,P3,P4,P5,A1,A2>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
inline typename _TessMemberResultCallback_5_2<false,R,T1,P1,P2,P3,P4,P5,A1,A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_2<false,R,T1,P1,P2,P3,P4,P5,A1,A2>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
class _TessFunctionResultCallback_5_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_2(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
class _TessFunctionResultCallback_5_2<del, void, P1, P2, P3, P4, P5, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_2(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
inline typename _TessFunctionResultCallback_5_2<true,R,P1,P2,P3,P4,P5,A1,A2>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_2<true,R,P1,P2,P3,P4,P5,A1,A2>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2>
inline typename _TessFunctionResultCallback_5_2<false,R,P1,P2,P3,P4,P5,A1,A2>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_2<false,R,P1,P2,P3,P4,P5,A1,A2>(function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
class _ConstTessMemberResultCallback_6_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
class _ConstTessMemberResultCallback_6_2<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_2(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
inline typename _ConstTessMemberResultCallback_6_2<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_2<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
inline typename _ConstTessMemberResultCallback_6_2<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_2<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
class _TessMemberResultCallback_6_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
class _TessMemberResultCallback_6_2<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_2(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
inline typename _TessMemberResultCallback_6_2<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_2<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
inline typename _TessMemberResultCallback_6_2<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_2<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
class _TessFunctionResultCallback_6_2 : public TessResultCallback2<R,A1,A2> {
 public:
  typedef TessResultCallback2<R,A1,A2> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_2(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
class _TessFunctionResultCallback_6_2<del, void, P1, P2, P3, P4, P5, P6, A1, A2> : public TessCallback2<A1,A2> {
 public:
  typedef TessCallback2<A1,A2> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_2(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
inline typename _TessFunctionResultCallback_6_2<true,R,P1,P2,P3,P4,P5,P6,A1,A2>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_2<true,R,P1,P2,P3,P4,P5,P6,A1,A2>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2>
inline typename _TessFunctionResultCallback_6_2<false,R,P1,P2,P3,P4,P5,P6,A1,A2>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_2<false,R,P1,P2,P3,P4,P5,P6,A1,A2>(function, p1, p2, p3, p4, p5, p6);
}

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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_0_3<true,R,T1,A1,A2,A3>::base*
NewTessCallback(
     T1* obj, R (T2::*member)(A1,A2,A3)) {
  return new _TessMemberResultCallback_0_3<true,R,T1,A1,A2,A3>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_0_3<false,R,T1,A1,A2,A3>::base*
NewPermanentTessCallback(
     T1* obj, R (T2::*member)(A1,A2,A3)) {
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
      function_ = nullptr;
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
      function_ = nullptr;
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
      member_ = nullptr;
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
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_1_3<false,R,T1,P1,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
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
      member_ = nullptr;
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
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_3<true,R,T1,P1,A1,A2,A3>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_1_3<false,R,T1,P1,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3) , typename Identity<P1>::type p1) {
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
      function_ = nullptr;
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
      function_ = nullptr;
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

template <bool del, class R, class T, class P1, class P2, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_2_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_3(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_2_3<del, void, T, P1, P2, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_3(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_2_3<true,R,T1,P1,P2,A1,A2,A3>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_3<true,R,T1,P1,P2,A1,A2,A3>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_2_3<false,R,T1,P1,P2,A1,A2,A3>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_3<false,R,T1,P1,P2,A1,A2,A3>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2, class A3>
class _TessMemberResultCallback_2_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_3(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }


  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3>
class _TessMemberResultCallback_2_3<del, void, T, P1, P2, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_3(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_2_3<true,R,T1,P1,P2,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_3<true,R,T1,P1,P2,A1,A2,A3>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_2_3<false,R,T1,P1,P2,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_3<false,R,T1,P1,P2,A1,A2,A3>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2, class A3>
class _TessFunctionResultCallback_2_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(P1,P2,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_3(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (*function_)(p1_,p2_,a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2, class A3>
class _TessFunctionResultCallback_2_3<del, void, P1, P2, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(P1,P2,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_3(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (*function_)(p1_,p2_,a1,a2,a3);
    } else {
      (*function_)(p1_,p2_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_2_3<true,R,P1,P2,A1,A2,A3>::base*
NewTessCallback(R (*function)(P1,P2,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_3<true,R,P1,P2,A1,A2,A3>(function, p1, p2);
}

template <class R, class P1, class P2, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_2_3<false,R,P1,P2,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(P1,P2,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_3<false,R,P1,P2,A1,A2,A3>(function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_3_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_3_3<del, void, T, P1, P2, P3, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_3_3<true,R,T1,P1,P2,P3,A1,A2,A3>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_3<true,R,T1,P1,P2,P3,A1,A2,A3>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_3_3<false,R,T1,P1,P2,P3,A1,A2,A3>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_3<false,R,T1,P1,P2,P3,A1,A2,A3>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2, class A3>
class _TessMemberResultCallback_3_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2, class A3>
class _TessMemberResultCallback_3_3<del, void, T, P1, P2, P3, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_3_3<true,R,T1,P1,P2,P3,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_3<true,R,T1,P1,P2,P3,A1,A2,A3>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_3_3<false,R,T1,P1,P2,P3,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_3<false,R,T1,P1,P2,P3,A1,A2,A3>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2, class A3>
class _TessFunctionResultCallback_3_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(P1,P2,P3,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_3(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2, class A3>
class _TessFunctionResultCallback_3_3<del, void, P1, P2, P3, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(P1,P2,P3,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_3(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,a1,a2,a3);
    } else {
      (*function_)(p1_,p2_,p3_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_3_3<true,R,P1,P2,P3,A1,A2,A3>::base*
NewTessCallback(R (*function)(P1,P2,P3,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_3<true,R,P1,P2,P3,A1,A2,A3>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_3_3<false,R,P1,P2,P3,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_3<false,R,P1,P2,P3,A1,A2,A3>(function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_4_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_4_3<del, void, T, P1, P2, P3, P4, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_4_3<true,R,T1,P1,P2,P3,P4,A1,A2,A3>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_3<true,R,T1,P1,P2,P3,P4,A1,A2,A3>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_4_3<false,R,T1,P1,P2,P3,P4,A1,A2,A3>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_3<false,R,T1,P1,P2,P3,P4,A1,A2,A3>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
class _TessMemberResultCallback_4_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
class _TessMemberResultCallback_4_3<del, void, T, P1, P2, P3, P4, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_4_3<true,R,T1,P1,P2,P3,P4,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_3<true,R,T1,P1,P2,P3,P4,A1,A2,A3>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_4_3<false,R,T1,P1,P2,P3,P4,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_3<false,R,T1,P1,P2,P3,P4,A1,A2,A3>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
class _TessFunctionResultCallback_4_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_3(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
class _TessFunctionResultCallback_4_3<del, void, P1, P2, P3, P4, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_3(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_4_3<true,R,P1,P2,P3,P4,A1,A2,A3>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_3<true,R,P1,P2,P3,P4,A1,A2,A3>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_4_3<false,R,P1,P2,P3,P4,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_3<false,R,P1,P2,P3,P4,A1,A2,A3>(function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_5_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_5_3<del, void, T, P1, P2, P3, P4, P5, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_5_3<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_3<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_5_3<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_3<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
class _TessMemberResultCallback_5_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
class _TessMemberResultCallback_5_3<del, void, T, P1, P2, P3, P4, P5, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_5_3<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_3<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_5_3<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_3<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
class _TessFunctionResultCallback_5_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_3(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
class _TessFunctionResultCallback_5_3<del, void, P1, P2, P3, P4, P5, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_3(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_5_3<true,R,P1,P2,P3,P4,P5,A1,A2,A3>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_3<true,R,P1,P2,P3,P4,P5,A1,A2,A3>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_5_3<false,R,P1,P2,P3,P4,P5,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_3<false,R,P1,P2,P3,P4,P5,A1,A2,A3>(function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_6_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_6_3<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_3(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_6_3<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_3<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
inline typename _ConstTessMemberResultCallback_6_3<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_3<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
class _TessMemberResultCallback_6_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
class _TessMemberResultCallback_6_3<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_3(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_6_3<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_3<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
inline typename _TessMemberResultCallback_6_3<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_3<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
class _TessFunctionResultCallback_6_3 : public TessResultCallback3<R,A1,A2,A3> {
 public:
  typedef TessResultCallback3<R,A1,A2,A3> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_3(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
class _TessFunctionResultCallback_6_3<del, void, P1, P2, P3, P4, P5, P6, A1, A2, A3> : public TessCallback3<A1,A2,A3> {
 public:
  typedef TessCallback3<A1,A2,A3> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_3(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_6_3<true,R,P1,P2,P3,P4,P5,P6,A1,A2,A3>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_3<true,R,P1,P2,P3,P4,P5,P6,A1,A2,A3>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3>
inline typename _TessFunctionResultCallback_6_3<false,R,P1,P2,P3,P4,P5,P6,A1,A2,A3>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2,A3), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_3<false,R,P1,P2,P3,P4,P5,P6,A1,A2,A3>(function, p1, p2, p3, p4, p5, p6);
}

template <bool del, class R, class T, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_0_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_4(const T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_0_4<del, void, T, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_4(const T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(a1,a2,a3,a4);
    } else {
      (object_->*member_)(a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_0_4<true,R,T1,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(A1,A2,A3,A4) const) {
  return new _ConstTessMemberResultCallback_0_4<true,R,T1,A1,A2,A3,A4>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_0_4<false,R,T1,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(A1,A2,A3,A4) const) {
  return new _ConstTessMemberResultCallback_0_4<false,R,T1,A1,A2,A3,A4>(obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_0_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_4(T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2,a3,a4);
     //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_0_4<del, void, T, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_4(T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(a1,a2,a3,a4);
    } else {
      (object_->*member_)(a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_0_4<true,R,T1,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(A1,A2,A3,A4)) {
  return new _TessMemberResultCallback_0_4<true,R,T1,A1,A2,A3,A4>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_0_4<false,R,T1,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(A1,A2,A3,A4)) {
  return new _TessMemberResultCallback_0_4<false,R,T1,A1,A2,A3,A4>(obj, member);
}
#endif

template <bool del, class R, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_0_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(A1,A2,A3,A4);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_4(FunctionSignature function)
    : function_(function) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_0_4<del, void, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(A1,A2,A3,A4);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_4(FunctionSignature function)
    : function_(function) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(a1,a2,a3,a4);
    } else {
      (*function_)(a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_0_4<true,R,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(A1,A2,A3,A4)) {
  return new _TessFunctionResultCallback_0_4<true,R,A1,A2,A3,A4>(function);
}

template <class R, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_0_4<false,R,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(A1,A2,A3,A4)) {
  return new _TessFunctionResultCallback_0_4<false,R,A1,A2,A3,A4>(function);
}

template <bool del, class R, class T, class P1, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_1_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_4(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_1_4<del, void, T, P1, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_4(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_1_4<true,R,T1,P1,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,A1,A2,A3,A4) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_4<true,R,T1,P1,A1,A2,A3,A4>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_1_4<false,R,T1,P1,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,A1,A2,A3,A4) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_4<false,R,T1,P1,A1,A2,A3,A4>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_1_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_4(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_1_4<del, void, T, P1, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_4(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_1_4<true,R,T1,P1,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3,A4) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_4<true,R,T1,P1,A1,A2,A3,A4>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_1_4<false,R,T1,P1,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3,A4) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_4<false,R,T1,P1,A1,A2,A3,A4>(obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_1_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(P1,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_4(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(p1_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(p1_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_1_4<del, void, P1, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(P1,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_4(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(p1_,a1,a2,a3,a4);
    } else {
      (*function_)(p1_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_1_4<true,R,P1,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(P1,A1,A2,A3,A4), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_4<true,R,P1,A1,A2,A3,A4>(function, p1);
}

template <class R, class P1, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_1_4<false,R,P1,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(P1,A1,A2,A3,A4), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_4<false,R,P1,A1,A2,A3,A4>(function, p1);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_2_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_4(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_2_4<del, void, T, P1, P2, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_4(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_2_4<true,R,T1,P1,P2,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_4<true,R,T1,P1,P2,A1,A2,A3,A4>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_2_4<false,R,T1,P1,P2,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_4<false,R,T1,P1,P2,A1,A2,A3,A4>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_2_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_4(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_2_4<del, void, T, P1, P2, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_4(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_2_4<true,R,T1,P1,P2,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_4<true,R,T1,P1,P2,A1,A2,A3,A4>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_2_4<false,R,T1,P1,P2,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_4<false,R,T1,P1,P2,A1,A2,A3,A4>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_2_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(P1,P2,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_4(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(p1_,p2_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_2_4<del, void, P1, P2, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(P1,P2,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_4(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(p1_,p2_,a1,a2,a3,a4);
    } else {
      (*function_)(p1_,p2_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_2_4<true,R,P1,P2,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(P1,P2,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_4<true,R,P1,P2,A1,A2,A3,A4>(function, p1, p2);
}

template <class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_2_4<false,R,P1,P2,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(P1,P2,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_4<false,R,P1,P2,A1,A2,A3,A4>(function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_3_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_3_4<del, void, T, P1, P2, P3, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_3_4<true,R,T1,P1,P2,P3,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_4<true,R,T1,P1,P2,P3,A1,A2,A3,A4>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_3_4<false,R,T1,P1,P2,P3,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_4<false,R,T1,P1,P2,P3,A1,A2,A3,A4>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_3_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_3_4<del, void, T, P1, P2, P3, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_3_4<true,R,T1,P1,P2,P3,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_4<true,R,T1,P1,P2,P3,A1,A2,A3,A4>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_3_4<false,R,T1,P1,P2,P3,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_4<false,R,T1,P1,P2,P3,A1,A2,A3,A4>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_3_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(P1,P2,P3,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_4(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_3_4<del, void, P1, P2, P3, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(P1,P2,P3,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_4(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,a1,a2,a3,a4);
    } else {
      (*function_)(p1_,p2_,p3_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_3_4<true,R,P1,P2,P3,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(P1,P2,P3,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_4<true,R,P1,P2,P3,A1,A2,A3,A4>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_3_4<false,R,P1,P2,P3,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_4<false,R,P1,P2,P3,A1,A2,A3,A4>(function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_4_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_4_4<del, void, T, P1, P2, P3, P4, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_4_4<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_4<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_4_4<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_4<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_4_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_4_4<del, void, T, P1, P2, P3, P4, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_4_4<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_4<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_4_4<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_4<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_4_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_4(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_4_4<del, void, P1, P2, P3, P4, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_4(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_4_4<true,R,P1,P2,P3,P4,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_4<true,R,P1,P2,P3,P4,A1,A2,A3,A4>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_4_4<false,R,P1,P2,P3,P4,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_4<false,R,P1,P2,P3,P4,A1,A2,A3,A4>(function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_5_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_5_4<del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_5_4<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_4<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_5_4<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_4<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_5_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_5_4<del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_5_4<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_4<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_5_4<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_4<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_5_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_4(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_5_4<del, void, P1, P2, P3, P4, P5, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_4(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_5_4<true,R,P1,P2,P3,P4,P5,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_4<true,R,P1,P2,P3,P4,P5,A1,A2,A3,A4>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_5_4<false,R,P1,P2,P3,P4,P5,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_4<false,R,P1,P2,P3,P4,P5,A1,A2,A3,A4>(function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_6_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
class _ConstTessMemberResultCallback_6_4<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_4(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_6_4<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_4<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _ConstTessMemberResultCallback_6_4<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_4<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_6_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_6_4<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_4(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_6_4<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_4<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _TessMemberResultCallback_6_4<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_4<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_6_4 : public TessResultCallback4<R,A1,A2,A3,A4> {
 public:
  typedef TessResultCallback4<R,A1,A2,A3,A4> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_4(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
class _TessFunctionResultCallback_6_4<del, void, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4> : public TessCallback4<A1,A2,A3,A4> {
 public:
  typedef TessCallback4<A1,A2,A3,A4> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_4(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_6_4<true,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_4<true,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _TessFunctionResultCallback_6_4<false,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_4<false,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4>(function, p1, p2, p3, p4, p5, p6);
}

template <class A1,class A2,class A3,class A4,class A5>
class TessCallback5 {
 public:
  virtual ~TessCallback5() { }
  virtual void Run(A1,A2,A3,A4,A5) = 0;
};

template <class R, class A1,class A2,class A3,class A4,class A5>
class TessResultCallback5 {
 public:
  virtual ~TessResultCallback5() { }
  virtual R Run(A1,A2,A3,A4,A5) = 0;
};

template <bool del, class R, class T, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_0_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_5(const T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_0_5<del, void, T, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_5(const T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_0_5<true,R,T1,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(A1,A2,A3,A4,A5) const) {
  return new _ConstTessMemberResultCallback_0_5<true,R,T1,A1,A2,A3,A4,A5>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_0_5<false,R,T1,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(A1,A2,A3,A4,A5) const) {
  return new _ConstTessMemberResultCallback_0_5<false,R,T1,A1,A2,A3,A4,A5>(obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_0_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_5(T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_0_5<del, void, T, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_5(T* object, MemberSignature member)
    : object_(object),
      member_(member) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_0_5<true,R,T1,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(A1,A2,A3,A4,A5)) {
  return new _TessMemberResultCallback_0_5<true,R,T1,A1,A2,A3,A4,A5>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_0_5<false,R,T1,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(A1,A2,A3,A4,A5)) {
  return new _TessMemberResultCallback_0_5<false,R,T1,A1,A2,A3,A4,A5>(obj, member);
}
#endif

template <bool del, class R, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_0_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_5(FunctionSignature function)
    : function_(function) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_0_5<del, void, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;

 public:
  inline _TessFunctionResultCallback_0_5(FunctionSignature function)
    : function_(function) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(a1,a2,a3,a4,a5);
    } else {
      (*function_)(a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_0_5<true,R,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(A1,A2,A3,A4,A5)) {
  return new _TessFunctionResultCallback_0_5<true,R,A1,A2,A3,A4,A5>(function);
}

template <class R, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_0_5<false,R,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(A1,A2,A3,A4,A5)) {
  return new _TessFunctionResultCallback_0_5<false,R,A1,A2,A3,A4,A5>(function);
}

template <bool del, class R, class T, class P1, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_1_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_5(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_1_5<del, void, T, P1, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstTessMemberResultCallback_1_5(const T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_1_5<true,R,T1,P1,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_5<true,R,T1,P1,A1,A2,A3,A4,A5>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_1_5<false,R,T1,P1,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1) {
  return new _ConstTessMemberResultCallback_1_5<false,R,T1,P1,A1,A2,A3,A4,A5>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_1_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_5(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_1_5<del, void, T, P1, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessMemberResultCallback_1_5(T* object, MemberSignature member, P1 p1)
    : object_(object),
      member_(member),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_1_5<true,R,T1,P1,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_5<true,R,T1,P1,A1,A2,A3,A4,A5>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_1_5<false,R,T1,P1,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1) {
  return new _TessMemberResultCallback_1_5<false,R,T1,P1,A1,A2,A3,A4,A5>(obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_1_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(P1,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_5(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(p1_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(p1_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_1_5<del, void, P1, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(P1,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_5(FunctionSignature function, P1 p1)
    : function_(function),      p1_(p1) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(p1_,a1,a2,a3,a4,a5);
    } else {
      (*function_)(p1_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_1_5<true,R,P1,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(P1,A1,A2,A3,A4,A5), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_5<true,R,P1,A1,A2,A3,A4,A5>(function, p1);
}

template <class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_1_5<false,R,P1,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(P1,A1,A2,A3,A4,A5), typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_5<false,R,P1,A1,A2,A3,A4,A5>(function, p1);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_2_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_5(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_2_5<del, void, T, P1, P2, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstTessMemberResultCallback_2_5(const T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_2_5<true,R,T1,P1,P2,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_5<true,R,T1,P1,P2,A1,A2,A3,A4,A5>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_2_5<false,R,T1,P1,P2,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _ConstTessMemberResultCallback_2_5<false,R,T1,P1,P2,A1,A2,A3,A4,A5>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_2_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_5(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_2_5<del, void, T, P1, P2, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_5(T* object, MemberSignature member, P1 p1, P2 p2)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_2_5<true,R,T1,P1,P2,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_5<true,R,T1,P1,P2,A1,A2,A3,A4,A5>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_2_5<false,R,T1,P1,P2,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_5<false,R,T1,P1,P2,A1,A2,A3,A4,A5>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_2_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(P1,P2,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_5(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(p1_,p2_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_2_5<del, void, P1, P2, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(P1,P2,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessFunctionResultCallback_2_5(FunctionSignature function, P1 p1, P2 p2)
    : function_(function),      p1_(p1),      p2_(p2) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(p1_,p2_,a1,a2,a3,a4,a5);
    } else {
      (*function_)(p1_,p2_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_2_5<true,R,P1,P2,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(P1,P2,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_5<true,R,P1,P2,A1,A2,A3,A4,A5>(function, p1, p2);
}

template <class R, class P1, class P2, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_2_5<false,R,P1,P2,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(P1,P2,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2) {
  return new _TessFunctionResultCallback_2_5<false,R,P1,P2,A1,A2,A3,A4,A5>(function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_3_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_3_5<del, void, T, P1, P2, P3, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstTessMemberResultCallback_3_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_3_5<true,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_5<true,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_3_5<false,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _ConstTessMemberResultCallback_3_5<false,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_3_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_3_5<del, void, T, P1, P2, P3, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessMemberResultCallback_3_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_3_5<true,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_5<true,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_3_5<false,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessMemberResultCallback_3_5<false,R,T1,P1,P2,P3,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_3_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(P1,P2,P3,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_5(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_3_5<del, void, P1, P2, P3, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(P1,P2,P3,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _TessFunctionResultCallback_3_5(FunctionSignature function, P1 p1, P2 p2, P3 p3)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
    } else {
      (*function_)(p1_,p2_,p3_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_3_5<true,R,P1,P2,P3,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(P1,P2,P3,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_5<true,R,P1,P2,P3,A1,A2,A3,A4,A5>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_3_5<false,R,P1,P2,P3,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3) {
  return new _TessFunctionResultCallback_3_5<false,R,P1,P2,P3,A1,A2,A3,A4,A5>(function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_4_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_4_5<del, void, T, P1, P2, P3, P4, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstTessMemberResultCallback_4_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_4_5<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_5<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_4_5<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _ConstTessMemberResultCallback_4_5<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_4_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_4_5<del, void, T, P1, P2, P3, P4, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessMemberResultCallback_4_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_4_5<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_5<true,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_4_5<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessMemberResultCallback_4_5<false,R,T1,P1,P2,P3,P4,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_4_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_5(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_4_5<del, void, P1, P2, P3, P4, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _TessFunctionResultCallback_4_5(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_4_5<true,R,P1,P2,P3,P4,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_5<true,R,P1,P2,P3,P4,A1,A2,A3,A4,A5>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_4_5<false,R,P1,P2,P3,P4,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4) {
  return new _TessFunctionResultCallback_4_5<false,R,P1,P2,P3,P4,A1,A2,A3,A4,A5>(function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_5_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }


  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_5_5<del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_5_5<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_5<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_5_5<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_5<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_5_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_5_5<del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessMemberResultCallback_5_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_5_5<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_5<true,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_5_5<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessMemberResultCallback_5_5<false,R,T1,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_5_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_5(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_5_5<del, void, P1, P2, P3, P4, P5, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _TessFunctionResultCallback_5_5(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_5_5<true,R,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_5<true,R,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_5_5<false,R,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5) {
  return new _TessFunctionResultCallback_5_5<false,R,P1,P2,P3,P4,P5,A1,A2,A3,A4,A5>(function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_6_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
class _ConstTessMemberResultCallback_6_5<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _ConstTessMemberResultCallback_6_5(const T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_6_5<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>::base*
NewTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_5<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstTessMemberResultCallback_6_5<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) const, typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_5<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_6_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
class _TessMemberResultCallback_6_5<del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (T::*MemberSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) ;

 private:
   T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessMemberResultCallback_6_5(T* object, MemberSignature member, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : object_(object),
      member_(member),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
    } else {
      (object_->*member_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_6_5<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>::base*
NewTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_5<true,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _TessMemberResultCallback_6_5<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5) , typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessMemberResultCallback_6_5<false,R,T1,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_6_5 : public TessResultCallback5<R,A1,A2,A3,A4,A5> {
 public:
  typedef TessResultCallback5<R,A1,A2,A3,A4,A5> base;
  typedef R (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_5(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual R Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      return result;
    } else {
      R result = (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
class _TessFunctionResultCallback_6_5<del, void, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5> : public TessCallback5<A1,A2,A3,A4,A5> {
 public:
  typedef TessCallback5<A1,A2,A3,A4,A5> base;
  typedef void (*FunctionSignature)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _TessFunctionResultCallback_6_5(FunctionSignature function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    : function_(function),      p1_(p1),      p2_(p2),      p3_(p3),      p4_(p4),      p5_(p5),      p6_(p6) { }

  virtual void Run(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) {
    if (!del) {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
    } else {
      (*function_)(p1_,p2_,p3_,p4_,p5_,p6_,a1,a2,a3,a4,a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_6_5<true,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>::base*
NewTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_5<true,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _TessFunctionResultCallback_6_5<false,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>::base*
NewPermanentTessCallback(R (*function)(P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5), typename Identity<P1>::type p1, typename Identity<P2>::type p2, typename Identity<P3>::type p3, typename Identity<P4>::type p4, typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _TessFunctionResultCallback_6_5<false,R,P1,P2,P3,P4,P5,P6,A1,A2,A3,A4,A5>(function, p1, p2, p3, p4, p5, p6);
}

#endif  // TESS_CALLBACK_SPECIALIZATIONS_H_
