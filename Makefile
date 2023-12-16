CC=gcc
CFLAGS=-I/opt/homebrew/Cellar/cjson/1.7.16/include/ -I/opt/homebrew/Cellar/openssl@3/3.2.0_1/include/
LDFLAGS=-L/opt/homebrew/Cellar/cjson/1.7.16/lib/ -L/opt/homebrew/Cellar/openssl@3/3.2.0_1/lib/
LIBS=-lcjson -lssl -lcrypto

server: server.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o server server.c $(LIBS)

