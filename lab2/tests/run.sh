#!/bin/sh
set -e

HUFF=./huff
TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

check_roundtrip() {
    name="$1"
    src="$2"
    comp="$TMP_DIR/$name.huf"
    dec="$TMP_DIR/$name.dec"

    "$HUFF" c "$src" "$comp"
    "$HUFF" d "$comp" "$dec"

    if cmp -s "$src" "$dec"; then
        src_sz=$(wc -c < "$src")
        comp_sz=$(wc -c < "$comp")
        echo "[OK] $name (oryginal: ${src_sz} B, skompresowany: ${comp_sz} B)"
    else
        echo "[FAIL] $name: plik po dekompresji rozni sie od oryginalu!"
        exit 1
    fi
}

echo "=== Punkt kontrolny L2: abracadabra.txt ==="
"$HUFF" t tests/abracadabra.txt
echo ""

echo "=== Testy kompresji i dekompresji (bajt w bajt) ==="
check_roundtrip "abracadabra" "tests/abracadabra.txt"
check_roundtrip "pusty" "../lab1/tests/pusty.bin"
check_roundtrip "powtorzenia" "../lab1/tests/powtorzenia.bin"
check_roundtrip "polski" "../lab1/tests/polski.txt"
check_roundtrip "losowy" "../lab1/tests/losowy.bin"

# Przypadek szczegolny: plik 1-bajtowy
printf 'A' > "$TMP_DIR/jeden_bajt.bin"
check_roundtrip "jeden_bajt" "$TMP_DIR/jeden_bajt.bin"

# Przypadek szczegolny: wszystkie 256 wartosci bajtow (0x00..0xFF)
python3 -c 'import sys; sys.stdout.buffer.write(bytes(range(256)))' > "$TMP_DIR/wszystkie256.bin"
check_roundtrip "wszystkie256" "$TMP_DIR/wszystkie256.bin"

# Weryfikacja naglowka HUF1 dla abracadabra.txt (4 B 'HUF1' + 8 B LE n=11 + 1024 B + 3 B danych = 1039 B)
"$HUFF" c tests/abracadabra.txt "$TMP_DIR/abra_hdr.huf"
python3 - "$TMP_DIR/abra_hdr.huf" <<'PYEOF'
import struct, sys
data = open(sys.argv[1], "rb").read()
assert data[:4] == b"HUF1", f"Zla sygnatura: {data[:4]!r}"
n = struct.unpack("<Q", data[4:12])[0]
assert n == 11, f"Zle n: {n}"
freqs = struct.unpack("<256I", data[12:1036])
assert sum(freqs) == 11
assert freqs[ord('a')] == 5
assert freqs[ord('b')] == 2
assert freqs[ord('c')] == 1
assert freqs[ord('d')] == 1
assert freqs[ord('r')] == 2
# 23 bity -> 3 bajty danych (ostatni bajt dopelniony zerem)
assert len(data) == 1036 + 3, f"Oczekiwano 1039 B, jest {len(data)} B"
print("[OK] Weryfikacja formatu naglowka HUF1 i rozmiaru strumienia bitow (1039 B)")
PYEOF

# Test blednej sygnatury
printf 'BAD!' > "$TMP_DIR/bad.huf"
if "$HUFF" d "$TMP_DIR/bad.huf" "$TMP_DIR/bad.dec" 2>/dev/null; then
    echo "[FAIL] Dekoder powinien odrzucic bledna sygnature!"
    exit 1
else
    echo "[OK] Odrzucenie nieprawidlowej sygnatury"
fi

echo "Wszystkie testy zakonczone powodzeniem!"
