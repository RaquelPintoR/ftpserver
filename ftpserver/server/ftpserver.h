#ifndef FTSERVE_H
#define FTSERVE_H

#include "../common/common.h"


// Envía archivo solicitado a través de conexión de datos
void ftserve_retr(int sock_control, int sock_data, char* filename);

// Envía lista de archivos a través de conexión de datos
int ftserve_list(int sock_data, int sock_control);

// Abre la conexión al cliente y retorna socket para la conexión
int ftserve_start_data_conn(int sock_control);

// Autentica los datos del usuario
int ftserve_check_user(char*user, char*pass);

// Inicia sesión del cliente
int ftserve_login(int sock_control);

// Espera comando del cliente y retorna codigo
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg);

// Maneja conexión con el cliente
void ftserve_process(int sock_control);


#endif
