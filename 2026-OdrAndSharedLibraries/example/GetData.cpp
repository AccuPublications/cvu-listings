#include <string>

#ifdef LIB_EXPORT
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
#else
#define EXPORT 
#endif // LIB_EXPORT

struct Data
{
    std::string first;
    std::string second;
    std::string last;
};

EXPORT Data getData()
{
   // Implementation details omitted...
   return { "Roger", "Martin", "Orr" };
}
