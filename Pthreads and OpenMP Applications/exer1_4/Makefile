# Makefile

CC = gcc
CFLAGS = -g -Wall -pthread
TARGET = exercise1_4
SRCS_EXERCISE = exercise1_4.c ../my_rand.c
SRCS_RWLOCKS = rwlocks.c
OBJS_EXERCISE = $(SRCS_EXERCISE:.c=.o)
OBJS_RWLOCKS = $(SRCS_RWLOCKS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS_EXERCISE) $(OBJS_RWLOCKS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS_EXERCISE) $(OBJS_RWLOCKS)

exercise1_4.o: exercise1_4.c rwlocks.h
	$(CC) $(CFLAGS) -c exercise1_4.c

my_rand.o: ../my_rand.c
	$(CC) $(CFLAGS) -c ../my_rand.c

rwlocks.o: rwlocks.c rwlocks.h
	$(CC) $(CFLAGS) -c rwlocks.c

clean:
	rm -f $(TARGET) $(OBJS_EXERCISE) $(OBJS_RWLOCKS)

run: $(TARGET)
	./$(TARGET) $(ARGS)

# Phony targets
.PHONY: all clean run
