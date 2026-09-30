#!/usr/bin/env python3

import sys
from typing import Literal, TextIO

reg_names = [
    ["al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"],
    ["ax", "cx", "dx", "bx", "sp", "bp", "si", "di"],
]
eff_addr_exprs = [
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx",
]
dir_addrs = [
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
literals = [
    # "- 32768",
    # "- 32767",
    # "- 256",
    # "- 255",
    "- 128",
    "- 127",
    "- 1",
    "+ 0",
    "+ 1",
    "+ 126",
    "+ 127",
    # "+ 255",
    # "+ 256",
    # "+ 32766",
    # "+ 32767",
]


def append_mov_line(
    lines: list[str], d: int, reg_operand: str, rm_operand: str
) -> None:
    if d == 1:
        dst = reg_operand
        src = rm_operand
    else:
        dst = rm_operand
        src = reg_operand

    lines.append(f"mov {dst}, {src}\n")


def main():
    script_name = sys.argv[0]
    if len(sys.argv) != 2:
        sys.exit(f"usage: {script_name} OUTFILE")

    output_path = sys.argv[1]
    with open(output_path, "w") as file:
        file.write("bits 16\n\n")

        # reg <-> reg
        lines: list[str] = []
        for w in range(2):
            for dst in reg_names[w]:
                for src in reg_names[w]:
                    lines.append(f"mov {dst}, {src}\n")
        file.writelines(lines)

        # reg <-> eff_addr
        lines: list[str] = []
        for w in range(2):
            for d in range(2):
                for reg_name in reg_names[w]:
                    for eff_addr in eff_addr_exprs:
                        if eff_addr == "bp":
                            continue
                        for lit in literals:
                            append_mov_line(
                                lines,
                                d,
                                reg_name,
                                f"[{eff_addr} {lit}]",
                            )
        file.writelines(lines)

        # reg <-> dir addr
        lines: list[str] = []
        for w in range(2):
            for d in range(2):
                for reg_name in reg_names[w][1:]:
                    for dir_addr in dir_addrs:
                        append_mov_line(lines, d, reg_name, dir_addr)
        file.writelines(lines)


if __name__ == "__main__":
    main()
