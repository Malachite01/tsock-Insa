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
	printf("usage: cmd [-p|-s][-n ##][-l ##]\n");
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
void buildMessage(int num, char *message, char motif, int lg) {
	// Insérer le numéro du message en ASCII sur 5 caractères
	sprintf(message, "%5d", num);  //"   1" -> "----1"
	for (int i = 0; i < 5; i++) {
			if (message[i] == ' ') message[i] = '-';
	}
	memset(message + 5, motif, lg - 5); // Remplir le reste du message avec le motif
	message[lg] = '\0'; // Ajout du caractère de fin de chaîne pour éviter les erreurs
}

// Fonction pour afficher un message correspondance fonction afficher_message()
void printMessage(char *buffer, int lg) {
	char numStr[6];
	char message[lg + 1];  // +1 pour le '\0'

	strncpy(numStr, buffer, 5); // Copier les 5 premiers caractères (numéro)
	numStr[5] = '\0';

	for (int i = 0; i < 5; i++) { // Remplacer les '-' par des espaces
			if (numStr[i] == '-') numStr[i] = ' ';
	}
	int messageNumber = atoi(numStr); // Convertir la chaîne en int

	// Copier le message à afficher (en respectant lg)
	strncpy(message, buffer + 5, lg);
	message[lg] = '\0';  // terminaison

	printf("PUITS : Réception n°%d (%d) [%s]\n", messageNumber, lg, message);
}

int main (int argc, char **argv) {
	//! |========|
	//! |__VARS__|
	int c;
	extern char *optarg;
	extern int optind;
	const char* IP= "127.0.0.1";
	int PORT = 9000;
	
	int retcode; // code de retour des fonctions pour tester les erreurs

	int messageNb = -1; // Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception
	//TODO ici le 30 représente la longueur, modifier pour -l
	int messageLen = 30;
	int source = -1 ; // 0=puits, 1=source
	int protocolFlag = 0; // 0=TCP, 1=UDP default TCP

	//! |=====================|
	//! |__PARSING ARGUMENTS__|
	// getopt permet de parser les options de la ligne de commande
	while ((c = getopt(argc, argv, "psun:l:")) != -1) {
		switch (c) {
			case 'p': // puit (serveur)
				if (source == 1) usage();
				source = 0;
				PORT = atoi(argv[argc-1]);
				break;

			case 's': // source (client)
				if (source == 0) usage();
				source = 1;
				IP = argv[argc-2];
				PORT = atoi(argv[argc-1]);
				messageNb = 10; // Valeur par défaut
				break;

			case 'n': // nombre de messages
				messageNb = atoi(optarg);
				break;

			case 'l': // longueur du message
				messageLen = atoi(optarg);
				break;

			case 'u': // UDP
				protocolFlag = 1;
				break;

			default: // erreur
				usage();
				break;
		}
	}
	//! DEBUG
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
  socklen_t adressLen = sizeof(adress);
  int socket;

	//? en premier if TCP, en deuxieme if UDP
	socket = (protocolFlag == 0 ? createSocket(SOCK_STREAM) : createSocket(SOCK_DGRAM));
	printf("SOCKET : lg_mesg_emis=%d, nb_envois=%d\n", messageLen, messageNb);
	char buffer[4096];
	if(source == 1){
		//! |====================|
		//! |___SOURCE(CLIENT)___|
		//* Construction du message
		char message[messageLen];
		printf("SOURCE : lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest=%s\n", messageLen, PORT, messageNb, protocolFlag==0?"tcp":"udp", IP);

		if(protocolFlag == 0) { //? CO CLIENT TCP
			// Connexion au serveur en mode TCP pour pas se connecter a chaque message
			retcode = connect(socket, (struct sockaddr*)&adress, sizeof(adress));
			errorManager(retcode, "Erreur de connexion!", -1); 
		}

		//* Boucle d'envoi des messages
		for(int i = 0; i < messageNb; i++) {
			buildMessage(i+1, message, 'a'+i, sizeof(message));
			printf("SOURCE : Envoi n°%d (%ld) [%s]\n", i+1, sizeof(message), message);
			
			//* Envoi du message avec le bon protocole
			if (protocolFlag == 0) {  //? CLIENT TCP
				//si co acceptée, envoi du message
				retcode = send(socket, message, strlen(message), 0);
				errorManager(retcode, "Erreur d'envoi!", -1);

			} else if(protocolFlag == 1) { //? CLIENT UDP
				retcode = sendto(socket, message, strlen(message), 0, (struct sockaddr *)&adress, sizeof(adress));
				errorManager(retcode, "Erreur d'envoi du message source vers le puits", -1);	
			}
		}
		printf("SOURCE : fin\n");
		//* Fermeture du socket dans les deux cas
		close(socket); //! fin du client
		
	} else if(source == 0){
		//! |==================|
		//! |__PUITS(SERVER)___|
		retcode = bind(socket,(struct sockaddr*)&adress,sizeof(adress)); // bind de notre socket
		errorManager(retcode, "Erreur de bind", -1);

		printf("PUITS : lg_mesg_emis=%d, port=%d, nb_reception=infini, TP=%s\n", messageLen, PORT, protocolFlag==0?"tcp":"udp");
		
		if (protocolFlag == 0) { //? PUITS TCP
			//notre socket créé tte a lheure devient un socket d'ecoute:
			retcode = listen(socket, 5);
			errorManager(retcode, "Erreur lors de la mise en ecoute", -1);
			printf("PUITS : En attente de connexion sur %d...\n", PORT);
			
			//acceptation de la connexion(on est en TCP): nouveau socket
			int socketCom = accept(socket, (struct sockaddr*)&adress, &adressLen);
			errorManager(socketCom, "Erreur d'acceptation de la connexion TCP", -1);

			printf("Connexion TCP acceptée.\n");
			if (messageNb == -1) messageNb = 10;
			memset(buffer, 0, sizeof(buffer)); // Efface le buffer avant réception
			retcode = recv(socketCom, buffer, messageLen*messageNb, 0); // reception du message (pas besoin de recvfrom car pas de retour)
			errorManager(retcode, "Erreur de reception", -1);
			//TODO resoudre buffer recep(val aleatoires a chaque fois )
			printf("%s \n",buffer);			

			//on clear le buffer(remplissage avec des 0)

			//TODO une histoire de shutdown
			//retcode = shutdown(socketCom, SHUT_RDWR);
			//errorManager(retcode, "Erreur de fermeture de la connexion", -1);
			
			close(socketCom);

		} else if(protocolFlag == 1) { //? PUITS UDP
			// Boucle infinie si messageNb == -1 sinon on s'arrête à messageNb
			for(int i = 0; i < messageNb || messageNb == -1; i++) {
				retcode = recv(socket, buffer, messageLen, 0); // reception du message (pas besoin de recvfrom car pas de retour)
				errorManager(retcode, "Erreur de reception", -1);
				// Affichage du message reçu
				printMessage(buffer, messageLen);
				//on clear le buffer(remplissage avec des 0)
				memset(buffer, 0, messageLen);
			}
		}
		close(socket); //! fin du puits (udp et tcp)
		printf("PUITS : fin\n");
	}
	
	// Affichage du nombre de messages à envoyer ou à recevoir
	if (messageNb != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", messageNb);
		else
			printf("nb de tampons à recevoir : %d\n", messageNb);
	} else {
		if (source == 1) {
			messageNb = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");
	}
	
	return 0;
}

