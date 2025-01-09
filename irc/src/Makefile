CC = gcc
OPENSSL = "C:/PROGRA~1/OPENSSL-WIN64"
CFLAGS = -Wall -Wextra -g -I"../include" -I"$(OPENSSL)/include"
LDFLAGS = -L"$(OPENSSL)/lib/VC" -lssl -lcrypto -lws2_32 -lgdi32 -lcrypt32 -luser32
SOURCES = handle_client.c main.c message.c server.c win-poll.c applink.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = irc_server.exe

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm *.o $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET)
