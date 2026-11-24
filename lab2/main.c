#include "huff.h"

#include <stdio.h>
#include <string.h>

static void print_usage(const char *prog)
{
    fprintf(stderr,
            "Uzycie:\n"
            "  %s c <wejscie> <wyjscie>     # kompresja\n"
            "  %s c -v <wejscie> <wyjscie>  # kompresja z wydrukiem tablicy kodow\n"
            "  %s d <wejscie> <wyjscie>     # dekompresja\n"
            "  %s t <wejscie>               # wydruk tablicy kodow, L i entropii H\n",
            prog, prog, prog, prog);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "c") == 0)
    {
        if (argc == 4)
        {
            return huff_compress(argv[2], argv[3], 0) == 0 ? 0 : 1;
        }
        if (argc == 5 && strcmp(argv[2], "-v") == 0)
        {
            return huff_compress(argv[3], argv[4], 1) == 0 ? 0 : 1;
        }
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "d") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }
        return huff_decompress(argv[2], argv[3]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "t") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        uint32_t freq[HUFF_ALPHABET];
        uint64_t total_bytes = 0;
        if (huff_count_frequencies(argv[2], freq, &total_bytes) != 0)
        {
            return 1;
        }

        HuffTree tree;
        HuffCode codes[HUFF_ALPHABET];
        huff_build_tree(freq, &tree);
        huff_build_codes(&tree, codes);
        huff_print_table(freq, total_bytes, codes);
        return 0;
    }

    print_usage(argv[0]);
    return 1;
}
