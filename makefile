myServer : server.o fileTransfer.o
	gcc -o myServer server.o fileTransfer.o -lpthread

myClient : client.o
	gcc -o myClient client.o


server.o : server.c
	gcc -c server.c -lpthread

fileTransfer.o : fileTransfer.c mainheader.h
	gcc -c fileTransfer.c

client.o : client.c
	gcc -c client.c


clean :
	rm myServer myClient server.o fileTransfer.o client.o