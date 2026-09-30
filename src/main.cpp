#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <print>
#include <string_view>

using u8 = uint8_t;
using namespace std::string_view_literals;

constexpr u8 bitmask3 = 0b111;
constexpr u8 dir_addr = 0b110;
constexpr std::format_string<std::string_view&, std::string_view&> mov_fmt{
        "mov {}, {}\n"
};
constexpr std::array<std::array<std::string_view, 8>, 2> reg_names{
        std::array{
                "al"sv, "cl"sv, "dl"sv, "bl"sv, "ah"sv, "ch"sv, "dh"sv, "bh"sv
        },
        std::array{
                "ax"sv, "cx"sv, "dx"sv, "bx"sv, "sp"sv, "bp"sv, "si"sv, "di"sv
        },
};
constexpr std::array eff_addr_exprs{
        "bx + si"sv,
        "bx + di"sv,
        "bp + si"sv,
        "bp + di"sv,
        "si"sv,
        "di"sv,
        "bp"sv,
        "bx"sv,
};

enum class Mod : u8
{
        MemModeNoDisp,
        MemMode8BitDisp,
        MemMode16BitDisp,
        RegMode,
};

auto disassemble_mov(
        std::ofstream& outfile_stream,
        bool d,
        std::string_view reg_name,
        std::string_view eff_addr
) -> void
{
        std::string_view dst;
        std::string_view src;

        if (d)
        {
                dst = reg_name;
                src = eff_addr;
        }
        else
        {
                dst = eff_addr;
                src = reg_name;
        }

        outfile_stream << std::format(mov_fmt, dst, src);
}

auto main(int argc, char** argv) -> int
{
        auto program_name = argv[0];
        if (argc != 3)
        {
                std::println(
                        stderr,
                        "error: incorrect amount of arguments\nexpected 2, got "
                        "{}\nusage: {} INFILE OUTFILE",
                        argc - 1,
                        program_name
                );
                return EXIT_FAILURE;
        }

        auto infile_name = argv[1];
        std::ifstream infile_stream{infile_name, std::ios::binary};
        if (!infile_stream)
        {
                std::println(stderr, "error: failed to open {}", infile_name);
                return EXIT_FAILURE;
        }

        auto outfile_name = argv[2];
        std::ofstream outfile_stream{outfile_name};
        if (!outfile_stream)
        {
                std::println(stderr, "error: failed to open {}", outfile_name);
                return EXIT_FAILURE;
        }

        outfile_stream << "bits 16\n\n";
        u8 byte;
        while (infile_stream.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
        {
                switch (byte)
                {
                case 0x88 ... 0x8b:
                {
                        bool d = (byte >> 1) & 1;
                        bool w = byte & 1;

                        if (!infile_stream.read(
                                    reinterpret_cast<char*>(&byte), sizeof(byte)
                            ))
                        {
                                std::println(
                                        stderr, "error: unexpeced EOF"

                                );
                                return EXIT_FAILURE;
                        }
                        Mod mod = static_cast<Mod>(byte >> 6);
                        u8 reg = (byte >> 3) & bitmask3;
                        u8 rm = byte & bitmask3;

                        std::string_view reg_operand{reg_names[w][reg]};
                        std::string rm_operand;
                        switch (mod)
                        {
                        case Mod::MemModeNoDisp:
                        {
                                if (dir_addr == rm)
                                {
                                        std::println(
                                                stderr,
                                                "error: direct address not "
                                                "decoded yet"
                                        );
                                        return EXIT_FAILURE;
                                }

                                rm_operand =
                                        std::format("[{}]", eff_addr_exprs[rm]);
                        }
                        break;

                        case Mod::RegMode:
                        {
                                rm_operand = reg_names[w][rm];
                        }
                        break;
                        }

                        disassemble_mov(
                                outfile_stream, d, reg_operand, rm_operand
                        );
                }
                break;

                default:
                {
                        std::println(
                                stderr,
                                "error: "
                                "unknown "
                                "opcode: "
                                "{:04b} {:04b}",
                                byte >> 4,
                                byte & 0x0f
                        );
                        return EXIT_FAILURE;
                }
                break;
                }
        }

        return EXIT_SUCCESS;
}
