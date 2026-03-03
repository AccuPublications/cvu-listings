#include "Lib.h"

#include <iostream>

void Test(const Mine::LibraryClass &lib) {
  std::cout << "in lib - lib.first: " << lib.first << '\n';
  std::cout << "in lib - lib.last: " << lib.last << '\n';
  Mine::foo();
  Mine::bar();
}
