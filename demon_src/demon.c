#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define SHM_NAME     "/mon_shm"  

#define SHM_LENGTH      30

#define FUN_SUCCESS   0
#define FUN_FAILURE   -1

#define BUFFER_SIZE   128


int main(void) {
      int shm_fd;
if ((shm_fd = shm_open(SHM_NAME, O_RDWR | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
    perror("receive_result_from_thread: Impossible d'ouvrir le shm");
    return FUN_FAILURE;
  }
  if (ftruncate(shm_fd, (long int) SHM_LENGTH) == -1) {
    perror("receive_result_from_thread: Impossible de projéter le shm");
    return FUN_FAILURE;
  }
  void *ptr
    = mmap(NULL, SHM_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("receive_result_from_thread: Impossible de fixer la taille de shm");
    return FUN_FAILURE;
  }
  volatile int *flag = (int *) ptr;
  while (*flag != 2) {
  }
  char *result = (char *) ptr + sizeof(int);
  size_t i = 0;
  while (result[i] != '\0') {
    putchar(result[i]);
    ++i;
  }
    return 0;
}