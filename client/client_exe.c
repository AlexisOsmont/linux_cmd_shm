#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FINISH  "exit"
#define START   "\nVous pouvez entrer vos commandes (exit pour quitter): "
#define CMD   "\nCommande : "

#define FUN_SUCCESS      0
#define FUN_FAILURE     -1


#define BUFFER_SIZE   128

int main(void) {

  char buffer[BUFFER_SIZE];
  printf(START);
  while (fgets(buffer, BUFFER_SIZE, stdin) != NULL
    && strncmp(buffer, FINISH, strlen(FINISH)) != 0) {
    buffer[strlen(buffer) - 1] = '\0';
    printf("%s%s\n", CMD, buffer);
  }
  return EXIT_SUCCESS;
}