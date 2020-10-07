CC=g++
CFLAGS=-std=c++17
LDFLAGS=-lSDL2 -lvulkan


headers=$(wildcard *.hpp)
src=$(wildcard *.cpp)
obj=$(src:.cpp=.o)

prog=vulest

$(prog): $(obj) $(headers)
#$(prog): $(obj)
	$(CC) $(CFLAGS) -g -o  $@ $^ $(LDFLAGS)

.PHONY: test clean

test:
	./$(prog)

clean:
	rm -f $(obj) $(prog)


