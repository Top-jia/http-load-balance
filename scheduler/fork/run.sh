#!/bin/sh
#echo "hello world"

if [ -n "$1" ]; then
	kill -9 $(pidof "server")
	kill -9 $(pidof "main")
else
	
	#nohup ./server 172.17.18.168  7903 &
	nohup ./server 127.0.0.1 7903 &
	nohup ./server 127.0.0.1 7901 &
	nohup ./server 127.0.0.1 7902 &   
fi

