CFLAGS += -std=c99 -Wall -g
CXXFLAGS += -Wall -g
SSL_LIBS += -lssl -lcrypto -ldl
LDFLAGS += -g

TARGETS=resolve connect markov aperf

all: $(TARGETS)

connect: connect.o
	$(CC) $(LDFLAGS) $^ -o $@ $(SSL_LIBS)

resolve: resolve.o
	$(CC) $(LDFLAGS) $^ -o $@ 

markov: markov.o
	$(CXX) $(LDFLAGS) $^ -o $@  -lrt

aperf: aperf.o
	$(CXX) $(LDFLAGS) $^ -o $@  -lrt

clean:
	rm -f *.o $(TARGETS)
