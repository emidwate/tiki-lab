Projekt realizuje zadanie Z2 (kodowanie Huffmana) na podstawie [TIiK_lab_L2_instrukcja.pdf](../TIiK_lab_L2_instrukcja.pdf). Zawiera program `huff` do kompresji (`c`), dekompresji (`d`) oraz wydruku tablicy kodów (`t`), wykorzystujący moduł bitowy `../lab1/bitio.{c,h}` oraz model źródła i obliczenia entropii z `../lab1/entropy.c`.

## Kompilacja i testowanie

Linux (Ubuntu/WSL), z katalogu `lab2`:

```bash
make clean
make
make test
```

Windows (PowerShell), z katalogu `lab2`:

```powershell
& "D:\Dev-Cpp\MinGW64\bin\gcc.exe" -std=c11 -Wall -Wextra -O2 -I. -I..\lab1 main.c huff.c ..\lab1\bitio.c -o huff.exe -lm
powershell -ExecutionPolicy Bypass -File tests\run.ps1
```

Program można uruchomić jako:

```bash
./huff c <wejscie> <wyjscie>     # kompresja
./huff d <wejscie> <wyjscie>     # dekompresja
./huff t <wejscie>               # wydruk tablicy kodow, L i entropii H
```

(w PowerShellu `./huff.exe`).

## Format pliku skompresowanego (`HUF1`)

- Sygnatura: `HUF1` (4 B).
- Rozmiar oryginału `n`: `uint64_t` (8 B, little-endian).
- Tablica liczności: `256 × uint32_t` (1024 B, little-endian).
- Dane: strumień kodów (MSB first, ostatni bajt dopełniony zerami przez `BitWriter`).

Obsługiwane przypadki szczególne: plik pusty (`n = 0`) oraz plik zawierający tylko jeden rodzaj symbolu (słowo kodowe długości 1 bitu: `0`). Remisy w kopcu binarnym typu min są rozstrzygane deterministycznie (mniejsza waga, a przy równej wadze mniejszy numer węzła: liście `0–255`, węzły wewnętrzne od `256` w kolejności tworzenia).

## Punkt kontrolny L2 (`tests/abracadabra.txt`)

Dla pliku `abracadabra.txt` (11 bajtów: `abracadabra`, bez znaku końca linii):

| Symbol | Kod | Liczność | Długość | Słowo kodowe |
| --- | --- | --- | --- | --- |
| `'a'` | 97 | 5 | 1 | `0` |
| `'b'` | 98 | 2 | 3 | `110` |
| `'c'` | 99 | 1 | 3 | `100` |
| `'d'` | 100 | 1 | 3 | `101` |
| `'r'` | 114 | 2 | 3 | `111` |

- Entropia: `H = 2.040373 bit/symbol`
- Średnia długość słowa kodowego: `L = 2.090909 bit/symbol`
- Nierówność `H <= L < H + 1`: `2.040373 <= 2.090909 < 3.040373` (OK)

## Kontrola pamięci

Kompilacja z flagami `-std=c11 -Wall -Wextra -O2` przechodzi bez ostrzeżeń. Testy z AddressSanitizer i UndefinedBehaviorSanitizer (`-fsanitize=address,undefined`) na plikach `abracadabra.txt`, `pusty.bin` oraz `powtorzenia.bin` zakończyły się bez błędów i wycieków pamięci.
