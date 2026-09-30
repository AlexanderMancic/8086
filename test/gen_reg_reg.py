#!/usr/bin/env python3

import sys

reg_names = [
    ["al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"],
    ["ax", "cx", "dx", "bx", "sp", "bp", "si", "di"],
]


def main():
    script_name = sys.argv[0];
    if len(sys.argv) != 2:
        sys.exit(f"usage: {script_name} OUTFILE")

    output_path = sys.argv[1]
    with open(output_path, "w") as file:
        file.write("bits 16\n\n")

        for w in range(2):
            for dst in reg_names[w]:
                for src in reg_names[w]:
                    file.write(f"mov {dst}, {src}\n")


if __name__ == '__main__':
    main()
