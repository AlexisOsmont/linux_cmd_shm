#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#include "../file_src/file.h"
#include "../thread_pool/thread_pool.h"

#define BUFFER_SIZE 20
#define FINISH "exit"


int get_pid(char *commande);

void dispose(threads *th);

int main(void) {
  int shm_fd = shm_open(NOM_SHM, O_RDWR |O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, (long int) 1000) == -1) {
    perror("send_command_to_thread: Impossible de projéter le shm");
    return -1;
  }
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("sem_open");
    dispose(th);
    exit(EXIT_FAILURE);
  }
  file* file_p = create_shm_file(shm_ptr);
  threads *th = thread_ini();
  int pid;
  char *req;
  int thc_return;
  while (1) {
    req = defiler(file_p);
    if (strstr(req,"close_demon") != NULL) {
      break;
    }
    printf("requete defilée dans demon.c : %s\n", req);
    thc_return = thread_create(th, req);
    if (thc_return == -1) {
      dispose(th);
       return EXIT_FAILURE;
    } else if (thc_return == -2) {
      enfiler("0",file_p);
      while ((pid = get_pid(defiler(file_p))) != 0) {
          kill((pid_t)pid, SIGUSR1);
      }
    }
  }
  dispose(th);
  return EXIT_SUCCESS;
}

int get_pid(char *commande) {
  char cmd[strlen(commande)];
    char *com = (char*)commande;
    size_t l = (size_t)com[0];
    char *pid = malloc(l);
    strcpy(cmd, (char *)commande);
    int i;
    printf("split func/cmd : %s \nsplit func/pid :", cmd);
    for (i = 1; i <= cmd[0] - '0'; i++) {
            pid[i - 1] = cmd[i];
            printf("%c", pid[i - 1]);
    }
  return atoi(pid);
}

void dispose(threads *th) {
  if (shm_unlink(NOM_SHM) == -1) {
		perror("shm_unlink");
		exit(EXIT_FAILURE);
  }
  free(th); 
}