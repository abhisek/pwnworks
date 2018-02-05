#!/bin/bash

cd /root
git clone --branch 1.0.1 https://github.com/unicorn-engine/unicorn
cd /root/unicorn

./make.sh 
./make.sh install

exit 0
