#include <iostream>
#include <stdlib.h>
#include <stdint.h>
template<typename inttype_t> int log2i(inttype_t value);

template <typename T> struct Masks { };
    template <> struct Masks<uint8_t > {
    static const uint8_t size() { return 2; }
    static const uint8_t startmask() { return 0xf0; }
}; 

template <> struct Masks<uint16_t > {
    static const uint16_t size() { return 3; }
    static const uint16_t startmask() { return 0xff00; }
};

template <> struct Masks<uint32_t > {
    static const uint32_t size() { return 4; }
    static const uint32_t startmask() { return 0xffff0000; }
};

template <> struct Masks<uint64_t> {
    static const uint64_t size() { return log2i<uint64_t>(sizeof (uint64_t)); }
    static const uint64_t startmask() { return 0xffffffff00000000; }
};

template<typename inttype_t> int log2i(inttype_t value) {
    typedef Masks<inttype_t> MyMasks;
    inttype_t mask = MyMasks::startmask();
    int result = 0;
    int i = 1 << MyMasks::size();
    for (;;) {
        if (value & mask) {
            value >>= i;
            result |= i;
        }
        int nexti = i >> 1;
        if (nexti == 0)
            break;
        mask = mask >> (i | nexti) << nexti;
        i = nexti;
    }
    return result;
}

typedef uint64_t testint;
int
main(int argc, char *argv[])
{
    char *p;
    testint value = strtoull(argv[1], &p, 0);
    std::cout << "log2(" << value << ") == " << log2i(value) << std::endl;
}
