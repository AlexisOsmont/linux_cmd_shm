#include "file.h"

//~ int main(void) {
//~ int shm_fd = shm_open(NOM_SHM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  //~ if (shm_fd == -1) {
    //~ perror("shm_open");
    //~ exit(EXIT_SUCCESS);
  //~ }

  //~ if (shm_unlink(NOM_SHM) == -1) {
    //~ perror("shm_unlink");
    //~ exit(EXIT_FAILURE);
  //~ }

  //~ if (ftruncate(shm_fd, TAILLE_SHM) == -1) {
    //~ perror("ftruncate");
    //~ exit(EXIT_FAILURE);
  //~ }
  //~ create_shm_file(shm_fd);
  //~ char s_stdin[100];
  //~ fgets(s_stdin, 100, stdin);
  //~ printf("s_stdin : %s\n", s_stdin);
  //~ printf("Affichage de la longueur de strlen(stdin) : %ld\n", strlen(s_stdin));
  //~ char *req;
  //~ enfiler(s_stdin);
  //~ req = defiler();
  //~ printf("Requete : %s\n", req);
  //~ destroy_semaphore();
  //~ return EXIT_SUCCESS;
//~ }

file *create_shm_file(char* shm_ptr) {
  file *file_p;
  file_p = (file*) shm_ptr;

  // Initialisation des variables
  if (sem_init(&file_p->mutex, 1, 1) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&file_p->vide, 1, N) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&file_p->plein, 1, 0) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  file_p->tete = 0;
  file_p->queue = 0;
  return file_p;
}

void destroy_semaphore(file *file_p) {
  if (sem_destroy(&file_p->mutex) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (sem_destroy(&file_p->plein) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (sem_destroy(&file_p->vide) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
}

void enfiler(char *command, file* file_p) {
	printf("\ncommande envoyé par le client : %s\n",command);
	if (sem_wait(&file_p->vide) == -1) {
		perror("producteur : sem_wait(vide)");
		exit(EXIT_FAILURE);
	}
	if (sem_wait(&file_p->mutex) == -1) {
		perror("producteur : sem_wait(mutex)");
		exit(EXIT_FAILURE);
	}
	  //écrit dans la shm
	  //printf("J'écris dans la shm\n");
  
	if (strcpy(&file_p->buffer[file_p->tete], command) == NULL) {
		fprintf(stderr, "Probleme strcopy dans la shm");
		exit(EXIT_FAILURE);
	} 

    //MODIFICATION DU POINTEUR DE TETE DE LA TAILLE DES DONNÉES ENTREE
    file_p->tete = (file_p->tete + (int)strlen(command)) % N;
	if (sem_post(&file_p->mutex) == -1) {
		perror("enfiler sem_post(mutex)");
		exit(EXIT_FAILURE);
	}
	if (sem_post(&file_p->plein) == -1) {
		perror("enfiler sem_post(plein)");
		exit(EXIT_FAILURE);
	}
}

char *defiler(file* file_p) {
  //printf("J'accède à défiler\n");
  if (sem_wait(&file_p->plein) == -1) {
    perror("defiler : sem_wait(plein)");
    exit(EXIT_FAILURE);
  }
  if (sem_wait(&file_p->mutex) == -1) {
    perror("defiler : sem_wait(mutex)");
    exit(EXIT_FAILURE);
  }

  // Récupération de la longueur du Pid
  char *lengthOfPid = malloc(1); // Car 1 octet pour la taille du pid
  if (strncpy(lengthOfPid, &file_p->buffer[file_p->queue], 1) == NULL) {
    fprintf(stderr, "defiler strncpy pid problem");
    exit(EXIT_FAILURE);
  }
  int lengthPid = atoi(lengthOfPid);
  free(lengthOfPid);


  // Récupération longueur de la requête
  //printf("Affichage du caratère : %c\n", file_p->buffer[file_p->queue]);
  //printf("Affichage de l'entier : %d\n", file_p->buffer[file_p->queue]);
  int lengthRequest = atoi(&file_p->buffer[file_p->queue] + lengthPid + 1); //+1 car stockage longueur pid
  //printf("lengthRequest : %d\n", lengthRequest); 
  int taille_lengthRequest = 0;
  if (lengthRequest <= 9) {
    taille_lengthRequest = 1;
  } else if (lengthRequest > 9 && lengthRequest <= 99) {
    taille_lengthRequest = 2;
  } else {
    taille_lengthRequest = 3;
  }
 
  //printf("Chaine de caractère lengthOfRequest : %s\n", lengthOfRequest);
  //printf("Taille de lengthOfRequest : %ld\n", strlen(lengthOfRequest));
  //printf("lengthOfRequest : %d\n", atoi(lengthOfRequest)); 
  //printf("Affichage lengthRequest :  %d\n",  lengthRequest);
  
  //Récupération de la requête
  //printf("Affichage Requête :\n"); 
  char *request  = malloc((size_t)lengthRequest);
  printf("Affichage longueur de la requete : %d\n", lengthRequest);
  printf("Longueur du pid : %d\n", lengthPid);
  printf("Taille du nbr représentant la taille de la requete : %d\n", taille_lengthRequest);
  if (strncpy(request, &file_p->buffer[file_p->queue], (size_t)lengthRequest 
  + (size_t)lengthPid + /*strlen(lengthOfRequest) +*/ 1 + (size_t)taille_lengthRequest) == NULL) { //+2 car stockage longueur pid et requete
    fprintf(stderr, "problem pour la récupération de la requête");
    exit(EXIT_FAILURE);
  }
  printf("requete dans defiler file_p->buff : %s\n", &file_p->buffer[file_p->queue]);

  // Déplacement du pointeur de queue de la file de la taille de la requête
  int decalage = lengthPid + lengthRequest + 1 + taille_lengthRequest;
  printf("Le décalage : %d\n", decalage);
  file_p->queue = (file_p->queue + decalage) % N;  
  
  //printf("\nJ'ai fini d'écrire sur la sortie standard\n");
  if (sem_post(&file_p->vide) == -1) {
    perror("defiler : sem_wait(vide)");
    exit(EXIT_FAILURE);
  }
  if (sem_post(&file_p->mutex) == -1) {
    perror("defiler : sem_wait(mutex)");
    exit(EXIT_FAILURE);
  }
  return request;
}