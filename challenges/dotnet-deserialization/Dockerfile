FROM ubuntu:16.04

RUN apt-get update
RUN apt-get install -y apt-transport-https 

COPY data/microsoft.asc /tmp/microsoft.asc
RUN gpg --dearmor < /tmp/microsoft.asc > /tmp/microsoft.gpg
RUN mv /tmp/microsoft.gpg /etc/apt/trusted.gpg.d/microsoft.gpg
RUN echo "deb [arch=amd64] https://packages.microsoft.com/repos/microsoft-ubuntu-xenial-prod xenial main" > /etc/apt/sources.list.d/dotnetdev.list

RUN apt-get update
RUN apt-get install -y build-essential redir curl dotnet-sdk-2.1.4 zip

WORKDIR /app/src/restapp
COPY restapp/. .
RUN dotnet publish --configuration Release --output /app/bin
RUN rm -rf /app/bin/restapp.pdb

WORKDIR /app
RUN rm -rf /app/src

COPY start.sh /start.sh
RUN chmod 755 /start.sh

RUN useradd -m bob
EXPOSE 9000

CMD ["/start.sh"]



