#pragma once

namespace Mine {
struct LibraryClass { // ODR
  LibraryClass() {}
  int first = 1;
#ifdef LIB_DEBUG
  int tracer = 42;
#endif // padding
  int last = 2;
};

struct Nested1 {
  struct Nested2 {
    struct Nested3 : LibraryClass {};
  };
};
Nested1::Nested2::Nested3 test3;

namespace NS1::NS2 {
struct NestedNs {
  LibraryClass l;
} test;
} // namespace NS1::NS2

void foo() {
  struct FunctionScope {
    LibraryClass l;
  } test; // ODR
}

struct SS {
  struct {
    char ch = 1;
    struct TT {
      LibraryClass l;
    } t;
  } s;
};
SS ss;
decltype(SS::s.t) u;

namespace {
struct NestedAnonymous {
  LibraryClass l;
} nested; // false positive

void bar() {
  struct AnonymousScope {
    LibraryClass l;
  } test; // false positive
}
} // namespace
} // namespace Mine

#ifdef _MSC_VER
#ifdef LIB_EXPORT
__declspec(dllexport)
#else
__declspec(dllimport)
#endif
#elif __GNUC__
__attribute__ ((visibility("default")))
#endif

void Test(const Mine::LibraryClass &lib);
