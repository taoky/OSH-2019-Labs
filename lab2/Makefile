objects = src/shell.o src/utils.o src/fatal_posix.o src/rl.o

CPPFLAGS = -Wall -Wextra -std=gnu++17
READLINE_LIB = -lreadline
CXX = g++

sh : $(objects) src/shell.h src/utils.h src/fatal_posix.h src/rl.h
	$(CXX) $(CPPFLAGS) -o src/sh $(objects) $(READLINE_LIB)

.PHONY : clean
clean :
	rm src/sh $(objects)
