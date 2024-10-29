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
//#include <pthread.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include "ccs811.h"

void error(const char *msg){
	perror(msg);
}

int closeServer, closeSocket, n;
int sockfd, newsockfd, portno;
char bufferAndroid[256];
int i,j;
int eCO2, eTVOC;
char bme280id;
float Temp;
float TempBME, PressBME, HumBME;
unsigned char bmecal[50];
unsigned char dataccs811[5];
unsigned char databme280[8];
unsigned char rawdataAndroid[40];
char co2D[10];
char tvocD[10];
char tempD[10];
char pressD[10];
char humD[10];
char newline[20];
char co2T[30];
char tvocT[30];
char tempT[30];
char pressT[30];
char humT[30];
char auxP[] = "-";

int trueXY;
int wM;
float a1 = 123.45;
float a2 = 232.6767676;
char b1[20];
char b2[20];

int main(int argc, char *argv[]) {

	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[256];
	int flags;
	trueXY = 1;

	i = ccs811Init(1, 0x5A);
	if (i != 1)
	{
		return -1; // problem - quit
	}
	printf("CCS811 device successfully opened.\n");
	printf("Allow 48 hours initial burn-in and 20 minutes to warm up (each use)\n");
	usleep(1000000); // wait for data to settle for first read
	j = bme280Init(1, 0x77, bmecal);
	if (j != 1){
		return -1;
	}
	printf("BME280 device successfully opened.\n");
	if (bme280ReadID(&bme280id)){
		printf("BME280 ID: %c \n", bme280id);
	}

     	while(1) {

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			error("ERROR opening socket");
			exit(1);
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueXY, sizeof(int)) == -1){
			perror("SetsockOpt");
			exit(1);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		portno = 55000;
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			error("ERROR on binding");
			close(sockfd);
			exit(1);
		}
		listen(sockfd,1);
		printf("Waiting for connection...\n");
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			error("ERROR on accept connection");
			close(sockfd);
			exit(1);
		}
		printf("Connection established\n");

		bzero(buffer,256);
		wM = 1;
		while(wM) {
	        	ioctl(newsockfd, FIONREAD, &n);
        		if (n > 0) {
				n = read(newsockfd, buffer, 1);
				printf("Here is the message: %s\n",buffer);

					if (buffer[0] == 0x41){
						if (ccs811ReadValues(&eCO2, &eTVOC, dataccs811)){
							if(bme280Read(&TempBME, &PressBME, &HumBME, databme280)){
								memcpy(rawdataAndroid,dataccs811,5);
								memcpy(rawdataAndroid+5,databme280,8);
								///////////////////////////////////////////////////////////////////////////////////
						    		int co2 = ((rawdataAndroid[0] << 8) & 0xFF00) | (rawdataAndroid[1] & 0x00FF);
						    		//
						    		int tvoc = ((rawdataAndroid[2] << 8) & 0xFF00) | (rawdataAndroid[3] & 0x00FF); 
						    		//
							    	int rawtemp = ((((rawdataAndroid[8] << 16) & 0xFF0000) | ((rawdataAndroid[9] << 8) & 0x00FF00) | (rawdataAndroid[10] & 0x0000FF)) >> 4) & 0x0FFFFF;
							    	float ut = (float)rawtemp;
							    	int T1f = ((bmecal[1] << 8) & 0xFF00) | (bmecal[0] & 0x00FF);
							    	int T2f = ((bmecal[3] << 8) & 0xFF00) | (bmecal[2] & 0x00FF);
							    	int T3f = ((bmecal[5] << 8) & 0xFF00) | (bmecal[4] & 0x00FF);
							    	float var1 = ((ut/((float)16384)) - ((float)T1f)/(((float)1024)))*((float)T2f);
							    	float var2 = ( ( (ut/((float)131072)) - (((float)T1f)/((float)8192)) )*((ut/((float)131072)) - (((float)T1f)/((float)8192))))*((float)T3f);
							    	float tempf = (var1 + var2)/((float)5120);
						    		//
							    	int P1f = ((bmecal[7] << 8) & 0xFF00) | (bmecal[6] & 0x00FF);
							    	int P2f = ((bmecal[9] << 8) & 0xFF00) | (bmecal[8] & 0x00FF);
							    	int P3f = ((bmecal[11] << 8) & 0xFF00) | (bmecal[10] & 0x00FF);
							    	int P4f = ((bmecal[13] << 8) & 0xFF00) | (bmecal[12] & 0x00FF);
							    	int P5f = ((bmecal[15] << 8) & 0xFF00) | (bmecal[14] & 0x00FF);
							    	int P6f = ((bmecal[17] << 8) & 0xFF00) | (bmecal[16] & 0x00FF);
							    	int P7f = ((bmecal[19] << 8) & 0xFF00) | (bmecal[18] & 0x00FF);
							    	int P8f = ((bmecal[21] << 8) & 0xFF00) | (bmecal[20] & 0x00FF);
							    	int P9f = ((bmecal[23] << 8) & 0xFF00) | (bmecal[22] & 0x00FF);
							    	int rawpress = ((((rawdataAndroid[5] << 16) & 0xFF0000) | ((rawdataAndroid[6] << 8) & 0x00FF00) | (rawdataAndroid[7] & 0x0000FF) ) >> 4) & 0x0FFFFF;
							    	float utp = (float)rawpress;
							    	float var1p = -64000.0;
							    	float var2p = var1p*var1p*((float)P6f)/(32768.0);
							    	var2p = var2p + var1p*((float)P5f)*(2.0);
							    	var2p = (var2p/(4.0)) + ((float)P4f)*(65536.0);
							    	var1p = ( ((float)P3f)*var1p*var1p/(524288.0) + ((float)P2f)*var1p)/(524288.0);
							    	var1p = (1.0 + var1p/(32768.0))*((float)P1f);
								float pressf;
							    	if (var1p == ((float)0)){
							    		pressf = 0.0;
							    	} else {
							    		utp = (1048576.0) - utp;
							    		utp = ((utp - var2p/(4096.0))*(6250.0))/var1p;
							    		var1p = ((float)P9f)*utp*utp/(2147483648.0);
							    		var2p = utp*((float)P8f)/(32768.0);
							    		pressf = utp + (var1p + var2p + ((float)P7f))/(16.0);
							    	}
							    	//
							    	int H1f = bmecal[24] & 0x0000FF;
							    	int H2f = (((bmecal[26] << 8) & 0xFF00) | (bmecal[25] & 0x00FF) & 0x00FFFF);
							    	int H3f = bmecal[27] & 0x0000FF;
							    	int H4f = (((bmecal[28] << 4) & 0x0FF0) | (bmecal[29] & 0x000F) & 0x0FFF);
							    	int H5f = (((bmecal[31] << 4) & 0x0FF0) | (bmecal[30] & 0x000F) & 0x0FFF);
							    	int H6f = bmecal[32] & 0x0000FF;
							    	float rawhum = (((rawdataAndroid[11] << 8) & 0xFF00) | (rawdataAndroid[12] & 0x00FF) & 0x00FFFF);
							    	float uth = (float)(-76800.0);
							    	uth = (rawhum-(((float)H4f)*((float)64.0)+(((float)H5f)/((float)16384.0))*uth))*((((float)H2f)/((float)65536.0))*(((float)1.0)+(((float)H6f)/((float)67108864.0))*uth*(((float)1.0)+(((float)H3f)/((float)67108864.0))*uth)));
							    	uth = uth*(((float)1.0) - ((float)H1f)*uth/((float)524288.0));
							    	float humf;
							    	if (uth > 100){
							    		humf = ((float)100.0);
							    	} else {
							    		if (uth < 0){ 
							    			humf = ((float)0.0); 
							    		} else {
							    			humf = uth;
							    		}
							    	}

								gcvt((float)co2, 3, co2D);
								gcvt((float)tvoc, 2, tvocD);
								gcvt(tempf, 4, tempD);
								gcvt(pressf, 5, pressD);
								gcvt(humf, 5, humD);

								printf("CO2: %s\n",co2D);
								printf("TVOC: %s\n",tvocD);
								printf("Temperature: %s\n",tempD);
								printf("Pressure: %s\n",pressD);
								printf("Humidity: %s\n",humD);

								char rF[strlen(co2D)+strlen(tvocD)+strlen(tempD)+strlen(pressD)+strlen(humD)+1+4*strlen(auxP)];
								strcpy(rF, co2D);
								strcat(rF, auxP);
								strcat(rF, tvocD);
								strcat(rF, auxP);
								strcat(rF, tempD);
								strcat(rF, auxP);
								strcat(rF, pressD);
								strcat(rF, auxP);
								strcat(rF, humD);
								strcat(rF, auxP);
								printf("rF = %s ---  %d\n", rF, strlen(rF));
								/////////////////////////////////////////////////////////////////////////////
								n = write(newsockfd,rF,strlen(rF));
								if (n == strlen(rF)){ printf("Data sent\n");

							}
						}
					}
				}

				if (n < 0) {
					error("ERROR writing to socket");
				}
				wM = 0;
				close(newsockfd);
				close(sockfd);
				printf("Connection closed...\n");
			}
		}
	}
	exit(0);
}
