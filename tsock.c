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
#include <arpa/inet.h> /* pour la conversion d'adresses */
#include <string.h> /* pour la gestion des chaînes de caractères */


//! |================|
//! |__TOOLBOX FUNC__|
// Fonction erreur usage
void usage() {
	printf("usage: cmd [-p|-s][-u][-n ##][-l ##]\n -p: option puit  -s: option source \n -u: choix du protocole UDP(défaut TCP) \n -n: nombre de messages a envoyer ou recevoir \n -l: longueur des messages a envoyer ou recevoir ");
	exit(1);
}

// Fonction pour gérer les codes erreurs
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
	adress.sin_port = htons(port); //htons pour convertir au format correspondant a notre machine(big endian/little endian)
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

// Fonction pour construire un message (correspondance fonction construire_message())
void buildMessage(int num, char *message, char motif, int lg) {
	// Insérer le numéro du message en ASCII sur 5 caractères
	sprintf(message, "%5d", num);  //"   1"
	memset(message + 5, motif, lg - 5); // Remplir le reste du message avec le motif
	message[lg] = '\0'; // Ajout du caractère de fin de chaîne pour éviter les erreurs
}


// Fonction pour afficher un message (correspondance fonction afficher_message())
void printMessage(char *buffer, int lg, int messageNumber) {
	char numStr[6];  // 5 char pour le chiffre et un de plus pour le \0
	strncpy(numStr, buffer, 5);
	numStr[5] = '\0';

	// On copie l'intégralité du message, y compris le préfixe numérique
	char message[lg + 1];  // +1 pour le '\0'
	strncpy(message, buffer, lg);
	message[lg] = '\0';  // Ajout du caractère de fin de chaîne
	// Affichage avec le bon format (préserve les espaces et le numéro)
	printf("PUITS : Réception n°%d (%ld) [%s]\n", messageNumber, strlen(buffer), message);
}


