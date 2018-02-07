#!/bin/bash

if [ -d "/shared" ] && [ -v CHALLENGE_NAME ]; then
   cp /pwnable /shared/$CHALLENGE_NAME.bin
   md5sum --tag /pwnable > /shared/$CHALLENGE_NAME.bin.md5
fi;

echo $PWNFLAG > /flag.txt
chown root:root /flag.txt
chmod 644 /flag.txt

while [ true ]; do
   su - bob -c "socat -dd TCP4-LISTEN:9000,fork,reuseaddr EXEC:/pwnable,pty,echo=0,raw,iexten=0"
done;

