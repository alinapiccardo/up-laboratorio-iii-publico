#include <ctype.h>
#include <stdio.h>
#include <string.h>

int contar_vocales(const char *s) {
  int i;
  int cuenta = 0;
  char c;

  for (i = 0; s[i] != '\0'; i++) {
    c = tolower(s[i]);
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
      cuenta++;
    }
  }
  return cuenta;
}

int es_palindromo(const char *s) {
  int i;
  int j;

  i = 0;
  j = strlen(s) - 1;
  while (i < j) {
    while (i < j && s[i] == ' ') {
      i++;
    }
    while (i < j && s[j] == ' ') {
      j--;
    }
    if (tolower(s[i]) != tolower(s[j])) {
      return 0;
    }
    i++;
    j--;
  }
  return 1;
}

void invertir_cadena(char *s) {
  int i;
  int j;
  char temp;

  i = 0;
  j = strlen(s) - 1;
  while (i < j) {
    temp = s[i];
    s[i] = s[j];
    s[j] = temp;
    i++;
    j--;
  }
}

int contar_palabras(const char *s) {
  int i;
  int cuenta = 0;
  int dentro_de_palabra = 0;

  for (i = 0; s[i] != '\0'; i++) {
    if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
      dentro_de_palabra = 0;
    } else if (!dentro_de_palabra) {
      dentro_de_palabra = 1;
      cuenta++;
    }
  }
  return cuenta;
}

void a_mayusculas(char *s) {
  int i;

  for (i = 0; s[i] != '\0'; i++) {
    s[i] = toupper(s[i]);
  }
}

int main(void) {
  char texto[] = "anita lava la tina";
  char copia[100];

  strcpy(copia, texto);

  printf("Texto: \"%s\"\n", texto);
  printf("Vocales: %d\n", contar_vocales(texto));
  printf("Palabras: %d\n", contar_palabras(texto));
  printf("Es palindromo: %s\n", es_palindromo(texto) ? "si" : "no");

  a_mayusculas(copia);
  printf("En mayusculas: \"%s\"\n", copia);

  invertir_cadena(copia);
  printf("Invertido: \"%s\"\n", copia);

  return 0;
}
