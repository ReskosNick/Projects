# Makefile

CC = gcc
CFLAGS = -g -Wall -fopenmp
TARGET = exercise1_2
SRCS = exercise1_2.c ../my_rand.c
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
