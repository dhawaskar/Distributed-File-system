CC=g++
RM=rm -f

default: all
all: server client
server: server.cpp
	$(CC) -o server server.cpp -lcrypto
client: client.cpp
	$(CC) -o client client.cpp -lcrypto
clean:
	$(RM) server client
