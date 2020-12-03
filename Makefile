CC = gcc 
CFLAGS = -std=c11 -Wall -Wconversion -Werror -Wextra -Wpedantic -pthread -D_XOPEN_SOURCE=500
LDFLAGS = -lrt
demon_objects = demon_src/demon.o
client_objects = client_src/client.o

all: demon client

demon: $(demon_objects)
	$(CC) $(demon_objects) $(CFLAGS) $(LDFLAGS) -o demon

client: $(client_objects)
	$(CC) $(client_objects) $(CFLAGS) $(LDFLAGS) -o client

clean:
	rm -rf $(demon_objects) $(client_objects) demon client

client.o: client.c client.h
demon.o: demon.c demon.h
