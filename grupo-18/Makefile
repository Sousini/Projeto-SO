CC = gcc
CFLAGS = -Wall -g

all: server client

server: server.o defs.o
    $(CC) $(CFLAGS) -o server server.o defs.o

client: client.o defs.o
    $(CC) $(CFLAGS) -o client client.o defs.o

server.o: server.c defs.h
    $(CC) $(CFLAGS) -c server.c

client.o: client.c defs.h
    $(CC) $(CFLAGS) -c client.c

defs.o: defs.h
    $(CC) $(CFLAGS) -c defs.c

clean:
    rm -f -r *.dSYM fifo* server client *.o
