FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y gcc-4.8-powerpc-linux-gnu qemu-user socat
RUN apt-get install -y net-tools

RUN mkdir /shared
WORKDIR /app

# Compile binary and remove source code
COPY src/fmt.c .
COPY server.sh /server.sh
RUN chmod 755 /server.sh
RUN powerpc-linux-gnu-gcc-4.8 -Wno-format-security -o /pwnable fmt.c -static
RUN rm -rf fmt.c

RUN useradd -m bob
EXPOSE 9000

CMD ["socat", "-dd", "TCP4-LISTEN:9000,fork,reuseaddr", "EXEC:/server.sh,pty,stderr,echo=0,raw,iexten=0"]


