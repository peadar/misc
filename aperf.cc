#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


#define MSR_IA32_MPERF                  0x000000e7
#define MSR_IA32_APERF                  0x000000e8

size_t cpucount;
int fds[100];
        
struct aperfmperf {
    uint64_t aperf, mperf;
};

aperfmperf oldMPs[100];

static uint64_t readmsr(size_t cpu, size_t regno)
{
    lseek(fds[cpu], regno, SEEK_SET);
    uint64_t val;
    if (read(fds[cpu], &val, sizeof val) != sizeof val) {
	std::clog << "read failed: " << strerror(errno) << std::endl;
        abort();
    }
    return val;
}

static void writemsr(size_t cpu, size_t regno, uint64_t val)
{
    lseek(fds[cpu], regno, SEEK_SET);
    if (write(fds[cpu], &val, sizeof val) != sizeof val) {
	std::clog << "write failed: " << strerror(errno) << std::endl;
        abort();
    }
}

static inline void get_aperfmperf(size_t cpu, struct aperfmperf *am)
{
    am->aperf = readmsr(cpu, MSR_IA32_APERF);
    am->mperf = readmsr(cpu, MSR_IA32_MPERF);
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
    << "new aperf(" << neu->aperf << ") - old aperf(" << old->aperf << ") = " << aperf << ")"
    << " / "
    << "(new mperf(" << neu->mperf << ") - old mperf(" << old->mperf << ") = " << mperf << ")"
    ;

    mperf >>= APERFMPERF_SHIFT;                   
    if (mperf)
	ratio = aperf / mperf;

    std::cout << " = " << ratio << "\n";

    return ratio;
}                    

static unsigned long scale_aperfmperf(size_t cpu)
{
    struct aperfmperf val, *old = &oldMPs[cpu];
    unsigned long ratio;
    get_aperfmperf(cpu, &val);
    
    std::cout << "cpu " << cpu << ": ";
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

static void
opencpu(int cpuno)
{
    char buf[1024];
    sprintf(buf, "/dev/cpu/%d/msr", cpuno);
    fds[cpucount] = open(buf, O_RDWR);
    if (fds[cpucount] == -1) {
        std::clog << "failed to open " << buf << ": " << strerror(errno) << std::endl;
        abort();
    }
    std::clog << "opened " << buf << std::endl;
    cpucount++;
}

int
main(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "m:c:w:r:s:q")) != -1) {
        switch (c) {
            case 'q': {
                for (size_t i = 0; i < cpucount; ++i) {
                    aperfmperf one, two;
                    get_aperfmperf(i, &one);
                    get_aperfmperf(i, &two);
                    calc_aperfmperf_ratio(&one, &two);
                }
                break;
            }

            case 'c':
                opencpu(atoi(optarg));
                break;

            case 'm':
                for (size_t i = 0; i < strtoul(optarg, 0, 0); ++i)
                    opencpu(i);
                break;

            case 's':
                for (int i = 0; i < atoi(optarg); ++i) {
                    for (size_t cpu = 0; cpu < cpucount; ++cpu)
                        scale_aperfmperf(cpu);
                    sleep(5);
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
                for (size_t i = 0; i < cpucount; ++i)
                    writemsr(i, reg, val);
                break;
            }
                
            case 'r': {
                int reg = getreg(optarg);
                for (size_t i = 0; i < cpucount; ++i) 
                    std::clog << "cpu " << i << ": read " << reg << ": " << readmsr(i, reg) << std::endl;
                break;
            }
        }
    }
}
