CFLAGS += -std=c99 -Wall -g -I $(HOME)/openssl/include
CXXFLAGS += -Wall -g
LIBS += -lssl -lcrypto -ldl
LDFLAGS += -g -L $(HOME)/openssl/lib

connect: connect.o
	$(CC) $(LDFLAGS) connect.o -o $@ $(LIBS)

clean:
	rm connect.o connect
