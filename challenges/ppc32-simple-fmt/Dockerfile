FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y gcc-4.8-powerpc-linux-gnu qemu-user socat
RUN apt-get install -y net-tools

RUN mkdir /shared
WORKDIR /app

# Wrapper scripts
COPY start.sh /start.sh
RUN chmod 755 /start.sh

# Compile binary and remove source code
COPY src/fmt.c .
RUN powerpc-linux-gnu-gcc-4.8 -Wno-format-security -s -o /pwnable fmt.c -static
RUN rm -rf fmt.c

RUN useradd -m bob
EXPOSE 9000

CMD ["/start.sh"]


