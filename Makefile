.SILENT:

CXX := clang++
CXXFLAGS := \
	-std=c++26 -stdlib=libc++ -Iinclude -O3 \
	-Wall -Wextra -Wpedantic -Wno-gnu-case-range

BUILD_DIR := build
TEST_DIR := test

TARGET := $(BUILD_DIR)/disassembler
SRC := src/main.cpp

TEST_GENERATORS := $(wildcard $(TEST_DIR)/gen_*.py)
TEST_ASM := $(patsubst \
	    $(TEST_DIR)/gen_%.py,\
	    $(BUILD_DIR)/test_%.asm,\
	    $(TEST_GENERATORS))
TEST_BIN := $(TEST_ASM:.asm=.bin)
TEST_OUTPUT_ASM := $(TEST_ASM:.asm=_output.asm)
TEST_OUTPUT_BIN := $(TEST_ASM:.asm=_output.bin)
TEST_OK := $(TEST_ASM:.asm=.ok)

.PHONY: disassembler
disassembler: | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/test_%.asm: $(TEST_DIR)/gen_%.py | $(BUILD_DIR)
	@python3 $< $@

$(BUILD_DIR)/test_%.bin: $(BUILD_DIR)/test_%.asm
	@nasm -f bin $< -o $@

$(BUILD_DIR)/test_%_output.asm: $(BUILD_DIR)/test_%.bin | disassembler
	@$(TARGET) $< $@

$(BUILD_DIR)/test_%_output.bin: $(BUILD_DIR)/test_%_output.asm
	@nasm -f bin $< -o $@

$(BUILD_DIR)/test_%.ok: \
	$(BUILD_DIR)/test_%.bin \
	$(BUILD_DIR)/test_%_output.bin
	@cmp $(BUILD_DIR)/test_$*.bin $(BUILD_DIR)/test_$*_output.bin
	@touch $@

.PHONY: test
test: disassembler $(TEST_OK)

.PHONY: clean
clean:
	@rm -rf build
