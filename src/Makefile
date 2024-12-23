CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -lm -lSDL2
TARGET = qrustyquake
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

BUILD_TYPE ?= release

DEBUG_FLAGS = -g -O0
RELEASE_FLAGS = -O3

ifeq ($(BUILD_TYPE), debug)
	CFLAGS += $(DEBUG_FLAGS)
else
	CFLAGS += $(RELEASE_FLAGS)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

debug:
	$(MAKE) BUILD_TYPE=debug

release:
	$(MAKE) BUILD_TYPE=release

clean:
	rm -f $(OBJS) $(TARGET)
