CC=gcc

all:
	gcc -m64 -z execstack -fno-stack-protector -o sandbox sandbox.c -lseccomp
	strip sandbox

.PHONY: clean
clean:
	-rm sandbox
	-rm *.o
	-rm core


