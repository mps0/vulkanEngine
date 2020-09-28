CC=g++
CFLAGS=-std=c++17
LDFLAGS=-lSDL2 -lvulkan

src=$(wildcard *.cpp)
obj=$(src:.cpp=.o)

prog=vulest

$(prog): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test clean

test:
	./$(prog)

clean:
	rm -f $(obj) $(prog)


