#pragma once

namespace One {
namespace Two {
class NamespaceClass { // ODR
  OdrViolation o_;
} n;
} // namespace Two
} // namespace One

namespace NSForFunction {
void FunctionInNamespace() {
  struct FunctionScope {
    OdrViolation o_;
  } test;
  (void)test;
}
} // namespace NSForFunction

class OuterClass {
public:
  class NestedClass { // ODR
    OdrViolation o_;
  };
  union NestedUnion { // ODR
    int i_{};
    OdrViolation o_;
  };
};
OuterClass::NestedClass oc;
OuterClass::NestedUnion ou;

// Cases that not ODR violations as they are in an anonymous namespace
namespace {
struct NotAnOdrViolation {
  int i;
  struct FalsePositiveTest {
    int i;
  };
};
void bar() {
  struct AnonymousFunctionScope : OdrViolation {
  } test;
  (void)test;
  NSForFunction::FunctionInNamespace();
}
} // namespace
