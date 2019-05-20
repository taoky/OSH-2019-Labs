objects = server.o fatal_posix.o

CXX = arm-linux-gnueabihf-g++
# CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=gnu++11

sh : $(objects)
	$(CXX) $(CXXFLAGS) -o server $(objects)

.PHONY : clean
clean :
	rm server $(objects)
