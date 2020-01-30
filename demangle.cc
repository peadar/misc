#include <cxxabi.h>
#include <iostream>

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
        }
    }
    free(buf);
}
