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
    exit(EXIT_FAILURE);
  }
  file* file_p = create_shm_file(shm_ptr);

  while (1) {
    char *req = defiler(file_p);
    printf("requete defilée dans demon.c : %s\n", req);
    if (thread_create(req) == -1) {
       return EXIT_FAILURE;
    }
  }
  if (shm_unlink(NOM_SHM) == -1) {
		perror("shm_unlink");
		exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
