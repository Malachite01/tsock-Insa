//! |====================================|
//! |   ANTUNES Mathieu & JOHNSON Lorcan |
//! |====================================|

#include <stdlib.h> /* pour getopt */
#include <unistd.h> /* déclaration des types de base */
#include <sys/types.h> /* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h> /* constantes et structures propres au domaine UNIX */
#include <sys/un.h> /* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h> /* constantes et structures propres au domaine INTERNET */
#include <netdb.h> /* structures retournées par les fonctions de gestion de la base de données du réseau */
#include <stdio.h> /* pour les entrées/sorties */
#include <errno.h> /* pour la gestion des erreurs */

// Fonction erreur usage
void usage() {
	printf("usage: cmd [-p|-s][-n ##]\n");
	exit(1);
}

// Fonction pour créer une adresse
struct sockaddr_in createAdress(int port, char IP) {
	struct sockaddr_in adress;
	adress.sin_family = AF_INET; //internet
	adress.sin_port = htons(port); //htons pour convertir le port format ordi
	adress.sin_addr.s_addr = inet_addr(IP); //inet addr pour que l'ordi comprenne lip
	char bufferRecep[BUFFER_SIZE]; //buffer de recep
	return adress;
}

// Fonction pour créer un socket
int createSocket(int sockType) {// sock
  //valeurs de sockType: SOCK_STREAM, SOCK_DGRAM
	int sock = socket(AF_INET, sockType, 0); //creation de la socket
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	return sock;
}

void main (int argc, char **argv) {
	//! |================|
	//! |__DEFAULT VARS__|
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int protocolFlag = 0; /* 0=TCP, 1=UDP */
	const char* IP= "127.0.0.1";
	int PORT = 9000;

	//! |=====================|
	//! |__PARSING ARGUMENTS__|
	// getopt permet de parser les options de la ligne de commande
	while ((c = getopt(argc, argv, "pn:s")) != -1) {
		switch (c) {
			case 'p': // puit
				if (source == 1) {
					usage();
				} 
				source = 0;
				break;

			case 's': // source
				if (source == 0) {
					usage();
				}
				source = 1;
				break;

			case 'n': // nombre de messages
				nb_message = atoi(optarg);
				break;

			case 'u': // UDP
				protocolFlag = 1;
				// si on est dans la source on set l'IP et le port
				if (source == 1) {
					IP = argv[2];
					PORT = atoi(argv[3]);
				} else { // si on est dans le puit on set le port (pas d'ip)
					PORT = atoi(argv[2]);
				}
				break;

			default: // erreur
				usage();
				break;
		}
	}
	//! INFOS
	if (source == -1) {
		usage();
	}
	if (source == 1)
		printf("Lancement en mode source\n");
	else
		printf("Lancement en mode puits\n");
    

	//! |=====================|
	//! |___SOCKET CREATION___|
  struct sockaddr_in adress = createAdress(PORT, IP);
  int socket;

	//? TCP
	if (protocolFlag == 0) {
	  socket = createSocket(SOCK_STREAM);
	} else if(protocolFlag == 1) { //? UDP
	  socket = createSocket(SOCK_DGRAM);
	}
	


	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");

	}
}

