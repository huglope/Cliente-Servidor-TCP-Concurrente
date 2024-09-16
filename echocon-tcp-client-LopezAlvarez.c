// Practica Tema 6: López Álvarez, Hugo
// @author Hugo López Álvarez
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
	// Se declaran las variables que se van a utilizar
	int puerto, sock;
	char cadena[80], respuesta[80];
	//struct in_addr dirIP;
	struct sockaddr_in dirsock, cliente;
	// Se comprueba que el comando tiene la estructura correcta
	if (argc!=3 && argc!=5){
		fprintf(stderr,"La estructura del comando debe ser ./echocon-udp-client-apellidos ip_servidor [-p puerto_servidor] cadena\n");
		exit(-1);
	}
	
	// Comprueba y transforma la direccion IP del servidor
	if (inet_aton(argv[1], &dirsock.sin_addr) == 0){
		fprintf(stderr, "La ip_servidor tiene un valor erroneo\n");
		exit(-1);
	}
	
	//Se comprueba si se ha optado por el comando en el que el usuario especificia el puerto o lo deja por defecto
	if (strcmp(argv[2],"-p") == 0){
		// En caso de que se especifique el puerto
		puerto = atoi(argv[3]);
		if(strlen(argv[4]) > 80){
			fprintf(stderr, "La cadena tiene que tener como maximo 80 caracteres");
			exit(-1);
		}

		if(strcpy(cadena, argv[4])==0){
                       fprintf(stderr, "Ha ocurrido un error");
                       exit(-1);
                }

	}
	else{
		// Si el segundo comando introducido no es un -p pero se han introducido 4 argumentos hay un fallo
		if (argc == 5){
			fprintf(stderr, "La estructura del comando debe ser ./echocon-udp-client-apellidos ip_servidor [-p puerto_servidor] cadena\n");
	                exit(-1);
       		 }

		// En caso de que NO se especifique el puerto
		puerto = 5;
		if(strlen(argv[2]) > 80){
			fprintf(stderr, "La cadena tiene que tener como maximo 80 caracteres");
			exit(-1);
		}

		if(strcpy(cadena, argv[2])==0){
			fprintf(stderr, "Ha ocurrido un error");
			exit(-1);
		}
	}
	
	// Se abre un socket para la comunicación
	if((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	

	// Se rellenan los campos de la estructura con la informacion del servidor
	dirsock.sin_family = AF_INET;
	dirsock.sin_port = htons(puerto);

	printf("ip=%s, puerto = %d, cadena=%s\n", inet_ntoa(dirsock.sin_addr), ntohs(dirsock.sin_port), cadena);


	// Se rellenan los campos de la estructura con la informacion que necesita el cliente
	cliente.sin_family = AF_INET;
	cliente.sin_port = htons(0);
	cliente.sin_addr.s_addr = INADDR_ANY;

	// Se enlaza el socket con cualquier IP
	if(bind(sock, (struct sockaddr *) &cliente, sizeof(cliente)) < 0){
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	// Conecta un descriptor de socket con un descriptor
	if(connect(sock, (struct sockaddr *) &dirsock, sizeof(dirsock)) < 0){
		perror("connect()");
		exit(EXIT_FAILURE);
	}

	// Se envía al servidor la cadena introducida
	if(send(sock, cadena, sizeof(cadena), 0) < 0){
		perror("sendto()");
		exit(EXIT_FAILURE);
	}


	// Recibe la cadena modificada que envía el servidor
	if(recv(sock, respuesta, sizeof(respuesta), 0) < 0){
		perror("recvfrom()");
		exit(EXIT_FAILURE);
	}

	// Imprime la respuesta y cierra el socket
	printf("respuesta: %s\n", respuesta);

	shutdown(sock, SHUT_RDWR);

	close(sock);
	return 0;
}
