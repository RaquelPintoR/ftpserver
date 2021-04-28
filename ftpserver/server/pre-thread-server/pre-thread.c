/*Pre-thread*/
#include "../ftpserver.h"

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

char client_message[2000];
char buffer[1024];
const char *root_dir;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Envía archivo solicitado a través de conexión de datos
void ftserve_retr(int sock_control, int sock_data, char* filename)
{	
	FILE* fd = NULL;
	char data[MAXSIZE];
	size_t num_read;							
	
	char new_filename[100];
    strcat(new_filename, root_dir);
    strcat(new_filename, filename);	
	fd = fopen(new_filename, "r");

	pthread_mutex_lock(&lock);
	
	if (!fd) {	//Envía error 550: Requested action not taken
		send_response(sock_control, 550);
		
	} else {	//Envía estado 150: File status okay
		send_response(sock_control, 150);
	
		do {
			num_read = fread(data, 1, MAXSIZE, fd);

			if (num_read < 0) {
				printf("error in fread()\n");
			}

			// Envía bloque
			if (send(sock_data, data, num_read, 0) < 0)
				perror("error sending file\n");

		} while (num_read > 0);													
			
		// Se envía mensaje 226
		// Se cierra la conexión y el archivo se transfiere correctamente
		send_response(sock_control, 226);

		fclose(fd);
    	pthread_mutex_unlock(&lock);
    	sleep(1);
	}
}

// Envía lista de archivos a través de conexión de datos
int ftserve_list(int sock_data, int sock_control)
{
	char data[MAXSIZE];
	size_t num_read;									
	FILE* fd;

  	pthread_mutex_lock(&lock);
	char command[100];

    strcpy(command, "ls -l ");
	strcat(command, root_dir);
	strcat(command, " | tail -n+2 > tmp.txt");
	int rs = system(command);
	if ( rs < 0) {
		exit(1);
	}
	
	fd = fopen("tmp.txt", "r");	
	if (!fd) {
		exit(1);
	}

	// Busca inicio de archivo
	fseek(fd, SEEK_SET, 0);

	send_response(sock_control, 1);

	memset(data, 0, MAXSIZE);
	while ((num_read = fread(data, 1, MAXSIZE, fd)) > 0) {
		if (send(sock_data, data, num_read, 0) < 0) {
			perror("err");
		}
		memset(data, 0, MAXSIZE);
	}

	fclose(fd);
  	pthread_mutex_unlock(&lock);
  	sleep(1);

	send_response(sock_control, 226);

	return 0;	
}

// Abre la conexión al cliente y retorna socket para la conexión
int ftserve_start_data_conn(int sock_control)
{
	char buf[1024];	
	int wait, sock_data;

	// Espera que se realice la conexión
	if (recv(sock_control, &wait, sizeof wait, 0) < 0 ) {
		perror("Error while waiting");
		return -1;
	}

	// Obtiene la dirección del cliente
	struct sockaddr_in client_addr;
	socklen_t len = sizeof client_addr;
	getpeername(sock_control, (struct sockaddr*)&client_addr, &len);
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	// Inicia la conexión con el cliente
	if ((sock_data = socket_connect(CLIENT_PORT_ID, buf)) < 0)
		return -1;

	return sock_data;		
}

// Autentica los datos del usuario
int ftserve_check_user(char*user, char*pass)
{
	char username[MAXSIZE];
	char password[MAXSIZE];
	char *pch;
	char buf[MAXSIZE];
	char *line = NULL;
	size_t num_read;									
	size_t len = 0;
	FILE* fd;
	int auth = 0;
	
	fd = fopen("../.auth", "r");
	if (fd == NULL) {
		perror("file not found");
		exit(1);
	}	

	while ((num_read = getline(&line, &len, fd)) != -1) {
		memset(buf, 0, MAXSIZE);
		strcpy(buf, line);
		
		pch = strtok (buf," ");
		strcpy(username, pch);

		if (pch != NULL) {
			pch = strtok (NULL, " ");
			strcpy(password, pch);
		}

		// ELimina espacios al final
		trimstr(password, (int)strlen(password));

		if ((strcmp(user,username)==0) && (strcmp(pass,password)==0)) {
			auth = 1;
			break;
		}		
	}
	free(line);	
	fclose(fd);	
	return auth;
}

