#!/bin/sh
#echo "hello world"

nohup ./server 127.0.0.1 7903 &
nohup ./server 127.0.0.1 7901 &
nohup ./server 127.0.0.1 7902 &
