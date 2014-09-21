all: hello

hello: mem.c 
	gcc -c -fpic mem.c
	gcc -shared -o libmem.so mem.o
