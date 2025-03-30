# Nom de l'exécutable final
EXEC = tsock

# Compilateur et options de compilation
CC = gcc
CFLAGS = -Wall -Wextra -Werror

# Fichiers sources et objets
SRCS = tsock.c
OBJS = $(SRCS:.c=.o)

# Règle de compilation principale
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Compilation des fichiers .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers objets et de l'exécutable
clean:
	rm -f $(OBJS) $(EXEC)

# Permet d'éviter les erreurs si "make clean" est utilisé sans cible existante
.PHONY: clean
