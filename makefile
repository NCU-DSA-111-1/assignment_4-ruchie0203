PHONY: all

all: server.o snaklient.o
	gcc server.o -o server -lpthread
	gcc snaklient.o -o snaklient -lpthread
snaklient.o: snaklient.c
	gcc snaklient.c -c -lpthread
server.o: server.c
	gcc server.c -c -lpthread

