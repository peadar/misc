#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

int
main(int argc, char *argv[])
{

    int base = 16;
    int c;

    while ((c = getopt(argc, argv, "d")) != -1) {
        switch (c) {
            case 'd':
                base = 10;
                break;
            default:
                std::clog
                    << "usage: u+ [-d] <codepoints...>" << std::endl
                    << " use -d for decimal codepoints. Default is hex." << std::endl;
                return -1;
        }
    }

    for (int i = optind; i < argc; ++i) {
        uint8_t buf[8], *utf8 = buf + 8, valmask, pfx;
        uint32_t value = strtoll(argv[i], 0, base);
        for (valmask = 0x7f, pfx = 0x00;;) {
            if ((value & valmask) == value) {
                *--utf8 = value | pfx;
                break;
            }
            // this byte won't fit. Output "10" + top 6 bits.
            *--utf8 = (value & 0x3f) | 0x80;
            if (pfx == 0) {
                // first octet: start mask at 110
                pfx = 0xc0;
                valmask = 0x1f;
            } else {
                pfx = (pfx >> 1) | 0x80;
                valmask = valmask >> 1;
            }
            value >>=6; // We've written 6 bits.
        }
        fwrite(utf8, 1, 8 - (utf8 - buf), stdout);
    }
    return 0;
}
