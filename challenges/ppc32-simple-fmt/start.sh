#!/bin/bash

if [ -d "/shared" ]; then
   cp /pwnable /shared/ppc32-simple-fmt.bin
fi;

socat -dd TCP4-LISTEN:9000,fork,reuseaddr EXEC:/server.sh,pty,stderr,echo=0,raw,iexten=0

