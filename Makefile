CC = gcc
CFLAGS = -Wall -Werror
OBJ_OPTION = -c

build: client

client: client.o helpers.o parson.o requests.o buffer.o
	$(CC) $(CFLAGS) $^ -o client

client.o: client.c
	$(CC) $(CFLAGS) $(OBJ_OPTION) $<

helpers.o: helpers.c
	$(CC) $(CFLAGS) $(OBJ_OPTION) $<

parson.o: parson.c
	$(CC) $(CFLAGS) $(OBJ_OPTION) $<

requests.o: requests.c
	$(CC) $(CFLAGS) $(OBJ_OPTION) $<

buffer.o: buffer.c
	$(CC) $(CFLAGS) $(OBJ_OPTION) $<

make clean:
	rm -f client *.o

zip:
	zip -FSr 324CB_Vladulescu_Denis-Marian_Tema4PC.zip Makefile *.c *.h README.md