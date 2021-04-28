#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEBUG				1
#define MAXSIZE 			512 
#define CLIENT_PORT_ID		30020

// Contiene el argumento y el codigo
struct command {
	char arg[255];
	char code[5];
};

// Se crea socket en el servidor
int socket_create(int port);

// Crea nuevo socket para clientes que solicitan conexión
int socket_accept(int sock_listen);

// Se conecta al host mediante el puerto recibido
int socket_connect(int port, char *host);

// Recibe datos
int recv_data(int sockfd, char* buf, int bufsize);

// Retorna la respuesta del código
int send_response(int sockfd, int rc);

// Elimina espacios al final
void trimstr(char *str, int n);

// Lee entrada
void read_input(char* buffer, int size);


#endif