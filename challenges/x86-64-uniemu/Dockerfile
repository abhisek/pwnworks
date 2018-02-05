FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y build-essential python socat git

COPY install_unicorn.sh /root/install_unicorn.sh
RUN chmod 755 /root/install_unicorn.sh
RUN /root/install_unicorn.sh

WORKDIR /work
COPY . .
RUN make
RUN cp pwnable /pwnable

WORKDIR /
RUN rm -rf /work

COPY start.sh /start.sh
RUN chmod 755 /start.sh

RUN useradd -m bob
EXPOSE 9000

CMD ["/start.sh"]



