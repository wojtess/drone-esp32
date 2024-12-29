CC = gcc
CFLAGS = -Wall -Wextra -I. -I$(IDF_PATH)/components/esp32/include
LDFLAGS = -L$(IDF_PATH)/components/esp32/lib -lesp32

SRCS = test.c protocol.c
OBJS = $(SRCS:.c=.o)

test: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: test
	./test

clean:
	rm -f test *.o
