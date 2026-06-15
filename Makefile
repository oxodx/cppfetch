CXX=clang++
BIN=/usr/local/bin
CXXFLAGS=-Wall -Wextra -I./src -std=c++26 -g
.PHONY: install uninstall

all: build

build: ./src/cppfetch.cpp
	@mkdir -p ./bin
	@$(CXX) -o ./bin/cppfetch $^ $(CXXFLAGS)

install: build
	@install -d ${BIN}
	@install ./bin/cppfetch ${BIN}

uninstall:
	@rm -rf ${BIN}/cppfetch
