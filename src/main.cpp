#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <print>

using u8 = uint8_t;

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

        u8 byte;
        while (infile_stream.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
        {
                switch (byte)
                {
                default:
                {
                        std::println(
                                stderr,
                                "error: unknown opcode: {:04b} {:04b}",
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
