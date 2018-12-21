all:
	make clean
	make client
	make lserver
	make gserver

client:
	gcc -g -o client client.c

lserver:
	gcc -g -o lserver lserver.c

gserver:
	gcc -g -o gserver gserver.c

clean:
	rm -f client client.o lserver lserver.o gserver gserver.o