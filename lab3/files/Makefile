objects = server.o fatal_posix.o

CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wextra -O2 -std=c99

sh : $(objects)
	$(CC) $(CFLAGS) -o server $(objects)

.PHONY : clean
clean :
	rm server $(objects)
