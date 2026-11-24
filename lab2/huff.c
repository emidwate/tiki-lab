#include "huff.h"
#include "bitio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

/* --------------------------------------------------------------------------
 * Pomocnicze funkcje zapisu i odczytu liczb w formacie little-endian
 * -------------------------------------------------------------------------- */

static int write_u32_le(FILE *f, uint32_t v)
{
    uint8_t b[4];
    b[0] = (uint8_t)(v & 0xFFu);
    b[1] = (uint8_t)((v >> 8) & 0xFFu);
    b[2] = (uint8_t)((v >> 16) & 0xFFu);
    b[3] = (uint8_t)((v >> 24) & 0xFFu);
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

static int write_u64_le(FILE *f, uint64_t v)
{
    uint8_t b[8];
    for (int i = 0; i < 8; i++)
    {
        b[i] = (uint8_t)((v >> (8 * i)) & 0xFFu);
    }
    return fwrite(b, 1, 8, f) == 8 ? 0 : -1;
}

static int read_u32_le(FILE *f, uint32_t *out)
{
    uint8_t b[4];
    if (fread(b, 1, 4, f) != 4)
    {
        return -1;
    }
    *out = ((uint32_t)b[0]) |
           ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) |
           ((uint32_t)b[3] << 24);
    return 0;
}

static int read_u64_le(FILE *f, uint64_t *out)
{
    uint8_t b[8];
    if (fread(b, 1, 8, f) != 8)
    {
        return -1;
    }
    uint64_t v = 0;
    for (int i = 0; i < 8; i++)
    {
        v |= ((uint64_t)b[i] << (8 * i));
    }
    *out = v;
    return 0;
}

/* --------------------------------------------------------------------------
 * Model zrodla i entropia (na podstawie lab1/entropy.c)
 * -------------------------------------------------------------------------- */

int huff_count_frequencies(const char *path, uint32_t freq[HUFF_ALPHABET], uint64_t *total_bytes)
{
    FILE *plik = fopen(path, "rb");
    if (plik == NULL)
    {
        fprintf(stderr, "Blad: nie mozna otworzyc pliku wejsciowego '%s'.\n", path);
        return -1;
    }

    memset(freq, 0, sizeof(uint32_t) * HUFF_ALPHABET);
    uint64_t n = 0;
    int bajt;

    while ((bajt = fgetc(plik)) != EOF)
    {
        unsigned char sym = (unsigned char)bajt;
        if (freq[sym] == UINT32_MAX)
        {
            fprintf(stderr, "Blad: przekroczono maksymalna licznosc symbolu (UINT32_MAX).\n");
            fclose(plik);
            return -1;
        }
        freq[sym]++;
        n++;
    }

    if (ferror(plik))
    {
        fprintf(stderr, "Blad podczas odczytu pliku '%s'.\n", path);
        fclose(plik);
        return -1;
    }

    fclose(plik);
    *total_bytes = n;
    return 0;
}

double huff_compute_entropy(const uint32_t freq[HUFF_ALPHABET], uint64_t total_bytes)
{
    if (total_bytes == 0)
    {
        return 0.0;
    }

    double H = 0.0;
    for (int i = 0; i < HUFF_ALPHABET; i++)
    {
        if (freq[i] > 0)
        {
            double p = (double)freq[i] / (double)total_bytes;
            H -= p * log2(p);
        }
    }
    return H;
}

/* --------------------------------------------------------------------------
 * Kolejka priorytetowa: kopiec binarny typu min
 * Deterministyczne rozstrzyganie remisow:
 * mniejsza waga, a przy rownej wadze mniejszy numer wezla (id).
 * -------------------------------------------------------------------------- */

static int node_less(const HuffNode *nodes, int idx_a, int idx_b)
{
    const HuffNode *a = &nodes[idx_a];
    const HuffNode *b = &nodes[idx_b];

    if (a->weight != b->weight)
    {
        return a->weight < b->weight;
    }
    return a->id < b->id;
}

void heap_init(MinHeap *h)
{
    h->size = 0;
}

int heap_push(MinHeap *h, const HuffNode *nodes, int node_idx)
{
    if (h->size >= HUFF_MAX_NODES)
    {
        return -1;
    }

    int pos = h->size++;
    h->items[pos] = node_idx;

    /* Przesiewanie w gore (sift-up) */
    while (pos > 0)
    {
        int parent = (pos - 1) / 2;
        if (node_less(nodes, h->items[pos], h->items[parent]))
        {
            int tmp = h->items[pos];
            h->items[pos] = h->items[parent];
            h->items[parent] = tmp;
            pos = parent;
        }
        else
        {
            break;
        }
    }

    return 0;
}

