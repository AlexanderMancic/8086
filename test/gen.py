#!/usr/bin/env python3

import sys

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


def append_mov_line_with_d(
    lines: list[str], d: int, reg_operand: str, rm_operand: str
) -> None:
    if d == 1:
        dst = reg_operand
        src = rm_operand
    else:
        dst = rm_operand
        src = reg_operand

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

        # reg <-> reg
        lines: list[str] = []
        for w in range(2):
            for dst in reg_names[w]:
                for src in reg_names[w]:
                    lines.append(f"mov {dst}, {src}\n")
        file.writelines(lines)

        # reg <-> eff_addr
        lines = []
        for w in range(2):
            for d in range(2):
                for reg_name in reg_names[w]:
                    for eff_addr in eff_addr_exprs:
                        if eff_addr == "bp":
                            continue
                        for lit in word_literals:
                            append_mov_line_with_d(
                                lines,
                                d,
                                reg_name,
                                f"[{eff_addr} {lit}]",
                            )
        file.writelines(lines)

        # reg (no acc) <-> dir addr
        lines = []
        for w in range(2):
            for d in range(2):
                for reg_name in reg_names[w][1:]:
                    for dir_addr in dir_addrs:
                        append_mov_line_with_d(lines, d, reg_name, dir_addr)
        file.writelines(lines)

        lines = []
        # imm -> eff addr
        for eff_addr in eff_addr_exprs:
            if eff_addr == "bp":
                continue
            for disp in word_literals:
                for width_mod, lit_for_width in imm_literals:
                    for imm in lit_for_width:
                        append_mov_line_no_d(
                            lines,
                            f"[{eff_addr} {disp}]",
                            f"{width_mod} {imm}",
                        )
        file.writelines(lines)

        # imm -> dir addr
        lines = []
        for width_mod, lit_for_width in imm_literals:
            for imm in lit_for_width:
                for dir_addr in dir_addrs:
                    append_mov_line_no_d(
                        lines,
                        dir_addr,
                        f"{width_mod} {imm}",
                    )
        file.writelines(lines)

        # imm -> reg
        lines = []
        for w in range(2):
            width_mod, lits_for_width = imm_literals[w]
            for imm in lits_for_width:
                for reg_name in reg_names[w]:
                    append_mov_line_no_d(
                        lines,
                        reg_name,
                        f"{width_mod} {imm}",
                    )
        file.writelines(lines)


if __name__ == "__main__":
    main()
