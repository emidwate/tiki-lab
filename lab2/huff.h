#ifndef HUFF_H
#define HUFF_H

#include <stdint.h>
#include <stddef.h>

#define HUFF_ALPHABET 256
#define HUFF_MAX_NODES (2 * HUFF_ALPHABET - 1) /* 511 */
#define HUFF_MAGIC "HUF1"
#define HUFF_HEADER_SIZE (4 + 8 + HUFF_ALPHABET * 4) /* 1036 B */

typedef struct {
    int id;          /* 0..255 dla lisci, 256..510 dla wezlow wewnetrznych */
    uint64_t weight; /* suma licznosci w poddrzewie */
    int left;        /* indeks lewego dziecka w tablicy nodes lub -1 */
    int right;       /* indeks prawego dziecka w tablicy nodes lub -1 */
    int symbol;      /* 0..255 dla liscia, -1 dla wezla wewnetrznego */
} HuffNode;

typedef struct {
    HuffNode nodes[HUFF_MAX_NODES];
    int node_count;
    int root; /* indeks korzenia w tablicy nodes lub -1 dla pustego pliku */
} HuffTree;

typedef struct {
    int items[HUFF_MAX_NODES]; /* indeksy wezlow w HuffTree.nodes */
    int size;
} MinHeap;

typedef struct {
    uint8_t bits[HUFF_ALPHABET]; /* bity slowa kodowego (MSB first: bits[0..len-1]) */
    int len;                     /* dlugosc slowa kodowego w bitach (0 = brak symbolu) */
} HuffCode;

/* Model zrodla i entropia (na podstawie lab1/entropy.c) */
int huff_count_frequencies(const char *path, uint32_t freq[HUFF_ALPHABET], uint64_t *total_bytes);
double huff_compute_entropy(const uint32_t freq[HUFF_ALPHABET], uint64_t total_bytes);

/* Kolejka priorytetowa (kopiec binarny typu min) */
void heap_init(MinHeap *h);
int heap_push(MinHeap *h, const HuffNode *nodes, int node_idx);
int heap_pop(MinHeap *h, const HuffNode *nodes);

/* Drzewo Huffmana i tablica kodow */
void huff_build_tree(const uint32_t freq[HUFF_ALPHABET], HuffTree *tree);
void huff_build_codes(const HuffTree *tree, HuffCode codes[HUFF_ALPHABET]);
double huff_average_code_length(const uint32_t freq[HUFF_ALPHABET],
                                uint64_t total_bytes,
                                const HuffCode codes[HUFF_ALPHABET]);
void huff_print_table(const uint32_t freq[HUFF_ALPHABET],
                      uint64_t total_bytes,
                      const HuffCode codes[HUFF_ALPHABET]);

/* Kompresja i dekompresja */
int huff_compress(const char *in_path, const char *out_path, int verbose);
int huff_decompress(const char *in_path, const char *out_path);

#endif
