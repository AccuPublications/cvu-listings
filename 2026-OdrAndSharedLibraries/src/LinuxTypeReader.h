/*
NAME
  LinuxTypeReader.h

DESCRIPTION
  Class definition for the Linux implementation of OdrCheck

PREREQUISITES
  sudo apt-get install libdw-dev
*/

#include <cstdio>
#include <cstring>
#include <elfutils/libdw.h>
#include <string>
#include <string_view>

#include "OdrCheck.h"

class TypeReaderImpl : public TypeReader {
public:
  TypeReaderImpl(TypeCallback &callback) : callback_{callback} {}

  // Load a module and check that type information is available
  void Load(std::string const &module) override;

  // Unload a module
  void Unload() override;

  // Enumerate the types in the loaded module
  void EnumerateTypes() override;

private:
  TypeCallback &callback_;
  int fd_{-1};
  Dwarf *dbg_{};
  std::map<Dwarf_Off, std::string> qualified_names_;

  struct NamedScope {
    NamedScope(const NamedScope *scope, const char *name)
        : parent_(scope), name_(name) {}

    const NamedScope *parent_;
    const std::string_view name_;
  };

  // Recurse through the DIE finding all (named) types in its scope
  void FindTypesInScope(Dwarf_Die &die, const NamedScope *scope = nullptr);

  // Process the entry for a named type
  void TypeEntry(Dwarf_Die &type, const NamedScope *scope);

  // Fully qualify the supplied name in this scope
  static std::string QualifiedName(const std::string_view name,
                                   const NamedScope *scope);
};
