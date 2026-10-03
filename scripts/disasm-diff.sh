#!/usr/bin/env bash

test="build/test.bin"
test_output="build/test_output.bin"

byte=$(
    cmp "$test" "$test_output" 2>&1 |
    sed -n 's/.*differ: byte \([0-9]*\).*/\1/p'
)

echo "$test"
dd if="$test" bs=1 skip=$((byte - 1)) count=16 2>/dev/null |
ndisasm -b 16 -

echo "$test_output"
dd if="$test_output" bs=1 skip=$((byte - 1)) count=16 2>/dev/null |
ndisasm -b 16 -
