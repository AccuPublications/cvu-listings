struct OdrViolation {
  int i;
  static void foo() {
    struct FunctionScope : OdrViolation {
    } test;
    (void)test;
  }
};

#include "TestOdrDll.h"

#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
  int odrtest2(const OdrViolation &odr) {
  NotAnOdrViolation not_an_odr;
  not_an_odr.i = odr.i;
  OdrViolation::foo();
  bar();
  return not_an_odr.i;
}
