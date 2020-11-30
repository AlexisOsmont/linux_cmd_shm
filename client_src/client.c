#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "client.h"
#include  <sys/shm.h>

#define SHM_LENGTH       100

#define SHM_FLAG         0 // Le shm est initialisée
#define COMMANDE_FLAG    1 // Le client a entré la commande
#define RESULT_FLAG      2 // Le client peut récupérer le resultat

#define SHM_NAME     "/mopn_shm"  

#define SHM_SIZE      100

#define FUN_SUCCESS   0
#define FUN_FAILURE   -1

#define BUFFER_SIZE   128

#define START   "\nVous pouvez entrer votre commande (exit pour quitter): "
#define FINISH  "exit"

// ---------------------- main -----------------------------

int main(void) {
  char buffer[BUFFER_SIZE];
   while (fgets(buffer, BUFFER_SIZE, stdin) != NULL
      && strncmp(buffer, FINISH, strlen(FINISH)) != 0) {
    buffer[strlen(buffer) - 1] = '\0';
    if (send_command_to_thread(SHM_NAME, buffer,
        (size_t) SHM_SIZE) == FUN_FAILURE) {
      exit(EXIT_FAILURE);
    }
    printf(START);
  }
  return EXIT_SUCCESS;
}

//  --------------------- fonctions utiles ---------------------------



int send_command_to_thread(char *shm_name, char *command, size_t shm_size) {
  int shm_fd;
  if ((shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
    perror("send_command_to_thread: Impossible d'ouvrir le shm");
    return FUN_FAILURE;
  }
    if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, (long int) shm_size) == -1) {
    perror("send_command_to_thread: Impossible de projéter le shm");
    return FUN_FAILURE;
  }
  void *ptr
    = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("send_command_to_thread: Impossible de fixer la taille de shm");
    return FUN_FAILURE;
  }
  char *cmdptr = (char *) ptr + sizeof(int);
  char *endptr = cmdptr + strlen(command);
  // Copie de la commande a exécuter dans la shm
  // On exclu 4 octets à cause du int indiquant la nature de la donnée
  strncpy(cmdptr, command, shm_size - sizeof(int));
  // On indique la fin de la donnée
  *endptr = '\0';
  volatile int *flag = (int *) ptr;
  *flag = COMMANDE_FLAG;
  if (close(shm_fd) == -1) {
    perror("send_command_to_thread: Impossible de fermer le shm");
    return FUN_FAILURE;
  }
  return FUN_SUCCESS;
}

int receive_result_from_thread(char *shm_name, size_t shm_size) {
  int shm_fd;
  if ((shm_fd = shm_open(shm_name, O_RDWR | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
    perror("receive_result_from_thread: Impossible d'ouvrir le shm");
    return FUN_FAILURE;
  }
  if (ftruncate(shm_fd, (long int) shm_size) == -1) {
    perror("receive_result_from_thread: Impossible de projéter le shm");
    return FUN_FAILURE;
  }
  void *ptr
    = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("receive_result_from_thread: Impossible de fixer la taille de shm");
    return FUN_FAILURE;
  }
  volatile int *flag = (int *) ptr;
  while (*flag != RESULT_FLAG) {
  }
  char *result = (char *) ptr + sizeof(int);
  size_t i = 0;
  while (result[i] != '\0') {
    putchar(result[i]);
    ++i;
  }
  *flag = SHM_FLAG;
  result[0] = '\0';
  if (close(shm_fd) == -1) {
    perror("receive_result_from_thread: Impossible de fermer le shm");
    return FUN_FAILURE;
  }
  return FUN_SUCCESS;
}
