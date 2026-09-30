#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <print>
#include <string_view>

using u8 = uint8_t;
using namespace std::string_view_literals;

constexpr u8 bitmask3 = 0b111;
constexpr std::array<std::array<std::string_view, 8>, 2> reg_names{
        std::array{
                "al"sv, "cl"sv, "dl"sv, "bl"sv, "ah"sv, "ch"sv, "dh"sv, "bh"sv
        },
        std::array{
                "ax"sv, "cx"sv, "dx"sv, "bx"sv, "sp"sv, "bp"sv, "si"sv, "di"sv
        },
};

enum class Mod : u8
{
        MemModeNoDisp,
        MemMode8BitDisp,
        MemMode16BitDisp,
        RegMode,
};

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

                        switch (mod)
                        {
                        case Mod::RegMode:
                        {
                                std::string_view dst;
                                std::string_view src;

                                if (d)
                                {
                                        dst = reg_names[w][reg];
                                        src = reg_names[w][rm];
                                }
                                else
                                {
                                        dst = reg_names[w][rm];
                                        src = reg_names[w][reg];
                                }
                                outfile_stream
                                        << std::format("mov {},{}\n", dst, src);
                        }
                        break;
                        }
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
