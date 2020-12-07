//  Partie implantation du module thread_pool

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <mqueue.h>

#include "thread_pool.h"

#define SHM_NAME     "/mon_shm"  
#define QUEUE_NAME   "/ma_file2"

#define TUBE_CLIENT  "mon_tube_client_"
#define TUBE_DEMON  "mon_tube_demonn_"


#define TUBE_NAME_LENGTH 128

#define ARG_SEPARATOR " "
#define MAX_ARG_COUNT 200
#define MAX_ARG_LENGTH 200

#define FUN_SUCCESS      0
#define FUN_FAILURE    -1


#define SHM_FLAG         0 // Le shm est initialisée
#define COMMANDE_FLAG    1 // Le client a entré la commande
#define RESULT_FLAG      2 // Le client peut récupérer le resultat

#define SHM_LENGTH       32

// Structure qui permet de stocker un thread avec les informations nécessaires.
typedef struct thread_one {
  pthread_t thread;
  pthread_mutex_t mutex;
  size_t connection;
  size_t shm_size;
  void *data;
  volatile bool end;
} thread_one;

// Structure principale stockant les threads
struct threads {
  thread_one **array;
  size_t count;
  size_t min_thread;
  size_t max_thread;
  size_t shm_size;
};

//  ini__thread_one_element: fonction statique qui permet d'allouer et
//  initialiser les champs de la structure thread_one. thread_one sera un
//  élément de la structure threads. Renvoie NULL en cas de dépassement de
// capacité ou d'erreur, un pointeur vers l'objet en cas de succès.
static thread_one *ini__thread_one_element(size_t shm_size);

//  waiting_command: fonction statique exécuté par chaque thread les mettant
//  en phase d'attente pour une écriture dans la shm. Renvoie NULL toujours.
static void *waiting_command(void *arg);

//  fork_thread: fonction statique qui permet à un thread d'exécuter une
//  commande lu dans sa shm. L'exécution est un appel à execv dans un fork et
//  le résultat est stocké dans le paramètre command. Renvoie 0 en cas de
//  succès, -1 en cas d'échec.
static int fork_thread(char *command, size_t size);

threads *ini_threads(size_t min_thread, size_t max_thread,
    size_t shm_size) {
  // creation du tube demon pour recup le pid du client
  if (mkfifo(TUBE_DEMON, S_IRUSR | S_IWUSR) == -1) {
    perror("connected_to_thread: Impossible de créer le tube du demon");
    return NULL;
  }
  threads *ptr = malloc(sizeof(threads));
  if (ptr == NULL) {
    return NULL;
  }
  ptr->count = min_thread;
  ptr->min_thread = min_thread;
  ptr->max_thread = max_thread;
  ptr->shm_size = shm_size;
  ptr->array = calloc(1, sizeof(thread_one *) * max_thread);
  if (ptr->array == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < min_thread; ++i) {
    ptr->array[i] = ini__thread_one_element(shm_size);
    if (ptr->array[i] == NULL) {
      return NULL;
    }
  }
  return ptr;
}

int connected_thread_to(threads *th) {
  for (size_t i = 0; i < th->count; ++i) {
    // On verifie s'il y a des threads libres
    if (th->array[i] != NULL
        && pthread_mutex_trylock(&(th->array[i]->mutex)) == 0) {
      // Un thread a été trouvé
      return (int) i;
    }
  }
  // Pas de thread disponible, on regarde si on a atteint le max
  if (th->count < th->max_thread) {
    // Si on a de place, on créé un nouveau thread
    th->array[th->count] = ini__thread_one_element(th->shm_size);
    ++th->count;
    // On renvoie le nouveau thread
    pthread_mutex_lock(&(th->array[th->count - 1]->mutex));
    return (int) th->count - 1;
  }
  return FUN_FAILURE;
}

int used_thread(threads *th, size_t number) {
  // On regarde si le thread à atteint sa fin de vie
  if (th->array[number]->connection == 1) {
    // On indique au thread qu'il s'arrête totalement
    th->array[number]->end = true;
    if (pthread_join(th->array[number]->thread, NULL) != 0) {
      perror("used_thread: pthread_join");
      return FUN_FAILURE;
    }
    // Libération de la shm
    if (shm_unlink(SHM_NAME) == -1) {
      perror("used_thread: shm_unlink");
      return FUN_FAILURE;
    }
    free(th->array[number]);
    th->array[number] = NULL;
    if (th->count == th->min_thread) {
      // Réinitialisation du thread
      th->array[number] = ini__thread_one_element(th->shm_size);
      return 1;
    }
    // On renvoit une autre valeur pour prévenir un changement d'état
    return 2;
  }
  if (th->array[number]->connection > 1) {
    th->array[number]->connection -= 1;
  }
  pthread_mutex_unlock(&(th->array[number]->mutex));
  return FUN_SUCCESS;
}

