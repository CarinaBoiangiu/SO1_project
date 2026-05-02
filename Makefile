CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET1 = city_manager
TARGET2 = monitor_reports

SRCS1 = city_manager.c filter.c
SRCS2 = monitor_reports.c

OBJS1 = $(SRCS1:.c=.o)
OBJS2 = $(SRCS2:.c=.o)

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET1) $(TARGET2) $(OBJS1) $(OBJS2)
	rm -f active_reports-*
	rm -f .monitor_pid

.PHONY: all clean