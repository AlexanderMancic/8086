#include <array>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <print>
#include <string_view>
#include <utility>

using u8 = uint8_t;
using u16 = uint16_t;
using i8 = int8_t;
using i16 = int16_t;
using namespace std::string_view_literals;

constexpr u8 bitmask3 = 0b111;
constexpr u8 dir_addr = 0b110;
constexpr u8 acc_reg = 0;
constexpr std::array width_mods{"byte"sv, "word"sv};
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

auto read_byte(std::ifstream& infile_stream) -> u8
{
        u8 byte;
        if (!infile_stream.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
        {
                std::println(stderr, "error: unexpeced EOF");
                std::exit(EXIT_FAILURE);
        }
        return byte;
}

auto read_word(std::ifstream& infile_stream) -> u16
{
        std::array<u8, 2> bytes;
        if (!infile_stream.read(
                    reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size())
            ))
        {
                std::println(stderr, "error: unexpected EOF");
                return EXIT_FAILURE;
        }
        u8 lo = bytes[0];
        u8 hi = bytes[1];
        return static_cast<u16>(lo) | static_cast<u16>(hi << 8);
}

auto read_i8(std::ifstream& infile_stream) -> i8
{
        u8 byte = read_byte(infile_stream);
        return std::bit_cast<i8>(byte);
}

auto read_i16(std::ifstream& infile_stream) -> i16
{
        u16 word = read_word(infile_stream);
        return std::bit_cast<i16>(word);
}

auto disasm_dir_addr(std::ifstream& infile_stream) -> std::string
{
        u16 dir_addr = read_word(infile_stream);
        return std::format("[{}]", dir_addr);
}

auto disasm_modrm(std::ifstream& infile_stream, bool w)
        -> std::pair<u8, std::string>
{
        const u8 byte = read_byte(infile_stream);

        const Mod mod = static_cast<Mod>(byte >> 6);
        const u8 extension = (byte >> 3) & bitmask3;
        const u8 rm = byte & bitmask3;

        std::string rm_operand;

        switch (mod)
        {
        case Mod::MemModeNoDisp:
        {
                if (rm == dir_addr)
                {
                        rm_operand = disasm_dir_addr(infile_stream);
                }
                else
                {
                        rm_operand = std::format("[{}]", eff_addr_exprs[rm]);
                }
        }
        break;

        case Mod::MemMode8BitDisp:
        {
                i8 disp = read_i8(infile_stream);
                rm_operand = std::format(
                        "[{} {} {}]",
                        eff_addr_exprs[rm],
                        disp < 0 ? '-' : '+',
                        std::abs(disp)
                );
        }
        break;

        case Mod::MemMode16BitDisp:
        {
                i16 disp = read_i16(infile_stream);
                rm_operand = std::format(
                        "[{} {} {}]",
                        eff_addr_exprs[rm],
                        disp < 0 ? '-' : '+',
                        std::abs(disp)
                );
        }
        break;

        case Mod::RegMode:
        {
                rm_operand = reg_names[w][rm];
        }
        break;
        }

        return {extension, rm_operand};
}

auto disasm_imm(std::ifstream& infile_stream, bool w) -> std::string
{
        i16 imm{};
        if (w)
        {
                imm = read_i16(infile_stream);
        }
        else
        {
                imm = read_i8(infile_stream);
        }
        return std::format("{} {}", width_mods[w], imm);
}

auto write_mov(
        std::ofstream& outfile_stream,
        bool d,
        std::string_view dst_if_d,
        std::string_view src_if_d
) -> void
{
        std::string_view dst;
        std::string_view src;

        if (d)
        {
                dst = dst_if_d;
                src = src_if_d;
        }
        else
        {
                dst = src_if_d;
                src = dst_if_d;
        }

        outfile_stream << std::format(mov_fmt, dst, src);
}

auto write_mov(
        std::ofstream& outfile_stream,
        std::string_view dst,
        std::string_view src
) -> void
{
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
                        // mov rm <-> reg
                case 0x88 ... 0x8b:
                {
                        bool d = (byte >> 1) & 1;
                        bool w = byte & 1;

                        auto [reg, rm_operand] = disasm_modrm(infile_stream, w);
                        std::string_view reg_operand{reg_names[w][reg]};

                        write_mov(outfile_stream, d, reg_operand, rm_operand);
                }
                break;

                        // mov mem <-> acc
                case 0xa0 ... 0xa3:
                {
                        bool pseudo_d = (byte >> 1) & 1;
                        bool w = byte & 1;

                        auto reg_operand = reg_names[w][acc_reg];
                        auto dir_addr_operand = disasm_dir_addr(infile_stream);

                        write_mov(
                                outfile_stream,
                                pseudo_d,
                                dir_addr_operand,
                                reg_operand
                        );
                }
                break;

                        // mov imm -> reg
                case 0xb0 ... 0xbf:
                {
                        bool w = (byte >> 3) & 1;
                        u8 reg = byte & bitmask3;

                        auto imm_operand = disasm_imm(infile_stream, w);

                        write_mov(
                                outfile_stream, reg_names[w][reg], imm_operand
                        );
                }
                break;

                        // mov imm -> rm
                case 0xc6 ... 0xc7:
                {
                        bool w = byte & 1;

                        auto [extension, rm_operand] =
                                disasm_modrm(infile_stream, w);

                        if (extension != 0)
                        {
                                std::println("error: invalid extension");
                                return EXIT_FAILURE;
                        }

                        auto imm_operand = disasm_imm(infile_stream, w);

                        write_mov(outfile_stream, rm_operand, imm_operand);
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
