CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o LinkedList.o Hashtable.o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c
	
LinkedList.o: LinkedList.h LinkedList.c
	$(CC) $(CFLAGS) $^ -c
	
Hashtable.o: Hashtable.h Hashtable.c
	$(CC) $(CFLAGS) $^ -c
	
run:
	./tema2
	
pack:
	zip -FSr 313CA_MihneaBlotiu_Tema2.zip README Makefile *.c *.h

clean:
	rm -f *.o tema2 *.h.gch
