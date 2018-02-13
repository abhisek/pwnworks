FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y build-essential socat libseccomp-dev

WORKDIR /app
COPY src/. ./src/.
COPY Makefile .
RUN make
RUN cp sandbox /pwnable

WORKDIR /
RUN rm -rf /app/src

COPY start.sh /start.sh
RUN chmod 755 /start.sh

RUN useradd -m bob
EXPOSE 9000

CMD ["/start.sh"]



