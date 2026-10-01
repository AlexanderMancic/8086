#!/usr/bin/env python3

import sys
from typing import TextIO

reg_names = [
    ["al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"],
    ["ax", "cx", "dx", "bx", "sp", "bp", "si", "di"],
]
eff_addr_exprs: list[str] = [
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx",
]
dir_addrs: list[str] = [
    "[0]",
    "[1]",
    "[127]",
    "[128]",
    "[255]",
    "[256]",
    "[32767]",
    "[32768]",
    "[65534]",
    "[65535]",
]
byte_literals: list[str] = [
    "- 128",
    "- 127",
    "- 1",
    "+ 0",
    "+ 1",
    "+ 126",
    "+ 127",
]
word_literals: list[str] = byte_literals + [
    "- 32768",
    "- 32767",
    "- 256",
    "- 255",
    "+ 255",
    "+ 256",
    "+ 32766",
    "+ 32767",
]
imm_literals: list[tuple[str, list[str]]] = [
    ("byte", byte_literals),
    ("word", word_literals),
]
rm_operands: list[list[str]] = [[], []]
for w in range(2):
    rm_operands[w].extend(reg_names[w])
    rm_operands[w].extend(dir_addrs)

    for eff_addr in eff_addr_exprs:
        if eff_addr == "bp":
            continue

        for disp in word_literals:
            rm_operands[w].append(f"[{eff_addr} {disp}]")


def append_mov_line_with_d(
    lines: list[str], d: int, dst_if_d: str, src_if_d: str
) -> None:
    if d == 1:
        dst = dst_if_d
        src = src_if_d
    else:
        dst = src_if_d
        src = dst_if_d

    lines.append(f"mov {dst}, {src}\n")


def append_mov_line_no_d(lines: list[str], dst: str, src: str) -> None:
    lines.append(f"mov {dst}, {src}\n")


def main():
    script_name = sys.argv[0]
    if len(sys.argv) != 2:
        sys.exit(f"usage: {script_name} OUTFILE")

    output_path = sys.argv[1]
    with open(output_path, "w") as file:
        file.write("bits 16\n\n")

        # reg <-> rm
        lines: list[str] = []
        for d in range(2):
            for w in range(2):
                for rm_operand in rm_operands[w]:
                    for reg_operand in reg_names[w]:
                        append_mov_line_with_d(lines, d, reg_operand, rm_operand)
        file.writelines(lines)

        # imm -> rm
        lines = []
        for w in range(2):
            width_mod, lits_for_width = imm_literals[w]
            for imm in lits_for_width:
                for rm_operand in rm_operands[w]:
                    append_mov_line_no_d(
                        lines,
                        rm_operand,
                        f"{width_mod} {imm}",
                    )
        file.writelines(lines)


if __name__ == "__main__":
    main()
