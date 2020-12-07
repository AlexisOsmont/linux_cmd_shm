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
#include <mqueue.h>


#define SHM_LENGTH       100

#define SHM_FLAG         0 // Le shm est initialisée
#define COMMANDE_FLAG    1 // Le client a entré la commande
#define RESULT_FLAG      2 // Le client peut récupérer le resultat

#define SHM_NAME     "/mon_shm"  
#define QUEUE_NAME   "/ma_file2"

#define SYNC "SYNC"
#define END  "END"
#define RST  "RST"
#define TUBE_CLIENT  "mon_tube_client_"
#define TUBE_DEMON  "mon_tube_demonn_"


#define SHM_SIZE      100

#define FUN_SUCCESS   0
#define FUN_FAILURE   -1

#define BUFFER_SIZE   256
#define TUBE_NAME_LENGTH 128

#define START   "\nVous pouvez entrer vos commandes (exit pour quitter):\n"
#define FINISH  "exit"

static int fd_demon;


// ---------------------- main -----------------------------

int main(void) {
  // creation tube client pour recup le resultat de l'exec de commande--------------------
  char tube_client[TUBE_NAME_LENGTH];
  pid_t pid = getpid();
  printf("A1\n");
  sprintf(tube_client, "%s%d", TUBE_CLIENT, pid);
  if (mkfifo(tube_client, S_IRUSR | S_IWUSR) == -1) {
    perror("client: Impossible de créer le tube du client");
    exit(EXIT_FAILURE);
  }
  printf("B1\n");
  // ouverture du tube demon pour transmettre le pid du client a la foction d'execution---
  // pour pouvoir ouvrir le tube du client
  if ((fd_demon = open(TUBE_DEMON, O_WRONLY)) == -1) {
    perror("client_exe: Impossible d'ouvrir le tube du demon");
    exit(EXIT_FAILURE);
  }
  printf("C1\n");
  // entre le pid du porcessus client dans le tube demon pour le donner a l'exec
  if (start_end_connection(fd_demon, (size_t) pid) == FUN_FAILURE) {
    exit(EXIT_FAILURE);
  }
  printf("D1\n");
  // ouverture de la file (mqueu) pour y mettre les commandes-----------------------
  char buffer[BUFFER_SIZE];
  printf(START);
  int shm_fd;
  mqd_t mqd = mq_open(QUEUE_NAME, O_RDWR, S_IRUSR | S_IWUSR);
  if (mqd == (mqd_t) -1) {
    perror("mq_open client.c :");
    exit(EXIT_FAILURE);
  }
  printf("E1\n");
  // ouverture du SHM pour y mettre les commandes de la queue -----------------
  if ((shm_fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
    perror("send_command_to_thread: Impossible d'ouvrir le shm");
    return FUN_FAILURE;
  }
  printf("F1\n");
  if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  printf("G1\n");
  //envoie des commandes eet des requetes dans la queue ---------------------
   while (fgets(buffer, BUFFER_SIZE, stdin) != NULL
      && strncmp(buffer, FINISH, strlen(FINISH)) != 0) {
        printf("A\n");
    buffer[strlen(buffer) - 1] = '\0';
    if (send_command_to_thread(shm_fd, mqd, buffer,
        (size_t) SHM_SIZE) == FUN_FAILURE) {
      exit(EXIT_FAILURE);
    }
    if (thread_answer(atoi(tube_client)) == FUN_FAILURE) {
      exit(EXIT_FAILURE);
    }
  }
  printf("Z1\n");
  if (close(shm_fd) == -1) {
    perror("send_command_to_thread: Impossible de fermer le shm");
    return FUN_FAILURE;
  }
  return EXIT_SUCCESS;
}

//  --------------------- fonctions utiles ---------------------------

int send_command_to_thread(int shm_fd, mqd_t mqd,  char *command, size_t shm_size) {
  printf("H1\n");
  if (ftruncate(shm_fd, (long int) shm_size) == -1) {
    perror("send_command_to_thread: Impossible de projéter le shm");
    return FUN_FAILURE;
  }
  printf("I1\n");
  void *ptr
    = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("send_command_to_thread: Impossible de fixer la taille de shm");
    return FUN_FAILURE;
  }
  printf("J1\n");
  char *cmdptr = (char *) ptr + sizeof(int);
  char *endptr = cmdptr + strlen(command);
  // Copie de la commande a exécuter dans la shm
  // On exclu 4 octets à cause du int indiquant la nature de la donnée
  strncpy(cmdptr, command, shm_size - sizeof(int));
  if (mq_send(mqd, command, 128, 0) == -1) {
    perror("mq_send error");
    exit(EXIT_FAILURE);
  }
  printf("K1\n");
  // On indique la fin de la donnée
  *endptr = '\0';
  volatile int *flag = (int *) ptr;
  *flag = COMMANDE_FLAG;
  return FUN_SUCCESS;
}

//  Renvoie le nombre des chiffres d'un naturel.
static int digit__count(int n) {
  if (n < 10) {
    return 1;
  }
  return 1 + digit__count(n / 10);
}

int start_end_connection(int fd_tube, size_t label) {
  printf("start end 1\n");
  char msg1[digit__count((int) label) + 1];
  sprintf(msg1, "%zu", label);
  printf("start end 2\n");
  if (write(fd_tube, msg1, sizeof msg1) == -1) {
    perror("start_end_connection: Impossible d'écrire dans le tude du demon");
    return FUN_FAILURE;
  }
  printf("start end 3\n");
  return FUN_SUCCESS;
}

int thread_answer(int fd_client) {
  printf("L1\n");
  char answer[TUBE_NAME_LENGTH];
  if (read(fd_client, answer, sizeof answer) == -1) {
    perror("demon_answer: Impossible de lire dans le tube du client");
    return FUN_FAILURE;
  }
  printf("M1\n");
  char size[SHM_SIZE];
  int i = 0;
  while (answer[i] <= '9') {
    size[i] = answer[i];
    ++i;
  }
  printf("N1\n");
  size[i] = '\0';
  int shm_size = atoi(size);
  if (shm_size == 0) {
    perror("demon_answer: Invalid taille de shm");
    return FUN_FAILURE;
  }
  printf("O1\n");
  if (close(fd_client) == -1) {
    perror("demon_answer: Impossible de fermer le tube de client");
    return FUN_FAILURE;
  }
  printf("P1\n");
  return shm_size;
}
