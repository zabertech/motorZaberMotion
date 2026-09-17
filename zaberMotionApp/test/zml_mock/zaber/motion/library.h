#ifndef MOCK_LIBRARY
#define MOCK_LIBRARY

#include <string>

namespace zaber {
namespace motion {

class Library {
public:
    static void setHostApplication(const std::string& s) { (void)s; }
};

} // motion
} // zaber

#endif
