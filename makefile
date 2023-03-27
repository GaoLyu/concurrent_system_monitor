CC=gcc
CCFLAGS=-Wall -Werror

all: stat.o main.o
	$(CC) $^ -o all
%.o: %.c stat.h
	$(CC) $(CCFLAGS) $< -c

.PHONY: clean
clean:
	rm -f *.o all