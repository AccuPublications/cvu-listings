/*
NAME
  WinTypeReader.h

DESCRIPTION
  Class definition for the Windows implementation of OdrCheck
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dbghelp.h>
#include <string>

#include "OdrCheck.h"

class TypeReaderImpl : public TypeReader {
public:
  TypeReaderImpl(TypeCallback &callback);

  // Load a module and check that type information is available
  void Load(std::string const &module) override;

  // Unload a module
  void Unload() override;

  // Enumerate the types in the loaded module
  void EnumerateTypes() override;

private:
  TypeCallback &callback_;
  HANDLE self_{GetCurrentProcess()};
  HMODULE hmod_{};
  DWORD64 base_address_{};

  // Called by Debug engine during ODR detection
  static BOOL CALLBACK OdrCallback(PSYMBOL_INFO pSym, ULONG SymbolSize,
                                   PVOID thisObject);

  void ProcessType(const SYMBOL_INFO &sym);

  bool FalsePositive(const SYMBOL_INFO &sym);
};
