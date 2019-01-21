// Copyright 2010-2014 Google
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

#ifndef OR_TOOLS_BASE_CALLBACK_H_
#define OR_TOOLS_BASE_CALLBACK_H_

//#include "base/logging.h"

namespace operations_research {

struct CallbackUtils_ {
  static void FailIsRepeatable(const char* name);
};

}  // namespace operations_research

using operations_research::CallbackUtils_;

// ----- base types -----

class Closure {
 public:
  virtual ~Closure() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual void Run() = 0;
};

template <class R>
class ResultCallback {
 public:
  virtual ~ResultCallback() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual R Run() = 0;
};

template <class A1>
class Callback1 {
 public:
  virtual ~Callback1() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual void Run(A1) = 0;
};

template <class R, class A1>
class ResultCallback1 {
 public:
  virtual ~ResultCallback1() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual R Run(A1) = 0;
};

template <class A1, class A2>
class Callback2 {
 public:
  virtual ~Callback2() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual void Run(A1, A2) = 0;
};

template <class R, class A1, class A2>
class ResultCallback2 {
 public:
  virtual ~ResultCallback2() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual R Run(A1, A2) = 0;
};

template <class A1, class A2, class A3>
class Callback3 {
 public:
  virtual ~Callback3() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual void Run(A1, A2, A3) = 0;
};

template <class R, class A1, class A2, class A3>
class ResultCallback3 {
 public:
  virtual ~ResultCallback3() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual R Run(A1, A2, A3) = 0;
};

template <class A1, class A2, class A3, class A4>
class Callback4 {
 public:
  virtual ~Callback4() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual void Run(A1, A2, A3, A4) = 0;
};

template <class R, class A1, class A2, class A3, class A4>
class ResultCallback4 {
 public:
  virtual ~ResultCallback4() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual R Run(A1, A2, A3, A4) = 0;
};

template <class A1, class A2, class A3, class A4, class A5>
class Callback5 {
 public:
  virtual ~Callback5() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual void Run(A1, A2, A3, A4, A5) = 0;
};

template <class R, class A1, class A2, class A3, class A4, class A5>
class ResultCallback5 {
 public:
  virtual ~ResultCallback5() {}
  virtual bool IsRepeatable() const { return false; }
  virtual void CheckIsRepeatable() const {}
  virtual R Run(A1, A2, A3, A4, A5) = 0;
};

// ----- Utility template code used by the callback specializations -----

// c_enable_if, equivalent semantics to c++11 std::enable_if, specifically:
//   "If B is true, the member typedef type shall equal T; otherwise, there
//    shall be no member typedef type."
// Specified by 20.9.7.6 [Other transformations]
template <bool cond, class T = void>
struct c_enable_if {
  typedef T type;
};
template <class T>
struct c_enable_if<false, T> {};

typedef char small_;

struct big_ {
  char dummy[2];
};

template <class T>
struct is_class_or_union {
  template <class U>
  static small_ tester(void (U::*)());
  template <class U>
  static big_ tester(...);
  #ifndef SWIG
  static const bool value = sizeof(tester<T>(0)) == sizeof(small_);
  #endif  // SWIG
};

template <typename T>
struct remove_reference {
  typedef T type;
};
template <typename T>
struct remove_reference<T&> {
  typedef T type;
};

template <typename T>
struct ConstRef {
  typedef typename remove_reference<T>::type base_type;
  typedef const base_type& type;
};

// ----- Callback specializations -----

template <bool del, class R, class T, class OnlyIf = typename c_enable_if<
                                          is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_0_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)() const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_0(const T* object, MemberSignature member)
      : ResultCallback<R>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
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
class _ConstMemberResultCallback_0_0<
    del, void, T,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)() const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_0(const T* object, MemberSignature member)
      : Closure(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
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
inline typename _ConstMemberResultCallback_0_0<true, R, T1>::base* NewCallback(
    const T1* obj, R (T2::*member)() const) {
  return new _ConstMemberResultCallback_0_0<true, R, T1>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _ConstMemberResultCallback_0_0<false, R, T1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)() const) {
  return new _ConstMemberResultCallback_0_0<false, R, T1>(obj, member);
}
#endif

template <bool del, class R, class T, class OnlyIf = typename c_enable_if<
                                          is_class_or_union<T>::value>::type>
class _MemberResultCallback_0_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)();

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_0(T* object, MemberSignature member)
      : ResultCallback<R>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
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
class _MemberResultCallback_0_0<
    del, void, T,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)();

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_0(T* object, MemberSignature member)
      : Closure(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
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
inline typename _MemberResultCallback_0_0<true, R, T1>::base* NewCallback(
    T1* obj, R (T2::*member)()) {
  return new _MemberResultCallback_0_0<true, R, T1>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R>
inline typename _MemberResultCallback_0_0<false, R, T1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)()) {
  return new _MemberResultCallback_0_0<false, R, T1>(obj, member);
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
  explicit inline _FunctionResultCallback_0_0(FunctionSignature function)
      : ResultCallback<R>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
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
class _FunctionResultCallback_0_0<del, void> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)();

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_0(FunctionSignature function)
      : Closure(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
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
inline typename _FunctionResultCallback_0_0<true, R>::base* NewCallback(
    R (*function)()) {
  return new _FunctionResultCallback_0_0<true, R>(function);
}

template <class R>
inline typename _FunctionResultCallback_0_0<false, R>::base*
NewPermanentCallback(R (*function)()) {
  return new _FunctionResultCallback_0_0<false, R>(function);
}

template <
    bool del, class R, class T, class P1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_1_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : ResultCallback<R>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_);
      return result;
    } else {
      R result = (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1>
class _ConstMemberResultCallback_1_0<
    del, void, T, P1,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : Closure(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_);
    } else {
      (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _ConstMemberResultCallback_1_0<true, R, T1, P1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1) const,
            typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_0<true, R, T1, P1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _ConstMemberResultCallback_1_0<false, R, T1, P1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1) const,
                     typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_0<false, R, T1, P1>(obj, member, p1);
}
#endif