void thread_dispose(threads *th) {
  for (size_t i = 0; i < th->count; ++i) {
    if (th->array[i] != NULL) {
      // On arrête le thread
      th->array[i]->end = true;
      pthread_join(th->array[i]->thread, NULL);
      free(th->array[i]);
      th->array[i] = NULL;
    }
  } 
  free(th->array);
  free(th);
  th = NULL;
}

thread_one *ini__thread_one_element(size_t shm_size) {
  thread_one *ptr = malloc(sizeof(thread_one));
  if (ptr == NULL) {
    return NULL;
  }
  if (pthread_mutex_init(&(ptr->mutex), NULL) != 0) {
    perror("ini__thread_one_element: pthread_mutex_init");
    return NULL;
  } 
  ptr->shm_size = shm_size;
 mqd_t mqd = mq_open(QUEUE_NAME, O_RDWR, S_IRUSR | S_IWUSR);
  if (mqd == (mqd_t) -1) {
    perror("mq_open ini_thread_one.c ");
    exit(EXIT_FAILURE);
  }
   if (mq_receive(mqd ,ptr->data ,128 , 0) == -1) {
      perror("mq_send: impossible d'ajouter une commande");
      return NULL;
  }
  if (ptr->data == MAP_FAILED) {
    perror("ini__thread_one_element: mmap");
    return NULL;
  }
  if (close(mqd) == (mqd_t)-1) {
    perror("ini__thread_one_element: close shm");
    return NULL;
  }
  ptr->end = false;
  int *flag = (int *) ptr->data;
  *flag = SHM_FLAG;
  pthread_create(&(ptr->thread), NULL, waiting_command, ptr);
  return ptr;
}

void *waiting_command(void *arg) {
  thread_one *ptr = (thread_one *) arg;
  while (!ptr->end) {
    // Lecture d'une commande
    volatile int *flag = (volatile int *) ptr->data;
    char *data = (char *) ptr->data + sizeof *flag;
    // En attente d'une ressource destiné au thread ou d'une demande
    //  d'arrêt immédiat.
    while (*flag != COMMANDE_FLAG && !ptr->end) {
    }
    // Si c'est un arrêt immédiat, on s'arrête.
    if (ptr->end) {
      return NULL;
    }
    // Execution de la commande, on enleve 4 octets car il faut compter
    // la taille du flag dans la shm
    if (fork_thread(data, ptr->shm_size - sizeof(int))
        == FUN_FAILURE) {
      return NULL;
    }
    *flag = RESULT_FLAG;
  }
  return NULL;
}

int fork_thread(char *command, size_t size) {
  char tube_client[TUBE_NAME_LENGTH];
  int fd_demon;
  if ((fd_demon = open(TUBE_DEMON, O_RDONLY)) == -1) {
    perror("thread pool fork thread: Impossible d'ouvrir le tube du demon");
    return FUN_FAILURE;
  }
  char clientpid[SHM_LENGTH];
  if (read(fd_demon, clientpid, sizeof clientpid) == -1) {
    perror("thread_pool fork_thread: Impossible de lire dans le tube du demon");
    return FUN_FAILURE;
  }
  // On enlève 1 de size pour pouvoir insérer le caractère de fin de
  // chaine dans le cas ou toute la taille est utilisée.
  size = size - 1;
  // Split des arguments dans un tableau de tableau de caractère
  char *arr[MAX_ARG_COUNT];
  size_t i = 0;
  char *token = strtok(command, ARG_SEPARATOR);
  while (token != NULL) {
    arr[i] = token;
    ++i;
    token = strtok(NULL, ARG_SEPARATOR);
  }
  arr[i] = NULL;
  sprintf(tube_client, "%s%d", TUBE_CLIENT, atoi(clientpid));
  int fd = open(tube_client, O_WRONLY);
  ssize_t r;
  switch (fork()) {
    case -1:
      perror("fork_thread: fork");
      return FUN_FAILURE;
    case 0:
      execv(arr[0], arr);
      printf("Votre commande est invalide.\n");
      exit(EXIT_SUCCESS);
      break;
    default:
      r = 0;
      // reponse dans le tube du client
      if ((r = write(fd, command, size)) == -1) {
        perror("fork_thread: read");
        return FUN_SUCCESS;
      }
      command[r] = '\0';
      break;
  }
  return FUN_SUCCESS;
}
