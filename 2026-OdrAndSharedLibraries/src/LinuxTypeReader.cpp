/*
NAME
  LinuxTypeReader

DESCRIPTION
  The Linux implementation of OdrCheck
*/

#include "LinuxTypeReader.h"

#include <cstring>
#include <dwarf.h>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace {
[[noreturn]] void raise_dwarf_error(std::string text, int errcode = -1) {
  throw std::runtime_error("Error from " + text + ": " + dwarf_errmsg(errcode));
}
} // namespace

// Load a module and check that type information is available
void TypeReaderImpl::Load(std::string const &file_name) {
  Unload();

  fd_ = open(file_name.c_str(), O_RDONLY);
  if (fd_ < 0) {
    throw std::runtime_error("Cannot open file");
  }

  // Initialize libdw for this file
  dbg_ = dwarf_begin(fd_, DWARF_C_READ);
  if (dbg_ == nullptr) {
    raise_dwarf_error("dwarf_begin");
  }
}

void TypeReaderImpl::Unload() {
  if (dbg_) {
    (void)dwarf_end(dbg_);
    dbg_ = nullptr;
  }
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

// Enumerate the types in the loaded module
void TypeReaderImpl::EnumerateTypes() {
  Dwarf_Off off = 0;
  constexpr auto done{static_cast<Dwarf_Off>(-1)};
  Dwarf_Off next_off;
  size_t header_size;

  // Iterate through compilation units (.debug_info)
  while (off != done && dwarf_nextcu(dbg_, off, &next_off, &header_size,
                                     nullptr, nullptr, nullptr) == 0) {
    Dwarf_Die cu_die;
    // Get the DIE for the compilation unit
    if (dwarf_offdie(dbg_, off + header_size, &cu_die) == nullptr) {
      raise_dwarf_error("dwarf_offdie");
    }
    qualified_names_.clear();
    FindTypesInScope(cu_die);
    off = next_off;
  }
}

// Recurse through the DIE finding all (named) types in its scope
void TypeReaderImpl::FindTypesInScope(Dwarf_Die &die, const NamedScope *scope) {
  const auto tag{dwarf_tag(&die)};

  switch (tag) {
  case DW_TAG_class_type:
  case DW_TAG_structure_type:
  case DW_TAG_union_type:
    TypeEntry(die, scope);
    break;

  // For function only the declarations appear to be nested inside their scope,
  // so save the fully qualified name for use when processing a later definition
  case DW_TAG_subprogram:
    if (dwarf_hasattr(&die, DW_AT_declaration)) {
      if (scope && dwarf_hasattr(&die, DW_AT_name)) {
        qualified_names_[dwarf_dieoffset(&die)] =
            QualifiedName(dwarf_diename(&die), scope);
      }
      return;
    }
  }

  /* Find first child, if any */
  Dwarf_Die child{};
  if (dwarf_child(&die, &child) != 0) {
    return;
  }

  // Check for named scopes that can contain types
  const char *scope_name{};
  switch (tag) {
  case DW_TAG_subprogram:
    if (!dwarf_hasattr_integrate(&die, DW_AT_external)) {
      return;
    }
    // recover saved scope, if any, when none present from the declaration
    if (!scope && dwarf_hasattr(&die, DW_AT_specification)) {
      Dwarf_Attribute attr;
      if (dwarf_attr(&die, DW_AT_specification, &attr) == nullptr) {
        raise_dwarf_error("draft_attr for specification");
      }
      Dwarf_Die spec{};
      if (dwarf_formref_die(&attr, &spec) == nullptr) {
        raise_dwarf_error("dwarf_formref_die");
      }
      auto it = qualified_names_.find(dwarf_dieoffset(&spec));
      if (it != qualified_names_.cend()) {
        scope_name = it->second.c_str();
        break;
      }
    }
    scope_name = dwarf_diename(&die);
    break;

  case DW_TAG_namespace:
    if (!dwarf_hasattr(&die, DW_AT_name)) {
      return; // anonymous namespace
    }
    scope_name = dwarf_diename(&die);
    break;

  case DW_TAG_class_type:
  case DW_TAG_structure_type:
  case DW_TAG_union_type:
    if (dwarf_hasattr(&die, DW_AT_name)) {
      scope_name = dwarf_diename(&die);
    }
    break;

  case DW_TAG_compile_unit:
    break;

  default:
    return;
  }

  if (scope_name) {
    const NamedScope this_scope{scope, scope_name};
    scope = &this_scope;
  }

  // Recurse over all children
  do {
    FindTypesInScope(child, scope);
  } while (dwarf_siblingof(&child, &child) == 0);
}

// Process the entry for a type
void TypeReaderImpl::TypeEntry(Dwarf_Die &type_die, const NamedScope *scope) {
  if (!dwarf_hasattr(&type_die, DW_AT_byte_size)) {
    // Not a complete type
    return;
  }

  Dwarf_Attribute attr;
  if (dwarf_attr(&type_die, DW_AT_byte_size, &attr) == nullptr) {
    raise_dwarf_error("dwarf_attr for byte_size");
  }
  Dwarf_Word byte_size{};
  if (dwarf_formudata(&attr, &byte_size) != 0) {
    raise_dwarf_error("dwarf_formudata for byte_size");
  }

  if (const char *name = dwarf_diename(&type_die)) {
    callback_.OnDefinedType(QualifiedName(name, scope), byte_size);
  }
}

// Fully qualify the supplied name in this scope
std::string TypeReaderImpl::QualifiedName(const std::string_view name,
                                          const NamedScope *scope) {
  static const std::string_view delim("::");

  // Calculate length
  size_t length{name.size()};
  for (const NamedScope *p = scope; p; p = p->parent_) {
    length += delim.size() + p->name_.size();
  }

  // Build fully qualified name
  std::string full_name(length, '\0');
  auto it = full_name.rbegin();
  auto prepend = [&it](std::string_view sv) {
    it = std::copy(sv.rbegin(), sv.rend(), it);
  };

  prepend(name);
  for (const NamedScope *p = scope; p; p = p->parent_) {
    prepend(delim);
    prepend(p->name_);
  }

  return full_name;
}