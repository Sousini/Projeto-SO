CC = gcc
CFLAGS = -Wall -g -Iinclude

LDFLAGS =

all: folders server client

server: bin/orchestrator

client: bin/client

folders:
	@mkdir -p src include obj bin tmp

bin/orchestrator: obj/orchestrator.o obj/defs.o
	$(CC) $(LDFLAGS) $^ -o $@

bin/client: obj/client.o obj/defs.o
	$(CC) $(LDFLAGS) $^ -o $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* tmp/* bin/* task_log.txt

	