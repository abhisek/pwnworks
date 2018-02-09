FROM ubuntu:16.04
RUN apt-get update && apt-get install -y build-essential socat

WORKDIR /app
COPY src /app/src/.
COPY Makefile .
RUN make
RUN cp cbin /pwnable

WORKDIR /
RUN rm -rf /app

COPY start.sh /start.sh
RUN chmod 755 /start.sh
RUN useradd -m bob

EXPOSE 9000
CMD ["/start.sh"]


