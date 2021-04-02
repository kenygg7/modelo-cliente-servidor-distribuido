#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490" 

int id = 0;	//identificador de cliente
#define BACKLOG 10	 //Conexiones permitidas


void sigchld_handler(int s)
{
	(void)s;
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int send_file(int socket);

int main(void){
	int sockfd, new_fd;  
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; 
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); 

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("\nserver: Servidor enviara un archivo.\n");
	printf("\nserver: Esperando conexion...\n");

	while(1) {  
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
			id++;
	switch (id)
	{
	case 1:
		printf("server: Conexion establecida con cliente A desde %s\n", s);
		break;
	case 2:
		printf("server: Conexion establecida con cleinte B desde %s\n", s);
		break;
	case 3:
		printf("server: Conexion establecida con cliente C desde %s\n", s);
		break;
	default:
		break;
	}
	//printf("server: Conexion establecida con %s\n", s);
		
		
		if (!fork()) { 
			close(sockfd); 
/******************************************************************************************/
			char integer[4];                  
  			recv(new_fd, integer, 4, 0 );        
  			int menu = *integer;
			printf("\nOpcion seleccionada: %d\n", menu);
			
			int number_to_send = id; 
			int converted_number = htonl(number_to_send);

			write(new_fd, &converted_number, sizeof(converted_number));
			if (menu == 1){	
				send_file(new_fd);
				printf("Archivo enviado.");
				fflush(stdout);	
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd);
	}
	return 0;
}
int send_file(int socket){

   FILE *fileToWork;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
   packet_index = 1;
	printf("Numero de cliente: %d\n",id);
	switch (id)
	{
	case 1:/* constant-expression */
		/* code */
		printf("Se atendio al cliente A");
		fileToWork = fopen("music.mp3", "r");
		
		break;
	case 2:
		printf("Se atendio al cliente B");
		fileToWork = fopen("video.mp4", "r");
		break;
	case 3:
		printf("Se atendio al cliente C");
		fileToWork = fopen("programaUno.pdf", "r");
		break;
	default:
		break;
	}
   //fileToWork = fopen("asis.pdf", "r");
 

   if(fileToWork == NULL) {
        printf("Error al abrir el archivo");
		exit(1);
	} 
	//tamanio imagen
   fseek(fileToWork, 0, SEEK_END);
   size = ftell(fileToWork);
   fseek(fileToWork, 0, SEEK_SET);
   
   //Send fileToWork Size
   write(socket, (void *)&size, sizeof(int));


   do { //Read while we get errors that are due to signals.
      stat=read(socket, &read_buffer , 255);
   } while (stat < 0);

   while(!feof(fileToWork)) {

      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, fileToWork);

      //Send data through our socket 
      do{
        stat = write(socket, send_buffer, read_size);  
      }while (stat < 0);

      printf(" \n");


      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     }
	 return 1;
}