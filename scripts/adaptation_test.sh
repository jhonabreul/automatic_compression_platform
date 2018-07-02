#!/bin/bash

echo "6 Mbits/s"
./external/wondershaper/wondershaper -a enp3s0 -c
./external/wondershaper/wondershaper -a enp3s0 -d 15000 -u 15000

./bin/autocomp -H 192.168.221.50 -f /home/mserver/Documents/jhonathan/zero -d ../tmp/ &
pid=$!

sleep 4

#echo "1Gbit Mbits/s"
#./external/wondershaper/wondershaper -a enp3s0 -c

#usleep 100000

#echo "100 Mbits/s"
./external/wondershaper/wondershaper -a enp3s0 -c
#./external/wondershaper/wondershaper -a enp3s0 -d 10000 -u 10000 

sleep 5

kill -SIGTERM $pid
./external/wondershaper/wondershaper -a enp3s0 -c