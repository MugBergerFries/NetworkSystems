This code is written in C++. It creates an HTTP server that can support multiple connections simultaneously.

IMPORTANT: The server expected to be in a directory alongside a www file containing the files that will be requested.
It will not work if it is inside the www folder.

Usage: make clean && make && ./server <port> or rm -f server && g++ -o server server.cpp

How to connect: visit localhost:<port> in a browser (must support HTTP/1.1, not tested with other versions).