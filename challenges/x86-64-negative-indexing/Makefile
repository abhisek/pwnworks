CC=gcc
CFLAGS=
LDFLAGS=-s

all: cbin
cbin: src/ngix.o
		 $(CC) -o cbin src/ngix.o $(LDFLAGS)

.PHONY: clean
clean:
	-rm src/*.o
	-rm cbin
