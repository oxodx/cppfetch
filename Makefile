CXX ?= clang++
BIN ?= /usr/local/bin
VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null || echo "unknown")
CXXFLAGS ?= -Wall -Wextra -I./src -I ./vendor/nlohmann -g
override CXXFLAGS += -std=c++26 -DVERSION=\"$(VERSION)\"
LDFLAGS ?= -lX11 -lXrandr

all: build

build: ./src/cppfetch.cpp ./src/system.cpp
	@mkdir -p ./bin
	@$(CXX) -o ./bin/cppfetch $^ $(CXXFLAGS) $(LDFLAGS)

install: build
	@install -d ${BIN}
	@install ./bin/cppfetch ${BIN}

uninstall:
	@rm -rf ${BIN}/cppfetch

.PHONY: install uninstall
