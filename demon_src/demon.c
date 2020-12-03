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


#define SHM_NAME     "/mon_shm"  
#define QUEUE_NAME   "/ma_file17"

#define SHM_LENGTH      100

#define FUN_SUCCESS   0
#define FUN_FAILURE   -1

#define BUFFER_SIZE   128


int main(void) {
  struct mq_attr attr;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = 128;
  mqd_t mqd = mq_open(QUEUE_NAME , O_RDWR | O_CREAT | O_EXCL ,S_IRUSR | S_IWUSR, &attr);
  if (mqd == (mqd_t) -1) {
    perror("mq_open demon.c");
    exit(EXIT_FAILURE);
  }
 int shm_fd = shm_open(SHM_NAME, O_RDWR |O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
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
  int *i = (int *)(ptr);
  char *result = (char *) ptr + sizeof(int);
  *i = 0;
  char msg[SHM_LENGTH];
  printf("-------------------------\nMessages envoyés par le client :\n");
  while (strcmp(result,"stop") != 0) {
        while (*i == 0);
        printf("%s\n", result);
		*i = 0;    
  }
  printf("-------------------------\nMessages dans la queue :\n");
  while (strcmp(msg,"stop")) {
    if (mq_receive(mqd ,msg ,128 , 0) == -1) {
      perror("mq_send: impossible d'ajouter une commande");
      return FUN_FAILURE;
    }
   printf("%s\n", msg);  
  }
  if (close(shm_fd) == -1) {
    perror("send_command_to_thread: Impossible de fermer le shm");
    return FUN_FAILURE;
  }
  if(mq_close(mqd) == (mqd_t) -1) {
    perror("mq_close: ");
    return FUN_FAILURE;
  }
  printf("probleme ici : \n");  
    return 0;
}
