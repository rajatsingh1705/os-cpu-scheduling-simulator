# ============================================================
#  OS CPU Scheduling Simulator — Makefile
# ============================================================
CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -O2 -std=c11
TARGET  = scheduler
SRCS    = main.c scheduler.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "  Build successful → ./$(TARGET)"

%.o: %.c scheduler.h
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: all

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "  Cleaned build artefacts."