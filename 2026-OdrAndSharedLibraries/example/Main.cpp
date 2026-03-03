#include <iostream>
#include <string>

struct Data
{
   std::string first;
   std::string last;
};

extern Data getData();

int main()
{
    Data const details( getData() );
    std::cout << details.first << " " << details.last << std::endl;
}
