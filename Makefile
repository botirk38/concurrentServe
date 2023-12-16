CC = gcc
CFLAGS = -I/opt/homebrew/Cellar/cjson/1.7.16/include/ -I/opt/homebrew/Cellar/openssl@3/3.2.0_1/include/
LDFLAGS = -L/opt/homebrew/Cellar/cjson/1.7.16/lib/ -L/opt/homebrew/Cellar/openssl@3/3.2.0_1/lib/ -lcjson -lssl -lcrypto
OBJ = server.o ssl_utils.o networking.o request_handler.o http_utils.o file_utils.o
TARGET = server

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
