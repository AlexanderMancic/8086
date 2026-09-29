CXX := clang++
CXXFLAGS := -std=c++26 -Wall -Wextra -Wpedantic -Iinclude -O3

TARGET := build/disassembler
SRC := src/main.cpp

.PHONY: all clean test build

all: build

build:
	@mkdir -p build
	@$(CXX) $(CXXFLAGS) -stdlib=libc++ $(SRC) -o $(TARGET)
