@echo off
setlocal
set "GCC=D:\Dev-Cpp\MinGW64\bin\gcc.exe"

if not exist "%GCC%" (
    echo Blad: nie znaleziono gcc.exe pod podana sciezka.
    exit /b 1
)

"%GCC%" -std=c11 -Wall -Wextra -O2 entropy.c -o entropy.exe -lm
if errorlevel 1 exit /b 1

"%GCC%" -std=c11 -Wall -Wextra -O2 -I. bitio.c tests\test_bitio.c -o test_bitio.exe
if errorlevel 1 exit /b 1

echo Kompilacja zakonczona poprawnie.
endlocal
