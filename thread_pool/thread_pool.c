#include "thread_pool.h"

threads *thread_ini() {
 	th = malloc(sizeof(th));
	th->count = 0;
	th->max_thread = 5;
	return th;
}

int thread_create(threads *th,char *commande) {
	thread_one *ptr = malloc(sizeof(ptr));
	if (ptr == NULL) {
		return FUN_FAILURE;
	}
	if (th->count < th->max_thread) {
		th->count += 1;
	} else {
		return -2;
	}
	pthread_create(&(ptr->thread), NULL, split_func, commande);
	free(ptr);
	return FUN_SUCCESS;
}

void *split_func(void *commande) {
	printf("------------ SPLIT_FUNC------------------\n");
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

		// Split la commande des options et des arguments de la commande
		char command[100];
		char *option[100];
		char *args[100];
		while(isdigit(cmd[i])) {
              i += 1;
        }
		int k = 0;

		// commande sans option ni arguments
		while (cmd[i] != ' ' &&  i < (int)strlen(cmd)) {
				command[k] = cmd[i];
				k += 1;
				i += 1;
		}
		printf("\nsplit func/cmd = %s\n",command);

		//    les options et les arguments
		int oi = 0;
		if (strncmp(command,"/bin/",5) == 0) {
			option[oi] = malloc(strlen(command));
			strcpy(option[oi],command + 5);
		} else {
			option[oi] = malloc(strlen(command));
			strcpy(option[oi],command);
		}
		int ai = 0;
		oi += 1;
		while (i < (int)strlen(cmd)) {

			//    les options de la commande
			if (cmd[i] == '-') {
					char buf[100];
					size_t y = 0;
					while (cmd[i] != ' ') {
							buf[y] = cmd[i];
							y += 1;
							i += 1;
					}
					buf[y] = '\0';
					option[oi] = malloc(strlen(buf));
					strcpy(option[oi], buf);
					oi += 1;
			}
			//    les arguments de la commandes
			if (cmd[i] != '-' && cmd[i] != ' ' && cmd[i-1] == ' ' ) {
					char buf2[100];
					size_t y = 0;
					while (cmd[i] != ' ') {
							buf2[y] = cmd[i];
							y += 1;
							i += 1;
					}
					buf2[y] = '\0';
					args[ai] = malloc(strlen(buf2));
					strcpy(args[ai], buf2);
					ai += 1;
			}
			i+=1;
		}
		for (int p = 0; p < ai ; p++) {
				printf("args[%d] = %s\n",p, args[p]);
				option[oi] = malloc(strlen(args[p]));
				strcpy(option[oi],args[p]);
				free(args[p]);
				oi += 1;
		};
		for (int p = 0; p < oi ; p++) {
				printf("option[%d] = %s\n", p, option[p]);
		}
		option[oi] = NULL;
		printf("----------- FIN SPLIT_FUNC -----------\n");
		if (fork_thread(pid, command,option) == -1) {
			printf("erreur d'ouverture de fork thread'");
			return NULL;
		}
		free(commande);
		return NULL;
}

int fork_thread(char *pid, char *cmd, char *args[]) { //char* option[]
	printf("--------  FORK_THREAD -------------\n");
	printf("pid : %s    cdm : %s arg = %s\n", pid, cmd,args[0]);
	char tube_client[strlen(TUBE_CLIENT) + strlen(pid) + 1];
	int fd_client;
	sprintf(tube_client,"%s%s",TUBE_CLIENT,pid);
	printf("fork_thread/ouverture du tube : %s \n", tube_client);
	if ((fd_client = open(tube_client, O_WRONLY)) == -1) {
			perror("thread pool fork thread: Impossible d'ouvrir le tube du client");
			return FUN_FAILURE;
	}
	printf("fork_thread/tube ouvert\n");
	switch (fork()) {
		case -1:
			perror("fork_thread: fork");
			return FUN_FAILURE;
		case 0:
			printf("fork_thread/commande = %s\n", cmd);
			if (dup2(fd_client, STDOUT_FILENO) == -1) {
					perror("fork_thread: dup2 stdout");
					return FUN_FAILURE;
			}
			if (dup2(fd_client, STDERR_FILENO) == -1) {
					perror("fork_thread: dup2 stderr");
					return FUN_FAILURE;
			}
			execv(cmd, args);
			perror("execv");
			exit(EXIT_FAILURE);
			break;
		default:
			if (wait(NULL) == -1) {
				return FUN_FAILURE;
			}
			th->count -= 1;
			if (close(fd_client) == -1) {
				return FUN_FAILURE;
			}
			break;
	}
	printf("------------- FIN FORK_THREAD ----------\n");
	return FUN_SUCCESS;
}