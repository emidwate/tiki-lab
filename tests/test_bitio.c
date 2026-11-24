#include <stdio.h>
#include <stdint.h>
#include "bitio.h"

int main(void)
{
    FILE *f;
    BitWriter w;
    BitReader r;

    uint32_t a, b, c;
    int bajt1, bajt2;

    /* Zapis bitow */
    f = fopen("test.bin", "wb");

    if (f == NULL)
    {
        fprintf(stderr, "Blad otwarcia pliku do zapisu.\n");
        return 1;
    }

    bw_init(&w, f);

    bw_write_bits(&w, 0x5, 3);
    bw_write_bits(&w, 0x1F, 5);
    bw_write_bits(&w, 0x1, 2);

    bw_flush(&w);

    if (fclose(f) != 0)
    {
        fprintf(stderr, "Blad zamkniecia pliku.\n");
        return 1;
    }

    /* Sprawdzenie zapisanych bajtow */
    f = fopen("test.bin", "rb");

    if (f == NULL)
    {
        fprintf(stderr, "Blad otwarcia pliku do odczytu.\n");
        return 1;
    }

    bajt1 = fgetc(f);
    bajt2 = fgetc(f);

    fclose(f);

    if (bajt1 != 0xBF || bajt2 != 0x40)
    {
        printf("BLAD zapisu bitow!\n");
        return 1;
    }

    /* Odczyt zapisanych bitow */
    f = fopen("test.bin", "rb");

    if (f == NULL)
    {
        fprintf(stderr, "Blad otwarcia pliku.\n");
        return 1;
    }

    br_init(&r, f);

    if (br_read_bits(&r, 3, &a) != 0 ||
        br_read_bits(&r, 5, &b) != 0 ||
        br_read_bits(&r, 2, &c) != 0)
    {
        fprintf(stderr, "Blad odczytu bitow.\n");
        fclose(f);
        return 1;
    }

    fclose(f);

    /* Porownanie wynikow */
    if (a == 0x5 && b == 0x1F && c == 0x1)
    {
        printf("TEST ZAKONCZONY SUKCESEM!\n");
        printf("Odczytane wartosci: 0x%X, 0x%X, 0x%X\n",
               (unsigned int)a, (unsigned int)b, (unsigned int)c);
    }
    else
    {
        printf("BLAD odczytu bitow!\n");
        return 1;
    }

    return 0;
}

