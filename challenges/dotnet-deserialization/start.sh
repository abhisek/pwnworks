#!/bin/bash

if [ -d "/shared" ] && [ -v CHALLENGE_NAME ]; then
   cp /pwnable /shared/$CHALLENGE_NAME.bin
   md5sum --tag /pwnable > /shared/$CHALLENGE_NAME.bin.md5
fi;

while [ true ]; do
   sleep 1
done;

