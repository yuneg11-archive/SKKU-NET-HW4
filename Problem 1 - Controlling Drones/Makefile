all:
	make clean
	make client
	make server

client:
	gcc -o client client.c

server:
	gcc -o server server.c

clean:
	rm -f client client.o server server.o