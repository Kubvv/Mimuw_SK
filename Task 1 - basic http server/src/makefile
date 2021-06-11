CC = g++
CFLAGS = -c -Wall -Wextra -pedantic

default: serwer

reader.o: reader.cpp
	$(CC) $(CFLAGS) reader.cpp -o reader.o

writer.o: writer.cpp defs.h
	$(CC) $(CFLAGS) writer.cpp -o writer.o

packet.o: packet.cpp reader.h writer.h defs.h
	$(CC) $(CFLAGS) packet.cpp -o packet.o

server.o: server.cpp packet.h writer.h defs.h
	$(CC) $(CFLAGS) server.cpp -o server.o

main.o: main.cpp server.h reader.h defs.h
	$(CC) $(CFLAGS) main.cpp -o main.o

serwer: main.o server.o packet.o reader.o writer.o
	$(CC) main.o server.o packet.o reader.o writer.o -o serwer

clean:
	rm -f reader.o
	rm -f writer.o
	rm -f server.o
	rm -f packet.o
	rm -f main.o
	rm -f serwer