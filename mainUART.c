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

void error(const char *msg) {
	perror(msg);
}

int n;
int fd;
struct termios termAttr;
speed_t baudRate;

char buffer[256];

int main(int argc, char *argv[]) {
	
	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		perror("open_port: Unable to open /dev/ttyUSB0\n");
		exit(1);
	}

	fcntl(fd, F_SETFL, FNDELAY); // to not wait until next input data

	tcgetattr(fd,&termAttr);
	baudRate = B9600;//B115200;
	cfsetispeed(&termAttr,B9600);//B115200);
	cfsetospeed(&termAttr,B9600);//B115200);
	//termAttr.c_cflag &= ~CNEW_RTSCTS;
	termAttr.c_cflag &= ~PARENB;
	termAttr.c_cflag &= ~CSTOPB;
	termAttr.c_cflag &= ~CSIZE;
	termAttr.c_cflag |= CS8;
	termAttr.c_cflag |= (CLOCAL | CREAD);
	termAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
	termAttr.c_oflag &= ~OPOST;
	tcsetattr(fd,TCSANOW,&termAttr);
	printf("ttyUSB0 configured....\n");
	bzero(buffer,255);
	while (1) {
		n = read(fd,buffer,255);
		if (n > 0) {
			//printf("n = %i\n",n);
			printf("received: %s\n",buffer);
			sleep(1);
			n = 0;
		}
		bzero(buffer,255);
	}	
	close(fd);
     	exit(0);
}
