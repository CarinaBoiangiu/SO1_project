CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET1 = city_manager
TARGET2 = monitor_reports
TARGET3 = scorer
TARGET4 = city_hub

SRCS1 = city_manager.c filter.c
SRCS2 = monitor_reports.c
SRCS3 = scorer.c
SRCS4 = city_hub.c

OBJS1 = $(SRCS1:.c=.o)
OBJS2 = $(SRCS2:.c=.o)
OBJS3 = $(SRCS3:.c=.o)
OBJS4 = $(SRCS4:.c=.o)

all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET3): $(OBJS3)
	$(CC) $(CFLAGS) -o $@ $^
	
$(TARGET4): $(OBJS4)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET3) $(OBJS1) $(OBJS2) $(OBJS3)
	rm -f active_reports-*
	rm -f .monitor_pid

.PHONY: all clean