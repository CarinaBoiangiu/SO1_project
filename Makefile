CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = $(wildcard *.c)

OBJS = $(SRCS:.c=.o)

TARGET = city_manager

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f active_reports-*

.PHONY: all clean