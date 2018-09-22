all: client server

client:
	gcc -o client udp_client.c

server:
	gcc -o udp/server udp/udp_server.c

clean:
	rm -f client udp/server