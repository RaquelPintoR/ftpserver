#ifndef FTCLIENT_H
#define FTCLIENT_H

#include "../common/common.h"

// Lee respuesta del servidor
int read_reply();

// Imrpime mensaje recibido
void print_reply(int rc);

// Lee comando ingresado
int ftclient_read_command(char* buf, int size, struct command *cstruct);

// Ejecuta el comando GET y obtiene el archivo solicitado
int ftclient_get(int data_sock, int sock_control, char* arg);

// Inicia conexión
int ftclient_open_conn(int sock_con);

// Ejecuta el comando LS
int ftclient_list(int sock_data, int sock_con);

// Une el codigo con el arg y lo envia al servidor
int ftclient_send_cmd(struct command *cmd);

// Realiza autenticación para el inicio de sesión del cliente
void ftclient_login();

#endif