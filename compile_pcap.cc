#define _DEFAULT_SOURCE
#include <pcap/pcap.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

static void compilePcap(bpf_program *program, const char *text) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *pcap = pcap_open_dead(1 /* LINKTYPE_ETHERNET */, 65535);
    if (pcap == 0)
        throw std::string("pcap open failed: ") + errbuf;
    int err = pcap_compile(pcap, program, text, 1, PCAP_NETMASK_UNKNOWN);
    pcap_close(pcap);
    if (err == PCAP_ERROR)
        throw std::string("pcap compile failed: ") + pcap_geterr(pcap);
}

int
main(int argc, char *argv[])
{
    bpf_program prog;
    std::vector<std::string> args;
    std::copy(argv + 1, argv + argc, std::back_inserter(args));

    std::string all;
    all = std::reduce(args.begin(),
                        args.end(),
                        std::string(""),
                        [] (std::string &lhs, const std::string &rhs) { return lhs = "" ? rhs : lhs + " " + rhs; });
    std::cout << "compiling '" << all << "'" << std::endl;
    int rc = compilePcap(&prog, all.c_str());
    std::cout << "result: " << rc << std::endl;
    std::clog << "compiled to " << program.bf_len << " instructions" << std::endl;
    return rc != 0;
}

