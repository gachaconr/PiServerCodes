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

void error(const char *msg)
{
	perror(msg);
}

void *Android(); // control propeller
void *CC3000(); // get info from potenciometers

int n;
int fd;
struct termios termAttr;
speed_t baudRate;

int true;

int sockfdAndroid, newsockfdAndroid;
int sockfdCC3000, newsockfdCC3000;

int AndroidConn, CC3000Conn;

pthread_t threadAndroid, threadCC3000;
int  iretAndroid, iretCC3000;
pthread_mutex_t mutexsum;

int main(int argc, char *argv[])
{
	true = 1;
	AndroidConn = 0;
	CC3000Conn = 0;

     	pthread_mutex_init(&mutexsum, NULL);    

     	iretAndroid = pthread_create( &threadAndroid, NULL, Android, NULL);
     	iretCC3000 = pthread_create( &threadCC3000, NULL, CC3000, NULL);

     	pthread_join(threadAndroid, NULL);
     	pthread_join(threadCC3000, NULL);

     	pthread_mutex_destroy(&mutexsum);
     	pthread_exit(NULL);

     	exit(0);
}

void *Android()
{
	char buffer[256];	
	while (1) {
		int closeSocket = 0;
		int portno;
		socklen_t clilen;
		struct sockaddr_in serv_addr, cli_addr;
		int errorX = 0;
		socklen_t len = sizeof(error);
		int retval = 0;
		struct sockaddr_storage addrA;
		socklen_t lenA = sizeof(addrA);

        	sockfdAndroid = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfdAndroid < 0) { 
			error("ERROR opening socket for Android"); 
			break;
		}
		if (setsockopt(sockfdAndroid,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1){
			perror("SetsockOpt");
			break;
		}
		//bzero((char *) &serv_addr, sizeof(serv_addr));
		portno = 55000;
        	serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		bzero(&(serv_addr.sin_zero),8);
		if (bind(sockfdAndroid, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
			error("ERROR on binding for Android"); 
			close(sockfdAndroid);
			break;
		}
		listen(sockfdAndroid,1);
		clilen = sizeof(cli_addr);
		printf("Waiting for Android connection...\n");
		newsockfdAndroid = accept(sockfdAndroid, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfdAndroid < 0) { 
			error("ERROR on accept for Android");
			close(sockfdAndroid);	
			closeSocket = 1;
		}
		printf("Connection established for Android...\n");
		AndroidConn = 1;
		
		while (closeSocket == 0)//((closeSocket == 0) && (retval == 0))
		{
			bzero(buffer,256);
			if (pthread_mutex_trylock(&mutexsum) == 0) {
				n=read(newsockfdAndroid,buffer,256);       // wait for info????
				printf("n = %d\n",n);
				if (n <= 0) { 
					error("ERROR reading from Android socket"); 
				} else {
					printf("Here is the Android command: %c\n",buffer[0]);
					if (buffer[0] == 'c'){
						closeSocket = 1;
					} else {
						if (CC3000Conn == 1) {
							n = write(newsockfdCC3000,buffer,1);	
							if (n < 0) { printf("can't transmitt to CC3000\n"); }
						}
					}	
				}
				n = 0;	
				pthread_mutex_unlock(&mutexsum);
			}
			retval = getsockopt(sockfdAndroid, SOL_SOCKET, SO_ERROR, &errorX, &len );
			printf("retval1 = %d\n",retval);
			retval = getpeername(sockfdAndroid, (struct sockaddr*)&addrA, &lenA);
			printf("retval2 = %d\n",retval);
		}
		close(sockfdAndroid);
		close(newsockfdAndroid);
		printf("Android Connection closed...\n");
	}
	
}

void *CC3000()
{
	char buffer[256];
	while (1) {
		int closeSocket = 0;
		int portno;
		socklen_t clilen;
		struct sockaddr_in serv_addr, cli_addr;
		int true = 1;

        	sockfdCC3000 = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfdCC3000 < 0) { 
			error("ERROR opening socket CC3000"); 
			break;
		}
		if (setsockopt(sockfdCC3000,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1){
			perror("SetsockOpt");
			break;
		}
		//bzero((char *) &serv_addr, sizeof(serv_addr));
		portno = 56000;
        	serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		bzero(&(serv_addr.sin_zero),8);
		if (bind(sockfdCC3000, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
			error("ERROR on binding CC3000"); 
			close(sockfdCC3000);
			break;
		}
		listen(sockfdCC3000,1);
		clilen = sizeof(cli_addr);
		printf("Waiting for CC3000 connection...\n");
		newsockfdCC3000 = accept(sockfdCC3000, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfdCC3000 < 0) { 
			error("ERROR on accept CC3000");
			close(sockfdCC3000);	
			closeSocket = 1;
		}
		printf("Connection established CC3000...\n");
		CC3000Conn = 1;

		//sleep(1);
		
		while (closeSocket == 0)
		{
			bzero(buffer,256);
			//printf("Waiting for CC3000 command...\n"); //?????
			if (pthread_mutex_trylock(&mutexsum) == 0) {
				n=read(newsockfdCC3000,buffer,255);       // wait for info????
				if (n < 0) { 
					error("ERROR reading from CC3000 socket"); 
				} else {
					printf("Here is the CC3000 command: %c\n",buffer[0]);
					if (AndroidConn == 1) {
						n = write(newsockfdAndroid,buffer,1);	
						if (n < 0) {
							printf("can't transmitt to Android\n");
						}
					}	
				}
				pthread_mutex_unlock(&mutexsum);
			}
		}
		close(sockfdCC3000);
		close(newsockfdCC3000);
		printf("Connection closed PM...\n");
	}	
}
