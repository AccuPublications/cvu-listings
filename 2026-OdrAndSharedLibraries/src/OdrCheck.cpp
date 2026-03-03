/*
NAME
  OdrCheck

DESCRIPTION
  Check for ODR violations in the debug information
*/

#include "OdrCheck.h"
#ifdef WIN32
#include "WinTypeReader.h"
#else
#include "LinuxTypeReader.h"
#endif // WIN32

#include <iostream>
#include <map>
#include <set>
#include <string>

class OdrCheck : private TypeCallback {
public:
  int Run(const std::string &file_name);

protected:
  // Called for each possible type
  void OnDefinedType(const std::string &name, size_t size) override;

private:
  std::map<std::string, std::set<size_t>> sizes_;
  int violations_{};
  TypeReaderImpl reader_{*this};
};

// Look for potential One Definition Rule (odr) violations
int OdrCheck::Run(const std::string &file_name) {
  violations_ = 0;

  try {
    reader_.Load(file_name);

    std::cout << "Checking " << file_name << '\n';

    reader_.EnumerateTypes();
  } catch (const std::exception &ex) {
    std::cerr << "Exception processing " << file_name << ": " << ex.what()
              << '\n';
  }
  reader_.Unload();

  return violations_;
}

// Called for each possible type
void OdrCheck::OnDefinedType(const std::string &type, size_t size) {
  auto &set = sizes_[type];
  if (set.insert(size).second && set.size() > 1) {
    ++violations_;
    if (set.size() == 2) {
      std::cout << type << " has changed size (" << *set.rbegin()
                << " != " << *set.begin() << ")\n";
    } else {
      std::cout << type << " has another size (" << size << ")\n";
    }
  }
}

// Main program
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "OdrCheck <binary_file> <binary_file>\n";
    std::cout << "Report any types with definition of different sizes in the "
                 "debug information for the binary files provided\n";
    return 1;
  }

  int result{};
  try {
    OdrCheck app;

    for (int idx = 1; idx < argc; ++idx) {
      result += app.Run(argv[idx]);
    }
  } catch (std::exception &ex) {
    std::cerr << "Unexpected exception: " << ex.what() << '\n';
    result = 1;
  }
  return result;
}
