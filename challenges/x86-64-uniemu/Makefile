CC=gcc
CFLAGS=-Wformat=0
LDFLAGS=-lunicorn -lpthread

all: cbin 
cbin: src/unimu.o
		 $(CC) -o pwnable src/unimu.o $(LDFLAGS)

test: src/unimu.o
	gcc $(CFLAGS) -DDEBUG -DTEST_RUN -o runtest src/unimu.c $(LDFLAGS)

.PHONY: clean
clean:
	-rm src/*.o
	-rm pwnable
	-rm a.out
	-rm runtest
