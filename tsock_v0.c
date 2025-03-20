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
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

//! |================|
//! |__TOOLBOX FUNC__|
// Fonction erreur usage
void usage() {
	printf("usage: cmd [-p|-s][-n ##]\n");
	exit(1);
}

// Fonction pour gérer les erreurs
void errorManager(int code, char *msg, int codeError) {
	if(code == codeError) {
		perror(msg);
		exit(1);
	}
}

// Fonction pour créer une adresse
struct sockaddr_in createAdress(int port, const char* IP) {
	struct sockaddr_in adress;
	adress.sin_family = AF_INET; //internet
	adress.sin_port = htons(port); //htons pour verifier little endian big endian
	adress.sin_addr.s_addr = inet_addr(IP); //inet addr
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

// Fonction pour construire un message correspondance fonction construire_message()
void buildMessage(char *message, char motif, int lg) {
	int i;
	for (i = 0; i < lg; i++)
		message[i] = motif;
}

// Fonction pour afficher un message correspondance fonction afficher_message()
void printMessage(char *message, int lg) {
	int i;
	printf("message construit : ");
	for (i = 0; i < lg; i++)
	printf("%c", message[i]);
	printf("\n");
}

int main (int argc, char **argv) {
	//! |========|
	//! |__VARS__|
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; // Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception
	int source = -1 ; // 0=puits, 1=source
	int protocolFlag = 0; // 0=TCP, 1=UDP
	const char* IP= "127.0.0.1";
	int PORT = 9000;
	char buffer[BUFFER_SIZE];
	ssize_t dataClientSize;
	int retcode; // code de retour des fonctions pour tester les erreurs

	//! |=====================|
	//! |__PARSING ARGUMENTS__|
	// getopt permet de parser les options de la ligne de commande
	while ((c = getopt(argc, argv, "pn:su")) != -1) {
		switch (c) {
			case 'p': // puit (serveur)
				if (source == 1) {
					usage();
				} 
				source = 0;
				PORT = atoi(argv[argc-1]);
				break;

			case 's': // source (client)
				if (source == 0) {
					usage();
				}
				source = 1;
				IP = argv[argc-2];
				PORT = atoi(argv[argc-1]);
				break;

			case 'n': // nombre de messages
				nb_message = atoi(optarg);
				break;

			case 'u': // UDP
				protocolFlag = 1;
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
	//! |___SOCKET_CREATION___|
  struct sockaddr_in adress = createAdress(PORT, IP);
  int socket;

	//? en premier if TCP, en deuxieme if UDP
	socket = (protocolFlag == 0 ? createSocket(SOCK_STREAM) : createSocket(SOCK_DGRAM));
	
	if(source == 1){
		//! |====================|
		//! |___SOURCE(CLIENT)___|
		//* Construction du message
		//TODO ici le 30 représente la longueur, modifier pour -l
		char message[30];
		nb_message = 2;
		printf("SOURCE : lg_mesg_emis=30, port=9000, nb_envois=%d, TP=udp, dest=gauthier\n", nb_message);

		for(int i = 0; i < nb_message; i++) {
			buildMessage(message, 'a'+i, 30);
			//TODO formater correctement et envoyer chaque message
			// sprintf("");
		}

		//* Envoi du message avec le bon protocole
		if (protocolFlag == 0) { //? TCP
			
		} else if(protocolFlag == 1) { //? UDP
			retcode = sendto(socket, message, strlen(message), 0, (struct sockaddr *)&adress, sizeof(adress));
			errorManager(retcode, "Erreur d'envoi", -1);	
		}

		//* Fermeture du socket
		close(socket);
		
	} else if(source == 0){
		//! |==================|
		//! |__PUITS(SERVER)___|
		//TODO
		retcode = bind(socket,(struct sockaddr*)&adress,sizeof(adress)); // bind de notre socket
		errorManager(retcode, "Erreur de bind", -1);

		if (protocolFlag == 0) { //? TCP
			
		} else if(protocolFlag == 1) { //? UDP
			dataClientSize = recv(socket,buffer,BUFFER_SIZE,0);
			errorManager(dataClientSize, "Erreur de reception", -1);
			printf("SOURCE : lg_mesg_emis=30, port=%d, nb_envois=a faire, TP=udp, dest=a faire", PORT);
			printf("Réponse du serveur : %s\n", buffer); // Affichage du message reçu
		}
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
	
	return 0;
}

