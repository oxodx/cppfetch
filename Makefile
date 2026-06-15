CXX=clang++
BIN=/usr/local/bin
CXXFLAGS=-Wall -Wextra -I./src -std=c++26 -g
.PHONY: install clean purge

all: build install

build: ./src/cppfetch.cpp
	@$(CXX) -o cppfetch $^ $(CXXFLAGS)

install:
	@sudo cp ./cppfetch $(BIN)

clean:
	@rm ./cppfetch

purge: clean
	@sudo rm $(BIN)/cppfetch
