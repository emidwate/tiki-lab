@echo off
setlocal
set "GCC=D:\Dev-Cpp\MinGW64\bin\gcc.exe"

if not exist "%GCC%" (
    echo Blad: nie znaleziono gcc.exe pod podana sciezka.
    exit /b 1
)

"%GCC%" -std=c11 -Wall -Wextra -O2 -I. -I..\lab1 main.c huff.c ..\lab1\bitio.c -o huff.exe -lm
if errorlevel 1 exit /b 1

echo Kompilacja zakonczona poprawnie.
endlocal
