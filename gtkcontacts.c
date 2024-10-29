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
//GtkWidget *hseparator;
int argcX, DeleteLen;
GtkWidget *window; GtkWidget *table;
GtkWidget *label1; GtkWidget *label2; GtkWidget *label3;
GtkWidget *entry1; GtkWidget *entry2; GtkWidget *entry3;
GtkWidget *btn;

MYSQL *con;
MYSQL_RES *result;
MYSQL_ROW row;

char newline[20];
char co2T[200];
char tvocT[30];
char tempT[30];
char pressT[30];
char humT[30];

void combo_selected(GtkWidget *widget, gpointer window) {

	gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);
	gtk_text_buffer_get_iter_at_offset(bufferT, &iter2, DeleteLen);
	gtk_text_buffer_delete(bufferT, &iter, &iter2);

	gchar *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
	char auX[] = "SELECT phone FROM contacts WHERE name LIKE '";
	char auxZ[] = "'";
	char auY[100];
	char sdX[30];
	strcpy(auY, auX);
	strcat(auY, text);
	strcat(auY, auxZ);
  	if (mysql_query(con, auY)) { finish_with_error(con); }
  	result = mysql_store_result(con);
  	if (result == NULL) { finish_with_error(con); }
  	while (row = mysql_fetch_row(result)) {
		strcpy(sdX, row[0]);
  	}
  	gtk_text_buffer_insert(bufferT, &iter, sdX, -1);
	DeleteLen = strlen(sdX);
}

void button_clicked(GtkWidget *widget, gpointer data) {
	const gchar *text1 = gtk_entry_get_text(GTK_ENTRY(entry1));
	const gchar *text2 = gtk_entry_get_text(GTK_ENTRY(entry2));
	int c1 = strcmp(text1, "");
	int c2 = strcmp(text2, "");
	if ((c1 != 0) && (c2 != 0)){
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, text1);
		char auX[] = "INSERT INTO contacts (name, phone) VALUES ('";
		char auxZ1[] = "', '";
		char auxZ2[] = "')";
		char auY[100];
		strcpy(auY, auX);
		strcat(auY, text1);
		strcat(auY, auxZ1);
		strcat(auY, text2);
		strcat(auY, auxZ2);
		if (mysql_query(con, auY)) { finish_with_error(con); }
		gtk_entry_set_text(GTK_ENTRY(entry1), "");
		gtk_entry_set_text(GTK_ENTRY(entry2), "");
	}
}

int main(int argc, char *argv[]) {

  argcX = argc;

  //hseparator = gtk_hseparator_new();

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

  // WINDOWS
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Contacts");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  // BOX
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  // VIEW
  view = gtk_text_view_new();
  bufferT = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
  gtk_text_buffer_get_iter_at_offset(bufferT, &iter, 0);

  gtk_text_buffer_insert(bufferT, &iter, co2T, -1);
  DeleteLen = 1;

  //COMBO
  combo = gtk_combo_box_text_new();
  if (mysql_query(con, "SELECT name FROM contacts")) { finish_with_error(con); }
  result = mysql_store_result(con);
  if (result == NULL) { finish_with_error(con); }
  while ((row = mysql_fetch_row(result))) {
      gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, row[0]);
  }

  gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0); // ADDING COMBO

  gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);  // ADDING VIEW

  // TABLE
  table = gtk_grid_new();
  label1 = gtk_label_new("Name");
  label2 = gtk_label_new("Phone");
  gtk_grid_attach(GTK_GRID(table), label1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(table), label2, 0, 1, 1, 1);
  entry1 = gtk_entry_new();
  entry2 = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(table), entry1, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(table), entry2, 1, 1, 1, 1);
  gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0); // ADDING TABLE

  // BUTTON
  btn = gtk_button_new_with_label("Add");
  gtk_widget_set_size_request(btn, 70, 30);
  gtk_box_pack_start(GTK_BOX(vbox), btn, TRUE, TRUE, 0); // ADDING BUTTON

  gtk_container_add(GTK_CONTAINER(window), vbox);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), G_OBJECT(window));//NULL);
  g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_selected), NULL);
  g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(button_clicked), NULL);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}