// Inicia sesión del cliente
int ftserve_login(int sock_control)
{	
	char buf[MAXSIZE];
	char user[MAXSIZE];
	char pass[MAXSIZE];	
	memset(user, 0, MAXSIZE);
	memset(pass, 0, MAXSIZE);
	memset(buf, 0, MAXSIZE);
	
	// Recibe nombre de usuario
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}	

	int i = 5;
	int n = 0;
	while (buf[i] != 0)
		user[n++] = buf[i++];
	
	// Envía mensaje para recibir contraseña
	send_response(sock_control, 331);					
	
	// Espera a recibir la contraseña
	memset(buf, 0, MAXSIZE);
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}
	
	i = 5;
	n = 0;
	while (buf[i] != 0) {
		pass[n++] = buf[i++];
	}
	
	return (ftserve_check_user(user, pass));
}

// Espera comando del cliente y retorna codigo
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg)
{	
	int rc = 200;
	char buffer[MAXSIZE];
	
	memset(buffer, 0, MAXSIZE);
	memset(cmd, 0, 5);
	memset(arg, 0, MAXSIZE);
		
	// Wait to recieve command
	if ((recv_data(sock_control, buffer, sizeof(buffer)) ) == -1) {
		perror("recv error\n"); 
		return -1;
	}
	
	strncpy(cmd, buffer, 4);
	char *tmp = buffer + 5;
	strcpy(arg, tmp);
	
	if (strcmp(cmd, "QUIT")==0) {
		rc = 221;
	} else if((strcmp(cmd, "USER")==0) || (strcmp(cmd, "PASS")==0) ||
			(strcmp(cmd, "LIST")==0) || (strcmp(cmd, "RETR")==0)) {
		rc = 200;
	} else { //invalid command
		rc = 502;
	}

	send_response(sock_control, rc);	
	return rc;
}

// Maneja conexión con el cliente
void * ftserve_process2(void *argument)
{
    int sock_control = *((int *)argument);
	int sock_data;
	char cmd[5];
	char arg[MAXSIZE];

	// Envía mensaje de bienvenida
	send_response(sock_control, 220);

	// Authenticate user
	if (ftserve_login(sock_control) == 1) {
		send_response(sock_control, 230);
	} else {
		send_response(sock_control, 430);	
		exit(0);
	}	
	
	while (1) {
		// Espera comando
		int rc = ftserve_recv_cmd(sock_control, cmd, arg);
		
		if ((rc < 0) || (rc == 221)) {
			break;
		}
		
		if (rc == 200 ) {	// Abre conexión de datos con el cliente
			if ((sock_data = ftserve_start_data_conn(sock_control)) < 0) {
				close(sock_control);
				exit(1); 
			}

			// Ejecuta comando
			if (strcmp(cmd, "LIST")==0) { // Ejecuta comando LS
				ftserve_list(sock_data, sock_control);
			} else if (strcmp(cmd, "RETR")==0) { // Obtiene archivo
				ftserve_retr(sock_control, sock_data, arg);
			}
		
			// Cierra conexión de datos
			close(sock_data);
		} 
	}
	close(sock_control);
	pthread_exit(NULL);
}

//Pasa de string a uint16
bool str_to_uint16(const char *str, uint16_t *res) {
    char *end;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (errno || end == str || *end != '\0' || val < 0 || val >= 0x10000) {
        return false;
    }
    *res = (uint16_t)val;
    return true;
}

int main(int argc, char* argv[]){
  int serverSocket, newSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

	if (argc != 7) {
		printf("uso: ./ftclient -n  <cantidad-hilos> -w <ftp-root> -p <puerto>\n");
		exit(0);
	}

	int nprocess = atoi(argv[2]);
	root_dir = argv[4];
	char *port = argv[6];

	uint16_t port_number;
	str_to_uint16(port, &port_number);


  //Se crea socket
  serverSocket = socket(PF_INET, SOCK_STREAM, 0);

  // Se configuran los ajustes de la estructura de la dirección del servidor
  serverAddr.sin_family = AF_INET;

  // Se establece número de puerto 
  serverAddr.sin_port = htons(port_number);

  // Se establece la dirección ip del localhost
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Se establecen los bits a 0 
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  // Vincula la estructura de la dirección al socket
  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  // Atiende un máximo de 50 solicitudes
  if(listen(serverSocket,nprocess)==0){
    printf("Listening\n");}
  else{
    printf("Error\n");}
    pthread_t tid[60];
    int i = 0;
    while(1)
    {
        // Se acepta llamada y crea un nuevo socket para la conexión entrante
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

	   // Para cada cliente se crea un hilo y se le asigna solicitud
        if( pthread_create(&tid[i++], NULL, ftserve_process2, &newSocket) != 0 )
           printf("Failed to create thread\n");

        if( i >= nprocess)
        {
          i = 0;
          while(i < nprocess)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
    }
  return 0;
}