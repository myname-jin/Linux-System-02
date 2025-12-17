CC = gcc
# GTK 라이브러리 설정을 변수에 저장
CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
LIBS = $(shell pkg-config --libs gtk+-3.0) -lpthread

all: server client

server: server.c common.h
	$(CC) server.c -o server $(CFLAGS) $(LIBS)

client: main.c network.c ui.c common.h network.h ui.h
	$(CC) main.c network.c ui.c -o client $(CFLAGS) $(LIBS)

clean:
	rm -f server client temp_* down_*