CXX=clang++
BIN=/usr/local/bin
CXXFLAGS=-Wall -Wextra -I./src -std=c++26 -g
.PHONY: install clean purge

all: build install

build: ./src/cppfetch.cpp
	@mkdir -p ./bin
	@$(CXX) -o ./bin/cppfetch $^ $(CXXFLAGS)

install:
	@sudo cp ./bin/cppfetch $(BIN)

clean:
	@rm ./bin/cppfetch

purge: clean
	@sudo rm $(BIN)/cppfetch
