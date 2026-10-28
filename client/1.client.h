#ifndef CLIENT
#define CLIENT

#include<ctype.h>
#include<fcntl.h>

void menu_display(void);

void select_mode(int *MODE, int *MODE_BETWEEN);

int connect_to_server(int *server_fd, struct sockaddr_in *server);
int validate_ip(char *ip);

int put(int *server_fd, struct sockaddr_in *server, int MODE, int MODE_BETWEEN);
size_t process_data(char *block, size_t bytes_read, int MODE_BETWEEN);

int get(int *server_fd, struct sockaddr_in *server, int MODE, int MODE_BETWEEN);

int validate_file(char *filename);


#endif