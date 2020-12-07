#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>




#ifndef CLIENT__H
#define CLIENT__H

//  send_command_to_thread: envoie les commandes (requêtes du client) au thread
// associé au client. Renvoie -1 en cas d'erreur et 0 en cas de succès.
extern int send_command_to_thread(int shm_fd,  mqd_t mqd,char *command,
    size_t shm_size);

//  start_end_connection : écrit la commande « SYNC » ou « END » de client, dans
// le tube du demon pour effectuer respectivement une demande de connexion ou de
// déconnexion. La commande est de la forme SYNCpid\0 ou ENDnuméro shm\0. La
// fonction renvoie -1 si la fonction write a echouée, 0 sinon.
extern int start_end_connection(int fd_demon, size_t label);

//  demon_answer: réceptionne les données à partir du tube du démon. Si le
// message envoyé est « RST » alors la fonction renvoie -2 car aucun thread
// n'est disponible. Renvoie la taille de la shm associé au thread disponible
// ou le crée, et -1 en cas d'erreur.
extern int thread_answer(int fd_client);

#endif
