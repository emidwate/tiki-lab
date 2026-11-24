# Lab 2 — Kodowanie Huffmana (`huff`)

Implementacja zadania Z2 na podstawie [TIiK_lab_L2_instrukcja.pdf](../TIiK_lab_L2_instrukcja.pdf), z wykorzystaniem modułu operacji bitowych [`lab1/bitio.h`](../lab1/bitio.h) / [`lab1/bitio.c`](../lab1/bitio.c) oraz modelu źródła i obliczeń entropii z [`lab1/entropy.c`](../lab1/entropy.c).

## Funkcjonalność

- Zliczanie liczności 256 symboli (`uint32_t`) i obliczanie entropii $H$.
- Kolejka priorytetowa zaimplementowana jako kopiec binarny typu min z deterministycznym rozstrzyganiem remisów (mniejsza waga, a przy równej wadze mniejszy numer węzła: liście `0..255`, węzły wewnętrzne od `256` w kolejności tworzenia).
- Budowa drzewa Huffmana i tablicy kodów (obsługa dowolnej długości słowa kodowego do 255 bitów).
- Obsługa przypadków szczególnych: plik pusty (`n = 0`) oraz plik zawierający tylko jeden rodzaj symbolu (słowo kodowe długości 1 bitu: `0`).
- Zapis i odczyt formatu `HUF1`:
  - `HUF1` (4 B),
  - rozmiar oryginału `n` (`uint64_t`, 8 B, little-endian),
  - tablica liczności (`256 × uint32_t`, 1024 B, little-endian),
  - strumień bitów (MSB first, ostatni bajt dopełniony zerami przez `BitWriter`).
- Kompresja (`c`), dekompresja (`d`) bajt w bajt oraz wydruk tablicy kodów (`t`).

## Kompilacja i uruchomienie

Z katalogu `lab2`:

```bash
make
./huff c <wejscie> <wyjscie>     # kompresja
./huff d <wejscie> <wyjscie>     # dekompresja
./huff t <wejscie>               # wydruk tablicy kodow, L i entropii H
```

Uruchomienie punktu kontrolnego (`abracadabra.txt`) oraz testów:

```bash
make abracadabra
make test
```

## Punkt kontrolny L2 (`tests/abracadabra.txt`)

Dla pliku `abracadabra.txt` (11 bajtów: `abracadabra`, bez znaku końca linii):

| Symbol | Kod | Liczność | Długość | Słowo kodowe |
| --- | --- | --- | --- | --- |
| `'a'` | 97 | 5 | 1 | `0` |
| `'b'` | 98 | 2 | 3 | `110` |
| `'c'` | 99 | 1 | 3 | `100` |
| `'d'` | 100 | 1 | 3 | `101` |
| `'r'` | 114 | 2 | 3 | `111` |

- Entropia $H = 2.040373\text{ bit/symbol}$
- Średnia długość słowa kodowego $L = 2.090909\text{ bit/symbol}$
- Sprawdzenie nierówności: $2.040373 \le 2.090909 < 3.040373$ (OK)
