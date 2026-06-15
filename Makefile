CXX ?= clang++
BIN ?= /usr/local/bin
VERSION ?= $(shell git describe --tags --dirty 2>/dev/null || echo "unknown")
CXXFLAGS ?= -Wall -Wextra -I./src -std=c++26 -g -DVERSION=\"$(VERSION)\"

all: build

build: ./src/cppfetch.cpp
	@mkdir -p ./bin
	@$(CXX) -o ./bin/cppfetch $^ $(CXXFLAGS)

install: build
	@install -d ${BIN}
	@install ./bin/cppfetch ${BIN}

uninstall:
	@rm -rf ${BIN}/cppfetch

.PHONY: install uninstall
