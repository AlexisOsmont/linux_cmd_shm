#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include  <sys/shm.h> 
#include <mqueue.h>
#include "thread_pool/thread_pool.h"
#include "thread_pool/thread_pool.c"
#include "demon.h"
 
#define SHM_NAME     "/mon_shm"
#define QUEUE_NAME   "/ma_file2"

#define TUBE_CLIENT  "mon_tube_client_"
#define TUBE_DEMON  "mon_tube_demonn_"

#define FUN_SUCCESS   0
#define FUN_FAILURE   -1

#define BUFFER_SIZE   128

#define MIN_THREAD  2
#define MAX_THREAD  5
#define SHM_SIZE  1024


// Variable global de la structe du connected_thread de thread, elle est global
// pour qu'elle puisse être manipuler après un signal.
static threads *connected_thread;

//    dispose: libère proprement les ressources utilisées (données système et
// mémoire) et gère correctement les demandes de terminaisons via des signaux.
void dispose(int sig);


typedef struct info {
  size_t min_thread;
  size_t max_thread;
  size_t max_connected_thread;
  size_t shm_size;
} info;

int main(void) {
  printf("B\n");
  info *info_struct = malloc(sizeof(info));
  if (info_struct == NULL) {
    perror("demon: Erreur d'allocation pour info_struct");
    return EXIT_FAILURE;
  }
  printf("C\n");
  info_struct->min_thread = MIN_THREAD;
  info_struct->max_thread = MAX_THREAD;
  info_struct->shm_size = SHM_SIZE;
  struct mq_attr attr;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = 128;
  printf("D\n");
  mqd_t mqd = mq_open(QUEUE_NAME , O_RDWR | O_CREAT | O_EXCL ,S_IRUSR | S_IWUSR, &attr);
  if (mqd == (mqd_t) -1) {
    perror("mq_open demon.c");
    exit(EXIT_FAILURE);
  }
  printf("E\n");
  connected_thread = ini_threads(info_struct->min_thread,
          info_struct->max_thread, info_struct->shm_size);
  if (connected_thread == NULL) {
    free(info_struct);
    return EXIT_FAILURE;
  }
  printf("F\n");
  signal(SIGINT, dispose);
  signal(SIGTERM, dispose);
  if (connected_to_thread(info_struct->shm_size) == -1) {
    free(info_struct);
    return EXIT_FAILURE; 
  }
  printf("Z\n");
  free(info_struct);
  dispose(0);
  return EXIT_SUCCESS;
}

int connected_to_thread(size_t shm_size) {
  printf("G\n");
 int shm_fd = shm_open(SHM_NAME, O_RDWR |O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  printf("H\n");
  if (ftruncate(shm_fd, (long int) shm_size) == -1) {
    perror("send_command_to_thread: Impossible de projéter le shm");
    return FUN_FAILURE;
  }
  printf("I\n");
  void *ptr
    = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("receive_result_from_thread: Impossible de fixer la taille de shm");
    return FUN_FAILURE;
  }
  printf("J\n");
  int *i = (int *)(ptr);
  char *result = (char *) ptr + sizeof(int);
  *i = 0;
  char msg[shm_size];
  printf("-------------------------\nMessages envoyés par le client :\n");
   mqd_t mqd = mq_open(QUEUE_NAME, O_RDWR, S_IRUSR | S_IWUSR);
printf("K\n");
  while (strcmp(result,"stop") != 0) {
      while (*i == 0);
      if (mq_receive(mqd ,msg ,128 , 0) == -1) {
        perror("mq_send: impossible d'ajouter une commande");
        return FUN_FAILURE;
      }
      printf("%s\n", msg);
		*i = 0;    
  }
  printf("L\n");
  if (close(shm_fd) == -1) {
    perror("send_command_to_thread: Impossible de fermer le shm");
    return FUN_FAILURE;
  }
  if(mq_close(mqd) == (mqd_t) -1) {
    perror("mq_close: ");
    return FUN_FAILURE;
  }
  printf("M\n");
  printf("probleme ici : \n");  
    return 0;
}

void dispose(int sig) {
  if (connected_thread != NULL) {
    thread_dispose(connected_thread);
  }
  sig = 0;
  int i = 0;
  char shm_name[SHM_LENGTH];
  do {
    sprintf(shm_name, "%s%d", SHM_NAME, i);
    ++i;
    sig = shm_unlink(shm_name);
  } while (sig != FUN_FAILURE);
  exit(EXIT_SUCCESS);
}
