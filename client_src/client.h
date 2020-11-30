#include <stdio.h>
#include <stdlib.h>



#ifndef CLIENT__H
#define CLIENT__H

//  send_command_to_thread: envoie les commandes (requêtes du client) au thread
// associé au client. Renvoie -1 en cas d'erreur et 0 en cas de succès.
extern int send_command_to_thread(char *shm_name, char *command,
    size_t shm_size);

    //  receive_result_from_thread: écrit les résultats de la commande exécutée par
// le thread sur la sortie standard. Renvoie -1 en cas d'erreur et 0 en cas de
// succès.
extern int receive_result_from_thread(char *shm_name, size_t shm_size);

#endif
