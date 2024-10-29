#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <stdbool.h>

void error(const char *msg){
	perror(msg);
}

int n;

int main(int argc, char *argv[]) {

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int closeSocket, closeServer, n;
	char buffer[256];
	
	closeServer = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) { 
		error("ERROR opening socket for Helicopter"); 
		exit(1);
	}

	/*if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1){
		perror("SetsockOpt");
		exit(1);
	}*/
	//bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 55000;
        serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	bzero(&(serv_addr.sin_zero),8);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
		error("ERROR on binding"); 
		close(sockfd);
		exit(1);
	}

     while(closeServer == 0) {

	listen(sockfd,1);
	clilen = sizeof(cli_addr);
	printf("Waiting for connection...\n");
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) { 
		error("ERROR on accept connection");
		close(sockfd);	
		exit(1);
	}
	printf("Connection established\n");
	
	closeSocket = 0;
	while(closeSocket == 0) {
		bzero(buffer,256);
		n = read(newsockfd,buffer,256);
		printf("n = %d\n",n);
		if (n <= 0){
			error("error reading from android");
		} else {
			char bufferAux[n], bufferComp[5];
			char bufferexit[5] = "exit";
			snprintf(bufferAux,n,buffer);
			printf("command from socket: %s \n", bufferAux);
			closeSocket = 1;
			snprintf(bufferComp,5,bufferAux);
			int comprcv = strcmp(bufferComp,bufferexit);
			printf("bufferComp = %s \n", bufferComp);
			printf("bufferexit = %s \n", bufferexit);
                        printf("comprcv = %d \n", comprcv);
 			if (comprcv == 0) {
				closeServer = 1;
				printf("stopping server...\n");
			}
		}
	}

	close(newsockfd);
	printf("Connection closed...\n");

    }

	close(sockfd);

	exit(0);
}