int heap_pop(MinHeap *h, const HuffNode *nodes)
{
    if (h->size <= 0)
    {
        return -1;
    }

    int min_idx = h->items[0];
    h->size--;

    if (h->size > 0)
    {
        h->items[0] = h->items[h->size];

        /* Przesiewanie w dol (sift-down) */
        int pos = 0;
        while (1)
        {
            int left = 2 * pos + 1;
            int right = 2 * pos + 2;
            int smallest = pos;

            if (left < h->size && node_less(nodes, h->items[left], h->items[smallest]))
            {
                smallest = left;
            }
            if (right < h->size && node_less(nodes, h->items[right], h->items[smallest]))
            {
                smallest = right;
            }

            if (smallest != pos)
            {
                int tmp = h->items[pos];
                h->items[pos] = h->items[smallest];
                h->items[smallest] = tmp;
                pos = smallest;
            }
            else
            {
                break;
            }
        }
    }

    return min_idx;
}

/* --------------------------------------------------------------------------
 * Budowa drzewa Huffmana i wyznaczanie tablicy kodow
 * -------------------------------------------------------------------------- */

void huff_build_tree(const uint32_t freq[HUFF_ALPHABET], HuffTree *tree)
{
    MinHeap heap;
    heap_init(&heap);

    tree->node_count = 0;
    tree->root = -1;

    /* 1. Liscie maja numery rowne wartosci symbolu (0..255) */
    for (int sym = 0; sym < HUFF_ALPHABET; sym++)
    {
        if (freq[sym] > 0)
        {
            int idx = tree->node_count++;
            tree->nodes[idx].id = sym;
            tree->nodes[idx].weight = freq[sym];
            tree->nodes[idx].left = -1;
            tree->nodes[idx].right = -1;
            tree->nodes[idx].symbol = sym;
            heap_push(&heap, tree->nodes, idx);
        }
    }

    if (heap.size == 0)
    {
        return; /* Plik pusty */
    }

    /* 2. Laczenie dwoch wezlow o najmniejszych wagach; wezly wewnetrzne od id = 256 */
    int next_id = HUFF_ALPHABET;
    while (heap.size >= 2)
    {
        int left_idx = heap_pop(&heap, tree->nodes);
        int right_idx = heap_pop(&heap, tree->nodes);

        int idx = tree->node_count++;
        tree->nodes[idx].id = next_id++;
        tree->nodes[idx].weight = tree->nodes[left_idx].weight + tree->nodes[right_idx].weight;
        tree->nodes[idx].left = left_idx;
        tree->nodes[idx].right = right_idx;
        tree->nodes[idx].symbol = -1;

        heap_push(&heap, tree->nodes, idx);
    }

    /* 3. Ostatni wezel w kolejce jest korzeniem drzewa */
    tree->root = heap_pop(&heap, tree->nodes);
}

static void build_codes_dfs(const HuffTree *tree,
                            int node_idx,
                            uint8_t path[HUFF_ALPHABET],
                            int depth,
                            HuffCode codes[HUFF_ALPHABET])
{
    const HuffNode *node = &tree->nodes[node_idx];

    if (node->symbol >= 0)
    {
        HuffCode *c = &codes[node->symbol];
        c->len = depth;
        if (depth > 0)
        {
            memcpy(c->bits, path, (size_t)depth);
        }
        return;
    }

    if (node->left >= 0)
    {
        path[depth] = 0;
        build_codes_dfs(tree, node->left, path, depth + 1, codes);
    }
    if (node->right >= 0)
    {
        path[depth] = 1;
        build_codes_dfs(tree, node->right, path, depth + 1, codes);
    }
}

void huff_build_codes(const HuffTree *tree, HuffCode codes[HUFF_ALPHABET])
{
    memset(codes, 0, sizeof(HuffCode) * HUFF_ALPHABET);

    if (tree->root < 0)
    {
        return; /* Plik pusty */
    }

    const HuffNode *root = &tree->nodes[tree->root];
    if (root->symbol >= 0)
    {
        /* Przypadek szczegolny: tylko jeden rodzaj symbolu -> slowo kodowe o dlugosci 1 bitu ("0") */
        codes[root->symbol].bits[0] = 0;
        codes[root->symbol].len = 1;
        return;
    }

    uint8_t path[HUFF_ALPHABET];
    build_codes_dfs(tree, tree->root, path, 0, codes);
}

double huff_average_code_length(const uint32_t freq[HUFF_ALPHABET],
                                uint64_t total_bytes,
                                const HuffCode codes[HUFF_ALPHABET])
{
    if (total_bytes == 0)
    {
        return 0.0;
    }

    double total_bits = 0.0;
    for (int i = 0; i < HUFF_ALPHABET; i++)
    {
        if (freq[i] > 0)
        {
            total_bits += (double)freq[i] * (double)codes[i].len;
        }
    }
    return total_bits / (double)total_bytes;
}

