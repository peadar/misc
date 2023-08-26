#include <cxxabi.h>
#include <iostream>
#include <map>

static std::map<int, std::string> errors = {
   { 0,  "The demangling operation succeeded" },
   { -1, "A memory allocation failure occurred" },
   { -2, "mangled_name is not a valid name under the C++ ABI mangling rules." },
   { -3, "One of the arguments is invalid." }
};

int
main(int argc, char *argv[])
{
    size_t initial_size = 1024;
    size_t size = initial_size;
    char *buf = (char *)malloc(size);
    int status;
    for (int i = 1; i < argc; ++i) {
        buf = __cxxabiv1::__cxa_demangle(argv[i], buf, &size, &status);
        if (status == 0) {
            std::cout << buf << std::endl;
        } else {
           std::cerr << "failed to demangle: ";
           auto it = errors.find(status);
           if (it != errors.end())
              std::cerr << it->second;
           else
              std::cerr << status;
           std::clog << std::endl;
        }
    }
    free(buf);
}
