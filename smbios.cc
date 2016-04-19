#include <sys/mman.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <unistd.h>
#include <iostream>

class Error : public std::exception {
   std::string msg;
public:
   const char *what() const throw() { return msg.c_str(); }
   Error(const std::string &msg_) : msg(msg_) {}
   ~Error() throw () {}
};

class SysError : public Error {
public:
   SysError(const std::string &err) : Error(err + "(" + strerror(errno) + ")") {}
   ~SysError() throw () {}
};

struct Closer {
    int fd;
    Closer(int fd_) : fd(fd_) {}
    ~Closer() { close(fd); }
    operator int() { return fd; }
};

namespace SMBIOS {
// Definitions from Version 2.8.0 DMTF Standard.

struct EntryPoint { // Section 5.2.1, Table 1
    char anchor[4];
    uint8_t checksum;
    uint8_t length;
    struct {
        uint8_t major;
        uint8_t minor;
    } version;
    uint16_t maxStructureSize;
    uint8_t entryPointRevision;
    uint8_t formattedArea[5];
    uint8_t intermediateAnchorString[5];
    uint8_t intermediateChecksum;
    uint16_t structureTableLength;
    uint32_t structureTableAddr;
    uint16_t smbiosStructureCount;
    uint16_t bcdRevision;
} __attribute__((packed));

struct StructureHeader { // Section 6.1.2, table 2.
    uint8_t type;
    uint8_t length;
    uint16_t handle;
    const char *getstring(size_t off);
    const uint8_t *end() { return (const uint8_t *)this + length; }
} __attribute__((packed)); 

class BIOSInfo {
    const uint8_t *structures;
    EntryPoint entrypoint;
public:
    class iterator {
        BIOSInfo &bios;
        StructureHeader *current;
    public:
        iterator &operator++() {
            const uint8_t *string = current->end();
            if (string[0] == 0)
                string++;
            while (*string)
                string += strlen((const char *)string) + 1;
            string++;
            current = (StructureHeader *)string;
            return *this;
        }
        bool operator == (const iterator &rhs) { return current == rhs.current; }
        bool operator != (const iterator &rhs) { return ! (*this == rhs); }
        iterator(BIOSInfo &bios_, const uint8_t *begin_) : bios(bios_) , current((StructureHeader *)begin_) {}
        StructureHeader *operator *() { return (StructureHeader *)current; }
    };
    iterator begin() { return iterator(*this, structures); }
    iterator end() { return iterator(*this, structures + entrypoint.structureTableLength); }
    BIOSInfo();
    ~BIOSInfo() { delete[] structures; }
};

BIOSInfo::BIOSInfo()
{
    uint8_t bios[0x20000];
    Closer fd = open("/dev/mem", O_RDONLY);
    if (fd == -1)
        throw SysError("cannot open /dev/mem");

    int rc = pread(fd, (void *)bios, sizeof bios, 0xe0000);
    if (rc != sizeof bios)
        throw SysError("pread");
    auto endBios = bios + rc;
    auto p = bios;
    for (p = bios; memcmp(p, "_SM_", 4) != 0; p += 16)
        if (endBios - p < 16)
            throw Error("SM BIOS Not Found");
    entrypoint = *(EntryPoint *)p;

    unsigned len = p[0x05] == 0x1E ? 0x1F : p[0x05];
    uint8_t sum=0;
    while (len--)
        sum += *p++;
    if (sum != 0)
        throw Error("invalid BIOS checksum");

    structures = new uint8_t[entrypoint.structureTableLength];
    rc = pread(fd, (void *)structures, entrypoint.structureTableLength, entrypoint.structureTableAddr);
    if (rc != entrypoint.structureTableLength) {
        delete[] structures;
        throw SysError("pread");
    }
}

const char *
StructureHeader::getstring(size_t field)
{
    uint8_t *p = (uint8_t *)this;
    uint8_t off = p[field];

    const char *str = (const char *)this + length;
    if (off == 0)
        return "(Not Specified)";
    for (size_t i = 0; ++i != off;) {
        if (str[0] == 0)
            return "(invalid string reference)";
        str = str + strlen(str) + 1;
    }
    return str;
}

}

int
main(int, char *[])
{
   SMBIOS::BIOSInfo bios;
   for (auto header : bios) {
      switch (header->type) {
         case 1:
            std::cout << "\nSystem:" << std::endl
            << "manufacturer: " << header->getstring(4) << std::endl
            << "product: " << header->getstring(5) << std::endl
            << "version: " << header->getstring(6) << std::endl
            << "serial: " << header->getstring(7) << std::endl
            ;
            break;
         default:
            const char *p = reinterpret_cast<const char *>(header->end());
            while (*p) {
               std::clog << p << std::endl;
               p += strlen(p);
            }
            break;

      }
   }
   return 0;
}

