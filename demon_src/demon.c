#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include  <sys/shm.h>

#define SHM_NAME     "/mopn_shm"  

#define SHM_FLAG         0

#define SHM_LENGTH      100

#define FUN_SUCCESS   0
#define FUN_FAILURE   -1

#define BUFFER_SIZE   128


int main(void) {
 int shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, (long int) SHM_LENGTH) == -1) {
    perror("send_command_to_thread: Impossible de projéter le shm");
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
   printf("demon = %s%s\n",result,(char *)ptr);
    *flag = SHM_FLAG;
   if (close(shm_fd) == -1) {
    perror("send_command_to_thread: Impossible de fermer le shm");
    return FUN_FAILURE;
  }
    return 0;
}