//! |========|
//! |__MAIN__|
int main (int argc, char **argv) {
	//! |========|
	//! |__VARS__|
	int c;
	extern char *optarg;
	extern int optind;
  char* IP= "127.0.0.1";// IP par defaut 
	int PORT = 9000; //port par défaut
	int retcode; // code de retour des fonctions pour tester les erreurs
	int messageNb = -1; // Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception
	int messageLen = 30; // longueur des messages: par defaut 30
	int source = -1 ; // 0=puits, 1=source
	int protocolFlag = 0; // 0=TCP, 1=UDP default TCP
  int bufferSize=30; // taille du buffer par defaut (30) pour 1 message de 30 octets

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
	if (source == -1) usage(); // Debug


	//! |=====================|
	//! |___SOCKET_CREATION___|
  struct sockaddr_in adress = createAdress(PORT, IP);
  socklen_t adressLen = sizeof(adress);// taille de l'adresse en type socklen_t
  
	//? en premier if TCP, en deuxieme if UDP
  int socket;
	socket = (protocolFlag == 0 ? createSocket(SOCK_STREAM) : createSocket(SOCK_DGRAM));
	
  // gestion de la taille du buffer dans le cas ou messageNb est changé (-n)
  if(messageNb>=0){
    bufferSize=messageLen;
  }
  //creation de notre buffer 
  char buffer[bufferSize];

  //! |====================|
	//! |___SOURCE(CLIENT)___|
	if(source == 1){
		//* Construction du message
		char message[messageLen];
		printf("SOURCE : lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest=%s\n", messageLen, PORT, messageNb, protocolFlag==0?"tcp":"udp", IP);

		if(protocolFlag == 0) { //? CO CLIENT TCP si protocol choisi TCP
			// Connexion au serveur en mode TCP pour ne pas se connecter a chaque message
			retcode = connect(socket, (struct sockaddr*)&adress, sizeof(adress));
			errorManager(retcode, "Erreur de connexion!", -1); 
		}

		//* Boucle d'envoi des messages (utilisée pour TCP et UDP)
		for(int i = 0; i < messageNb; i++) {
			// Reinitialisation du message
			memset(message, 0, sizeof(message)); // Remplissage du message avec des 0

			// Construction du message avec le bon motif (%26 pour 26 lettres de l'alphabet)
			buildMessage(i+1, message, 'a'+i%26, sizeof(message));
			printf("SOURCE : Envoi n°%d (%ld) [%s]\n", i+1, sizeof(message), message);
			
			//* Envoi du message avec le bon protocole
			if (protocolFlag == 0) {  //? CLIENT TCP
				//si co acceptée, envoi du message
				retcode = send(socket, message, strlen(message), 0);
				errorManager(retcode, "Erreur d'envoi du message TCP", -1);

			} else if(protocolFlag == 1) { //? CLIENT UDP
        //envoi du message 
				retcode = sendto(socket, message, strlen(message), 0, (struct sockaddr *)&adress, adressLen);
				errorManager(retcode, "Erreur d'envoi du message UDP", -1);	
			}
		}
		printf("SOURCE : fin\n");
		//* Fermeture du socket dans les deux cas
		close(socket); //! fin du client


	//! |==================|
	//! |__PUITS(SERVER)___|
	} else if(source == 0){
		retcode = bind(socket,(struct sockaddr*)&adress,adressLen); // bind de notre socket(meme operation pour TCP ou UDP)
		errorManager(retcode, "Erreur de bind", -1);

		char receptionNb[messageNb+1]; //Convertir messageNb en char* pour l'affichage
		sprintf(receptionNb, "%d", messageNb);
		printf("PUITS : lg_mesg_lu(buff_size)=%d, port=%d, nb_reception=%s, TP=%s\n", messageLen, PORT, messageNb==-1?"infini":receptionNb, protocolFlag==0?"tcp":"udp");
		
		if (protocolFlag == 0) { //? PUITS TCP
			//notre socket créé tte a lheure devient un socket d'ecoute
			retcode = listen(socket, 5);
			errorManager(retcode, "Erreur lors de la mise en ecoute", -1);

			printf("PUITS : En attente de connexion sur le port %d...\n", PORT);
			
			//acceptation de la connexion TCP = nouveau socket
			int socketTcp = accept(socket, (struct sockaddr*)&adress, &adressLen);
			errorManager(socketTcp, "Erreur d'acceptation de la connexion TCP", -1);
			//on affiche l'adresse IP de la source
			char ip[INET_ADDRSTRLEN]; // taille max d'une adresse IPv4
			inet_ntop(AF_INET, &adress.sin_addr, ip, sizeof(ip)); // conversion de l'adresse IP binaire en chaîne de caractères
			printf("PUITS : Connexion TCP acceptée avec %s\n", ip);

			//reception et affichage des messages TCP
			int receivedBytes = 0; // Nombre d'octets reçus
			int receivedCount = 0; // Compteur de messages reçu

			// Boucle infinie si messageNb == -1 sinon on s'arrête à messageNb. receivedCount est incrémenté à chaque message reçu
			// receivedBytes est mis à jour à chaque réception pour savoir si tous les messages ont été reçus
			// On utilise recv pour recevoir les messages TCP
			while ((messageNb == -1 || receivedCount < messageNb) && 					
						(receivedBytes = recv(socketTcp, buffer, messageLen, 0)) > 0) {
					buffer[receivedBytes] = '\0'; // Terminaison de la chaîne
					printMessage(buffer, receivedBytes, receivedCount+1);
					memset(buffer, 0, messageLen); // Nettoyage du buffer
					receivedCount++;
			}

			if (receivedBytes == 0) {
				printf("PUITS : Connexion fermée par la source.\n");
			} else if (receivedBytes == -1) {
				perror("Erreur lors de la réception du message TCP");
			}

			// Fermeture de la connexion TCP
			retcode = shutdown(socketTcp, SHUT_RDWR);
			errorManager(retcode, "Erreur de fermeture de la connexion", -1);
			close(socketTcp);

		} else if(protocolFlag == 1) { //? PUITS UDP
			// Boucle infinie si messageNb == -1 sinon on s'arrête à messageNb
			for(int i = 0; i < messageNb || messageNb == -1; i++) {
				retcode = recv(socket, buffer, messageLen, 0); // reception du message (pas besoin de recvfrom car pas de retour)
				errorManager(retcode, "Erreur de reception", -1);
				// Affichage du message reçu
				printMessage(buffer, messageLen, i+1);
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
		} else if(protocolFlag == 0) { // pour tcp on lit par defaut tous les messages envoyés
			printf("nb de tampons à recevoir = infini\n");
		} else
			printf("nb de tampons à envoyer = infini\n");
	}
	
	return 0;
}