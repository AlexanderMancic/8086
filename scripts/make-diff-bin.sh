#!/usr/bin/env bash

test="build/test.bin"
test_output="build/test_output.bin"

byte=$(
    cmp "$test" "$test_output" 2>&1 |
    sed -n 's/.*differ: byte \([0-9]*\).*/\1/p'
)

dd if="$test" of=build/diff.bin bs=1 skip=$((byte - 1)) status=none
