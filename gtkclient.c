#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <time.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}


void delayG(int number_of_seconds) {
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
    // Stroing start time
    clock_t start_time = clock();
    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}

int n, n2;
int fd;
struct termios termAttr;
speed_t baudRate;

unsigned char *bu;
char buffer[256];
unsigned char buf[40], bufdata[100];
//volatile unsigned char bu[100];
unsigned char cmdAT[] = "AT\r";
unsigned char cmdCGATT[] = "AT+CGATT?\r"; // GPRS service's status
unsigned char cmdCSTT[] = "AT+CSTT=\"tingdata\"\r"; // set APN
unsigned char cmdCIICR[] = "AT+CIICR\r"; // bring up wireless connection
unsigned char cmdCIFSR[] = "AT+CIFSR\r"; // get local IP address
unsigned char cmdCIPMUXQ[] = "AT+CIPMUX?\r"; // set single connection
unsigned char cmdCIPSTARTQ[] = "AT+CIPSTART=?\r";
unsigned char cmdCIPSTART[] = "AT+CIPSTART=\"TCP\",\"173.88.131.63\",\"55000\"\r"; // starts an IP TCP connection
unsigned char cmdCIPSEND[] = "AT+CIPSEND\r"; // send data once connetion is ready
unsigned char cmdCIPSHUT[] = "AT+CIPSHUT\r"; // reset the connection if any
unsigned char *cmdP;
unsigned char text[1];
int n_written, spot;//, naux;
unsigned char naux;
int intX;

void *SocketConn();
void *gtk();

// gtk variables
GtkWidget *window;
GtkWidget *fixed;
GtkWidget *halign;
GtkWidget *btn;
GtkWidget *table;
GtkWidget *frame1;
GtkWidget *frame2;
GtkWidget *frame3;
GtkWidget *frame4;
GtkTextBuffer *bufferT;
GtkTextIter start, end;
GtkTextIter iter, iter2;
GtkWidget *view;
GtkWidget *vbox;
GtkWidget *hbox;
int argcX;

// threads
pthread_mutex_t mutexsum;
// pthread variables socket
pthread_t socketThread;
int iretSocket;
// pthread variables gtk
pthread_t gtkThread;
int iretgtk;

// socket variables
int sockfd, portno, n, ndata;
struct sockaddr_in serv_addr;
struct hostent *server;
//char buffer[256];
char paramServer[50];
//char data[13];
char co2D[10];
char tvocD[10];
char tempD[10];
char pressD[10];
char humD[10];

char dataS[30];

char newline[20];
char co2T[30];
char tvocT[30];
char tempT[30];
char pressT[30];
char humT[30];
char finalM[300];
char auxtext[20];

int Connected;
//int connectN;

int initialD, DeleteLen;
int aa, bb, cc, dd, ee;
int swq;
int inprocess;

void button_clicked(GtkWidget *widget, gpointer data) {
	if (Connected == 0){
		Connected = 1;
        //connectN = 1;
		gtk_button_set_label(GTK_BUTTON(btn), "Disconnect");//btn,"Disconnect");
	} else {
		Connected = 0;
		while (inprocess == 1){
			gtk_button_set_label(GTK_BUTTON(btn),"Connect");

			gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);
			gtk_text_buffer_get_iter_at_offset(bufferT, &iter2, DeleteLen);
			gtk_text_buffer_delete(bufferT, &iter, &iter2);

			strcat(co2T,newline);
			strcat(tvocT,newline);
			strcat(tempT,newline);
			strcat(pressT,newline);
			strcat(humT,newline); 

			gtk_text_buffer_insert(bufferT, &iter, co2T, -1); 
			gtk_text_buffer_insert(bufferT, &iter, tvocT, -1); 
			gtk_text_buffer_insert(bufferT, &iter, tempT, -1); 
			gtk_text_buffer_insert(bufferT, &iter, pressT, -1);
			gtk_text_buffer_insert(bufferT, &iter, humT, -1); 

			aa = strlen(co2T);
			bb = strlen(tvocT);
			cc = strlen(tempT);
			dd = strlen(pressT);
			ee = strlen(humT);
			DeleteLen = aa +  bb + cc + dd + ee;

			strcpy(co2T,"CO2: ");  
			strcpy(tvocT,"TVOC: ");
			strcpy(tempT,"Temperature: ");
			strcpy(pressT,"Pressure: ");
			strcpy(humT,"Humidity: ");
		}
	}
}


