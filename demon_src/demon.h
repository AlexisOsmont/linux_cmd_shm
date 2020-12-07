#include <stdlib.h>
#include <stdio.h>



#ifndef DEMON__H
#define DEMON__H


//    connected_to_thread: permet de connecter le client avec un thread
// disponible. Si le nombre MAX_THREAD n’est pas atteint, alors le démon
// initialise un nouveau thread pour gérer la connexion et transmet au client
// les informations nécessaire pour se connecter à la SHM associée à ce nouveau
// thread. Si aucun thread n’est disponible et si le nombre MAX_THREAD est
// atteint, alors le démon répond négativement par l’envoi du message « RST ».
// Cette fonction est prévu pour ne jamais s'arrêter, seul les signaux peuvent
// y mettre fin. Renvoie -1 en cas d'erreur et 0 en cas de succès.
static int connected_to_thread(size_t shm_size);

#endif