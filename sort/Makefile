.PHONY:clean all
CC=g++
CXXFLAGS= -std=c++11 -Wall -g
BIN=bubble_sort selection_sort
all:$(BIN)
%.o:%.cpp
	$(CC) --std=c++11 -o $@ $(CFLAGS) -c $^
clean:
	rm -rf *.o $(BIN)
