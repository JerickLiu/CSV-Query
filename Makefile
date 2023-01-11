DEBUG= -g
EXECS= server client ppsServer.c clean

all: $(EXECS)

client: pqcClient.o
	gcc $(DEBUG) -o client pqcClient.o

server: ppsServer.o
	gcc $(DEBUG) -o server ppsServer.o -lpthread

ppsServer.o: ppsServer.c ppsServer.h
	gcc -c ppsServer.c

main: main.c
	gcc -o main main.c


clean:
	rm -f *.o