void huff_print_table(const uint32_t freq[HUFF_ALPHABET],
                      uint64_t total_bytes,
                      const HuffCode codes[HUFF_ALPHABET])
{
    printf("Tablica kodow Huffmana:\n");
    printf("%-8s %-8s %-10s %-8s %s\n", "Symbol", "Kod", "Licznosc", "Dlugosc", "Slowo kodowe");
    printf("--------------------------------------------------------\n");

    int distinct = 0;
    for (int i = 0; i < HUFF_ALPHABET; i++)
    {
        if (freq[i] == 0)
        {
            continue;
        }
        distinct++;

        char label[16];
        if (isprint(i))
        {
            snprintf(label, sizeof(label), "'%c'", i);
        }
        else if (i == '\n')
        {
            snprintf(label, sizeof(label), "'\\n'");
        }
        else if (i == '\r')
        {
            snprintf(label, sizeof(label), "'\\r'");
        }
        else if (i == '\t')
        {
            snprintf(label, sizeof(label), "'\\t'");
        }
        else
        {
            snprintf(label, sizeof(label), "0x%02X", i);
        }

        printf("%-8s %-8d %-10u %-8d ", label, i, freq[i], codes[i].len);
        for (int b = 0; b < codes[i].len; b++)
        {
            putchar(codes[i].bits[b] ? '1' : '0');
        }
        putchar('\n');
    }

    double H = huff_compute_entropy(freq, total_bytes);
    double L = huff_average_code_length(freq, total_bytes, codes);

    printf("--------------------------------------------------------\n");
    printf("Rozmiar pliku (n): %llu bajtow\n", (unsigned long long)total_bytes);
    printf("Liczba roznych symboli: %d\n", distinct);
    printf("Entropia H: %.6f bit/symbol\n", H);
    printf("Srednia dlugosc slowa kodowego L: %.6f bit/symbol\n", L);
    if (total_bytes > 0)
    {
        printf("Nierownosc H <= L < H + 1: %.6f <= %.6f < %.6f (%s)\n",
               H, L, H + 1.0, (H - 1e-12 <= L && L < H + 1.0 + 1e-12) ? "OK" : "BLAD");
    }
}

/* --------------------------------------------------------------------------
 * Kompresja: zapis naglowka HUF1 oraz strumienia kodow przez BitWriter
 * -------------------------------------------------------------------------- */

int huff_compress(const char *in_path, const char *out_path, int verbose)
{
    uint32_t freq[HUFF_ALPHABET];
    uint64_t total_bytes = 0;

    if (huff_count_frequencies(in_path, freq, &total_bytes) != 0)
    {
        return -1;
    }

    HuffTree tree;
    HuffCode codes[HUFF_ALPHABET];
    huff_build_tree(freq, &tree);
    huff_build_codes(&tree, codes);

    if (verbose)
    {
        huff_print_table(freq, total_bytes, codes);
    }

    FILE *fin = fopen(in_path, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "Blad: nie mozna otworzyc pliku wejsciowego '%s'.\n", in_path);
        return -1;
    }

    FILE *fout = fopen(out_path, "wb");
    if (fout == NULL)
    {
        fprintf(stderr, "Blad: nie mozna otworzyc pliku wyjsciowego '%s'.\n", out_path);
        fclose(fin);
        return -1;
    }

    /* Zapis naglowka: sygnatura (4 B), n (8 B LE), licznosci (256 x 4 B LE) */
    if (fwrite(HUFF_MAGIC, 1, 4, fout) != 4 || write_u64_le(fout, total_bytes) != 0)
    {
        fprintf(stderr, "Blad zapisu naglowka do pliku '%s'.\n", out_path);
        fclose(fin);
        fclose(fout);
        return -1;
    }

    for (int i = 0; i < HUFF_ALPHABET; i++)
    {
        if (write_u32_le(fout, freq[i]) != 0)
        {
            fprintf(stderr, "Blad zapisu tablicy licznosci do pliku '%s'.\n", out_path);
            fclose(fin);
            fclose(fout);
            return -1;
        }
    }

    /* Zapis strumienia kodow za pomoca BitWriter (MSB first, dopelnienie zerami) */
    if (total_bytes > 0)
    {
        BitWriter bw;
        bw_init(&bw, fout);

        int bajt;
        while ((bajt = fgetc(fin)) != EOF)
        {
            const HuffCode *c = &codes[(unsigned char)bajt];
            for (int b = 0; b < c->len; b++)
            {
                bw_write_bit(&bw, c->bits[b]);
            }
        }

        if (ferror(fin))
        {
            fprintf(stderr, "Blad odczytu pliku wejsciowego '%s'.\n", in_path);
            fclose(fin);
            fclose(fout);
            return -1;
        }

        bw_flush(&bw);
    }

    if (ferror(fout))
    {
        fprintf(stderr, "Blad zapisu pliku wyjsciowego '%s'.\n", out_path);
        fclose(fin);
        fclose(fout);
        return -1;
    }

    fclose(fin);
    fclose(fout);
    return 0;
}

