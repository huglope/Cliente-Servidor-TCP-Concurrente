// Practica 6: López Álvarez, Hugo
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
#include <signal.h>

int sock, nsock;
char* convertir(char* recibido, char* respuesta);
void signal_handler (int senal);
int main(int argc, char *argv[]){

	// Se declaran las variables que se van a utilizar
	int puerto, longitud, pid; 
	socklen_t tamcliente;
	char recibido[80], *respuesta;
	struct sockaddr_in dirsock, dircliente;

	// Se comprueba que el numero de argumentos sea correcto
	if(argc !=1 && argc!=3 ){
		fprintf(stderr, "La estructura que debe tener es : ./echocon-udp-server-apellidos [-p puerto-servidor]");
		exit(-1);
	}

	// En caso de que haya 3 argumentos, se comprueba que la flag introducida sea -p
	if(argc == 3 && strcmp(argv[1], "-p") != 0){
		fprintf(stderr, "La estructura que debe tener es : ./echocon-udp-server-apellidos [-p puerto-servidor]");
                exit(-1);
        }

	// Se asignan los puertos segun lo que haya introducido el usuario
	if (argc == 1) puerto = 5;
	else puerto = atoi(argv[2]);

	// Se prepara al servidor para cuando reciba un CNTR + C
	signal(SIGINT, signal_handler);

	// Se abre el socket principal
        if((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0){
                perror("socket()");
                exit(EXIT_FAILURE);
        }

	// Se guarda la informacion pertinente en la estructura
	dirsock.sin_family = AF_INET;
	dirsock.sin_port = htons(puerto);	
	dirsock.sin_addr.s_addr = INADDR_ANY;
	
	// Se permite tener varias peticiones sobre el mismo puerto
	if (setsockopt( sock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int) ) < 0) {
	    perror("setsockopt(SO_REUSEPORT)");
	    exit(EXIT_FAILURE);
	}	

	// Se enlaza un socket con una “dirección” (IP, puerto y protocolo) local
	if(bind(sock, (struct sockaddr *) &dirsock, sizeof(struct sockaddr_in)) < 0){
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	if(listen(sock, 100) == -1) {
		perror("listen()");
		exit(EXIT_FAILURE);
	}

	// Empieza el bucle en el que se responderan las solicitudes
	while(1){
		tamcliente = sizeof(dircliente);

		if ((nsock = accept(sock,(struct sockaddr *) &dircliente, &tamcliente)) == -1){
			perror("accept()");
			exit(EXIT_FAILURE);
		}

		pid = fork();
		
		// Ejecución del proceso hijo
		if(pid == 0){
			// Se reciben los datos de la solicitud del cliente
			if ((longitud = recv(nsock, recibido, 80, MSG_DONTWAIT)) == -1){
				perror("recv()");
				exit(EXIT_FAILURE);
			}

			// Se imprime la información del cliente
			printf("Solicitud recibida de la IP= %s\n", inet_ntoa(dircliente.sin_addr));
			// Se asigna un tamaño al atributo repuesta
			if ((respuesta = (char*) malloc (strlen(recibido)+1)) == NULL){
				perror("malloc()");
				exit(EXIT_FAILURE);
			}
	
			// Se transforma la cadena
			respuesta = convertir(recibido, respuesta);

			// Se envia la cadena convertida al cliente
			if (send(nsock, respuesta, longitud, MSG_DONTWAIT) == -1){
				perror("send()");
				exit(EXIT_FAILURE);
			}

			// Se libera la memoria y los atributos se reestablecen
			free(respuesta);

			// Se cierra la conexión
			shutdown(nsock, SHUT_RDWR);

			// Se comprueba que el otro extremo (el cliente) se ha enterado de que se debe cerrar la conexión
			if (recv(nsock, recibido, 80, 0) != 0){
				perror("recv2()");
				exit(EXIT_FAILURE);
			}

			// Se cierra el socket que se usaba para el cliente
			close(nsock);
			exit(0);
		}
	}

	return 0;	
}

char* convertir(char* recibido, char* respuesta){
	char letra;
	int num_letra;

	// Se transforma la cadena
	for(num_letra = 0; num_letra < strlen(recibido); num_letra++){
		letra = recibido[num_letra];
		if(letra >= 'A' && letra <= 'Z') letra =  letra + ('a' - 'A');
		else if (letra >= 'a' && letra <= 'z') letra = letra - ('a' - 'A');
		respuesta[num_letra] = letra;
	}		

	respuesta [num_letra] = '\0';
	return respuesta;
}

void signal_handler( int senal){
	if (senal == SIGINT){
		// Se cierra la conexión
		shutdown(nsock, SHUT_RDWR);

		// Se comprueba que el otro extremo (el cliente) se ha enterado de que se debe cerrar la conexión
		if (recv(nsock, NULL, 80, 0) != 0){
			perror("recv_signal()");
			exit(EXIT_FAILURE);
		}

		// Se cierra el socket del hijo
		close(nsock);

		// Se repite lo mismo pero para el socket principal
		shutdown(sock, SHUT_RDWR);
		close(sock);
	}

	else printf(" signal ha detectado algo que no ha sido CTRL + C\n");
	exit(0);
}









