#!/bin/bash

if [ -d "/shared" ] && [ -v CHALLENGE_NAME ]; then
   cp /app/restapp.dll /shared/$CHALLENGE_NAME.bin
   md5sum --tag /app/restapp.dll > /shared/$CHALLENGE_NAME.bin.md5
fi;

cd /app
redir --laddr=0.0.0.0 --lport=9000 --caddr=127.0.0.1 --cport=8000 > /dev/null 2>&1 &

while [ true ]; do
   su bob -c "dotnet /app/restapp.dll"
   sleep 1
done;