int main(int argc, char *argv[]) {

	argcX = argc;

	strcpy(newline,"\n");
	strcpy(co2T,  "CO2: ");  //5 + 2
	strcpy(tvocT, "TVOC: "); //6 + 2
	strcpy(tempT, "Temperature: "); //13 + 2
	strcpy(pressT,"Pressure: "); //10 + 2
	strcpy(humT,  "Humidity: "); //10 + 2 

	Connected = 0;
	//connectN = 0;
	initialD = 1;

	pthread_mutex_init(&mutexsum, NULL);
	iretgtk = pthread_create( &gtkThread, NULL, gtk, NULL); 
	iretSocket = pthread_create( &socketThread, NULL, SocketConn, NULL); 
	pthread_join(gtkThread, NULL); 
	pthread_join(socketThread, NULL); 

	pthread_mutex_destroy(&mutexsum);
	pthread_exit(NULL);

	return 0;
}

void *gtk() {

	gtk_init(&argcX, NULL);

	// WINDOW
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Env. Sensing");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// BOX
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	// VIEW
	view = gtk_text_view_new();
	bufferT = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);
	strcat(co2T,newline);
	strcat(tvocT,newline);
	strcat(tempT,newline);
	strcat(pressT,newline);
	strcat(humT,newline); 
	gtk_text_buffer_insert(bufferT, &iter, co2T, -1); 
	gtk_text_buffer_insert(bufferT, &iter, tvocT, -1); 
	gtk_text_buffer_insert(bufferT, &iter, tempT, -1); 
	gtk_text_buffer_insert(bufferT, &iter, pressT, -1);
	gtk_text_buffer_insert(bufferT, &iter, humT, -1); 
	aa = strlen(co2T);
	bb = strlen(tvocT);
	cc = strlen(tempT);
	dd = strlen(pressT);
	ee = strlen(humT);
	DeleteLen = aa +  bb + cc + dd + ee;
	strcpy(co2T,"CO2: ");  
	strcpy(tvocT,"TVOC: ");
	strcpy(tempT,"Temperature: ");
	strcpy(pressT,"Pressure: ");
	strcpy(humT,"Humidity: ");
	gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);
	//gtk_container_add(GTK_CONTAINER(window), vbox);

	// BUTTON
	btn = gtk_button_new_with_label("Connect");
	gtk_widget_set_size_request(btn, 80, 30);
	gtk_box_pack_start(GTK_BOX(vbox), btn, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(window), vbox);

	g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(button_clicked), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), G_OBJECT(window));//NULL);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}

