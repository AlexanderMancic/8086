#!/usr/bin/env python3

import sys

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


def main():
    script_name = sys.argv[0];
    if len(sys.argv) != 2:
        sys.exit(f"usage: {script_name} OUTFILE")

    output_path = sys.argv[1]
    with open(output_path, "w") as file:
        file.write("bits 16\n\n")

        # reg <-> reg
        for w in range(2):
            for dst in reg_names[w]:
                for src in reg_names[w]:
                    file.write(f"mov {dst}, {src}\n")

        # reg <-> eff_addr
        for w in range(2):
            for d in range(2):
                for reg in reg_names[w]:
                    for eff_addr in eff_addr_exprs:
                        if eff_addr == "bp":
                            continue

                        if d == 1:
                            dst = reg
                            src = f"[{eff_addr}]"
                        else:
                            dst = f"[{eff_addr}]"
                            src = reg

                        file.write(f"mov {dst}, {src}\n")


if __name__ == '__main__':
    main()
