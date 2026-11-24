$ErrorActionPreference = "Stop"

# Katalog projektu lab2
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

$Tests = Join-Path $Root "tests"
$Lab1Tests = Join-Path (Split-Path -Parent $Root) "lab1\tests"
$Huff = Join-Path $Root "huff.exe"

if (-not (Test-Path $Huff)) {
    Write-Error "Brak huff.exe. Skompiluj program (build.bat)."
    exit 1
}

$TmpDir = Join-Path $env:TEMP ("huff_test_" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $TmpDir | Out-Null

try {
    Write-Host "`n=== PUNKT KONTROLNY L2: abracadabra.txt ==="
    & $Huff t (Join-Path $Tests "abracadabra.txt")
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Blad wydruku tablicy kodow."
        exit 1
    }

    function Test-RoundTrip {
        param(
            [string]$Name,
            [string]$SrcPath
        )

        $CompPath = Join-Path $TmpDir ($Name + ".huf")
        $DecPath = Join-Path $TmpDir ($Name + ".dec")

        & $Huff c $SrcPath $CompPath
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Blad kompresji dla: $Name"
            exit 1
        }

        & $Huff d $CompPath $DecPath
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Blad dekompresji dla: $Name"
            exit 1
        }

        $SrcBytes = [IO.File]::ReadAllBytes($SrcPath)
        $DecBytes = [IO.File]::ReadAllBytes($DecPath)

        if ($SrcBytes.Length -ne $DecBytes.Length) {
            Write-Error "[FAIL] $Name - niezgodna dlugosc pliku po dekompresji!"
            exit 1
        }

        for ($i = 0; $i -lt $SrcBytes.Length; $i++) {
            if ($SrcBytes[$i] -ne $DecBytes[$i]) {
                Write-Error "[FAIL] $Name - roznica na bajcie $i!"
                exit 1
            }
        }

        $CompLen = (Get-Item $CompPath).Length
        Write-Host "[OK] $Name (oryginal: $($SrcBytes.Length) B, skompresowany: $CompLen B)"
    }

    Write-Host "`n=== TESTY KOMPRESJI I DEKOMPRESJI (BAJT W BAJT) ==="
    Test-RoundTrip "abracadabra" (Join-Path $Tests "abracadabra.txt")
    Test-RoundTrip "pusty" (Join-Path $Lab1Tests "pusty.bin")
    Test-RoundTrip "powtorzenia" (Join-Path $Lab1Tests "powtorzenia.bin")
    Test-RoundTrip "polski" (Join-Path $Lab1Tests "polski.txt")
    Test-RoundTrip "losowy" (Join-Path $Lab1Tests "losowy.bin")

    $OneBytePath = Join-Path $TmpDir "jeden_bajt.bin"
    [IO.File]::WriteAllBytes($OneBytePath, [byte[]]@(65))
    Test-RoundTrip "jeden_bajt" $OneBytePath

    $All256Path = Join-Path $TmpDir "wszystkie256.bin"
    $All256 = New-Object byte[] 256
    for ($i = 0; $i -lt 256; $i++) { $All256[$i] = [byte]$i }
    [IO.File]::WriteAllBytes($All256Path, $All256)
    Test-RoundTrip "wszystkie256" $All256Path

    Write-Host "`nWSZYSTKIE TESTY ZAKONCZONE SUKCESEM!"
}
finally {
    Remove-Item -Recurse -Force $TmpDir -ErrorAction SilentlyContinue
}
