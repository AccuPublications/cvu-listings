struct OdrViolation {
  int i;
  int j;
  struct FalsePositiveTest {
    int i;
    int j;
  };
  static void foo() {
    struct FunctionScope : OdrViolation {
    } test;
    (void)test;
  }
};

#include "TestOdrDll.h"

extern
#ifdef _WIN32
    __declspec(dllimport)
#endif
    int
    odrtest2(const OdrViolation &odr);

#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
int odrtest1() {
  OdrViolation odr;
  odr.i = 1;
  NotAnOdrViolation not_an_odr;
  not_an_odr.i = odrtest2(odr);
  OdrViolation::foo();
  bar();
  return not_an_odr.i;
}