template <
    bool del, class R, class T, class P1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_1_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : ResultCallback<R>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_);
      return result;
    } else {
      R result = (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1>
class _MemberResultCallback_1_0<
    del, void, T, P1,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : Closure(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_);
    } else {
      (object_->*member_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _MemberResultCallback_1_0<true, R, T1, P1>::base* NewCallback(
    T1* obj, R (T2::*member)(P1), typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_0<true, R, T1, P1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1>
inline typename _MemberResultCallback_1_0<false, R, T1, P1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1),
                     typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_0<false, R, T1, P1>(obj, member, p1);
}
#endif

template <bool del, class R, class P1>
class _FunctionResultCallback_1_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)(P1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : ResultCallback<R>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_);
      return result;
    } else {
      R result = (*function_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1>
class _FunctionResultCallback_1_0<del, void, P1> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)(P1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : Closure(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_);
    } else {
      (*function_)(p1_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1>
inline typename _FunctionResultCallback_1_0<true, R, P1>::base* NewCallback(
    R (*function)(P1), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_0<true, R, P1>(function, p1);
}

template <class R, class P1>
inline typename _FunctionResultCallback_1_0<false, R, P1>::base*
NewPermanentCallback(R (*function)(P1), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_0<false, R, P1>(function, p1);
}

template <
    bool del, class R, class T, class P1, class P2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_2_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2>
class _ConstMemberResultCallback_2_0<
    del, void, T, P1, P2,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : Closure(), object_(object), member_(member), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_);
    } else {
      (object_->*member_)(p1_, p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _ConstMemberResultCallback_2_0<true, R, T1, P1, P2>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_0<true, R, T1, P1, P2>(obj, member,
                                                                 p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _ConstMemberResultCallback_2_0<false, R, T1, P1, P2>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_0<false, R, T1, P1, P2>(obj, member,
                                                                  p1, p2);
}
#endif

template <
    bool del, class R, class T, class P1, class P2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_2_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2>
class _MemberResultCallback_2_0<
    del, void, T, P1, P2,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : Closure(), object_(object), member_(member), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_);
    } else {
      (object_->*member_)(p1_, p2_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _MemberResultCallback_2_0<true, R, T1, P1, P2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_0<true, R, T1, P1, P2>(obj, member, p1,
                                                            p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2>
inline typename _MemberResultCallback_2_0<false, R, T1, P1, P2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_0<false, R, T1, P1, P2>(obj, member, p1,
                                                             p2);
}
#endif

template <bool del, class R, class P1, class P2>
class _FunctionResultCallback_2_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)(P1, P2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : ResultCallback<R>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_, p2_);
      return result;
    } else {
      R result = (*function_)(p1_, p2_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2>
class _FunctionResultCallback_2_0<del, void, P1, P2> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)(P1, P2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : Closure(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_, p2_);
    } else {
      (*function_)(p1_, p2_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2>
inline typename _FunctionResultCallback_2_0<true, R, P1, P2>::base* NewCallback(
    R (*function)(P1, P2), typename ConstRef<P1>::type p1,
    typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_0<true, R, P1, P2>(function, p1, p2);
}

template <class R, class P1, class P2>
inline typename _FunctionResultCallback_2_0<false, R, P1, P2>::base*
NewPermanentCallback(R (*function)(P1, P2), typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_0<false, R, P1, P2>(function, p1, p2);
}

template <
    bool del, class R, class T, class P1, class P2, class P3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_3_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3>
class _ConstMemberResultCallback_3_0<
    del, void, T, P1, P2, P3,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_);
    } else {
      (object_->*member_)(p1_, p2_, p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _ConstMemberResultCallback_3_0<true, R, T1, P1, P2, P3>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_0<true, R, T1, P1, P2, P3>(
      obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _ConstMemberResultCallback_3_0<false, R, T1, P1, P2, P3>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_0<false, R, T1, P1, P2, P3>(
      obj, member, p1, p2, p3);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_3_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3>
class _MemberResultCallback_3_0<
    del, void, T, P1, P2, P3,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_);
    } else {
      (object_->*member_)(p1_, p2_, p3_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _MemberResultCallback_3_0<true, R, T1, P1, P2, P3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_0<true, R, T1, P1, P2, P3>(obj, member, p1,
                                                                p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3>
inline typename _MemberResultCallback_3_0<false, R, T1, P1, P2, P3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_0<false, R, T1, P1, P2, P3>(obj, member,
                                                                 p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3>
class _FunctionResultCallback_3_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)(P1, P2, P3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : ResultCallback<R>(), function_(function), p1_(p1), p2_(p2), p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3>
class _FunctionResultCallback_3_0<del, void, P1, P2, P3> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)(P1, P2, P3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : Closure(), function_(function), p1_(p1), p2_(p2), p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_, p2_, p3_);
    } else {
      (*function_)(p1_, p2_, p3_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3>
inline typename _FunctionResultCallback_3_0<true, R, P1, P2, P3>::base*
NewCallback(R (*function)(P1, P2, P3), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2, typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_0<true, R, P1, P2, P3>(function, p1, p2,
                                                              p3);
}

template <class R, class P1, class P2, class P3>
inline typename _FunctionResultCallback_3_0<false, R, P1, P2, P3>::base*
NewPermanentCallback(R (*function)(P1, P2, P3), typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_0<false, R, P1, P2, P3>(function, p1, p2,
                                                               p3);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_4_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4>
class _ConstMemberResultCallback_4_0<
    del, void, T, P1, P2, P3, P4,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _ConstMemberResultCallback_4_0<true, R, T1, P1, P2, P3,
                                               P4>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_0<true, R, T1, P1, P2, P3, P4>(
      obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _ConstMemberResultCallback_4_0<false, R, T1, P1, P2, P3,
                                               P4>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_0<false, R, T1, P1, P2, P3, P4>(
      obj, member, p1, p2, p3, p4);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_4_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4>
class _MemberResultCallback_4_0<
    del, void, T, P1, P2, P3, P4,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _MemberResultCallback_4_0<true, R, T1, P1, P2, P3, P4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_0<true, R, T1, P1, P2, P3, P4>(
      obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4>
inline typename _MemberResultCallback_4_0<false, R, T1, P1, P2, P3, P4>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_0<false, R, T1, P1, P2, P3, P4>(
      obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4>
class _FunctionResultCallback_4_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : ResultCallback<R>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4>
class _FunctionResultCallback_4_0<del, void, P1, P2, P3, P4> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : Closure(), function_(function), p1_(p1), p2_(p2), p3_(p3), p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_);
    } else {
      (*function_)(p1_, p2_, p3_, p4_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4>
inline typename _FunctionResultCallback_4_0<true, R, P1, P2, P3, P4>::base*
NewCallback(R (*function)(P1, P2, P3, P4), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2, typename ConstRef<P3>::type p3,
            typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_0<true, R, P1, P2, P3, P4>(function, p1,
                                                                  p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4>
inline typename _FunctionResultCallback_4_0<false, R, P1, P2, P3, P4>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_0<false, R, P1, P2, P3, P4>(function, p1,
                                                                   p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_5_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5>
class _ConstMemberResultCallback_5_0<
    del, void, T, P1, P2, P3, P4, P5,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5>
inline typename _ConstMemberResultCallback_5_0<true, R, T1, P1, P2, P3, P4,
                                               P5>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_0<true, R, T1, P1, P2, P3, P4, P5>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5>
inline typename _ConstMemberResultCallback_5_0<false, R, T1, P1, P2, P3, P4,
                                               P5>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_0<false, R, T1, P1, P2, P3, P4, P5>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_5_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5>
class _MemberResultCallback_5_0<
    del, void, T, P1, P2, P3, P4, P5,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5>
inline typename _MemberResultCallback_5_0<true, R, T1, P1, P2, P3, P4,
                                          P5>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_0<true, R, T1, P1, P2, P3, P4, P5>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5>
inline typename _MemberResultCallback_5_0<false, R, T1, P1, P2, P3, P4,
                                          P5>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_0<false, R, T1, P1, P2, P3, P4, P5>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5>
class _FunctionResultCallback_5_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : ResultCallback<R>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5>
class _FunctionResultCallback_5_0<del, void, P1, P2, P3, P4,
                                  P5> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : Closure(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5>
inline typename _FunctionResultCallback_5_0<true, R, P1, P2, P3, P4, P5>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2, typename ConstRef<P3>::type p3,
            typename ConstRef<P4>::type p4, typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_0<true, R, P1, P2, P3, P4, P5>(
      function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5>
inline typename _FunctionResultCallback_5_0<false, R, P1, P2, P3, P4, P5>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_0<false, R, P1, P2, P3, P4, P5>(
      function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class P6, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_6_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6) const;

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
  inline _ConstMemberResultCallback_6_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6>
class _ConstMemberResultCallback_6_0<
    del, void, T, P1, P2, P3, P4, P5, P6,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6) const;

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
  inline _ConstMemberResultCallback_6_0(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6>
inline typename _ConstMemberResultCallback_6_0<true, R, T1, P1, P2, P3, P4, P5,
                                               P6>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_0<
      true, R, T1, P1, P2, P3, P4, P5, P6>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6>
inline typename _ConstMemberResultCallback_6_0<false, R, T1, P1, P2, P3, P4, P5,
                                               P6>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6) const,
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_0<false, R, T1, P1, P2, P3, P4, P5,
                                            P6>(obj, member, p1, p2, p3, p4, p5,
                                                p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class P6, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _MemberResultCallback_6_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6);

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
  inline _MemberResultCallback_6_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : ResultCallback<R>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6>
class _MemberResultCallback_6_0<
    del, void, T, P1, P2, P3, P4, P5, P6,
    typename c_enable_if<is_class_or_union<T>::value>::type> : public Closure {
 public:
  typedef Closure base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6);

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
  inline _MemberResultCallback_6_0(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : Closure(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6>
inline typename _MemberResultCallback_6_0<true, R, T1, P1, P2, P3, P4, P5,
                                          P6>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_0<true, R, T1, P1, P2, P3, P4, P5, P6>(
      obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6>
inline typename _MemberResultCallback_6_0<false, R, T1, P1, P2, P3, P4, P5,
                                          P6>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_0<false, R, T1, P1, P2, P3, P4, P5, P6>(
      obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class P6>
class _FunctionResultCallback_6_0 : public ResultCallback<R> {
 public:
  typedef ResultCallback<R> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, P6);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : ResultCallback<R>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback<R>");
  }

  virtual R Run() {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6>
class _FunctionResultCallback_6_0<del, void, P1, P2, P3, P4, P5,
                                  P6> : public Closure {
 public:
  typedef Closure base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, P6);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_0(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : Closure(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Closure");
  }

  virtual void Run() {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _FunctionResultCallback_6_0<true, R, P1, P2, P3, P4, P5,
                                            P6>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, P6),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_0<true, R, P1, P2, P3, P4, P5, P6>(
      function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6>
inline typename _FunctionResultCallback_6_0<false, R, P1, P2, P3, P4, P5,
                                            P6>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, P6),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_0<false, R, P1, P2, P3, P4, P5, P6>(
      function, p1, p2, p3, p4, p5, p6);
}

template <
    bool del, class R, class T, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_0_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(A1) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_1(const T* object, MemberSignature member)
      : ResultCallback1<R, A1>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
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
class _ConstMemberResultCallback_0_1<
    del, void, T, A1, typename c_enable_if<is_class_or_union<
                          T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(A1) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_1(const T* object, MemberSignature member)
      : Callback1<A1>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
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
inline typename _ConstMemberResultCallback_0_1<true, R, T1, A1>::base*
NewCallback(const T1* obj, R (T2::*member)(A1) const) {
  return new _ConstMemberResultCallback_0_1<true, R, T1, A1>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _ConstMemberResultCallback_0_1<false, R, T1, A1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(A1) const) {
  return new _ConstMemberResultCallback_0_1<false, R, T1, A1>(obj, member);
}
#endif

template <
    bool del, class R, class T, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_0_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(A1);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_1(T* object, MemberSignature member)
      : ResultCallback1<R, A1>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
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
class _MemberResultCallback_0_1<del, void, T, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(A1);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_1(T* object, MemberSignature member)
      : Callback1<A1>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
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
inline typename _MemberResultCallback_0_1<true, R, T1, A1>::base* NewCallback(
    T1* obj, R (T2::*member)(A1)) {
  return new _MemberResultCallback_0_1<true, R, T1, A1>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1>
inline typename _MemberResultCallback_0_1<false, R, T1, A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(A1)) {
  return new _MemberResultCallback_0_1<false, R, T1, A1>(obj, member);
}
#endif

template <bool del, class R, class A1>
class _FunctionResultCallback_0_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(A1);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_1(FunctionSignature function)
      : ResultCallback1<R, A1>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
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
class _FunctionResultCallback_0_1<del, void, A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(A1);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_1(FunctionSignature function)
      : Callback1<A1>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
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
inline typename _FunctionResultCallback_0_1<true, R, A1>::base* NewCallback(
    R (*function)(A1)) {
  return new _FunctionResultCallback_0_1<true, R, A1>(function);
}

template <class R, class A1>
inline typename _FunctionResultCallback_0_1<false, R, A1>::base*
NewPermanentCallback(R (*function)(A1)) {
  return new _FunctionResultCallback_0_1<false, R, A1>(function);
}

template <
    bool del, class R, class T, class P1, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_1_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : ResultCallback1<R, A1>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1>
class _ConstMemberResultCallback_1_1<
    del, void, T, P1, A1, typename c_enable_if<is_class_or_union<
                              T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : Callback1<A1>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, a1);
    } else {
      (object_->*member_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _ConstMemberResultCallback_1_1<true, R, T1, P1, A1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, A1) const,
            typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_1<true, R, T1, P1, A1>(obj, member,
                                                                 p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _ConstMemberResultCallback_1_1<false, R, T1, P1, A1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, A1) const,
                     typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_1<false, R, T1, P1, A1>(obj, member,
                                                                  p1);
}
#endif

template <
    bool del, class R, class T, class P1, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_1_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : ResultCallback1<R, A1>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1>
class _MemberResultCallback_1_1<del, void, T, P1, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : Callback1<A1>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, a1);
    } else {
      (object_->*member_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _MemberResultCallback_1_1<true, R, T1, P1, A1>::base*
NewCallback(T1* obj, R (T2::*member)(P1, A1), typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_1<true, R, T1, P1, A1>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1>
inline typename _MemberResultCallback_1_1<false, R, T1, P1, A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, A1),
                     typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_1<false, R, T1, P1, A1>(obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1>
class _FunctionResultCallback_1_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(P1, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : ResultCallback1<R, A1>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, a1);
      return result;
    } else {
      R result = (*function_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1>
class _FunctionResultCallback_1_1<del, void, P1, A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(P1, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : Callback1<A1>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_, a1);
    } else {
      (*function_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class A1>
inline typename _FunctionResultCallback_1_1<true, R, P1, A1>::base* NewCallback(
    R (*function)(P1, A1), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_1<true, R, P1, A1>(function, p1);
}

template <class R, class P1, class A1>
inline typename _FunctionResultCallback_1_1<false, R, P1, A1>::base*
NewPermanentCallback(R (*function)(P1, A1), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_1<false, R, P1, A1>(function, p1);
}

template <
    bool del, class R, class T, class P1, class P2, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_2_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1>
class _ConstMemberResultCallback_2_1<
    del, void, T, P1, P2, A1, typename c_enable_if<is_class_or_union<
                                  T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : Callback1<A1>(), object_(object), member_(member), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1);
    } else {
      (object_->*member_)(p1_, p2_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _ConstMemberResultCallback_2_1<true, R, T1, P1, P2, A1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, A1) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_1<true, R, T1, P1, P2, A1>(
      obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _ConstMemberResultCallback_2_1<false, R, T1, P1, P2, A1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, A1) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_1<false, R, T1, P1, P2, A1>(
      obj, member, p1, p2);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_2_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1>
class _MemberResultCallback_2_1<del, void, T, P1, P2, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : Callback1<A1>(), object_(object), member_(member), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1);
    } else {
      (object_->*member_)(p1_, p2_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _MemberResultCallback_2_1<true, R, T1, P1, P2, A1>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_1<true, R, T1, P1, P2, A1>(obj, member, p1,
                                                                p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1>
inline typename _MemberResultCallback_2_1<false, R, T1, P1, P2, A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_1<false, R, T1, P1, P2, A1>(obj, member,
                                                                 p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1>
class _FunctionResultCallback_2_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(P1, P2, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : ResultCallback1<R, A1>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, p2_, a1);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1>
class _FunctionResultCallback_2_1<del, void, P1, P2,
                                  A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(P1, P2, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : Callback1<A1>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_, p2_, a1);
    } else {
      (*function_)(p1_, p2_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1>
inline typename _FunctionResultCallback_2_1<true, R, P1, P2, A1>::base*
NewCallback(R (*function)(P1, P2, A1), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_1<true, R, P1, P2, A1>(function, p1, p2);
}

template <class R, class P1, class P2, class A1>
inline typename _FunctionResultCallback_2_1<false, R, P1, P2, A1>::base*
NewPermanentCallback(R (*function)(P1, P2, A1), typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_1<false, R, P1, P2, A1>(function, p1,
                                                               p2);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_3_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1>
class _ConstMemberResultCallback_3_1<
    del, void, T, P1, P2, P3, A1, typename c_enable_if<is_class_or_union<
                                      T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _ConstMemberResultCallback_3_1<true, R, T1, P1, P2, P3,
                                               A1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, A1) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_1<true, R, T1, P1, P2, P3, A1>(
      obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _ConstMemberResultCallback_3_1<false, R, T1, P1, P2, P3,
                                               A1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3, A1) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_1<false, R, T1, P1, P2, P3, A1>(
      obj, member, p1, p2, p3);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class A1,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_3_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1>
class _MemberResultCallback_3_1<del, void, T, P1, P2, P3, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _MemberResultCallback_3_1<true, R, T1, P1, P2, P3, A1>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_1<true, R, T1, P1, P2, P3, A1>(obj, member,
                                                                    p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1>
inline typename _MemberResultCallback_3_1<false, R, T1, P1, P2, P3, A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_1<false, R, T1, P1, P2, P3, A1>(
      obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1>
class _FunctionResultCallback_3_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(P1, P2, P3, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : ResultCallback1<R, A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, a1);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1>
class _FunctionResultCallback_3_1<del, void, P1, P2, P3,
                                  A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(P1, P2, P3, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : Callback1<A1>(), function_(function), p1_(p1), p2_(p2), p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, a1);
    } else {
      (*function_)(p1_, p2_, p3_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1>
inline typename _FunctionResultCallback_3_1<true, R, P1, P2, P3, A1>::base*
NewCallback(R (*function)(P1, P2, P3, A1), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2, typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_1<true, R, P1, P2, P3, A1>(function, p1,
                                                                  p2, p3);
}

template <class R, class P1, class P2, class P3, class A1>
inline typename _FunctionResultCallback_3_1<false, R, P1, P2, P3, A1>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_1<false, R, P1, P2, P3, A1>(function, p1,
                                                                   p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class A1, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_4_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1>
class _ConstMemberResultCallback_4_1<
    del, void, T, P1, P2, P3, P4, A1,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1>
inline typename _ConstMemberResultCallback_4_1<true, R, T1, P1, P2, P3, P4,
                                               A1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, A1) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_1<true, R, T1, P1, P2, P3, P4, A1>(
      obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1>
inline typename _ConstMemberResultCallback_4_1<false, R, T1, P1, P2, P3, P4,
                                               A1>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, A1) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_1<false, R, T1, P1, P2, P3, P4, A1>(
      obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class A1, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_4_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1>
class _MemberResultCallback_4_1<del, void, T, P1, P2, P3, P4, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1>
inline typename _MemberResultCallback_4_1<true, R, T1, P1, P2, P3, P4,
                                          A1>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_1<true, R, T1, P1, P2, P3, P4, A1>(
      obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1>
inline typename _MemberResultCallback_4_1<false, R, T1, P1, P2, P3, P4,
                                          A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_1<false, R, T1, P1, P2, P3, P4, A1>(
      obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1>
class _FunctionResultCallback_4_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : ResultCallback1<R, A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1>
class _FunctionResultCallback_4_1<del, void, P1, P2, P3, P4,
                                  A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : Callback1<A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, a1);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1>
inline typename _FunctionResultCallback_4_1<true, R, P1, P2, P3, P4, A1>::base*
NewCallback(R (*function)(P1, P2, P3, P4, A1), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2, typename ConstRef<P3>::type p3,
            typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_1<true, R, P1, P2, P3, P4, A1>(
      function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1>
inline typename _FunctionResultCallback_4_1<false, R, P1, P2, P3, P4, A1>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_1<false, R, P1, P2, P3, P4, A1>(
      function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class A1, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_5_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1>
class _ConstMemberResultCallback_5_1<
    del, void, T, P1, P2, P3, P4, P5, A1,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1>
inline typename _ConstMemberResultCallback_5_1<true, R, T1, P1, P2, P3, P4, P5,
                                               A1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_1<
      true, R, T1, P1, P2, P3, P4, P5, A1>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1>
inline typename _ConstMemberResultCallback_5_1<false, R, T1, P1, P2, P3, P4, P5,
                                               A1>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, P5, A1) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_1<
      false, R, T1, P1, P2, P3, P4, P5, A1>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class A1, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _MemberResultCallback_5_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1>
class _MemberResultCallback_5_1<del, void, T, P1, P2, P3, P4, P5, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1>
inline typename _MemberResultCallback_5_1<true, R, T1, P1, P2, P3, P4, P5,
                                          A1>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_1<true, R, T1, P1, P2, P3, P4, P5, A1>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1>
inline typename _MemberResultCallback_5_1<false, R, T1, P1, P2, P3, P4, P5,
                                          A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_1<false, R, T1, P1, P2, P3, P4, P5, A1>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class A1>
class _FunctionResultCallback_5_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : ResultCallback1<R, A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1>
class _FunctionResultCallback_5_1<del, void, P1, P2, P3, P4, P5,
                                  A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : Callback1<A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _FunctionResultCallback_5_1<true, R, P1, P2, P3, P4, P5,
                                            A1>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_1<true, R, P1, P2, P3, P4, P5, A1>(
      function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1>
inline typename _FunctionResultCallback_5_1<false, R, P1, P2, P3, P4, P5,
                                            A1>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_1<false, R, P1, P2, P3, P4, P5, A1>(
      function, p1, p2, p3, p4, p5);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_6_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1) const;

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
  inline _ConstMemberResultCallback_6_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1>
class _ConstMemberResultCallback_6_1<
    del, void, T, P1, P2, P3, P4, P5, P6, A1,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1) const;

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
  inline _ConstMemberResultCallback_6_1(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1>
inline typename _ConstMemberResultCallback_6_1<true, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_1<true, R, T1, P1, P2, P3, P4, P5, P6,
                                            A1>(obj, member, p1, p2, p3, p4, p5,
                                                p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1>
inline typename _ConstMemberResultCallback_6_1<false, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1) const,
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_1<
      false, R, T1, P1, P2, P3, P4, P5, P6, A1>(obj, member, p1, p2, p3, p4, p5,
                                                p6);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _MemberResultCallback_6_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1);

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
  inline _MemberResultCallback_6_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : ResultCallback1<R, A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1>
class _MemberResultCallback_6_1<del, void, T, P1, P2, P3, P4, P5, P6, A1,
                                typename c_enable_if<is_class_or_union<
                                    T>::value>::type> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1);

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
  inline _MemberResultCallback_6_1(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : Callback1<A1>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1>
inline typename _MemberResultCallback_6_1<true, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_1<true, R, T1, P1, P2, P3, P4, P5, P6, A1>(
      obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1>
inline typename _MemberResultCallback_6_1<false, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_1<false, R, T1, P1, P2, P3, P4, P5, P6,
                                       A1>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1>
class _FunctionResultCallback_6_1 : public ResultCallback1<R, A1> {
 public:
  typedef ResultCallback1<R, A1> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : ResultCallback1<R, A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback1<R,A1>");
  }

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1>
class _FunctionResultCallback_6_1<del, void, P1, P2, P3, P4, P5, P6,
                                  A1> : public Callback1<A1> {
 public:
  typedef Callback1<A1> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_1(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : Callback1<A1>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback1<A1>");
  }

  virtual void Run(A1 a1) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1>
inline typename _FunctionResultCallback_6_1<true, R, P1, P2, P3, P4, P5, P6,
                                            A1>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_1<true, R, P1, P2, P3, P4, P5, P6, A1>(
      function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1>
inline typename _FunctionResultCallback_6_1<false, R, P1, P2, P3, P4, P5, P6,
                                            A1>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_1<false, R, P1, P2, P3, P4, P5, P6, A1>(
      function, p1, p2, p3, p4, p5, p6);
}

template <
    bool del, class R, class T, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_0_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_2(const T* object, MemberSignature member)
      : ResultCallback2<R, A1, A2>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2>
class _ConstMemberResultCallback_0_2<
    del, void, T, A1, A2, typename c_enable_if<is_class_or_union<
                              T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_2(const T* object, MemberSignature member)
      : Callback2<A1, A2>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(a1, a2);
    } else {
      (object_->*member_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _ConstMemberResultCallback_0_2<true, R, T1, A1, A2>::base*
NewCallback(const T1* obj, R (T2::*member)(A1, A2) const) {
  return new _ConstMemberResultCallback_0_2<true, R, T1, A1, A2>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _ConstMemberResultCallback_0_2<false, R, T1, A1, A2>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(A1, A2) const) {
  return new _ConstMemberResultCallback_0_2<false, R, T1, A1, A2>(obj, member);
}
#endif

template <
    bool del, class R, class T, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_0_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(A1, A2);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_2(T* object, MemberSignature member)
      : ResultCallback2<R, A1, A2>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2>
class _MemberResultCallback_0_2<
    del, void, T, A1, A2, typename c_enable_if<is_class_or_union<
                              T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(A1, A2);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_2(T* object, MemberSignature member)
      : Callback2<A1, A2>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(a1, a2);
    } else {
      (object_->*member_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _MemberResultCallback_0_2<true, R, T1, A1, A2>::base*
NewCallback(T1* obj, R (T2::*member)(A1, A2)) {
  return new _MemberResultCallback_0_2<true, R, T1, A1, A2>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _MemberResultCallback_0_2<false, R, T1, A1, A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(A1, A2)) {
  return new _MemberResultCallback_0_2<false, R, T1, A1, A2>(obj, member);
}
#endif

template <bool del, class R, class A1, class A2>
class _FunctionResultCallback_0_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(A1, A2);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_2(FunctionSignature function)
      : ResultCallback2<R, A1, A2>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(a1, a2);
      return result;
    } else {
      R result = (*function_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2>
class _FunctionResultCallback_0_2<del, void, A1, A2> : public Callback2<A1,
                                                                        A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(A1, A2);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_2(FunctionSignature function)
      : Callback2<A1, A2>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(a1, a2);
    } else {
      (*function_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1, class A2>
inline typename _FunctionResultCallback_0_2<true, R, A1, A2>::base* NewCallback(
    R (*function)(A1, A2)) {
  return new _FunctionResultCallback_0_2<true, R, A1, A2>(function);
}

template <class R, class A1, class A2>
inline typename _FunctionResultCallback_0_2<false, R, A1, A2>::base*
NewPermanentCallback(R (*function)(A1, A2)) {
  return new _FunctionResultCallback_0_2<false, R, A1, A2>(function);
}

template <
    bool del, class R, class T, class P1, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_1_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2>
class _ConstMemberResultCallback_1_2<
    del, void, T, P1, A1, A2, typename c_enable_if<is_class_or_union<
                                  T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : Callback2<A1, A2>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2);
    } else {
      (object_->*member_)(p1_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _ConstMemberResultCallback_1_2<true, R, T1, P1, A1, A2>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, A1, A2) const,
            typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_2<true, R, T1, P1, A1, A2>(
      obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _ConstMemberResultCallback_1_2<false, R, T1, P1, A1, A2>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, A1, A2) const,
                     typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_2<false, R, T1, P1, A1, A2>(
      obj, member, p1);
}
#endif

template <
    bool del, class R, class T, class P1, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_1_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2>
class _MemberResultCallback_1_2<
    del, void, T, P1, A1, A2, typename c_enable_if<is_class_or_union<
                                  T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : Callback2<A1, A2>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2);
    } else {
      (object_->*member_)(p1_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _MemberResultCallback_1_2<true, R, T1, P1, A1, A2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, A1, A2),
            typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_2<true, R, T1, P1, A1, A2>(obj, member,
                                                                p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2>
inline typename _MemberResultCallback_1_2<false, R, T1, P1, A1, A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, A1, A2),
                     typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_2<false, R, T1, P1, A1, A2>(obj, member,
                                                                 p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2>
class _FunctionResultCallback_1_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(P1, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : ResultCallback2<R, A1, A2>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(p1_, a1, a2);
      return result;
    } else {
      R result = (*function_)(p1_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2>
class _FunctionResultCallback_1_2<del, void, P1, A1, A2> : public Callback2<
                                                               A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(P1, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : Callback2<A1, A2>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(p1_, a1, a2);
    } else {
      (*function_)(p1_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2>
inline typename _FunctionResultCallback_1_2<true, R, P1, A1, A2>::base*
NewCallback(R (*function)(P1, A1, A2), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_2<true, R, P1, A1, A2>(function, p1);
}

template <class R, class P1, class A1, class A2>
inline typename _FunctionResultCallback_1_2<false, R, P1, A1, A2>::base*
NewPermanentCallback(R (*function)(P1, A1, A2),
                     typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_2<false, R, P1, A1, A2>(function, p1);
}

template <
    bool del, class R, class T, class P1, class P2, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_2_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2>
class _ConstMemberResultCallback_2_2<
    del, void, T, P1, P2, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _ConstMemberResultCallback_2_2<true, R, T1, P1, P2, A1,
                                               A2>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, A1, A2) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_2<true, R, T1, P1, P2, A1, A2>(
      obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _ConstMemberResultCallback_2_2<false, R, T1, P1, P2, A1,
                                               A2>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, A1, A2) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_2<false, R, T1, P1, P2, A1, A2>(
      obj, member, p1, p2);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_2_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2>
class _MemberResultCallback_2_2<
    del, void, T, P1, P2, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _MemberResultCallback_2_2<true, R, T1, P1, P2, A1, A2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_2<true, R, T1, P1, P2, A1, A2>(obj, member,
                                                                    p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline typename _MemberResultCallback_2_2<false, R, T1, P1, P2, A1, A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_2<false, R, T1, P1, P2, A1, A2>(
      obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2>
class _FunctionResultCallback_2_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(P1, P2, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : ResultCallback2<R, A1, A2>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(p1_, p2_, a1, a2);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2>
class _FunctionResultCallback_2_2<del, void, P1, P2, A1, A2> : public Callback2<
                                                                   A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(P1, P2, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : Callback2<A1, A2>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(p1_, p2_, a1, a2);
    } else {
      (*function_)(p1_, p2_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2>
inline typename _FunctionResultCallback_2_2<true, R, P1, P2, A1, A2>::base*
NewCallback(R (*function)(P1, P2, A1, A2), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_2<true, R, P1, P2, A1, A2>(function, p1,
                                                                  p2);
}

template <class R, class P1, class P2, class A1, class A2>
inline typename _FunctionResultCallback_2_2<false, R, P1, P2, A1, A2>::base*
NewPermanentCallback(R (*function)(P1, P2, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_2<false, R, P1, P2, A1, A2>(function, p1,
                                                                   p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1,
          class A2, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_3_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2>
class _ConstMemberResultCallback_3_2<
    del, void, T, P1, P2, P3, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2>
inline typename _ConstMemberResultCallback_3_2<true, R, T1, P1, P2, P3, A1,
                                               A2>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, A1, A2) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_2<true, R, T1, P1, P2, P3, A1, A2>(
      obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2>
inline typename _ConstMemberResultCallback_3_2<false, R, T1, P1, P2, P3, A1,
                                               A2>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3, A1, A2) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_2<false, R, T1, P1, P2, P3, A1, A2>(
      obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1,
          class A2, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_3_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2>
class _MemberResultCallback_3_2<
    del, void, T, P1, P2, P3, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2>
inline typename _MemberResultCallback_3_2<true, R, T1, P1, P2, P3, A1,
                                          A2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_2<true, R, T1, P1, P2, P3, A1, A2>(
      obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2>
inline typename _MemberResultCallback_3_2<false, R, T1, P1, P2, P3, A1,
                                          A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_2<false, R, T1, P1, P2, P3, A1, A2>(
      obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2>
class _FunctionResultCallback_3_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(P1, P2, P3, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : ResultCallback2<R, A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, a1, a2);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2>
class _FunctionResultCallback_3_2<del, void, P1, P2, P3, A1,
                                  A2> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(P1, P2, P3, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : Callback2<A1, A2>(), function_(function), p1_(p1), p2_(p2), p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, a1, a2);
    } else {
      (*function_)(p1_, p2_, p3_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2>
inline typename _FunctionResultCallback_3_2<true, R, P1, P2, P3, A1, A2>::base*
NewCallback(R (*function)(P1, P2, P3, A1, A2), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2, typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_2<true, R, P1, P2, P3, A1, A2>(
      function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2>
inline typename _FunctionResultCallback_3_2<false, R, P1, P2, P3, A1, A2>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_2<false, R, P1, P2, P3, A1, A2>(
      function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class A1, class A2, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_4_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2>
class _ConstMemberResultCallback_4_2<
    del, void, T, P1, P2, P3, P4, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2>
inline typename _ConstMemberResultCallback_4_2<true, R, T1, P1, P2, P3, P4, A1,
                                               A2>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_2<true, R, T1, P1, P2, P3, P4, A1,
                                            A2>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2>
inline typename _ConstMemberResultCallback_4_2<false, R, T1, P1, P2, P3, P4, A1,
                                               A2>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, A1, A2) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_2<false, R, T1, P1, P2, P3, P4, A1,
                                            A2>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class A1, class A2, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _MemberResultCallback_4_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2>
class _MemberResultCallback_4_2<
    del, void, T, P1, P2, P3, P4, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2>
inline typename _MemberResultCallback_4_2<true, R, T1, P1, P2, P3, P4, A1,
                                          A2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_2<true, R, T1, P1, P2, P3, P4, A1, A2>(
      obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2>
inline typename _MemberResultCallback_4_2<false, R, T1, P1, P2, P3, P4, A1,
                                          A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_2<false, R, T1, P1, P2, P3, P4, A1, A2>(
      obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1,
          class A2>
class _FunctionResultCallback_4_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : ResultCallback2<R, A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2>
class _FunctionResultCallback_4_2<del, void, P1, P2, P3, P4, A1,
                                  A2> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : Callback2<A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _FunctionResultCallback_4_2<true, R, P1, P2, P3, P4, A1,
                                            A2>::base*
NewCallback(R (*function)(P1, P2, P3, P4, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_2<true, R, P1, P2, P3, P4, A1, A2>(
      function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2>
inline typename _FunctionResultCallback_4_2<false, R, P1, P2, P3, P4, A1,
                                            A2>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_2<false, R, P1, P2, P3, P4, A1, A2>(
      function, p1, p2, p3, p4);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_5_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2>
class _ConstMemberResultCallback_5_2<
    del, void, T, P1, P2, P3, P4, P5, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2>
inline typename _ConstMemberResultCallback_5_2<true, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_2<
      true, R, T1, P1, P2, P3, P4, P5, A1, A2>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2>
inline typename _ConstMemberResultCallback_5_2<false, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, P5, A1, A2) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_2<false, R, T1, P1, P2, P3, P4, P5,
                                            A1, A2>(obj, member, p1, p2, p3, p4,
                                                    p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _MemberResultCallback_5_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2>
class _MemberResultCallback_5_2<
    del, void, T, P1, P2, P3, P4, P5, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2>
inline typename _MemberResultCallback_5_2<true, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_2<true, R, T1, P1, P2, P3, P4, P5, A1, A2>(
      obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2>
inline typename _MemberResultCallback_5_2<false, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_2<false, R, T1, P1, P2, P3, P4, P5, A1,
                                       A2>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2>
class _FunctionResultCallback_5_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : ResultCallback2<R, A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2>
class _FunctionResultCallback_5_2<del, void, P1, P2, P3, P4, P5, A1,
                                  A2> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : Callback2<A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2>
inline typename _FunctionResultCallback_5_2<true, R, P1, P2, P3, P4, P5, A1,
                                            A2>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_2<true, R, P1, P2, P3, P4, P5, A1, A2>(
      function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2>
inline typename _FunctionResultCallback_5_2<false, R, P1, P2, P3, P4, P5, A1,
                                            A2>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_2<false, R, P1, P2, P3, P4, P5, A1, A2>(
      function, p1, p2, p3, p4, p5);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_6_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2) const;

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
  inline _ConstMemberResultCallback_6_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2>
class _ConstMemberResultCallback_6_2<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2) const;

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
  inline _ConstMemberResultCallback_6_2(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2>
inline typename _ConstMemberResultCallback_6_2<true, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_2<true, R, T1, P1, P2, P3, P4, P5, P6,
                                            A1, A2>(obj, member, p1, p2, p3, p4,
                                                    p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2>
inline typename _ConstMemberResultCallback_6_2<false, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2) const,
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_2<
      false, R, T1, P1, P2, P3, P4, P5, P6, A1, A2>(obj, member, p1, p2, p3, p4,
                                                    p5, p6);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_6_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2);

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
  inline _MemberResultCallback_6_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : ResultCallback2<R, A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2>
class _MemberResultCallback_6_2<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2);

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
  inline _MemberResultCallback_6_2(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : Callback2<A1, A2>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2>
inline typename _MemberResultCallback_6_2<true, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_2<true, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2>
inline typename _MemberResultCallback_6_2<false, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_2<false, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2>(obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2>
class _FunctionResultCallback_6_2 : public ResultCallback2<R, A1, A2> {
 public:
  typedef ResultCallback2<R, A1, A2> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : ResultCallback2<R, A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback2<R,A1,A2>");
  }

  virtual R Run(A1 a1, A2 a2) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2>
class _FunctionResultCallback_6_2<del, void, P1, P2, P3, P4, P5, P6, A1,
                                  A2> : public Callback2<A1, A2> {
 public:
  typedef Callback2<A1, A2> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_2(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : Callback2<A1, A2>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback2<A1,A2>");
  }

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2>
inline typename _FunctionResultCallback_6_2<true, R, P1, P2, P3, P4, P5, P6, A1,
                                            A2>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_2<true, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2>(function, p1, p2, p3, p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2>
inline typename _FunctionResultCallback_6_2<false, R, P1, P2, P3, P4, P5, P6,
                                            A1, A2>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_2<false, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2>(function, p1, p2, p3, p4, p5, p6);
}

template <
    bool del, class R, class T, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_0_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_3(const T* object, MemberSignature member)
      : ResultCallback3<R, A1, A2, A3>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3>
class _ConstMemberResultCallback_0_3<
    del, void, T, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_3(const T* object, MemberSignature member)
      : Callback3<A1, A2, A3>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(a1, a2, a3);
    } else {
      (object_->*member_)(a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_0_3<true, R, T1, A1, A2, A3>::base*
NewCallback(const T1* obj, R (T2::*member)(A1, A2, A3) const) {
  return new _ConstMemberResultCallback_0_3<true, R, T1, A1, A2, A3>(obj,
                                                                     member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_0_3<false, R, T1, A1, A2, A3>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(A1, A2, A3) const) {
  return new _ConstMemberResultCallback_0_3<false, R, T1, A1, A2, A3>(obj,
                                                                      member);
}
#endif

template <
    bool del, class R, class T, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_0_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_3(T* object, MemberSignature member)
      : ResultCallback3<R, A1, A2, A3>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3>
class _MemberResultCallback_0_3<
    del, void, T, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_3(T* object, MemberSignature member)
      : Callback3<A1, A2, A3>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(a1, a2, a3);
    } else {
      (object_->*member_)(a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _MemberResultCallback_0_3<true, R, T1, A1, A2, A3>::base*
NewCallback(T1* obj, R (T2::*member)(A1, A2, A3)) {
  return new _MemberResultCallback_0_3<true, R, T1, A1, A2, A3>(obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline typename _MemberResultCallback_0_3<false, R, T1, A1, A2, A3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(A1, A2, A3)) {
  return new _MemberResultCallback_0_3<false, R, T1, A1, A2, A3>(obj, member);
}
#endif

template <bool del, class R, class A1, class A2, class A3>
class _FunctionResultCallback_0_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(A1, A2, A3);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_3(FunctionSignature function)
      : ResultCallback3<R, A1, A2, A3>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2, class A3>
class _FunctionResultCallback_0_3<del, void, A1, A2, A3> : public Callback3<
                                                               A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(A1, A2, A3);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_3(FunctionSignature function)
      : Callback3<A1, A2, A3>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(a1, a2, a3);
    } else {
      (*function_)(a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1, class A2, class A3>
inline typename _FunctionResultCallback_0_3<true, R, A1, A2, A3>::base*
NewCallback(R (*function)(A1, A2, A3)) {
  return new _FunctionResultCallback_0_3<true, R, A1, A2, A3>(function);
}

template <class R, class A1, class A2, class A3>
inline typename _FunctionResultCallback_0_3<false, R, A1, A2, A3>::base*
NewPermanentCallback(R (*function)(A1, A2, A3)) {
  return new _FunctionResultCallback_0_3<false, R, A1, A2, A3>(function);
}

template <
    bool del, class R, class T, class P1, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_1_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3>
class _ConstMemberResultCallback_1_3<
    del, void, T, P1, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : Callback3<A1, A2, A3>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_1_3<true, R, T1, P1, A1, A2,
                                               A3>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, A1, A2, A3) const,
            typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_3<true, R, T1, P1, A1, A2, A3>(
      obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_1_3<false, R, T1, P1, A1, A2,
                                               A3>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, A1, A2, A3) const,
                     typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_3<false, R, T1, P1, A1, A2, A3>(
      obj, member, p1);
}
#endif

template <
    bool del, class R, class T, class P1, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_1_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3>
class _MemberResultCallback_1_3<
    del, void, T, P1, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : Callback3<A1, A2, A3>(), object_(object), member_(member), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _MemberResultCallback_1_3<true, R, T1, P1, A1, A2, A3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, A1, A2, A3),
            typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_3<true, R, T1, P1, A1, A2, A3>(obj, member,
                                                                    p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3>
inline typename _MemberResultCallback_1_3<false, R, T1, P1, A1, A2, A3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, A1, A2, A3),
                     typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_3<false, R, T1, P1, A1, A2, A3>(
      obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2, class A3>
class _FunctionResultCallback_1_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(P1, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : ResultCallback3<R, A1, A2, A3>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_, a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(p1_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2, class A3>
class _FunctionResultCallback_1_3<del, void, P1, A1, A2, A3> : public Callback3<
                                                                   A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(P1, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : Callback3<A1, A2, A3>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_, a1, a2, a3);
    } else {
      (*function_)(p1_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2, class A3>
inline typename _FunctionResultCallback_1_3<true, R, P1, A1, A2, A3>::base*
NewCallback(R (*function)(P1, A1, A2, A3), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_3<true, R, P1, A1, A2, A3>(function, p1);
}

template <class R, class P1, class A1, class A2, class A3>
inline typename _FunctionResultCallback_1_3<false, R, P1, A1, A2, A3>::base*
NewPermanentCallback(R (*function)(P1, A1, A2, A3),
                     typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_3<false, R, P1, A1, A2, A3>(function,
                                                                   p1);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2,
          class A3, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_2_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3>
class _ConstMemberResultCallback_2_3<
    del, void, T, P1, P2, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3>
inline typename _ConstMemberResultCallback_2_3<true, R, T1, P1, P2, A1, A2,
                                               A3>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, A1, A2, A3) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_3<true, R, T1, P1, P2, A1, A2, A3>(
      obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3>
inline typename _ConstMemberResultCallback_2_3<false, R, T1, P1, P2, A1, A2,
                                               A3>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, A1, A2, A3) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_3<false, R, T1, P1, P2, A1, A2, A3>(
      obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2,
          class A3, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_2_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3>
class _MemberResultCallback_2_3<
    del, void, T, P1, P2, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3>
inline typename _MemberResultCallback_2_3<true, R, T1, P1, P2, A1, A2,
                                          A3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_3<true, R, T1, P1, P2, A1, A2, A3>(
      obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3>
inline typename _MemberResultCallback_2_3<false, R, T1, P1, P2, A1, A2,
                                          A3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_3<false, R, T1, P1, P2, A1, A2, A3>(
      obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2, class A3>
class _FunctionResultCallback_2_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(P1, P2, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : ResultCallback3<R, A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_, p2_, a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2, class A3>
class _FunctionResultCallback_2_3<del, void, P1, P2, A1, A2,
                                  A3> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(P1, P2, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : Callback3<A1, A2, A3>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_, p2_, a1, a2, a3);
    } else {
      (*function_)(p1_, p2_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2, class A3>
inline typename _FunctionResultCallback_2_3<true, R, P1, P2, A1, A2, A3>::base*
NewCallback(R (*function)(P1, P2, A1, A2, A3), typename ConstRef<P1>::type p1,
            typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_3<true, R, P1, P2, A1, A2, A3>(function,
                                                                      p1, p2);
}

template <class R, class P1, class P2, class A1, class A2, class A3>
inline typename _FunctionResultCallback_2_3<false, R, P1, P2, A1, A2, A3>::base*
NewPermanentCallback(R (*function)(P1, P2, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_3<false, R, P1, P2, A1, A2, A3>(function,
                                                                       p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1,
          class A2, class A3, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_3_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2,
          class A3>
class _ConstMemberResultCallback_3_3<
    del, void, T, P1, P2, P3, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3>
inline typename _ConstMemberResultCallback_3_3<true, R, T1, P1, P2, P3, A1, A2,
                                               A3>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_3<true, R, T1, P1, P2, P3, A1, A2,
                                            A3>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3>
inline typename _ConstMemberResultCallback_3_3<false, R, T1, P1, P2, P3, A1, A2,
                                               A3>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, A1, A2, A3) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_3<false, R, T1, P1, P2, P3, A1, A2,
                                            A3>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1,
          class A2, class A3, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _MemberResultCallback_3_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2,
          class A3>
class _MemberResultCallback_3_3<
    del, void, T, P1, P2, P3, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3>
inline typename _MemberResultCallback_3_3<true, R, T1, P1, P2, P3, A1, A2,
                                          A3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_3<true, R, T1, P1, P2, P3, A1, A2, A3>(
      obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3>
inline typename _MemberResultCallback_3_3<false, R, T1, P1, P2, P3, A1, A2,
                                          A3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_3<false, R, T1, P1, P2, P3, A1, A2, A3>(
      obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2,
          class A3>
class _FunctionResultCallback_3_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(P1, P2, P3, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : ResultCallback3<R, A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2, class A3>
class _FunctionResultCallback_3_3<del, void, P1, P2, P3, A1, A2,
                                  A3> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(P1, P2, P3, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : Callback3<A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, a1, a2, a3);
    } else {
      (*function_)(p1_, p2_, p3_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _FunctionResultCallback_3_3<true, R, P1, P2, P3, A1, A2,
                                            A3>::base*
NewCallback(R (*function)(P1, P2, P3, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_3<true, R, P1, P2, P3, A1, A2, A3>(
      function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2, class A3>
inline typename _FunctionResultCallback_3_3<false, R, P1, P2, P3, A1, A2,
                                            A3>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_3<false, R, P1, P2, P3, A1, A2, A3>(
      function, p1, p2, p3);
}

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_4_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3>
class _ConstMemberResultCallback_4_3<
    del, void, T, P1, P2, P3, P4, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_4_3<true, R, T1, P1, P2, P3, P4, A1,
                                               A2, A3>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2, A3) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_3<true, R, T1, P1, P2, P3, P4, A1, A2,
                                            A3>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_4_3<false, R, T1, P1, P2, P3, P4, A1,
                                               A2, A3>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, A1, A2, A3) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_3<
      false, R, T1, P1, P2, P3, P4, A1, A2, A3>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _MemberResultCallback_4_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3>
class _MemberResultCallback_4_3<
    del, void, T, P1, P2, P3, P4, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3>
inline typename _MemberResultCallback_4_3<true, R, T1, P1, P2, P3, P4, A1, A2,
                                          A3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_3<true, R, T1, P1, P2, P3, P4, A1, A2, A3>(
      obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3>
inline typename _MemberResultCallback_4_3<false, R, T1, P1, P2, P3, P4, A1, A2,
                                          A3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_3<false, R, T1, P1, P2, P3, P4, A1, A2,
                                       A3>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3>
class _FunctionResultCallback_4_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : ResultCallback3<R, A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3>
class _FunctionResultCallback_4_3<del, void, P1, P2, P3, P4, A1, A2,
                                  A3> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : Callback3<A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3>
inline typename _FunctionResultCallback_4_3<true, R, P1, P2, P3, P4, A1, A2,
                                            A3>::base*
NewCallback(R (*function)(P1, P2, P3, P4, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_3<true, R, P1, P2, P3, P4, A1, A2, A3>(
      function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3>
inline typename _FunctionResultCallback_4_3<false, R, P1, P2, P3, P4, A1, A2,
                                            A3>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_3<false, R, P1, P2, P3, P4, A1, A2, A3>(
      function, p1, p2, p3, p4);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_5_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3>
class _ConstMemberResultCallback_5_3<
    del, void, T, P1, P2, P3, P4, P5, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_5_3<true, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2, A3>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_3<true, R, T1, P1, P2, P3, P4, P5, A1,
                                            A2, A3>(obj, member, p1, p2, p3, p4,
                                                    p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_5_3<false, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2, A3>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_3<
      false, R, T1, P1, P2, P3, P4, P5, A1, A2, A3>(obj, member, p1, p2, p3, p4,
                                                    p5);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_5_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3>
class _MemberResultCallback_5_3<
    del, void, T, P1, P2, P3, P4, P5, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3>
inline typename _MemberResultCallback_5_3<true, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2, A3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_3<true, R, T1, P1, P2, P3, P4, P5, A1, A2,
                                       A3>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3>
inline typename _MemberResultCallback_5_3<false, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2, A3>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_3<false, R, T1, P1, P2, P3, P4, P5, A1, A2,
                                       A3>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3>
class _FunctionResultCallback_5_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : ResultCallback3<R, A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3>
class _FunctionResultCallback_5_3<del, void, P1, P2, P3, P4, P5, A1, A2,
                                  A3> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : Callback3<A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3>
inline typename _FunctionResultCallback_5_3<true, R, P1, P2, P3, P4, P5, A1, A2,
                                            A3>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_3<true, R, P1, P2, P3, P4, P5, A1, A2,
                                         A3>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3>
inline typename _FunctionResultCallback_5_3<false, R, P1, P2, P3, P4, P5, A1,
                                            A2, A3>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_3<false, R, P1, P2, P3, P4, P5, A1, A2,
                                         A3>(function, p1, p2, p3, p4, p5);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_6_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3) const;

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
  inline _ConstMemberResultCallback_6_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3>
class _ConstMemberResultCallback_6_3<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3) const;

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
  inline _ConstMemberResultCallback_6_3(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_6_3<true, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2, A3>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_3<true, R, T1, P1, P2, P3, P4, P5, P6,
                                            A1, A2, A3>(obj, member, p1, p2, p3,
                                                        p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3>
inline typename _ConstMemberResultCallback_6_3<false, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2, A3>::base*
NewPermanentCallback(
    const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3) const,
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_3<
      false, R, T1, P1, P2, P3, P4, P5, P6, A1, A2, A3>(obj, member, p1, p2, p3,
                                                        p4, p5, p6);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2, class A3,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_6_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3);

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
  inline _MemberResultCallback_6_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : ResultCallback3<R, A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3>
class _MemberResultCallback_6_3<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3);

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
  inline _MemberResultCallback_6_3(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : Callback3<A1, A2, A3>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3>
inline typename _MemberResultCallback_6_3<true, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2, A3>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_3<true, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2, A3>(obj, member, p1, p2, p3, p4, p5,
                                               p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3>
inline typename _MemberResultCallback_6_3<false, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2, A3>::base*
NewPermanentCallback(
    T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3),
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_3<false, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2, A3>(obj, member, p1, p2, p3, p4, p5,
                                               p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3>
class _FunctionResultCallback_6_3 : public ResultCallback3<R, A1, A2, A3> {
 public:
  typedef ResultCallback3<R, A1, A2, A3> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : ResultCallback3<R, A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback3<R,A1,A2,A3>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3>
class _FunctionResultCallback_6_3<del, void, P1, P2, P3, P4, P5, P6, A1, A2,
                                  A3> : public Callback3<A1, A2, A3> {
 public:
  typedef Callback3<A1, A2, A3> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_3(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : Callback3<A1, A2, A3>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback3<A1,A2,A3>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3>
inline typename _FunctionResultCallback_6_3<true, R, P1, P2, P3, P4, P5, P6, A1,
                                            A2, A3>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2, A3),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_3<true, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2, A3>(function, p1, p2, p3, p4, p5,
                                                 p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3>
inline typename _FunctionResultCallback_6_3<false, R, P1, P2, P3, P4, P5, P6,
                                            A1, A2, A3>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2, A3),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_3<false, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2, A3>(function, p1, p2, p3, p4, p5,
                                                 p6);
}

template <
    bool del, class R, class T, class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_0_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_4(const T* object, MemberSignature member)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4>
class _ConstMemberResultCallback_0_4<
    del, void, T, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_4(const T* object, MemberSignature member)
      : Callback4<A1, A2, A3, A4>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(a1, a2, a3, a4);
    } else {
      (object_->*member_)(a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_0_4<true, R, T1, A1, A2, A3,
                                               A4>::base*
NewCallback(const T1* obj, R (T2::*member)(A1, A2, A3, A4) const) {
  return new _ConstMemberResultCallback_0_4<true, R, T1, A1, A2, A3, A4>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_0_4<false, R, T1, A1, A2, A3,
                                               A4>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(A1, A2, A3, A4) const) {
  return new _ConstMemberResultCallback_0_4<false, R, T1, A1, A2, A3, A4>(
      obj, member);
}
#endif

template <
    bool del, class R, class T, class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_0_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_4(T* object, MemberSignature member)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4>
class _MemberResultCallback_0_4<
    del, void, T, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_4(T* object, MemberSignature member)
      : Callback4<A1, A2, A3, A4>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(a1, a2, a3, a4);
    } else {
      (object_->*member_)(a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_0_4<true, R, T1, A1, A2, A3, A4>::base*
NewCallback(T1* obj, R (T2::*member)(A1, A2, A3, A4)) {
  return new _MemberResultCallback_0_4<true, R, T1, A1, A2, A3, A4>(obj,
                                                                    member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_0_4<false, R, T1, A1, A2, A3, A4>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(A1, A2, A3, A4)) {
  return new _MemberResultCallback_0_4<false, R, T1, A1, A2, A3, A4>(obj,
                                                                     member);
}
#endif

template <bool del, class R, class A1, class A2, class A3, class A4>
class _FunctionResultCallback_0_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(A1, A2, A3, A4);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_4(FunctionSignature function)
      : ResultCallback4<R, A1, A2, A3, A4>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2, class A3, class A4>
class _FunctionResultCallback_0_4<del, void, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(A1, A2, A3, A4);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_4(FunctionSignature function)
      : Callback4<A1, A2, A3, A4>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(a1, a2, a3, a4);
    } else {
      (*function_)(a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_0_4<true, R, A1, A2, A3, A4>::base*
NewCallback(R (*function)(A1, A2, A3, A4)) {
  return new _FunctionResultCallback_0_4<true, R, A1, A2, A3, A4>(function);
}

template <class R, class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_0_4<false, R, A1, A2, A3, A4>::base*
NewPermanentCallback(R (*function)(A1, A2, A3, A4)) {
  return new _FunctionResultCallback_0_4<false, R, A1, A2, A3, A4>(function);
}

template <bool del, class R, class T, class P1, class A1, class A2, class A3,
          class A4, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_1_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4>
class _ConstMemberResultCallback_1_4<
    del, void, T, P1, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4>
inline typename _ConstMemberResultCallback_1_4<true, R, T1, P1, A1, A2, A3,
                                               A4>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, A1, A2, A3, A4) const,
            typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_4<true, R, T1, P1, A1, A2, A3, A4>(
      obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4>
inline typename _ConstMemberResultCallback_1_4<false, R, T1, P1, A1, A2, A3,
                                               A4>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, A1, A2, A3, A4) const,
                     typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_4<false, R, T1, P1, A1, A2, A3, A4>(
      obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1, class A2, class A3,
          class A4, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_1_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4>
class _MemberResultCallback_1_4<
    del, void, T, P1, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4>
inline typename _MemberResultCallback_1_4<true, R, T1, P1, A1, A2, A3,
                                          A4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_4<true, R, T1, P1, A1, A2, A3, A4>(
      obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4>
inline typename _MemberResultCallback_1_4<false, R, T1, P1, A1, A2, A3,
                                          A4>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_4<false, R, T1, P1, A1, A2, A3, A4>(
      obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2, class A3, class A4>
class _FunctionResultCallback_1_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(P1, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : ResultCallback4<R, A1, A2, A3, A4>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(p1_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(p1_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2, class A3, class A4>
class _FunctionResultCallback_1_4<del, void, P1, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(P1, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : Callback4<A1, A2, A3, A4>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(p1_, a1, a2, a3, a4);
    } else {
      (*function_)(p1_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_1_4<true, R, P1, A1, A2, A3, A4>::base*
NewCallback(R (*function)(P1, A1, A2, A3, A4), typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_4<true, R, P1, A1, A2, A3, A4>(function,
                                                                      p1);
}

template <class R, class P1, class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_1_4<false, R, P1, A1, A2, A3, A4>::base*
NewPermanentCallback(R (*function)(P1, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_4<false, R, P1, A1, A2, A3, A4>(function,
                                                                       p1);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2,
          class A3, class A4, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_2_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3,
          class A4>
class _ConstMemberResultCallback_2_4<
    del, void, T, P1, P2, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4>
inline typename _ConstMemberResultCallback_2_4<true, R, T1, P1, P2, A1, A2, A3,
                                               A4>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, A1, A2, A3, A4) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_4<true, R, T1, P1, P2, A1, A2, A3,
                                            A4>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4>
inline typename _ConstMemberResultCallback_2_4<false, R, T1, P1, P2, A1, A2, A3,
                                               A4>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, A1, A2, A3, A4) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_4<false, R, T1, P1, P2, A1, A2, A3,
                                            A4>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2,
          class A3, class A4, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _MemberResultCallback_2_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3,
          class A4>
class _MemberResultCallback_2_4<
    del, void, T, P1, P2, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4>
inline typename _MemberResultCallback_2_4<true, R, T1, P1, P2, A1, A2, A3,
                                          A4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_4<true, R, T1, P1, P2, A1, A2, A3, A4>(
      obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4>
inline typename _MemberResultCallback_2_4<false, R, T1, P1, P2, A1, A2, A3,
                                          A4>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_4<false, R, T1, P1, P2, A1, A2, A3, A4>(
      obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2, class A3,
          class A4>
class _FunctionResultCallback_2_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(P1, P2, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(p1_, p2_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2, class A3, class A4>
class _FunctionResultCallback_2_4<del, void, P1, P2, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(P1, P2, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : Callback4<A1, A2, A3, A4>(), function_(function), p1_(p1), p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(p1_, p2_, a1, a2, a3, a4);
    } else {
      (*function_)(p1_, p2_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_2_4<true, R, P1, P2, A1, A2, A3,
                                            A4>::base*
NewCallback(R (*function)(P1, P2, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_4<true, R, P1, P2, A1, A2, A3, A4>(
      function, p1, p2);
}

template <class R, class P1, class P2, class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_2_4<false, R, P1, P2, A1, A2, A3,
                                            A4>::base*
NewPermanentCallback(R (*function)(P1, P2, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_4<false, R, P1, P2, A1, A2, A3, A4>(
      function, p1, p2);
}

template <bool del, class R, class T, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_3_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2,
          class A3, class A4>
class _ConstMemberResultCallback_3_4<
    del, void, T, P1, P2, P3, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_3_4<true, R, T1, P1, P2, P3, A1, A2,
                                               A3, A4>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3, A4) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_4<true, R, T1, P1, P2, P3, A1, A2, A3,
                                            A4>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_3_4<false, R, T1, P1, P2, P3, A1, A2,
                                               A3, A4>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, A1, A2, A3, A4) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_4<false, R, T1, P1, P2, P3, A1, A2,
                                            A3, A4>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _MemberResultCallback_3_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2,
          class A3, class A4>
class _MemberResultCallback_3_4<
    del, void, T, P1, P2, P3, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4>
inline typename _MemberResultCallback_3_4<true, R, T1, P1, P2, P3, A1, A2, A3,
                                          A4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_4<true, R, T1, P1, P2, P3, A1, A2, A3, A4>(
      obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4>
inline typename _MemberResultCallback_3_4<false, R, T1, P1, P2, P3, A1, A2, A3,
                                          A4>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_4<false, R, T1, P1, P2, P3, A1, A2, A3,
                                       A4>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2,
          class A3, class A4>
class _FunctionResultCallback_3_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(P1, P2, P3, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2, class A3,
          class A4>
class _FunctionResultCallback_3_4<del, void, P1, P2, P3, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(P1, P2, P3, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : Callback4<A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, a1, a2, a3, a4);
    } else {
      (*function_)(p1_, p2_, p3_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2, class A3,
          class A4>
inline typename _FunctionResultCallback_3_4<true, R, P1, P2, P3, A1, A2, A3,
                                            A4>::base*
NewCallback(R (*function)(P1, P2, P3, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_4<true, R, P1, P2, P3, A1, A2, A3, A4>(
      function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2, class A3,
          class A4>
inline typename _FunctionResultCallback_3_4<false, R, P1, P2, P3, A1, A2, A3,
                                            A4>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_4<false, R, P1, P2, P3, A1, A2, A3, A4>(
      function, p1, p2, p3);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_4_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3, class A4>
class _ConstMemberResultCallback_4_4<
    del, void, T, P1, P2, P3, P4, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_4_4<true, R, T1, P1, P2, P3, P4, A1,
                                               A2, A3, A4>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_4<
      true, R, T1, P1, P2, P3, P4, A1, A2, A3, A4>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_4_4<false, R, T1, P1, P2, P3, P4, A1,
                                               A2, A3, A4>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_4<false, R, T1, P1, P2, P3, P4, A1,
                                            A2, A3, A4>(obj, member, p1, p2, p3,
                                                        p4);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_4_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3, class A4>
class _MemberResultCallback_4_4<
    del, void, T, P1, P2, P3, P4, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_4_4<true, R, T1, P1, P2, P3, P4, A1, A2,
                                          A3, A4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_4<true, R, T1, P1, P2, P3, P4, A1, A2, A3,
                                       A4>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_4_4<false, R, T1, P1, P2, P3, P4, A1, A2,
                                          A3, A4>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_4<false, R, T1, P1, P2, P3, P4, A1, A2, A3,
                                       A4>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3, class A4>
class _FunctionResultCallback_4_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3, class A4>
class _FunctionResultCallback_4_4<del, void, P1, P2, P3, P4, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : Callback4<A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3, class A4>
inline typename _FunctionResultCallback_4_4<true, R, P1, P2, P3, P4, A1, A2, A3,
                                            A4>::base*
NewCallback(R (*function)(P1, P2, P3, P4, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_4<true, R, P1, P2, P3, P4, A1, A2, A3,
                                         A4>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3, class A4>
inline typename _FunctionResultCallback_4_4<false, R, P1, P2, P3, P4, A1, A2,
                                            A3, A4>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_4<false, R, P1, P2, P3, P4, A1, A2, A3,
                                         A4>(function, p1, p2, p3, p4);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_5_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3, class A4>
class _ConstMemberResultCallback_5_4<
    del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_5_4<true, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2, A3, A4>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_4<true, R, T1, P1, P2, P3, P4, P5, A1,
                                            A2, A3, A4>(obj, member, p1, p2, p3,
                                                        p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_5_4<false, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2, A3, A4>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_4<
      false, R, T1, P1, P2, P3, P4, P5, A1, A2, A3, A4>(obj, member, p1, p2, p3,
                                                        p4, p5);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_5_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3, class A4>
class _MemberResultCallback_5_4<
    del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_5_4<true, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2, A3, A4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_4<true, R, T1, P1, P2, P3, P4, P5, A1, A2,
                                       A3, A4>(obj, member, p1, p2, p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_5_4<false, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2, A3, A4>::base*
NewPermanentCallback(T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_4<false, R, T1, P1, P2, P3, P4, P5, A1, A2,
                                       A3, A4>(obj, member, p1, p2, p3, p4, p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3, class A4>
class _FunctionResultCallback_5_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3, class A4>
class _FunctionResultCallback_5_4<del, void, P1, P2, P3, P4, P5, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : Callback4<A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3, class A4>
inline typename _FunctionResultCallback_5_4<true, R, P1, P2, P3, P4, P5, A1, A2,
                                            A3, A4>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_4<true, R, P1, P2, P3, P4, P5, A1, A2,
                                         A3, A4>(function, p1, p2, p3, p4, p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3, class A4>
inline typename _FunctionResultCallback_5_4<false, R, P1, P2, P3, P4, P5, A1,
                                            A2, A3, A4>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_4<false, R, P1, P2, P3, P4, P5, A1, A2,
                                         A3, A4>(function, p1, p2, p3, p4, p5);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_6_4
    : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4) const;

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
  inline _ConstMemberResultCallback_6_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      return result;
    } else {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3, class A4>
class _ConstMemberResultCallback_6_4<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3,
                                     A4) const;

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
  inline _ConstMemberResultCallback_6_4(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_6_4<true, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2, A3, A4>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_4<true, R, T1, P1, P2, P3, P4, P5, P6,
                                            A1, A2, A3, A4>(obj, member, p1, p2,
                                                            p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _ConstMemberResultCallback_6_4<false, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2, A3, A4>::base*
NewPermanentCallback(
    const T1* obj,
    R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4) const,
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_4<
      false, R, T1, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4>(obj, member, p1, p2,
                                                            p3, p4, p5, p6);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2, class A3, class A4,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_6_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4);

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
  inline _MemberResultCallback_6_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      return result;
    } else {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3, class A4>
class _MemberResultCallback_6_4<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4,
    typename c_enable_if<
        is_class_or_union<T>::value>::type> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4);

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
  inline _MemberResultCallback_6_4(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : Callback4<A1, A2, A3, A4>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_6_4<true, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2, A3, A4>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_4<true, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2, A3, A4>(obj, member, p1, p2, p3, p4,
                                                   p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4>
inline typename _MemberResultCallback_6_4<false, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2, A3, A4>::base*
NewPermanentCallback(
    T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4),
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_4<false, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2, A3, A4>(obj, member, p1, p2, p3, p4,
                                                   p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3, class A4>
class _FunctionResultCallback_6_4 : public ResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef ResultCallback4<R, A1, A2, A3, A4> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : ResultCallback4<R, A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("ResultCallback4<R,A1,A2,A3,A4>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3, class A4>
class _FunctionResultCallback_6_4<del, void, P1, P2, P3, P4, P5, P6, A1, A2, A3,
                                  A4> : public Callback4<A1, A2, A3, A4> {
 public:
  typedef Callback4<A1, A2, A3, A4> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_4(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : Callback4<A1, A2, A3, A4>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback4<A1,A2,A3,A4>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_6_4<true, R, P1, P2, P3, P4, P5, P6, A1,
                                            A2, A3, A4>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_4<true, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2, A3, A4>(function, p1, p2, p3, p4,
                                                     p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3, class A4>
inline typename _FunctionResultCallback_6_4<false, R, P1, P2, P3, P4, P5, P6,
                                            A1, A2, A3, A4>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_4<false, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2, A3, A4>(function, p1, p2, p3, p4,
                                                     p5, p6);
}

template <bool del, class R, class T, class A1, class A2, class A3, class A4,
          class A5, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_0_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_5(const T* object, MemberSignature member)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4, class A5>
class _ConstMemberResultCallback_0_5<
    del, void, T, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstMemberResultCallback_0_5(const T* object, MemberSignature member)
      : Callback5<A1, A2, A3, A4, A5>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4,
          class A5>
inline typename _ConstMemberResultCallback_0_5<true, R, T1, A1, A2, A3, A4,
                                               A5>::base*
NewCallback(const T1* obj, R (T2::*member)(A1, A2, A3, A4, A5) const) {
  return new _ConstMemberResultCallback_0_5<true, R, T1, A1, A2, A3, A4, A5>(
      obj, member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4,
          class A5>
inline typename _ConstMemberResultCallback_0_5<false, R, T1, A1, A2, A3, A4,
                                               A5>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(A1, A2, A3, A4, A5) const) {
  return new _ConstMemberResultCallback_0_5<false, R, T1, A1, A2, A3, A4, A5>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3, class A4,
          class A5, class OnlyIf =
                        typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_0_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_5(T* object, MemberSignature member)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class A1, class A2, class A3, class A4, class A5>
class _MemberResultCallback_0_5<
    del, void, T, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _MemberResultCallback_0_5(T* object, MemberSignature member)
      : Callback5<A1, A2, A3, A4, A5>(), object_(object), member_(member) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4,
          class A5>
inline typename _MemberResultCallback_0_5<true, R, T1, A1, A2, A3, A4,
                                          A5>::base*
NewCallback(T1* obj, R (T2::*member)(A1, A2, A3, A4, A5)) {
  return new _MemberResultCallback_0_5<true, R, T1, A1, A2, A3, A4, A5>(obj,
                                                                        member);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4,
          class A5>
inline typename _MemberResultCallback_0_5<false, R, T1, A1, A2, A3, A4,
                                          A5>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(A1, A2, A3, A4, A5)) {
  return new _MemberResultCallback_0_5<false, R, T1, A1, A2, A3, A4, A5>(
      obj, member);
}
#endif

template <bool del, class R, class A1, class A2, class A3, class A4, class A5>
class _FunctionResultCallback_0_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_5(FunctionSignature function)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class A1, class A2, class A3, class A4, class A5>
class _FunctionResultCallback_0_5<del, void, A1, A2, A3, A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;

 public:
  explicit inline _FunctionResultCallback_0_5(FunctionSignature function)
      : Callback5<A1, A2, A3, A4, A5>(), function_(function) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(a1, a2, a3, a4, a5);
    } else {
      (*function_)(a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class A1, class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_0_5<true, R, A1, A2, A3, A4, A5>::base*
NewCallback(R (*function)(A1, A2, A3, A4, A5)) {
  return new _FunctionResultCallback_0_5<true, R, A1, A2, A3, A4, A5>(function);
}

template <class R, class A1, class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_0_5<false, R, A1, A2, A3, A4, A5>::base*
NewPermanentCallback(R (*function)(A1, A2, A3, A4, A5)) {
  return new _FunctionResultCallback_0_5<false, R, A1, A2, A3, A4, A5>(
      function);
}

template <bool del, class R, class T, class P1, class A1, class A2, class A3,
          class A4, class A5, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_1_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4,
          class A5>
class _ConstMemberResultCallback_1_5<
    del, void, T, P1, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _ConstMemberResultCallback_1_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4, class A5>
inline typename _ConstMemberResultCallback_1_5<true, R, T1, P1, A1, A2, A3, A4,
                                               A5>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, A1, A2, A3, A4, A5) const,
            typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_5<true, R, T1, P1, A1, A2, A3, A4,
                                            A5>(obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4, class A5>
inline typename _ConstMemberResultCallback_1_5<false, R, T1, P1, A1, A2, A3, A4,
                                               A5>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, A1, A2, A3, A4, A5) const,
                     typename ConstRef<P1>::type p1) {
  return new _ConstMemberResultCallback_1_5<false, R, T1, P1, A1, A2, A3, A4,
                                            A5>(obj, member, p1);
}
#endif

template <bool del, class R, class T, class P1, class A1, class A2, class A3,
          class A4, class A5, class OnlyIf = typename c_enable_if<
                                  is_class_or_union<T>::value>::type>
class _MemberResultCallback_1_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class A1, class A2, class A3, class A4,
          class A5>
class _MemberResultCallback_1_5<
    del, void, T, P1, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _MemberResultCallback_1_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4, class A5>
inline typename _MemberResultCallback_1_5<true, R, T1, P1, A1, A2, A3, A4,
                                          A5>::base*
NewCallback(T1* obj, R (T2::*member)(P1, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_5<true, R, T1, P1, A1, A2, A3, A4, A5>(
      obj, member, p1);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class A1, class A2, class A3,
          class A4, class A5>
inline typename _MemberResultCallback_1_5<false, R, T1, P1, A1, A2, A3, A4,
                                          A5>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1) {
  return new _MemberResultCallback_1_5<false, R, T1, P1, A1, A2, A3, A4, A5>(
      obj, member, p1);
}
#endif

template <bool del, class R, class P1, class A1, class A2, class A3, class A4,
          class A5>
class _FunctionResultCallback_1_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(P1, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(p1_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(p1_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class A1, class A2, class A3, class A4, class A5>
class _FunctionResultCallback_1_5<del, void, P1, A1, A2, A3, A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(P1, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _FunctionResultCallback_1_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1)
      : Callback5<A1, A2, A3, A4, A5>(), function_(function), p1_(p1) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(p1_, a1, a2, a3, a4, a5);
    } else {
      (*function_)(p1_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_1_5<true, R, P1, A1, A2, A3, A4,
                                            A5>::base*
NewCallback(R (*function)(P1, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_5<true, R, P1, A1, A2, A3, A4, A5>(
      function, p1);
}

template <class R, class P1, class A1, class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_1_5<false, R, P1, A1, A2, A3, A4,
                                            A5>::base*
NewPermanentCallback(R (*function)(P1, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1) {
  return new _FunctionResultCallback_1_5<false, R, P1, A1, A2, A3, A4, A5>(
      function, p1);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2,
          class A3, class A4, class A5, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_2_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3,
          class A4, class A5>
class _ConstMemberResultCallback_2_5<
    del, void, T, P1, P2, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _ConstMemberResultCallback_2_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_2_5<true, R, T1, P1, P2, A1, A2, A3,
                                               A4, A5>::base*
NewCallback(const T1* obj, R (T2::*member)(P1, P2, A1, A2, A3, A4, A5) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_5<true, R, T1, P1, P2, A1, A2, A3, A4,
                                            A5>(obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_2_5<false, R, T1, P1, P2, A1, A2, A3,
                                               A4, A5>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, A1, A2, A3, A4, A5) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _ConstMemberResultCallback_2_5<false, R, T1, P1, P2, A1, A2, A3,
                                            A4, A5>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class P1, class P2, class A1, class A2,
          class A3, class A4, class A5, class OnlyIf = typename c_enable_if<
                                            is_class_or_union<T>::value>::type>
class _MemberResultCallback_2_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class A1, class A2, class A3,
          class A4, class A5>
class _MemberResultCallback_2_5<
    del, void, T, P1, P2, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _MemberResultCallback_2_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4, class A5>
inline typename _MemberResultCallback_2_5<true, R, T1, P1, P2, A1, A2, A3, A4,
                                          A5>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_5<true, R, T1, P1, P2, A1, A2, A3, A4, A5>(
      obj, member, p1, p2);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2,
          class A3, class A4, class A5>
inline typename _MemberResultCallback_2_5<false, R, T1, P1, P2, A1, A2, A3, A4,
                                          A5>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _MemberResultCallback_2_5<false, R, T1, P1, P2, A1, A2, A3, A4,
                                       A5>(obj, member, p1, p2);
}
#endif

template <bool del, class R, class P1, class P2, class A1, class A2, class A3,
          class A4, class A5>
class _FunctionResultCallback_2_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(P1, P2, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(p1_, p2_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class A1, class A2, class A3, class A4,
          class A5>
class _FunctionResultCallback_2_5<del, void, P1, P2, A1, A2, A3, A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(P1, P2, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _FunctionResultCallback_2_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2)
      : Callback5<A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(p1_, p2_, a1, a2, a3, a4, a5);
    } else {
      (*function_)(p1_, p2_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class A1, class A2, class A3, class A4,
          class A5>
inline typename _FunctionResultCallback_2_5<true, R, P1, P2, A1, A2, A3, A4,
                                            A5>::base*
NewCallback(R (*function)(P1, P2, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_5<true, R, P1, P2, A1, A2, A3, A4, A5>(
      function, p1, p2);
}

template <class R, class P1, class P2, class A1, class A2, class A3, class A4,
          class A5>
inline typename _FunctionResultCallback_2_5<false, R, P1, P2, A1, A2, A3, A4,
                                            A5>::base*
NewPermanentCallback(R (*function)(P1, P2, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2) {
  return new _FunctionResultCallback_2_5<false, R, P1, P2, A1, A2, A3, A4, A5>(
      function, p1, p2);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class A1,
    class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_3_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2,
          class A3, class A4, class A5>
class _ConstMemberResultCallback_3_5<
    del, void, T, P1, P2, P3, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _ConstMemberResultCallback_3_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_3_5<true, R, T1, P1, P2, P3, A1, A2,
                                               A3, A4, A5>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, A1, A2, A3, A4, A5) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_5<true, R, T1, P1, P2, P3, A1, A2, A3,
                                            A4, A5>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_3_5<false, R, T1, P1, P2, P3, A1, A2,
                                               A3, A4, A5>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, A1, A2, A3, A4, A5) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _ConstMemberResultCallback_3_5<
      false, R, T1, P1, P2, P3, A1, A2, A3, A4, A5>(obj, member, p1, p2, p3);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class A1,
    class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_3_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class A1, class A2,
          class A3, class A4, class A5>
class _MemberResultCallback_3_5<
    del, void, T, P1, P2, P3, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _MemberResultCallback_3_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_3_5<true, R, T1, P1, P2, P3, A1, A2, A3,
                                          A4, A5>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_5<true, R, T1, P1, P2, P3, A1, A2, A3, A4,
                                       A5>(obj, member, p1, p2, p3);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class A1,
          class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_3_5<false, R, T1, P1, P2, P3, A1, A2, A3,
                                          A4, A5>::base*
NewPermanentCallback(T1* obj, R (T2::*member)(P1, P2, P3, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _MemberResultCallback_3_5<false, R, T1, P1, P2, P3, A1, A2, A3, A4,
                                       A5>(obj, member, p1, p2, p3);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class A1, class A2,
          class A3, class A4, class A5>
class _FunctionResultCallback_3_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(P1, P2, P3, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class A1, class A2, class A3,
          class A4, class A5>
class _FunctionResultCallback_3_5<del, void, P1, P2, P3, A1, A2, A3, A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(P1, P2, P3, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;

 public:
  inline _FunctionResultCallback_3_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3)
      : Callback5<A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
    } else {
      (*function_)(p1_, p2_, p3_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class A1, class A2, class A3,
          class A4, class A5>
inline typename _FunctionResultCallback_3_5<true, R, P1, P2, P3, A1, A2, A3, A4,
                                            A5>::base*
NewCallback(R (*function)(P1, P2, P3, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_5<true, R, P1, P2, P3, A1, A2, A3, A4,
                                         A5>(function, p1, p2, p3);
}

template <class R, class P1, class P2, class P3, class A1, class A2, class A3,
          class A4, class A5>
inline typename _FunctionResultCallback_3_5<false, R, P1, P2, P3, A1, A2, A3,
                                            A4, A5>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3) {
  return new _FunctionResultCallback_3_5<false, R, P1, P2, P3, A1, A2, A3, A4,
                                         A5>(function, p1, p2, p3);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class A1, class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_4_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3, class A4, class A5>
class _ConstMemberResultCallback_4_5<
    del, void, T, P1, P2, P3, P4, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _ConstMemberResultCallback_4_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_4_5<true, R, T1, P1, P2, P3, P4, A1,
                                               A2, A3, A4, A5>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4, A5) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_5<true, R, T1, P1, P2, P3, P4, A1, A2,
                                            A3, A4, A5>(obj, member, p1, p2, p3,
                                                        p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_4_5<false, R, T1, P1, P2, P3, P4, A1,
                                               A2, A3, A4, A5>::base*
NewPermanentCallback(const T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4, A5) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _ConstMemberResultCallback_4_5<
      false, R, T1, P1, P2, P3, P4, A1, A2, A3, A4, A5>(obj, member, p1, p2, p3,
                                                        p4);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class A1, class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_4_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3, class A4, class A5>
class _MemberResultCallback_4_5<
    del, void, T, P1, P2, P3, P4, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _MemberResultCallback_4_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_4_5<true, R, T1, P1, P2, P3, P4, A1, A2,
                                          A3, A4, A5>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_5<true, R, T1, P1, P2, P3, P4, A1, A2, A3,
                                       A4, A5>(obj, member, p1, p2, p3, p4);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class A1, class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_4_5<false, R, T1, P1, P2, P3, P4, A1, A2,
                                          A3, A4, A5>::base*
NewPermanentCallback(T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _MemberResultCallback_4_5<false, R, T1, P1, P2, P3, P4, A1, A2, A3,
                                       A4, A5>(obj, member, p1, p2, p3, p4);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class A1,
          class A2, class A3, class A4, class A5>
class _FunctionResultCallback_4_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3, class A4, class A5>
class _FunctionResultCallback_4_5<del, void, P1, P2, P3, P4, A1, A2, A3, A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;

 public:
  inline _FunctionResultCallback_4_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4)
      : Callback5<A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3, class A4, class A5>
inline typename _FunctionResultCallback_4_5<true, R, P1, P2, P3, P4, A1, A2, A3,
                                            A4, A5>::base*
NewCallback(R (*function)(P1, P2, P3, P4, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_5<true, R, P1, P2, P3, P4, A1, A2, A3,
                                         A4, A5>(function, p1, p2, p3, p4);
}

template <class R, class P1, class P2, class P3, class P4, class A1, class A2,
          class A3, class A4, class A5>
inline typename _FunctionResultCallback_4_5<false, R, P1, P2, P3, P4, A1, A2,
                                            A3, A4, A5>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4) {
  return new _FunctionResultCallback_4_5<false, R, P1, P2, P3, P4, A1, A2, A3,
                                         A4, A5>(function, p1, p2, p3, p4);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class A1, class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_5_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3, class A4, class A5>
class _ConstMemberResultCallback_5_5<
    del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4,
                                     A5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstMemberResultCallback_5_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_5_5<true, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2, A3, A4, A5>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_5<true, R, T1, P1, P2, P3, P4, P5, A1,
                                            A2, A3, A4, A5>(obj, member, p1, p2,
                                                            p3, p4, p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_5_5<false, R, T1, P1, P2, P3, P4, P5,
                                               A1, A2, A3, A4, A5>::base*
NewPermanentCallback(const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2,
                                                    A3, A4, A5) const,
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _ConstMemberResultCallback_5_5<
      false, R, T1, P1, P2, P3, P4, P5, A1, A2, A3, A4, A5>(obj, member, p1, p2,
                                                            p3, p4, p5);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class A1, class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_5_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3, class A4, class A5>
class _MemberResultCallback_5_5<
    del, void, T, P1, P2, P3, P4, P5, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _MemberResultCallback_5_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_5_5<true, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2, A3, A4, A5>::base*
NewCallback(T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_5<true, R, T1, P1, P2, P3, P4, P5, A1, A2,
                                       A3, A4, A5>(obj, member, p1, p2, p3, p4,
                                                   p5);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class A1, class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_5_5<false, R, T1, P1, P2, P3, P4, P5, A1,
                                          A2, A3, A4, A5>::base*
NewPermanentCallback(T1* obj,
                     R (T2::*member)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _MemberResultCallback_5_5<false, R, T1, P1, P2, P3, P4, P5, A1, A2,
                                       A3, A4, A5>(obj, member, p1, p2, p3, p4,
                                                   p5);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class A1, class A2, class A3, class A4, class A5>
class _FunctionResultCallback_5_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3, class A4, class A5>
class _FunctionResultCallback_5_5<del, void, P1, P2, P3, P4, P5, A1, A2, A3, A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _FunctionResultCallback_5_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5)
      : Callback5<A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_5_5<true, R, P1, P2, P3, P4, P5, A1, A2,
                                            A3, A4, A5>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_5<true, R, P1, P2, P3, P4, P5, A1, A2,
                                         A3, A4, A5>(function, p1, p2, p3, p4,
                                                     p5);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class A1,
          class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_5_5<false, R, P1, P2, P3, P4, P5, A1,
                                            A2, A3, A4, A5>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5) {
  return new _FunctionResultCallback_5_5<false, R, P1, P2, P3, P4, P5, A1, A2,
                                         A3, A4, A5>(function, p1, p2, p3, p4,
                                                     p5);
}

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _ConstMemberResultCallback_6_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4,
                                  A5) const;

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
  inline _ConstMemberResultCallback_6_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3, class A4, class A5>
class _ConstMemberResultCallback_6_5<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4,
                                     A5) const;

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
  inline _ConstMemberResultCallback_6_5(const T* object, MemberSignature member,
                                        typename ConstRef<P1>::type p1,
                                        typename ConstRef<P2>::type p2,
                                        typename ConstRef<P3>::type p3,
                                        typename ConstRef<P4>::type p4,
                                        typename ConstRef<P5>::type p5,
                                        typename ConstRef<P6>::type p6)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_6_5<true, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2, A3, A4, A5>::base*
NewCallback(const T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5) const,
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_5<true, R, T1, P1, P2, P3, P4, P5, P6,
                                            A1, A2, A3, A4, A5>(
      obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _ConstMemberResultCallback_6_5<false, R, T1, P1, P2, P3, P4, P5,
                                               P6, A1, A2, A3, A4, A5>::base*
NewPermanentCallback(
    const T1* obj,
    R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5) const,
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _ConstMemberResultCallback_6_5<false, R, T1, P1, P2, P3, P4, P5,
                                            P6, A1, A2, A3, A4, A5>(
      obj, member, p1, p2, p3, p4, p5, p6);
}
#endif

template <
    bool del, class R, class T, class P1, class P2, class P3, class P4,
    class P5, class P6, class A1, class A2, class A3, class A4, class A5,
    class OnlyIf = typename c_enable_if<is_class_or_union<T>::value>::type>
class _MemberResultCallback_6_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5);

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
  inline _MemberResultCallback_6_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result =
          (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3, class A4, class A5>
class _MemberResultCallback_6_5<
    del, void, T, P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5,
    typename c_enable_if<is_class_or_union<
        T>::value>::type> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (T::*MemberSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4,
                                     A5);

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
  inline _MemberResultCallback_6_5(T* object, MemberSignature member,
                                   typename ConstRef<P1>::type p1,
                                   typename ConstRef<P2>::type p2,
                                   typename ConstRef<P3>::type p3,
                                   typename ConstRef<P4>::type p4,
                                   typename ConstRef<P5>::type p5,
                                   typename ConstRef<P6>::type p6)
      : Callback5<A1, A2, A3, A4, A5>(),
        object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_6_5<true, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2, A3, A4, A5>::base*
NewCallback(T1* obj,
            R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_5<true, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2, A3, A4, A5>(obj, member, p1, p2, p3,
                                                       p4, p5, p6);
}
#endif

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6, class A1, class A2, class A3, class A4, class A5>
inline typename _MemberResultCallback_6_5<false, R, T1, P1, P2, P3, P4, P5, P6,
                                          A1, A2, A3, A4, A5>::base*
NewPermanentCallback(
    T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5),
    typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
    typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
    typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _MemberResultCallback_6_5<false, R, T1, P1, P2, P3, P4, P5, P6, A1,
                                       A2, A3, A4, A5>(obj, member, p1, p2, p3,
                                                       p4, p5, p6);
}
#endif

template <bool del, class R, class P1, class P2, class P3, class P4, class P5,
          class P6, class A1, class A2, class A3, class A4, class A5>
class _FunctionResultCallback_6_5
    : public ResultCallback5<R, A1, A2, A3, A4, A5> {
 public:
  typedef ResultCallback5<R, A1, A2, A3, A4, A5> base;
  typedef R (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : ResultCallback5<R, A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del)
      CallbackUtils_::FailIsRepeatable("ResultCallback5<R,A1,A2,A3,A4,A5>");
  }

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      return result;
    } else {
      R result = (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
      return result;
    }
  }
};

template <bool del, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3, class A4, class A5>
class _FunctionResultCallback_6_5<del, void, P1, P2, P3, P4, P5, P6, A1, A2, A3,
                                  A4,
                                  A5> : public Callback5<A1, A2, A3, A4, A5> {
 public:
  typedef Callback5<A1, A2, A3, A4, A5> base;
  typedef void (*FunctionSignature)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;
  typename remove_reference<P6>::type p6_;

 public:
  inline _FunctionResultCallback_6_5(FunctionSignature function,
                                     typename ConstRef<P1>::type p1,
                                     typename ConstRef<P2>::type p2,
                                     typename ConstRef<P3>::type p3,
                                     typename ConstRef<P4>::type p4,
                                     typename ConstRef<P5>::type p5,
                                     typename ConstRef<P6>::type p6)
      : Callback5<A1, A2, A3, A4, A5>(),
        function_(function),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  virtual bool IsRepeatable() const { return !del; }

  virtual void CheckIsRepeatable() const {
    if (del) CallbackUtils_::FailIsRepeatable("Callback5<A1,A2,A3,A4,A5>");
  }

  virtual void Run(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    if (!del) {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
    } else {
      (*function_)(p1_, p2_, p3_, p4_, p5_, p6_, a1, a2, a3, a4, a5);
      //  zero out the pointer to ensure segfault if used again
      function_ = NULL;
      delete this;
    }
  }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_6_5<true, R, P1, P2, P3, P4, P5, P6, A1,
                                            A2, A3, A4, A5>::base*
NewCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5),
            typename ConstRef<P1>::type p1, typename ConstRef<P2>::type p2,
            typename ConstRef<P3>::type p3, typename ConstRef<P4>::type p4,
            typename ConstRef<P5>::type p5, typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_5<true, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2, A3, A4, A5>(function, p1, p2, p3,
                                                         p4, p5, p6);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class A1, class A2, class A3, class A4, class A5>
inline typename _FunctionResultCallback_6_5<false, R, P1, P2, P3, P4, P5, P6,
                                            A1, A2, A3, A4, A5>::base*
NewPermanentCallback(R (*function)(P1, P2, P3, P4, P5, P6, A1, A2, A3, A4, A5),
                     typename ConstRef<P1>::type p1,
                     typename ConstRef<P2>::type p2,
                     typename ConstRef<P3>::type p3,
                     typename ConstRef<P4>::type p4,
                     typename ConstRef<P5>::type p5,
                     typename ConstRef<P6>::type p6) {
  return new _FunctionResultCallback_6_5<false, R, P1, P2, P3, P4, P5, P6, A1,
                                         A2, A3, A4, A5>(function, p1, p2, p3,
                                                         p4, p5, p6);
}

#endif  // OR_TOOLS_BASE_CALLBACK_H_
