#include <stdint.h>
#include <string.h>

namespace Vordel {
struct BIOSHandle {
    /*   0 */ char anchor[4];
    /*   4 */ uint8_t checksum;
    /*   5 */ uint8_t length;
    /*   6 */ uint8_t major;
    /*   7 */ uint8_t minor;
    /*   8 */ uint16_t maxStructureSize;
    /*   a */ uint8_t entryPointRevision;
    /*   b */ uint8_t formattedArea[5];
    /*  10 */ uint8_t intermediateAnchorString[5];
    /*  15 */ uint8_t intermediateChecksum;
    /*  16 */ uint16_t structureTableLength;
    /*  18 */ uint32_t structureTableAddr;
    /*  1c */ uint16_t smbiosStructureCount;
    /*  1e */ uint16_t bcdRevision;
} __attribute__((packed));

struct StructureHeader {
    uint8_t type;
    uint8_t length;
    uint16_t handle;
} __attribute__((packed));

class SMBIOS {
    static const unsigned biosLen = 0x20000;
    static const unsigned biosBase = 0xe0000;
    int memFd;
    struct BIOSHandle *handle;
    const uint8_t *biosMem;
    const uint8_t *structureTable;
public:
    class iterator {
        SMBIOS &bios;
        const uint8_t *place;
    public:
        iterator &operator++() {
            StructureHeader *head = (StructureHeader *)place;
            const uint8_t *string = place + head->length;
            if (string[0] == 0 && string[1] == 0) {
                place = string + 2;
            } else {
                while (*string)
                    string += strlen((char *)string) + 1;
                place = string + 1;
            }
            return *this;
        }

        bool operator == (const iterator &rhs) {
            return place == rhs.place || place >= bios.structureTable + bios.handle->structureTableLength && rhs.place >= bios.structureTable + bios.handle->structureTableLength;
        }

        bool operator != (const iterator &rhs) {
            return ! (*this == rhs);
        }

        iterator(SMBIOS &bios_, const uint8_t *place_)
            : bios(bios_)
            , place(place_)
        {
        }
        StructureHeader *operator *() {
            return (StructureHeader *)place;
        }
    };

    iterator begin() {
        return iterator(*this, structureTable);
    }

    iterator end() {
        return iterator(*this, structureTable + handle->structureTableLength);
    }

    int minor() { return handle->minor; }
    int major() { return handle->major; }

    DSO_EXPORT static const char *getStructureString(const StructureHeader *header, size_t off);
    DSO_EXPORT SMBIOS();
    DSO_EXPORT ~SMBIOS();
};

// Type 3
struct ChassisInfo {
    struct StructureHeader header;
    uint8_t manufacturer;
    uint8_t lockable;
    uint8_t version;
    uint8_t serial;
    uint8_t assetTag;
    uint8_t bootState;
    uint8_t psuState;
    uint8_t thermalState;
    uint8_t securityState;
    uint16_t oemDefined;
    uint8_t height;
    uint8_t powerCords;
    uint8_t containedElementCount;
    uint8_t containedElementLength;
} __attribute__((packed));

}
