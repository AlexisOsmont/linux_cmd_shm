#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include  <sys/shm.h>
#include "../file_src/file.h"

#define FUN_SUCCESS 0
#define FUN_FAILURE -1

#define PID_SIZE 6
#define FINISH "exit"

//TUBE CLIENT
#define TUBE_NAME_LENGTH 128
#define TUBE_CLIENT "mon_tube_"


#define BUFFER_SIZE 450
#define TUBE_SIZE 500

int main(void) {
	int shm_fd;
	if ((shm_fd = shm_open(NOM_SHM, O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
		perror("client : Impossible d'ouvrir le shm");
		exit(FUN_FAILURE);
  	}
	  if (shm_unlink(NOM_SHM) == -1) {
		perror("shm_unlink");
		exit(EXIT_FAILURE);
  }
	char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shm_ptr == MAP_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
 	pid_t pid = getpid();
	
	// CREATION DU TUBE
	char tube_client[TUBE_NAME_LENGTH];
	sprintf(tube_client, "%s%d", TUBE_CLIENT, pid);
	if (mkfifo(tube_client, S_IRUSR | S_IWUSR) == -1) {
		perror("client: Impossible de créer le tube du client");
		exit(EXIT_FAILURE);
	}
	
	char shm_buff[BUFFER_SIZE];  // Buffer de SHM pour les commandes entrantes
	char pipe_buff[TUBE_SIZE];        // Buffer de pipe de sortie
	char mypid[PID_SIZE];
	char* request;        // ex. 65367\0
	snprintf(mypid, PID_SIZE,"%d", pid);
	printf("pid = %s\n", mypid);
	int pipe_fd = 0;
	while (fgets(shm_buff, BUFFER_SIZE, stdin) != NULL
			&& strncmp(shm_buff, FINISH, strlen(FINISH)) != 0) {
		request = malloc(1 + strlen(mypid) + 3 + strlen(shm_buff));
		shm_buff[strlen(shm_buff)] = '\0';
		sprintf(request, "%ld%s%ld%s", strlen(mypid), mypid, strlen(shm_buff) - 1, shm_buff);
		request[strlen(request) - 1] = '\0';
		printf("enfile : %s\n",request);
		enfiler(request, (file*)shm_ptr);
		
		// LECTURE DANS LE TUBE CLIENT (RÉPONSE CLIENT)
		
		printf("attente d'ouverture du tube client en lecture client.c\n");
		printf("tube_name : %s\n",tube_client);
		pipe_fd = open(tube_client, O_RDONLY);
		printf("pipe_fd : %d\n",pipe_fd);
		printf("client.c/tube client ouvert en lecture\n");
		printf("--------  client.c/FORK_THREAD -------------\n");
		printf("client.c/read tube_client\n");

		if ((read(pipe_fd, pipe_buff, TUBE_SIZE)) == -1) {
			perror("client : read");
			exit(EXIT_FAILURE);
		}
		printf("\n");
		puts(pipe_buff);
		memset(pipe_buff,0,strlen(pipe_buff));
		if (close(pipe_fd) == -1) {
			perror("client : closeTubeClient");
			exit(EXIT_FAILURE);
		}
		free(request);
	}
	if (unlink(tube_client) == -1) {
		perror("unlink");
		exit(EXIT_FAILURE);
	}
}