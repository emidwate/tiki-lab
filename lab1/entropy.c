#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// argc liczba argumentow
// argv tablica argumentow
int main(int argc, char *argv[])
{
    FILE *plik;
    // tablica ma dokladnie 256 bo 2^8 poniewaz jeden bajt sklada sie z 8 bitow
    // moze przyjmowac wartosci od 0 do 255
	unsigned int licznik[256] = {0}; // tablica przechowujace liczbe wystapien kazdego bajtu
	unsigned int n = 0; // calkowita liczba bajtwo
    int bajt; // aktualny odczytany bajt
    int rozne = 0; // liczba roznych wartosci bajtow
    double H = 0.0; // entropia
    double minimalny_rozmiar; // teoretyczny rozmiar po kompresji

    /* Sprawdzenie argumentow czy uzytkownik podal 
	poprawna liczbe argumentow */
    if (argc != 2)
    {
        fprintf(stderr, "Uzycie: entropy <plik>\n");
        return 1;
    }

    /* Otwarcie pliku w trybie binarnym */
    plik = fopen(argv[1], "rb"); // rb bo pracujemy na bajtach

    if (plik == NULL)
    {
        fprintf(stderr, "Blad: nie mozna otworzyc pliku.\n");
        return 1;
    }

    /* Odczyt pliku bajt po bajcie */
    while ((bajt = fgetc(plik)) != EOF) // jedziemy do konca pliku
    {
    	// kiedy funkcja fgetc(plik) wczytac z pliku np litera a zwraca
    	// jej kod liczbowy a potem zwiekszamy licznik dla tej wartosci w tablicy
        licznik[(unsigned char)bajt]++;
        n++; // zwiekszamy calkowita liczbe bajtow
    }

    /* Sprawdzenie bledu odczytu */
    if (ferror(plik))
    {
        fprintf(stderr, "Blad podczas odczytu pliku.\n");
        fclose(plik);
        return 1;
    }

    fclose(plik);

    /* Liczenie roznych wartosci bajtow */
    // A A A B B C rozne wartosci bajtow = 3
	int i;
	
	for (i = 0; i < 256; i++)
    {
        if (licznik[i] > 0)
        {
            rozne++;
        }
    }

    /* Obliczanie entropii */
    if (n > 0)
    {
	int i;
	
	for (i = 0; i < 256; i++)
        {
            if (licznik[i] > 0)
            {
            	// obliczanie prawdopodobienstwa wystapienia danego bajtu
                double p = (double)licznik[i] / (double)n;
                // wzor matematyczny zawiera minus przed suma
                H -= p * log2(p);
            }
        }
    }

    /* Teoretyczny minimalny rozmiar w bajtach */
    // dziliemy przez 8 poniewaz jeden bajt ma 8 bitow 
	// a antropia jest wyrazona w bitach na symbol
    minimalny_rozmiar = (double)n * H / 8.0;

    /* Wyswietlenie wynikow */
	printf("Rozmiar pliku: %u bajtow\n", n);
    printf("Liczba roznych wartosci bajtow: %d\n", rozne);
    printf("Entropia: %.6f bit/symbol\n", H);
    printf("Teoretyczny minimalny rozmiar: %.6f bajtow\n", minimalny_rozmiar);

    return 0;
}

