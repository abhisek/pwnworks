FROM ubuntu:16.04

RUN curl https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > /tmp/microsoft.gpg
RUN mv microsoft.gpg /etc/apt/trusted.gpg.d/microsoft.gpg
RUN echo "deb [arch=amd64] https://packages.microsoft.com/repos/microsoft-ubuntu-xenial-prod xenial main" > /etc/apt/sources.list.d/dotnetdev.list

RUN apt-get update
RUN apt-get install -y build-essential redir curl dotnet-sdk-2.1.4

WORKDIR /app/src
COPY restapp/ .
RUN dotnet build --configuration Release --output /app
RUN rm -rf /app/src

COPY start.sh /start.sh
RUN chmod 755 /start.sh

RUN useradd -m bob
EXPOSE 9000

CMD ["/start.sh"]



