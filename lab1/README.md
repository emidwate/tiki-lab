Projekt realizuje zadanie Z1. Zawiera program `entropy`, który liczy rozmiar pliku, liczbę różnych bajtów, entropię oraz teoretyczny minimalny rozmiar po kompresji (`n * H / 8`), a także moduł `bitio` do zapisu i odczytu bitów.

## Kompilacja i testowanie

Linux (Ubuntu/WSL), z głównego katalogu projektu:

```bash
make clean
make
make test
```

Windows (PowerShell), z głównego katalogu projektu:

```powershell
& "D:\Dev-Cpp\MinGW64\bin\gcc.exe" -std=c11 -Wall -Wextra -O2 entropy.c -o entropy.exe -lm
& "D:\Dev-Cpp\MinGW64\bin\gcc.exe" -std=c11 -Wall -Wextra -O2 -I. bitio.c tests\test_bitio.c -o test_bitio.exe
powershell -ExecutionPolicy Bypass -File tests\run.ps1
```

Program można uruchomić jako `./entropy plik` w Linuxie lub `./entropy.exe plik` w PowerShellu.

Entropia losowego pliku i ZIP-a jest bliska 8 bitom/symbol. Małe archiwum `polski.zip`miało entropię 4,890974 bit/symbol; ze względu na mały rozmiar i udział nagłówków nie jest ono reprezentatywną próbką dużego archiwum.

Pliki wykonywalne zależą od systemu i kompilatora, dlatego ich wyniki są różne:

## Kontrola pamięci

Valgrind w Ubuntu/WSL:

- `entropy`: 3 alokacje, 3 zwolnienia, 0 B pozostawionej pamięci, `ERROR SUMMARY: 0 errors`.
- `test_bitio`: 7 alokacji, 7 zwolnień, 0 B pozostawionej pamięci, `ERROR SUMMARY: 0 errors`.

Dr. Memory 2.6.0 na Windowsie nie rozpoczął diagnostyki z powodu błędu uruchomieniowego `dbghelp.dll: library initializer failed`. Nie uzyskano raportu Dr. Memory; kontrola Valgrind została wykonana poprawnie w Linuxie.
