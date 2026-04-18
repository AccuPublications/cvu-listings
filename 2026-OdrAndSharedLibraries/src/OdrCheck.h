/*
NAME
  OdrCheck

DESCRIPTION
  Check for ODR violations in the debug information
*/
#pragma once

#include <map>
#include <set>
#include <string>

struct TypeCallback {
  // Called for each possible type
  virtual void OnDefinedType(const std::string &name, size_t size) = 0;

protected:
  virtual ~TypeCallback() = default;
};

class TypeReader {
public:
  virtual ~TypeReader() = default;

  // Load a module and check that type information is available
  virtual void Load(std::string const &module) = 0;

  // Unload a module
  virtual void Unload() = 0;

  // Enumerate the types in the loaded module
  virtual void EnumerateTypes() = 0;
};
