#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


int cpucount;
int fd;
        
struct aperfmperf {
    uint64_t aperf, mperf;
};

aperfmperf oldMP;

static uint64_t readmsr(size_t regno)
{
    lseek(fd, regno, SEEK_SET);
    uint64_t val;
    if (read(fd, &val, sizeof val) != sizeof val) {
	std::clog << "read failed: " << strerror(errno) << std::endl;
        abort();
    }
    return val;
}

static void writemsr(size_t regno, uint64_t val)
{
    lseek(fd, regno, SEEK_SET);
    if (write(fd, &val, sizeof val) != sizeof val) {
	std::clog << "write failed: " << strerror(errno) << std::endl;
        abort();
    }
}


#define rdmsrl(msr, val)                        \
        ((val) = readmsr((msr)))

#define MSR_IA32_MPERF                  0x000000e7
#define MSR_IA32_APERF                  0x000000e8


static inline void get_aperfmperf(struct aperfmperf *am)
{
    rdmsrl(MSR_IA32_APERF, am->aperf);
    rdmsrl(MSR_IA32_MPERF, am->mperf);
}               

#define APERFMPERF_SHIFT 10
static inline
unsigned long calc_aperfmperf_ratio(struct aperfmperf *old,
                                    struct aperfmperf *neu)
{       
    uint64_t aperf = neu->aperf - old->aperf;
    uint64_t mperf = neu->mperf - old->mperf;
    unsigned long ratio = aperf;

    std::cout
    << "(new aperf(" << neu->aperf << ") - old aperf(" << old->aperf << ") = " << aperf << ")"
    << " / "
    << "(new mperf(" << neu->mperf << ") - old mperf(" << old->mperf << ") = " << mperf << ")"
    ;

    mperf >>= APERFMPERF_SHIFT;                   
    if (mperf)
	ratio = aperf / mperf;

    std::cout << " = " << ratio << "\n";

    return ratio;
}                    
                   
        


static unsigned long scale_aperfmperf(void)
{
    struct aperfmperf val, *old = &oldMP;
    unsigned long ratio, flags;

    get_aperfmperf(&val);
    
    ratio = calc_aperfmperf_ratio(old, &val);
    *old = val;

    return ratio;
}


int getreg(const char *p) {
    if (strcasecmp(p, "mperf") == 0)
        return MSR_IA32_MPERF;
    if (strcasecmp(p, "aperf") == 0)
        return MSR_IA32_APERF;
    return strtoll(p, 0, 0);
}

int
main(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "c:w:r:s:")) != -1) {
        switch (c) {
            case 'c': {
                char buf[1024];
                sprintf(buf, "/dev/cpu/%s/msr", optarg);
                fd = open(buf, O_RDWR);
                if (fd == -1) {
                    std::clog << "failed to open " << buf << ": " << strerror(errno) << std::endl;
                    abort();
                }
                std::clog << "opened " << buf << std::endl;
                break;
            }

            case 's':
                for (int i = 0; i < atoi(optarg); ++i) {
                    std::clog << "scale: " << scale_aperfmperf() << "\n";
                    std::clog << "\n";
                    sleep(1);
                }
                break;

            case 'w': {
                char *p = optarg;
                char *eq = strchr(p, '=');
                if (eq == 0) {
                    std::clog << "invalid register assignment" << std::endl;
                    abort();
                }
                *eq++ = 0;
                int reg = getreg(p);
                uint64_t val = strtoull(eq, 0, 0);
                std::clog << "write " << val << " to " << reg << std::endl;
                writemsr(reg, val);
                break;
            }
                
            case 'r': {
                int reg = getreg(optarg);
                std::clog << "read " << reg << ": " << readmsr(reg) << std::endl;
                break;
            }
        }
    }
}
