#include "Lib.h"

#include <iostream>

int main() {
  Mine::LibraryClass lib;
  std::cout << "in main - lib.first: " << lib.first << '\n';
  std::cout << "in main - lib.last: " << lib.last << '\n';

  Test(lib);
  Mine::foo();
  Mine::bar();
}