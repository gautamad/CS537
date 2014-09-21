CC   = gcc
OPTS = -Wall

all: libmfs server client

# this is for client library

udp.o:
	$(CC) $(OPTS) -c -fpic udp.c -o udp.o
mfs.o:
	$(CC) $(OPTS) -c -fpic mfs.c -o mfs.o

libmfs: udp.o mfs.o
	$(CC) -shared -fpic -g -o libmfs.so mfs.c udp.c
       
# this generates the target executables
server: server.o udp.o	
	$(CC) -o server server.c udp.c 

client: client.o udp.o
	$(CC) -o client client.c udp.o libmfs.so 

# this is a generic rule for .o files 
%.o: %.c 
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm -f server.o udp.o client.o server client mfs.o libmfs.so *.img



