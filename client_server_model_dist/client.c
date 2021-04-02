#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490"  
int clientNumber=0;

#define MAXDATASIZE 100 

int receive_file(int socket);

void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); 
	
	int menu;
	do{
		printf("Ingrese 1 para solicitar la imagen: ");
		scanf("%d",&menu);
        
	}while(menu < 1 || menu > 3);
	//Se manda la opcion deseada
    
	char integer[4];                  
    *((int*)integer) = menu;         
    send( sockfd, integer, 4, 0 ); 
	int r;
    /*Recibir numero de cliente*/

    int received_int = 0;
    int return_status;
    return_status = read(sockfd, &received_int, sizeof(received_int));
    if (return_status > 0) {
        //fprintf(stdout, "Received int = %d\n", ntohl(received_int));
        clientNumber = ntohl(received_int);
    }       
	else {
   		// errores
	}
	
	/*recibir numero de cliente*/
	if (menu == 1 )
	{
        printf("\nSolicitud enviada.\n");
        
		r = receive_file(sockfd);
	}
	
	close(sockfd);

	return 0;
}

int receive_file(int socket){
    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

    char imagearray[10241],verify = '1';
    FILE *image;

    do
    {
        stat = read(socket, &size, sizeof(int));
    }
    while(stat < 0);

    printf("\nArchivo recibido.\n");
   
    char buffer[] = "Got it";

    do
    {
        stat = write(socket, &buffer, sizeof(int));
    }
    while(stat < 0);

   
    switch (clientNumber)
	{
	case 1:
		image = fopen("music_copy.mp3", "w");
		break;
	case 2:
		image = fopen("video_copy.mp4", "w");
		break;
	case 3:
		image = fopen("programaUno_copy.pdf", "w");
		break;
	default:
		break;
	}
    
    if( image == NULL)
    {
        printf("Error con con el archivo\n");
        return -1;
    }

    int need_exit = 0;
    struct timeval timeout = {10,0};

    fd_set fds;
    int buffer_fd, buffer_out;

    while(recv_size < size)
    {

        FD_ZERO(&fds);
        FD_SET(socket,&fds);

        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

        if (buffer_fd < 0)
            printf("error: bad file descriptor set.\n");

        if (buffer_fd == 0)
            printf("error: buffer read timeout expired.\n");

        if (buffer_fd > 0)
        {
            do
            {
                read_size = read(socket,imagearray, 10241);
            }
            while(read_size <0);
			write_size = fwrite(imagearray,1,read_size, image);

            if(read_size !=write_size)
            {
                printf("error al leer el archivo");
            }

            recv_size += read_size;
            packet_index++;
            printf(" \n");
        }
    }
    fclose(image);
    return 1;
}