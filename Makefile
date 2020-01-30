.PHONY: all clean install check

CFLAGS += -std=c99 -Wall -g
CXXFLAGS += -Wall -g -std=c++17
SSL_LIBS += -lssl -lcrypto -ldl
LDFLAGS += -g
PREFIX=/usr/local

TARGETS=resolve connect markov aperf u+ errno smbios nslurk demangle compile_pcap

all: $(TARGETS)

connect: connect.o
	$(CC) $(LDFLAGS) $^ -o $@ $(SSL_LIBS)

resolve: resolve.o
	$(CC) $(LDFLAGS) $^ -o $@ 

markov: markov.o
	$(CXX) $(LDFLAGS) $^ -o $@  -lrt

aperf: aperf.o
	$(CXX) $(LDFLAGS) $^ -o $@

errno: errno.o
	$(CC) $(LDFLAGS) $^ -o $@

smbios: smbios.o
	$(CXX) $(LDFLAGS) $^ -o $@

nslurk: nslurk.o
	$(CC) $(LDFLAGS) $^ -o $@

u+: u+.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGETS)

compile_pcap: compile_pcap.o
	$(CXX) $(LDFLAGS) $^ -o $@ -lpcap

install:
	cp $(TARGETS) $(PREFIX)/bin
