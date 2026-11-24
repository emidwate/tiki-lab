$ErrorActionPreference = "Stop"

# Katalog projektu
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

$Tests = Join-Path $Root "tests"
$Entropy = Join-Path $Root "entropy.exe"
$Bitio = Join-Path $Root "test_bitio.exe"
$Results = Join-Path $Tests "wyniki_pomiarow_windows.txt"

# Sprawdzenie programow
if (-not (Test-Path $Entropy)) {
    Write-Error "Brak entropy.exe. Skompiluj program."
    exit 1
}

if (-not (Test-Path $Bitio)) {
    Write-Error "Brak test_bitio.exe. Skompiluj test."
    exit 1
}

# Sprawdzenie plikow testowych
$Files = @(
    "pusty.bin",
    "powtorzenia.bin",
    "polski.txt",
    "losowy.bin",
    "losowy.zip"
)

foreach ($File in $Files) {
    if (-not (Test-Path (Join-Path $Tests $File))) {
        Write-Host "Brak pliku: $File"
        exit 1
    }
}

$AllOutput = @()
$AllOutput += "Wyniki testow Z1 - Windows"
$AllOutput += "Data: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"

# Test modulu bitowego
Write-Host "`n=== TEST MODULU BITOWEGO ==="

$Output = & $Bitio
$ExitCode = $LASTEXITCODE

$Output | ForEach-Object { Write-Host $_ }

if ($ExitCode -ne 0) {
    Write-Host "Blad testu bitowego."
    exit 1
}

if (($Output -join "`n") -notmatch "TEST ZAKONCZONY SUKCESEM") {
    Write-Host "Test bitowy nie zostal potwierdzony."
    exit 1
}

$AllOutput += $Output

# Testy entropii
$Cases = @(
    @{ Name = "Pusty plik"; Path = "pusty.bin" },
    @{ Name = "1000 identycznych bajtow"; Path = "powtorzenia.bin" },
    @{ Name = "Tekst polski"; Path = "polski.txt" },
    @{ Name = "Losowy plik 1 MB"; Path = "losowy.bin" },
    @{ Name = "Archiwum ZIP"; Path = "losowy.zip" }
)

foreach ($Case in $Cases) {
    Write-Host "`n=== $($Case.Name) ==="

    $Path = Join-Path $Tests $Case.Path
    $Output = & $Entropy $Path
    $ExitCode = $LASTEXITCODE

    $Output | ForEach-Object { Write-Host $_ }

    if ($ExitCode -ne 0) {
        Write-Host "Test nie powiodl sie."
        exit 1
    }

    $AllOutput += "=== $($Case.Name) ==="
    $AllOutput += $Output
}

# Test pliku wykonywalnego
Write-Host "`n=== TEST PLIKU EXE ==="

$Output = & $Entropy $Entropy
$ExitCode = $LASTEXITCODE

$Output | ForEach-Object { Write-Host $_ }

if ($ExitCode -ne 0) {
    Write-Host "Test pliku EXE nie powiodl sie."
    exit 1
}

$AllOutput += "=== TEST PLIKU EXE ==="
$AllOutput += $Output

# Test nieistniejacego pliku
Write-Host "`n=== TEST BRAKU PLIKU ==="

$MissingFile = Join-Path $Tests "nieistnieje.bin"
$StdOutFile = Join-Path $env:TEMP "entropy_stdout.txt"
$ErrorFile = Join-Path $env:TEMP "entropy_stderr.txt"

$Process = Start-Process -FilePath $Entropy `
    -ArgumentList ('"{0}"' -f $MissingFile) `
    -NoNewWindow -Wait -PassThru `
    -RedirectStandardOutput $StdOutFile `
    -RedirectStandardError $ErrorFile

$ExitCode = $Process.ExitCode
$ErrorText = Get-Content $ErrorFile -Raw

if ($ExitCode -eq 0 -or $ErrorText -notmatch "Blad") {
    Write-Host "Blad: nieprawidlowa obsluga braku pliku."
    exit 1
}

Write-Host "Poprawnie obsluzono brak pliku."
Write-Host "Kod wyjscia: $ExitCode"
Write-Host "Komunikat: $ErrorText"

$AllOutput += "Test braku pliku: OK, kod $ExitCode"
$AllOutput += $ErrorText

Remove-Item $StdOutFile, $ErrorFile -Force

# Zapis wynikow
$AllOutput | Set-Content -Path $Results -Encoding UTF8

Write-Host "`nWSZYSTKIE TESTY ZAKONCZONE SUKCESEM!"
Write-Host "Wyniki zapisano w: $Results"

