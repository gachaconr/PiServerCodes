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
#include <my_global.h>
#include <mysql.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void delayG(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
    // Stroing start time
    clock_t start_time = clock();
    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}


void finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

// gtk variables
GtkWidget *window;
GtkTextBuffer *bufferT;
GtkTextIter iter, iter2;
GtkWidget *view;
GtkWidget *vbox;
GtkWidget *combo;
int argcX, DeleteLen;
GtkWidget *window, *table;
GtkWidget *label1, *label2;
GtkWidget *entry;
GtkWidget *btn;

// message variables
int recn, st;
int n, n2;
int fd;
struct termios termAttr;
speed_t baudRate;
unsigned char buffer[256];
unsigned char bu[40], buf[40];
unsigned char cmd1[] = "AT+CMGF=1\r";
unsigned char cmd0[50];
unsigned char cmd2[] = "AT+CMGS=\"";
unsigned char cmd3[] = "\"\r";
unsigned char phone[11];
int n_written, spot, naux;

// mysql variables
MYSQL *con;
MYSQL_RES *result;
MYSQL_ROW row;

void combo_selected(GtkWidget *widget, gpointer window) {

	gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);
	gtk_text_buffer_get_iter_at_offset(bufferT, &iter2, DeleteLen);
	gtk_text_buffer_delete(bufferT, &iter, &iter2);

	gchar *name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
	char auX[] = "SELECT phone FROM contacts WHERE name LIKE '";
	char auZ[] = "'";
	char auY[100];
	char sdX[30];
	strcpy(auY, auX);
	strcat(auY, name);
	strcat(auY, auZ);
  	if (mysql_query(con, auY)) { finish_with_error(con); }
  	result = mysql_store_result(con);
  	if (result == NULL) { 
        	//finish_with_error(con); 
		strcpy(phone, "000");
		DeleteLen = 1;
	} else {
  		while ((row = mysql_fetch_row(result))) {
			strcpy(phone, row[0]);
  		}
  		gtk_text_buffer_insert(bufferT, &iter, name, -1);
		DeleteLen = strlen(name);
	}
	g_free(name);
}

void button_clicked(GtkWidget *widget, gpointer data) {
  if (strcmp(phone,"000") != 1) {
	  strcpy(cmd0, cmd2);
	  strcat(cmd0, phone);
	  strcat(cmd0, cmd3);
	  n_written = write(fd, cmd0, strlen(cmd0));
	  delayG(1000);
	  printf("n_written = %d\n", n_written);
	  printf("AT CMGS number sent...\n");
	  recn = 0;
	  st = 0;
	  while (1){
		ioctl(fd, FIONREAD, &n);
		if (n > 1) {
			do {
			   n2 = read(fd, &buffer, 1);
			   //printf("n2 = %d, st = %d, buffer[0] = %c \n", n2, st, buffer[0]);
			   naux = buffer[0];
			   if ((naux == 62) && (st == 0)){
				const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
				n_written = write(fd, text, strlen(text));
				delayG(1000);
				//printf("text sent\n");
				st = 1;
			   } else {
				if  (st == 1) {
					unsigned char cZ = 0x1A;
					unsigned char en[] = "\r";
					n_written = write(fd, &cZ, 1);
					delayG(1000);
					//printf("n_written = %d, cZ\n", n_written);
					n_written = write(fd, en, strlen(en));
					delayG(1000);
					//printf("n_written = %d, en\n", n_written);
					st = 2;
				} else {
					if ((st == 2) && (n2 > 0)){
						bu[recn] = naux;
						++recn;
					}
				}
			   }
			} while(n2 > 0);
			//printf("%s\n", bu);
			break;
		}
	  }
  }
}

int main(int argc, char *argv[]) {

  argcX = argc;

  // SET SERIAL PORT
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
  // AT+CMGF=1
  n_written = write(fd, cmd1, strlen(cmd1));
  delayG(1000);
  printf("n_written = %d\n", n_written);
  printf("AT CMGF sent...\n");
  recn = 0;
  int valZ = 1;
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
		} while( n2 > 0 );
		memcpy(buf , bu+strlen(cmd1)-1, recn-(strlen(cmd1)-1));
		printf("received %s\n", bu);
		valZ = 0;
	}
  }

  // SET MYSQL
  con = mysql_init(NULL);
  if (con == NULL) {
      fprintf(stderr, "mysql_init() failed\n");
      exit(1);
  }
  if (mysql_real_connect(con, "localhost", "gchaconr", "abc123", NULL, 0, NULL, 0) == NULL) {
      printf("exit 1\n");
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }
  if (mysql_query(con, "USE contacts")) {
      printf("exit 2\n");
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }
  gtk_init(&argcX, NULL);

  // SET GTK WINDOW
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Send Message");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  // BOX
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  /////////////////////////////
 
  // DEFINED VIEW
  view = gtk_text_view_new();
  bufferT = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
  gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);
  DeleteLen = 1;

  // COMBOBOX
  combo = gtk_combo_box_text_new();
  if (mysql_query(con, "SELECT name FROM contacts")) { finish_with_error(con); }
  result = mysql_store_result(con);
  if (result == NULL) { finish_with_error(con); }
  while ((row = mysql_fetch_row(result))) {
      gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, row[0]);
  }
  gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0); // ADDING COMBOBOX

  // TABLE
  table = gtk_grid_new();
  label1 = gtk_label_new("Name");
  label2 = gtk_label_new("Message");
  gtk_grid_attach(GTK_GRID(table), label1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(table), label2, 0, 1, 1, 1);
  entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(table), view, 1, 0 ,1, 1);
  gtk_grid_attach(GTK_GRID(table), entry, 1, 1, 1, 1);
  gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0); // ADDING TABLE

  // BUTTON
  btn = gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(btn, 70, 30);
  gtk_box_pack_start(GTK_BOX(vbox), btn, TRUE, TRUE, 0); // ADDING BUTTON
  
  // ADDING BOX TO WINDOW
  gtk_container_add(GTK_CONTAINER(window), vbox);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), G_OBJECT(window));
  g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_selected), NULL);
  g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(button_clicked), NULL);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}


