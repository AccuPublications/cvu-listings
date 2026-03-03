/*
NAME
  WinTypeReader

DESCRIPTION
  The Windows implementation of OdrCheck
*/

#include "WinTypeReader.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "DIA SDK/include/cvconst.h"

#pragma comment(lib, "dbgHelp.lib")

namespace {
[[noreturn]] void raise_win32_error(std::string text) {
  throw std::runtime_error("Error " + std::to_string(GetLastError()) + " " +
                           text);
}

// Convert NUL terminated wide string to a MB string
std::string StringFromWideChar(wchar_t const *const wide_string) {
  size_t const len = wcslen(wide_string) + 1;
  size_t const n_bytes = len * sizeof(wchar_t);
  std::vector<char> ch_array(n_bytes);
  size_t retval{};
  wcstombs_s(&retval, ch_array.data(), n_bytes, wide_string, n_bytes);
  return std::string(ch_array.data(), retval);
}
} // namespace

// Load a module and check that type information is available
void TypeReaderImpl::Load(std::string const &file_name) {
  Unload();

  hmod_ =
      ::LoadLibraryEx(file_name.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
  if (hmod_ == nullptr) {
    raise_win32_error("loading image");
  }

  base_address_ = SymLoadModule64(self_, nullptr, file_name.c_str(), nullptr,
                                  reinterpret_cast<DWORD64>(hmod_), 0);
  if (base_address_ == 0) {
    raise_win32_error("loading module information");
  }

  IMAGEHLP_MODULE64 module_info{};
  module_info.SizeOfStruct = sizeof(module_info);
  if (!SymGetModuleInfo64(self_, base_address_, &module_info)) {
    raise_win32_error("getting module information: ");
  }

  if (!module_info.TypeInfo) {
    throw std::runtime_error("No type information available");
  }
}

void TypeReaderImpl::Unload() {
  if (base_address_) {
    // Unload previous module
    SymUnloadModule(self_, base_address_);
    base_address_ = 0;
  }
  if (hmod_) {
    // Unload previous library
    ::FreeLibrary(hmod_);
    hmod_ = nullptr;
  }
}

// Construct ODR checker for 'file_name'
TypeReaderImpl::TypeReaderImpl(TypeCallback &callback) : callback_{callback} {
  if (!::SymInitialize(self_, nullptr, false)) {
    raise_win32_error("initializing DbgHelp library");
  }
  SetErrorMode(SEM_FAILCRITICALERRORS);
}

// Enumerate the types in the loaded module
void TypeReaderImpl::EnumerateTypes() {
  if (!SymEnumTypes(self_, base_address_, OdrCallback, (void *)this)) {
    raise_win32_error("enumerating types");
  }
}

// Called by Debug engine during ODR detection
BOOL CALLBACK TypeReaderImpl::OdrCallback(PSYMBOL_INFO pSym,
                                          ULONG /*SymbolSize*/,
                                          PVOID thisObject) {
  static_cast<TypeReaderImpl *>(thisObject)->ProcessType(*pSym);
  return true;
}

// Process a symbol info for a single type
void TypeReaderImpl::ProcessType(const SYMBOL_INFO &sym) {
  if (FalsePositive(sym)) {
    return;
  }

  std::string name{sym.Name, sym.NameLen};
  
  if (sym.NameLen == MAX_SYM_NAME - 1) {
    // The name is truncated, so explicitly fetch the full name
    wchar_t *p_name{};
    if (SymGetTypeInfo(self_, base_address_, sym.Index, TI_GET_SYMNAME,
                         &p_name)) {
      name = StringFromWideChar(p_name);
      LocalFree(p_name);
    }   
  }

  callback_.OnDefinedType(name, sym.Size);
}

// Check for various false positive conditions
bool TypeReaderImpl::FalsePositive(const SYMBOL_INFO &sym) {
  if (sym.Tag != SymTagUDT)
    return true;

  // Check for an incomplete type
  if (sym.Size == 0)
    return true;

  const std::string_view name{sym.Name, sym.NameLen};

  // Check for types in anonymous namespaces
  static const std::string anonymous{"`anonymous-namespace'::"};
  if (name.find(anonymous) != std::string::npos) {
    return true;
  }

  // Check for unnamed types
  const auto last_colon = name.find_last_of(':');
  static const std::string unnamed{"<unnamed-"};
  if (name.compare(last_colon + 1, unnamed.size(), unnamed) == 0) {
    return true;
  }

  // Check for unqualified entry for qualified types
  if (last_colon == std::string::npos) {
    DWORD64 nested{};
    (void)SymGetTypeInfo(self_, base_address_, sym.Index, TI_GET_NESTED,
                         &nested);
    if (nested) {
      // There will also be an unnested entry with the fully qualified name
      return true;
    }
  }

  return false;
}
