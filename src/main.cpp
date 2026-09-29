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
                        "error: incorrect amount of arguments\nusage: {} "
                        "INFILE OUTFILE",
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

        return EXIT_SUCCESS;
}
