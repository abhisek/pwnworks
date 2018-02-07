#!/bin/bash

if [ -d "/shared" ] && [ -v CHALLENGE_NAME ]; then
   cp /pwnable /shared/$CHALLENGE_NAME.bin
   md5sum --tag /pwnable > /shared/$CHALLENGE_NAME.bin.md5
fi;

while [ true ]; do
   socat -dd TCP4-LISTEN:9000,fork,reuseaddr EXEC:/pwnable,pty,setuid=bob,echo=0,raw,iexten=0
   sleep 1
done;

