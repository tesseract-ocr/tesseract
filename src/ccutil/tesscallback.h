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

class TessClosure {
 public:
  virtual ~TessClosure();
  virtual void Run() = 0;
};

template <class R>
class TessResultCallback {
 public:
  virtual ~TessResultCallback() = default;
  virtual R Run() = 0;
};

template <bool del, class R, class T>
class _TessMemberResultCallback_0_0 : public TessResultCallback<R> {
 public:
  using base = TessResultCallback<R>;
  using MemberSignature = R (T::*)();

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_0(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  R Run() override {
    if (!del) {
      R result = (object_->*member_)();
      return result;
    }
    R result = (object_->*member_)();
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class T>
class _TessMemberResultCallback_0_0<del, void, T> : public TessClosure {
 public:
  using base = TessClosure;
  using MemberSignature = void (T::*)();

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_0(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  void Run() override {
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
inline typename _TessMemberResultCallback_0_0<true, R, T1>::base*
NewTessCallback(T1* obj, R (T2::*member)()) {
  return new _TessMemberResultCallback_0_0<true, R, T1>(obj, member);
}
#endif

template <bool del, class R>
class _TessFunctionResultCallback_0_0 : public TessResultCallback<R> {
 public:
  using base = TessResultCallback<R>;
  using FunctionSignature = R (*)();

 private:
  FunctionSignature function_;

 public:
  inline explicit _TessFunctionResultCallback_0_0(FunctionSignature function)
      : function_(function) {}

  virtual R Run() {
    if (!del) {
      R result = (*function_)();
      return result;
    }
    R result = (*function_)();
    //  zero out the pointer to ensure segfault if used again
    function_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del>
class _TessFunctionResultCallback_0_0<del, void> : public TessClosure {
 public:
  using base = TessClosure;
  using FunctionSignature = void (*)();

 private:
  FunctionSignature function_;

 public:
  inline explicit _TessFunctionResultCallback_0_0(FunctionSignature function)
      : function_(function) {}

  void Run() override {
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
inline typename _TessFunctionResultCallback_0_0<true, R>::base* NewTessCallback(
    R (*function)()) {
  return new _TessFunctionResultCallback_0_0<true, R>(function);
}

// Specified by TR1 [4.7.2] Reference modifications.
template <class T>
struct remove_reference;
template <typename T>
struct remove_reference {
  using type = T;
};
template <typename T>
struct remove_reference<T&> {
  using type = T;
};

// Identity<T>::type is a typedef of T. Useful for preventing the
// compiler from inferring the type of an argument in templates.
template <typename T>
struct Identity {
  using type = T;
};

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5>
class _ConstTessMemberResultCallback_5_0 : public TessResultCallback<R> {
 public:
  using base = TessResultCallback<R>;
  using MemberSignature = R (T::*)(P1, P2, P3, P4, P5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_0(const T* object,
                                            MemberSignature member, P1 p1,
                                            P2 p2, P3 p3, P4 p4, P5 p5)
      : object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  R Run() override {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      return result;
    }
    R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5>
class _ConstTessMemberResultCallback_5_0<del, void, T, P1, P2, P3, P4, P5>
    : public TessClosure {
 public:
  using base = TessClosure;
  using MemberSignature = void (T::*)(P1, P2, P3, P4, P5) const;

 private:
  const T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;
  typename remove_reference<P3>::type p3_;
  typename remove_reference<P4>::type p4_;
  typename remove_reference<P5>::type p5_;

 public:
  inline _ConstTessMemberResultCallback_5_0(const T* object,
                                            MemberSignature member, P1 p1,
                                            P2 p2, P3 p3, P4 p4, P5 p5)
      : object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5) {}

  void Run() override {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5>
inline typename _ConstTessMemberResultCallback_5_0<false, R, T1, P1, P2, P3, P4,
                                                   P5>::base*
NewPermanentTessCallback(const T1* obj,
                         R (T2::*member)(P1, P2, P3, P4, P5) const,
                         typename Identity<P1>::type p1,
                         typename Identity<P2>::type p2,
                         typename Identity<P3>::type p3,
                         typename Identity<P4>::type p4,
                         typename Identity<P5>::type p5) {
  return new _ConstTessMemberResultCallback_5_0<false, R, T1, P1, P2, P3, P4,
                                                P5>(obj, member, p1, p2, p3, p4,
                                                    p5);
}
#endif

template <bool del, class R, class T, class P1, class P2, class P3, class P4,
          class P5, class P6>
class _ConstTessMemberResultCallback_6_0 : public TessResultCallback<R> {
 public:
  using base = TessResultCallback<R>;
  using MemberSignature = R (T::*)(P1, P2, P3, P4, P5, P6) const;

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
  inline _ConstTessMemberResultCallback_6_0(const T* object,
                                            MemberSignature member, P1 p1,
                                            P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
      : object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  R Run() override {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      return result;
    }
    R result = (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class T, class P1, class P2, class P3, class P4, class P5,
          class P6>
class _ConstTessMemberResultCallback_6_0<del, void, T, P1, P2, P3, P4, P5, P6>
    : public TessClosure {
 public:
  using base = TessClosure;
  using MemberSignature = void (T::*)(P1, P2, P3, P4, P5, P6) const;

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
  inline _ConstTessMemberResultCallback_6_0(const T* object,
                                            MemberSignature member, P1 p1,
                                            P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
      : object_(object),
        member_(member),
        p1_(p1),
        p2_(p2),
        p3_(p3),
        p4_(p4),
        p5_(p5),
        p6_(p6) {}

  void Run() override {
    if (!del) {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
    } else {
      (object_->*member_)(p1_, p2_, p3_, p4_, p5_, p6_);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class P3, class P4,
          class P5, class P6>
inline typename _ConstTessMemberResultCallback_6_0<false, R, T1, P1, P2, P3, P4,
                                                   P5, P6>::base*
NewPermanentTessCallback(
    const T1* obj, R (T2::*member)(P1, P2, P3, P4, P5, P6) const,
    typename Identity<P1>::type p1, typename Identity<P2>::type p2,
    typename Identity<P3>::type p3, typename Identity<P4>::type p4,
    typename Identity<P5>::type p5, typename Identity<P6>::type p6) {
  return new _ConstTessMemberResultCallback_6_0<false, R, T1, P1, P2, P3, P4,
                                                P5, P6>(obj, member, p1, p2, p3,
                                                        p4, p5, p6);
}
#endif

template <class A1>
class TessCallback1 {
 public:
  virtual ~TessCallback1() = default;
  virtual void Run(A1) = 0;
};

template <class R, class A1>
class TessResultCallback1 {
 public:
  virtual ~TessResultCallback1() = default;
  virtual R Run(A1) = 0;
};

template <class A1, class A2>
class TessCallback2 {
 public:
  virtual ~TessCallback2() = default;
  virtual void Run(A1, A2) = 0;
};

template <class R, class A1, class A2>
class TessResultCallback2 {
 public:
  virtual ~TessResultCallback2() = default;
  virtual R Run(A1, A2) = 0;
};

template <class A1, class A2, class A3>
class TessCallback3 {
 public:
  virtual ~TessCallback3() = default;
  virtual void Run(A1, A2, A3) = 0;
};

template <class R, class A1, class A2, class A3>
class TessResultCallback3 {
 public:
  virtual ~TessResultCallback3() = default;
  virtual R Run(A1, A2, A3) = 0;
};

template <class A1, class A2, class A3, class A4>
class TessCallback4 {
 public:
  virtual ~TessCallback4() = default;
  virtual void Run(A1, A2, A3, A4) = 0;
};

template <class R, class A1, class A2, class A3, class A4>
class TessResultCallback4 {
 public:
  virtual ~TessResultCallback4() = default;
  virtual R Run(A1, A2, A3, A4) = 0;
};

template <bool del, class R, class T, class A1>
class _TessMemberResultCallback_0_1 : public TessResultCallback1<R, A1> {
 public:
  typedef TessResultCallback1<R, A1> base;
  using MemberSignature = R (T::*)(A1);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_1(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  R Run(A1 a1) override {
    if (!del) {
      R result = (object_->*member_)(a1);
      return result;
    }
    R result = (object_->*member_)(a1);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class T, class A1>
class _TessMemberResultCallback_0_1<del, void, T, A1>
    : public TessCallback1<A1> {
 public:
  using base = TessCallback1<A1>;
  using MemberSignature = void (T::*)(A1);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_1(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  void Run(A1 a1) override {
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
inline typename _TessMemberResultCallback_0_1<false, R, T1, A1>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(A1)) {
  return new _TessMemberResultCallback_0_1<false, R, T1, A1>(obj, member);
}
#endif

template <bool del, class R, class A1>
class _TessFunctionResultCallback_0_1 : public TessResultCallback1<R, A1> {
 public:
  typedef TessResultCallback1<R, A1> base;
  using FunctionSignature = R (*)(A1);

 private:
  FunctionSignature function_;

 public:
  inline explicit _TessFunctionResultCallback_0_1(FunctionSignature function)
      : function_(function) {}

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(a1);
      return result;
    }
    R result = (*function_)(a1);
    //  zero out the pointer to ensure segfault if used again
    function_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class A1>
class _TessFunctionResultCallback_0_1<del, void, A1>
    : public TessCallback1<A1> {
 public:
  using base = TessCallback1<A1>;
  using FunctionSignature = void (*)(A1);

 private:
  FunctionSignature function_;

 public:
  inline explicit _TessFunctionResultCallback_0_1(FunctionSignature function)
      : function_(function) {}

  void Run(A1 a1) override {
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
inline typename _TessFunctionResultCallback_0_1<false, R, A1>::base*
NewPermanentTessCallback(R (*function)(A1)) {
  return new _TessFunctionResultCallback_0_1<false, R, A1>(function);
}

template <bool del, class R, class P1, class A1>
class _TessFunctionResultCallback_1_1 : public TessResultCallback1<R, A1> {
 public:
  typedef TessResultCallback1<R, A1> base;
  using FunctionSignature = R (*)(P1, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_1(FunctionSignature function, P1 p1)
      : function_(function), p1_(p1) {}

  virtual R Run(A1 a1) {
    if (!del) {
      R result = (*function_)(p1_, a1);
      return result;
    }
    R result = (*function_)(p1_, a1);
    //  zero out the pointer to ensure segfault if used again
    function_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class P1, class A1>
class _TessFunctionResultCallback_1_1<del, void, P1, A1>
    : public TessCallback1<A1> {
 public:
  using base = TessCallback1<A1>;
  using FunctionSignature = void (*)(P1, A1);

 private:
  FunctionSignature function_;
  typename remove_reference<P1>::type p1_;

 public:
  inline _TessFunctionResultCallback_1_1(FunctionSignature function, P1 p1)
      : function_(function), p1_(p1) {}

  void Run(A1 a1) override {
    if (!del) {
      (*function_)(p1_, a1);
    } else {
      (*function_)(p1_, a1);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class P1, class A1>
inline typename _TessFunctionResultCallback_1_1<false, R, P1, A1>::base*
NewPermanentTessCallback(R (*function)(P1, A1),
                         typename Identity<P1>::type p1) {
  return new _TessFunctionResultCallback_1_1<false, R, P1, A1>(function, p1);
}

template <bool del, class R, class T, class A1, class A2>
class _ConstTessMemberResultCallback_0_2
    : public TessResultCallback2<R, A1, A2> {
 public:
  typedef TessResultCallback2<R, A1, A2> base;
  using MemberSignature = R (T::*)(A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_2(const T* object,
                                            MemberSignature member)
      : object_(object), member_(member) {}

  R Run(A1 a1, A2 a2) override {
    if (!del) {
      R result = (object_->*member_)(a1, a2);
      return result;
    }
    R result = (object_->*member_)(a1, a2);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class T, class A1, class A2>
class _ConstTessMemberResultCallback_0_2<del, void, T, A1, A2>
    : public TessCallback2<A1, A2> {
 public:
  typedef TessCallback2<A1, A2> base;
  using MemberSignature = void (T::*)(A1, A2) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_2(const T* object,
                                            MemberSignature member)
      : object_(object), member_(member) {}

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(a1, a2);
    } else {
      (object_->*member_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _ConstTessMemberResultCallback_0_2<false, R, T1, A1, A2>::base*
NewPermanentTessCallback(const T1* obj, R (T2::*member)(A1, A2) const) {
  return new _ConstTessMemberResultCallback_0_2<false, R, T1, A1, A2>(obj,
                                                                      member);
}
#endif

template <bool del, class R, class T, class A1, class A2>
class _TessMemberResultCallback_0_2 : public TessResultCallback2<R, A1, A2> {
 public:
  typedef TessResultCallback2<R, A1, A2> base;
  using MemberSignature = R (T::*)(A1, A2);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_2(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  R Run(A1 a1, A2 a2) override {
    if (!del) {
      R result = (object_->*member_)(a1, a2);
      return result;
    }
    R result = (object_->*member_)(a1, a2);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class T, class A1, class A2>
class _TessMemberResultCallback_0_2<del, void, T, A1, A2>
    : public TessCallback2<A1, A2> {
 public:
  typedef TessCallback2<A1, A2> base;
  using MemberSignature = void (T::*)(A1, A2);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_2(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (object_->*member_)(a1, a2);
    } else {
      (object_->*member_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      member_ = nullptr;
      delete this;
    }
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2>
inline typename _TessMemberResultCallback_0_2<false, R, T1, A1, A2>::base*
NewPermanentTessCallback(T1* obj, R (T2::*member)(A1, A2)) {
  return new _TessMemberResultCallback_0_2<false, R, T1, A1, A2>(obj, member);
}
#endif

template <bool del, class R, class A1, class A2>
class _TessFunctionResultCallback_0_2 : public TessResultCallback2<R, A1, A2> {
 public:
  typedef TessResultCallback2<R, A1, A2> base;
  using FunctionSignature = R (*)(A1, A2);

 private:
  FunctionSignature function_;

 public:
  inline explicit _TessFunctionResultCallback_0_2(FunctionSignature function)
      : function_(function) {}

  R Run(A1 a1, A2 a2) override {
    if (!del) {
      R result = (*function_)(a1, a2);
      return result;
    }
    R result = (*function_)(a1, a2);
    //  zero out the pointer to ensure segfault if used again
    function_ = nullptr;
    delete this;
    return result;
  }
};

template <bool del, class A1, class A2>
class _TessFunctionResultCallback_0_2<del, void, A1, A2>
    : public TessCallback2<A1, A2> {
 public:
  typedef TessCallback2<A1, A2> base;
  using FunctionSignature = void (*)(A1, A2);

 private:
  FunctionSignature function_;

 public:
  inline explicit _TessFunctionResultCallback_0_2(FunctionSignature function)
      : function_(function) {}

  virtual void Run(A1 a1, A2 a2) {
    if (!del) {
      (*function_)(a1, a2);
    } else {
      (*function_)(a1, a2);
      //  zero out the pointer to ensure segfault if used again
      function_ = nullptr;
      delete this;
    }
  }
};

template <class R, class A1, class A2>
inline typename _TessFunctionResultCallback_0_2<false, R, A1, A2>::base*
NewPermanentTessCallback(R (*function)(A1, A2)) {
  return new _TessFunctionResultCallback_0_2<false, R, A1, A2>(function);
}

template <bool del, class R, class T, class P1, class P2, class A1, class A2>
class _TessMemberResultCallback_2_2 : public TessResultCallback2<R, A1, A2> {
 public:
  typedef TessResultCallback2<R, A1, A2> base;
  using MemberSignature = R (T::*)(P1, P2, A1, A2);

 private:
  T* object_;
  MemberSignature member_;
  typename remove_reference<P1>::type p1_;
  typename remove_reference<P2>::type p2_;

 public:
  inline _TessMemberResultCallback_2_2(T* object, MemberSignature member, P1 p1,
                                       P2 p2)
      : object_(object), member_(member), p1_(p1), p2_(p2) {}

  R Run(A1 a1, A2 a2) override {
    if (!del) {
      R result = (object_->*member_)(p1_, p2_, a1, a2);
      return result;
    }
    R result = (object_->*member_)(p1_, p2_, a1, a2);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class P1, class P2, class A1, class A2>
inline
    typename _TessMemberResultCallback_2_2<false, R, T1, P1, P2, A1, A2>::base*
    NewPermanentTessCallback(T1* obj, R (T2::*member)(P1, P2, A1, A2),
                             typename Identity<P1>::type p1,
                             typename Identity<P2>::type p2) {
  return new _TessMemberResultCallback_2_2<false, R, T1, P1, P2, A1, A2>(
      obj, member, p1, p2);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3>
class _ConstTessMemberResultCallback_0_3
    : public TessResultCallback3<R, A1, A2, A3> {
 public:
  typedef TessResultCallback3<R, A1, A2, A3> base;
  using MemberSignature = R (T::*)(A1, A2, A3) const;

 private:
  const T* object_;
  MemberSignature member_;

 public:
  inline _ConstTessMemberResultCallback_0_3(const T* object,
                                            MemberSignature member)
      : object_(object), member_(member) {}

  R Run(A1 a1, A2 a2, A3 a3) override {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3);
      return result;
    }
    R result = (object_->*member_)(a1, a2, a3);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3>
inline
    typename _ConstTessMemberResultCallback_0_3<false, R, T1, A1, A2, A3>::base*
    NewPermanentTessCallback(const T1* obj, R (T2::*member)(A1, A2, A3) const) {
  return new _ConstTessMemberResultCallback_0_3<false, R, T1, A1, A2, A3>(
      obj, member);
}
#endif

template <bool del, class R, class T, class A1, class A2, class A3, class A4>
class _TessMemberResultCallback_0_4
    : public TessResultCallback4<R, A1, A2, A3, A4> {
 public:
  typedef TessResultCallback4<R, A1, A2, A3, A4> base;
  using MemberSignature = R (T::*)(A1, A2, A3, A4);

 private:
  T* object_;
  MemberSignature member_;

 public:
  inline _TessMemberResultCallback_0_4(T* object, MemberSignature member)
      : object_(object), member_(member) {}

  virtual R Run(A1 a1, A2 a2, A3 a3, A4 a4) {
    if (!del) {
      R result = (object_->*member_)(a1, a2, a3, a4);
      return result;
    }
    R result = (object_->*member_)(a1, a2, a3, a4);
    //  zero out the pointer to ensure segfault if used again
    member_ = nullptr;
    delete this;
    return result;
  }
};

#ifndef SWIG
template <class T1, class T2, class R, class A1, class A2, class A3, class A4>
inline
    typename _TessMemberResultCallback_0_4<false, R, T1, A1, A2, A3, A4>::base*
    NewPermanentTessCallback(T1* obj, R (T2::*member)(A1, A2, A3, A4)) {
  return new _TessMemberResultCallback_0_4<false, R, T1, A1, A2, A3, A4>(
      obj, member);
}
#endif

#endif  // TESS_CALLBACK_SPECIALIZATIONS_H_
