all: client server

client:
	gcc -o client udp_client.c

server:
	gcc -o udp/server udp/udp_server.c

clean:
	rm -f client
	rm -f udp/server
	rm -f udp/foo3
	rm -f udp/foo2
	rm -f udp/foo1
	rm -f udp/.#foo3