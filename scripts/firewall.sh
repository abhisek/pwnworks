#!/bin/bash

iptables -F
iptables -I INPUT -p tcp --syn --match multiport --dports 9001,9002,9003 -m connlimit \
      --connlimit-above 3 -j DROP
