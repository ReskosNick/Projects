# Makefile

CC = gcc
CFLAGS = -g -Wall -fopenmp -lpthread
TARGET = exercise1_1
SRCS = exercise1_1.c ../my_rand.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET) $(ARGS)

# Phony targets
.PHONY: all clean run