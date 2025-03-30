# Projet tsock INSA Toulouse (tsock_v3 finalisé)

## Description

Le projet tsock consiste en une application permettant des échanges d'informations entre deux machines connectées à Internet via l'API socket en langage C. L'application permet de simuler un émetteur (source) et un récepteur (puits) pour échanger des messages via les protocoles TCP ou UDP.

## Fonctionnalités principales

- **Mode Source (Client) :** Envoie des messages à un puits distant en utilisant les protocoles TCP ou UDP.
- **Mode Puits (Serveur) :** Reçoit des messages d'une source distante en utilisant TCP ou UDP.

## Options disponibles

- `-p` : Active le mode puits (serveur).
- `-s` : Active le mode source (client).
- `-u` : Utilise le protocole UDP. Par défaut, le programme utilise le protocole TCP.
- `-n ##` : Définit le nombre de messages à envoyer (source) ou à recevoir (puits). Par défaut, 10 messages pour la source et infini pour le puits.
- `-l ##` : Définit la longueur des messages à envoyer ou à recevoir. Par défaut, la longueur est de 30 octets.

## Structure du programme

Le programme utilise les sockets pour établir une communication entre un émetteur et un récepteur.

1. **Source (Client)** :
   - Envoie des messages au puits via TCP ou UDP.
   - Chaque message contient un numéro unique et une chaîne de caractères répétée.
   - Affiche des informations sur l'envoi de chaque message.

2. **Puits (Serveur)** :
   - Attends la réception de messages en boucle infinie ou pour un nombre défini de messages.
   - Affiche des informations sur chaque message reçu.

## Fonctionnement

### Exemple de commande pour la source (client) :

```bash
tsock -s -u -n 4 gauthier 9000
```

Cela enverra 4 messages via UDP à l'adresse `gauthier` sur le port `9000`.

### Exemple de commande pour le puits (serveur) :

```bash
tsock -p -u 9000
```

Cela recevra des messages en mode UDP sur le port `9000`.

### Format des messages

Les messages envoyés sont structurés comme suit :

- Le premier champ est le numéro du message, codé sur 5 caractères ASCII.
- Le second champ est une chaîne de caractères répétée (ex : 'a', 'b', 'c'...).

**Exemple de message :**

```
----1aaaaaaaaaaaaaaaaaaaaaaaaa
----2bbbbbbbbbbbbbbbbbbbbbbbbb
```

### Affichage

- **Source** : Affiche la longueur du message, le port local, les options, le protocole de transport utilisé, et le destinataire.
- **Puits** : Affiche la longueur du message, le port local, les options, le protocole de transport utilisé, et les messages reçus.

## Installation et utilisation

1. Clonez ou téléchargez le projet sur votre machine locale.
2. Compilez le programme avec `gcc` ou `make` (qui utilisera le Makefile fourni):

   ```bash
   gcc -o tsock tsock.c
   ```
   ou
   ```bash
   make
   ```

3. Lancez l'application en mode source ou puits selon vos besoins.

   - Pour la source :

     ```bash
     ./tsock -s -u -n 4 127.0.0.1 9000
     ```

   - Pour le puits :

     ```bash
     ./tsock -p -u 9000
     ```

## Explication du Code

Le programme est divisé en plusieurs fonctions principales :

1. **`usage()`** : Affiche l'usage du programme et termine l'exécution si des arguments sont mal fournis.
2. **`errorManager()`** : Gère les erreurs et arrête le programme si une erreur se produit.
3. **`createAdress()`** : Crée une adresse avec un port et une IP.
4. **`createSocket()`** : Crée un socket en fonction du type de protocole (TCP ou UDP).
5. **`buildMessage()`** : Construit un message avec un numéro et un motif de caractères.
6. **`printMessage()`** : Affiche un message reçu avec son numéro et son contenu.
7. **`main()`** : Gère la logique du programme, le parsing des arguments, la création des sockets, et l'envoi ou la réception des messages.

## Gestion des erreurs

Le programme vérifie les erreurs dans chaque étape critique (création de socket, envoi, réception) et affiche des messages d'erreur détaillés si une erreur se produit.

## Conclusion

Ce projet implémente une communication de base via TCP et UDP en utilisant des sockets en C. Il fournit une bonne introduction à la programmation réseau avec des sockets et permet de comprendre les mécanismes de communication de base entre des applications distribuées.

## Contributions

- **Antunes Mathieu** et **Johnson Lorcan** **3MIC IR D1**
