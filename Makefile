.SILENT:

CXX := clang++
CXXFLAGS := \
	-std=c++26 -stdlib=libc++ -Iinclude -O3 \
	-Wall -Wextra -Wpedantic -Wno-gnu-case-range

BUILD_DIR := build
TEST_DIR := test

TARGET := $(BUILD_DIR)/disassembler
SRC := src/main.cpp

TEST_ASM := $(BUILD_DIR)/test.asm
TEST_BIN := $(BUILD_DIR)/test.bin
TEST_OUTPUT_ASM := $(BUILD_DIR)/test_output.asm
TEST_OUTPUT_BIN := $(BUILD_DIR)/test_output.bin

.PHONY: disassembler
disassembler: | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $@


.PHONY: test
test: disassembler
	python3 $(TEST_DIR)/gen.py $(TEST_ASM)
	nasm -f bin $(TEST_ASM) -o $(TEST_BIN)
	./$(TARGET) $(TEST_BIN) $(TEST_OUTPUT_ASM)
	nasm -f bin $(TEST_OUTPUT_ASM) -o $(TEST_OUTPUT_BIN)
	cmp $(TEST_BIN) $(TEST_OUTPUT_BIN)
	echo "PASS"

.PHONY: clean
clean:
	rm -rf build