/* --------------------------------------------------------------------------
 * Dekompresja: odczyt naglowka HUF1, odbudowa drzewa i dekodowanie przez BitReader
 * -------------------------------------------------------------------------- */

int huff_decompress(const char *in_path, const char *out_path)
{
    FILE *fin = fopen(in_path, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "Blad: nie mozna otworzyc pliku skompresowanego '%s'.\n", in_path);
        return -1;
    }

    char magic[4];
    uint64_t total_bytes = 0;
    uint32_t freq[HUFF_ALPHABET];

    if (fread(magic, 1, 4, fin) != 4 || memcmp(magic, HUFF_MAGIC, 4) != 0)
    {
        fprintf(stderr, "Blad: nieprawidlowa sygnatura pliku (oczekiwano '%s').\n", HUFF_MAGIC);
        fclose(fin);
        return -1;
    }

    if (read_u64_le(fin, &total_bytes) != 0)
    {
        fprintf(stderr, "Blad: uszkodzony naglowek (brak rozmiaru n).\n");
        fclose(fin);
        return -1;
    }

    uint64_t freq_sum = 0;
    for (int i = 0; i < HUFF_ALPHABET; i++)
    {
        if (read_u32_le(fin, &freq[i]) != 0)
        {
            fprintf(stderr, "Blad: uszkodzony naglowek (niepelna tablica licznosci).\n");
            fclose(fin);
            return -1;
        }
        freq_sum += freq[i];
    }

    if (freq_sum != total_bytes)
    {
        fprintf(stderr, "Blad: suma licznosci w naglowku nie zgadza sie z rozmiarem pliku.\n");
        fclose(fin);
        return -1;
    }

    FILE *fout = fopen(out_path, "wb");
    if (fout == NULL)
    {
        fprintf(stderr, "Blad: nie mozna otworzyc pliku wyjsciowego '%s'.\n", out_path);
        fclose(fin);
        return -1;
    }

    /* Przypadek szczegolny: plik pusty */
    if (total_bytes == 0)
    {
        fclose(fin);
        fclose(fout);
        return 0;
    }

    HuffTree tree;
    huff_build_tree(freq, &tree);

    BitReader br;
    br_init(&br, fin);

    const HuffNode *root = &tree.nodes[tree.root];

    if (root->symbol >= 0)
    {
        /* Przypadek szczegolny: jeden rodzaj symbolu (slowo kodowe 1-bitowe) */
        for (uint64_t i = 0; i < total_bytes; i++)
        {
            int bit = br_read_bit(&br);
            if (bit == -1)
            {
                fprintf(stderr, "Blad: przedwczesny koniec strumienia bitow.\n");
                fclose(fin);
                fclose(fout);
                return -1;
            }
            if (fputc(root->symbol, fout) == EOF)
            {
                fprintf(stderr, "Blad zapisu pliku wyjsciowego '%s'.\n", out_path);
                fclose(fin);
                fclose(fout);
                return -1;
            }
        }
    }
    else
    {
        /* Dekodowanie dokladnie n symboli; bity dopelnienia sa ignorowane */
        for (uint64_t i = 0; i < total_bytes; i++)
        {
            int cur = tree.root;
            while (tree.nodes[cur].symbol < 0)
            {
                int bit = br_read_bit(&br);
                if (bit == -1)
                {
                    fprintf(stderr, "Blad: przedwczesny koniec strumienia bitow.\n");
                    fclose(fin);
                    fclose(fout);
                    return -1;
                }
                cur = (bit == 0) ? tree.nodes[cur].left : tree.nodes[cur].right;
            }

            if (fputc(tree.nodes[cur].symbol, fout) == EOF)
            {
                fprintf(stderr, "Blad zapisu pliku wyjsciowego '%s'.\n", out_path);
                fclose(fin);
                fclose(fout);
                return -1;
            }
        }
    }

    if (ferror(fout))
    {
        fprintf(stderr, "Blad zapisu pliku wyjsciowego '%s'.\n", out_path);
        fclose(fin);
        fclose(fout);
        return -1;
    }

    fclose(fin);
    fclose(fout);
    return 0;
}