void *SocketConn() {

	int recn, st, ncmd, nCF, ck;
	char * pch;
	char * sed;
	char * pch2;
	int intX = 0;
	inprocess = 0;

	bu = malloc(100);

	fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		perror("open_port: Unable to open /dev/ttyUSB0\n");
		exit(1);
	}

	fcntl(fd, F_SETFL, FNDELAY); // to not wait until next input data

	tcgetattr(fd,&termAttr);
	baudRate = B9600;
	cfsetispeed(&termAttr,B9600);
	cfsetospeed(&termAttr,B9600);
	//termAttr.c_cflag &= ~CNEW_RTSCTS;
	termAttr.c_cflag &= ~PARENB;
	termAttr.c_cflag &= ~CSTOPB;
	termAttr.c_cflag &= ~CSIZE;
	termAttr.c_cflag |= CS8;
	termAttr.c_cflag |= (CLOCAL | CREAD);
	termAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
	termAttr.c_oflag &= ~OPOST;
	tcsetattr(fd, TCSANOW, &termAttr);
	printf("ttyUSB0 configured....\n");
	bzero(buffer,255);

	nCF = 6;
	ck = 0;
	ncmd = 0;
	while (ncmd < nCF) {
		switch (ncmd){
			case 0:
				cmdP = cmdAT;
				break;
			case 1:
				cmdP = cmdCGATT;
				break;
			case 2:
				cmdP = cmdCSTT;
				break;
			case 3:
				cmdP = cmdCIICR;
				break;
			case 4:
				cmdP = cmdCIFSR;
				break;
			case 5:
				cmdP = cmdCIPMUXQ;
				break;
		}
		//printf("command: %s\n",cmdP);
		n_written = write(fd, cmdP, strlen(cmdP));
		delayG(1000);
		recn = 0;
		st = 0;
		int valZ = 1;
		memset(bu, 0, strlen(bu));
		while (valZ) {
			//delayG(2000);
            ioctl(fd, FIONREAD, &n);
			//printf("waiting...\n");
			if (n > 1) {
				//printf("received something\n");
				do {
					n2 = read(fd, &buffer, 1);
					naux = buffer[0];
					if ( (naux > 31) && (naux < 127) ) {
						bu[recn] = naux;
						++recn;
					}
					delayG(100);
				} while( n2 > 0 );
				memset(buf, 0, strlen(buf));
				memcpy(buf, bu+strlen(cmdP)-1, recn-(strlen(cmdP)-1));
				//printf("received %s\n", buf);
				++ncmd;
				valZ = 0;
			}
		}
	}

	Connected = 1;
	// GET MEASUREMENTS
	text[0] = 0x41;
	while (1){
		while (Connected == 1){
			inprocess = 1;
			nCF = 4;
			ck = 0;
			ncmd = 0;
			while (ncmd < nCF) {
				switch (ncmd){
					case 0:
						cmdP = cmdCIPSTART;
						break;
					case 1:
						cmdP = cmdCIPSEND;
						break;
				}
				if (ncmd < (nCF - 2)){ // < 2
					//printf("command: %s\n",cmdP);
					n_written = write(fd, cmdP, strlen(cmdP));
					delayG(1000);
					recn = 0;
					st = 0;
					int valZ = 1;
					memset(bu, 0, strlen(bu));
					while (valZ) {
						ioctl(fd, FIONREAD, &n);
						if (n > 1) {
							do {
								n2 = read(fd, &buffer, 1);
								naux = buffer[0];
								if ( (naux > 31) && (naux < 127) ) {
									bu[recn] = naux;
									++recn;
								}
								delayG(100);
							} while( n2 > 0 );
							memset(buf, 0, strlen(buf));
							memcpy(buf, bu+strlen(cmdP)-1, recn-(strlen(cmdP)-1));
							printf("received %s\n", buf);
							++ncmd;
							valZ = 0;
						}		
					}
				} else {
					if (ncmd == 2){
						recn = 0;
						swq = 1;
						++ncmd;
						unsigned char cZ = 0x1A;
						unsigned char en[] = "\r";
						n_written = write(fd, text, strlen(text));
						printf("text sent %d\n",n_written);
						n_written = write(fd, &cZ, 1);
						n_written = write(fd, en, strlen(en));
						delayG(500);
						memset(bu, 0, strlen(bu));
					} else {
						ioctl(fd, FIONREAD, &n);
						if (n > 0){
							n2 = read(fd, &buffer, 1);
							naux = buffer[0];
							//bu[recn] = naux;
							if(swq == 1){
								strcpy(bu, &naux);
								swq = 2;
							} else {
								strcat(bu, &naux);
							}
							if (recn > 7){
								pch = strstr(bu, "CLOSED");
								sed = strstr(bu, "SEND OK\n");
								if (pch != 0){
									//printf("bu = %s\n", bu);
									memcpy(buffer, sed+9, (pch-2)-(sed+9));
									printf("final data = %s\n", buffer);
									printf("length data = %d\n", strlen(buffer));
									break; // to exit "ncmd < nCF"
								}
							}
							++recn;
						}
					}
				}
			}
			int pri = 0;
			int we = 0;
			pch = strchr(buffer, '-');
			//memset(dataS, 0, strlen(dataS));
			while(pch != NULL){
				memset(dataS, 0, strlen(dataS));
				printf("found at %d\n", pch-buffer+1);
				if (pri == 0) {
					memcpy(dataS, buffer, pch-buffer);
					pri = 1;
				} else { 
					memcpy(dataS, pch2+1, pch-pch2-1); 
				}
				printf("dataS = %s\n", dataS);
				switch (we){
					case 0:
						memset(co2D, 0, strlen(co2D));
						memcpy(co2D, dataS, strlen(dataS));
						break;
					case 1:
						memset(tvocD, 0, strlen(tvocD));
						memcpy(tvocD, dataS, strlen(dataS));
						break;
					case 2:
						memset(tempD, 0, strlen(tempD));
						memcpy(tempD, dataS, strlen(dataS));
						break;
					case 3:
						memset(pressD, 0, strlen(pressD));
						memcpy(pressD, dataS, strlen(dataS));
						break;
					case 4:
						memset(humD, 0, strlen(humD));
						memcpy(humD, dataS, strlen(dataS));
						break;
				}
				we = we + 1;
				pch2 = pch;
				pch = strchr(pch+1, '-');
			}
	
			gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);
			gtk_text_buffer_get_iter_at_offset(bufferT, &iter2, DeleteLen);
			gtk_text_buffer_delete(bufferT, &iter, &iter2);

			memset(finalM, 0, strlen(finalM));
			strcat(co2T, co2D);
			strcpy(finalM, co2T);
			strcat(finalM, newline);
			strcat(tvocT, tvocD);
			strcat(finalM, tvocT);
			strcat(finalM, newline);
			strcat(tempT, tempD);
			strcat(finalM, tempT);
			strcat(finalM, newline);
			strcat(pressT, pressD);
			strcat(finalM, pressT);
			strcat(finalM, newline);
			strcat(humT, humD);
			strcat(finalM, humT);

			gtk_text_buffer_insert(bufferT, &iter, finalM, -1);

			DeleteLen = strlen(finalM);

			strcpy(co2T,"CO2: ");
			strcpy(tvocT,"TVOC: ");
			strcpy(tempT,"Temperature: ");
			strcpy(pressT,"Pressure: ");
			strcpy(humT,"Humidity: ");

			delayG(500);
		}
		inprocess = 0;
    }
    free(bu);
}

