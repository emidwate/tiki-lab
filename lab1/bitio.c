#include "bitio.h"

/* Inicjalizacja zapisu */
void bw_init(BitWriter *w, FILE *f)
{
    w->f = f;
    w->buf = 0;
    w->nbits = 0;
}

/* Zapis jednego bitu ale fizycznie do pliku zapisuje bajt dopiero gdy zbierze 8 bitow */
void bw_write_bit(BitWriter *w, int bit)
{
    w->buf = (uint8_t)(w->buf << 1); // Przesuniecie bitowe w lewo o jeden bit.

    if (bit != 0)
    {
        w->buf |= 1; // Operator |= oznacza operacje OR i przypisanie wyniku.
    }

    w->nbits++; // Zwiekszamy liczbe zapisanych bitów:

    /* Jezeli mamy 8 bitow, zapisujemy bajt */
    if (w->nbits == 8)
    {
    	// Nastepnie czyscimy bufor i zaczynamy zbierac nastepny bajt.
        fputc(w->buf, w->f);
        w->buf = 0;
        w->nbits = 0;
    }
}

/* Zapis n bitow, zaczynajac od najstarszego */
void bw_write_bits(BitWriter *w, uint32_t value, int n)
{
    int i;

    if (n < 0 || n > 32)
    {
        return;
    }

    //Poniewaz zapisujemy bity od najbardziej znaczacego sposród wybranych bitów do najmniej znaczacego.
    for (i = n - 1; i >= 0; i--)
    {
        bw_write_bit(w, (value >> i) & 1);
    }
}

/* Dopelnienie ostatniego bajtu zerami */
// jesli chceby zapisac 101 to trzeba je uzupelnic do 10100000
void bw_flush(BitWriter *w)
{
    if (w->nbits > 0)
    {
        w->buf = (uint8_t)(w->buf << (8 - w->nbits));

        fputc(w->buf, w->f);

        w->buf = 0;
        w->nbits = 0;
    }

    fflush(w->f);
}

/* Inicjalizacja odczytu */
void br_init(BitReader *r, FILE *f)
{
    r->f = f;
    r->buf = 0;
    r->nbits = 0;
}

/* Odczyt jednego bitu */
int br_read_bit(BitReader *r)
{
    int c;

    /* Pobieramy nowy bajt, jezeli poprzedni zostal odczytany */
    if (r->nbits == 0)
    {
        c = fgetc(r->f);

        if (c == EOF)
        {
            return -1;
        }

        r->buf = (uint8_t)c;
        r->nbits = 8;
    }

    /* Odczyt od najstarszego bitu */
    {
    	// Odczytuje najstarszy bit bajtu
        int bit = (r->buf >> 7) & 1;

        r->buf = (uint8_t)(r->buf << 1);
        r->nbits--;

        return bit;
    }
}

/* Odczyt n bitow */
// br_read_bits(&r, 3, &a);
// Oznacza odczytaj 3 bity, zapisz odczytana wartosc do zmiennej a.
int br_read_bits(BitReader *r, int n, uint32_t *out)
{
    int i;
    int bit;
    uint32_t value = 0;

    if (n < 0 || n > 32 || out == NULL)
    {
        return -1;
    }

    for (i = 0; i < n; i++)
    {
        bit = br_read_bit(r);

        if (bit == -1)
        {
            return -1;
        }

        value = (value << 1) | (uint32_t)bit;
    }

    *out = value;

    return 0;
}

