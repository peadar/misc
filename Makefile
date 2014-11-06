CFLAGS += -std=c99 -Wall -g
CXXFLAGS += -Wall -g
SSL_LIBS += -lssl -lcrypto -ldl
LDFLAGS += -g

TARGETS=resolve connect markov aperf u+ errno

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

u+: u+.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGETS)
