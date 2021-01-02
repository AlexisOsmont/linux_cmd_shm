#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#include <unistd.h>

#define TUBE_CLIENT "mon_tube_"

#define FUN_SUCCESS 0
#define FUN_FAILURE -1

typedef struct thread_one
{
  pthread_t thread;
  pthread_mutex_t mutex; //je ne pense pas en avoir besoin
  size_t connection;
  void *data;
  volatile bool end;
} thread_one;

struct threads
{
  thread_one **array;
  size_t count;
  size_t max_thread;
};

/**
 * Crée un thread en lui envoyant une commande
 */
int thread_create(char *commande);

void *split_func(void *commande);

int fork_thread(char *pid, char *cmd, char *args[]);
