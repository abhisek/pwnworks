CC=gcc

all:
	gcc -m64 -z execstack -fno-stack-protector -o sandbox src/sandbox.c -lseccomp
	strip sandbox

.PHONY: clean
clean:
	-rm *.o
	-rm sandbox
	-rm core


