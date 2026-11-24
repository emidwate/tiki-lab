#!/bin/sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$ROOT_DIR"

echo "=== TEST MODULU BITOWEGO ==="
./test_bitio

echo
echo "=== TESTY ENTROPII ==="

for file in \
    tests/pusty.bin \
    tests/powtorzenia.bin \
    tests/polski.txt \
    tests/losowy.bin \
    tests/losowy.zip
do
    if [ ! -f "$file" ]; then
        echo "Blad: brak pliku $file" >&2
        exit 1
    fi

    echo
    echo "Plik: $file"
    ./entropy "$file"
done

echo
echo "=== TEST PLIKU WYKONYWALNEGO ==="
./entropy ./entropy

echo
echo "=== TEST BRAKU PLIKU ==="

error_file=$(mktemp)

if ./entropy tests/nieistnieje.bin >/dev/null 2>"$error_file"; then
    echo "Blad: program powinien zwrocic kod niezerowy"
    rm -f "$error_file"
    exit 1
else
    echo "Poprawnie obsluzono brak pliku."
fi

rm -f "$error_file"

echo
echo "=== WSZYSTKIE TESTY ZAKONCZONE ==="
