#ifndef SERVER
#define SERVER

#include<fcntl.h>

int connect_to_server(int *server_fd, struct sockaddr_in *server);

int receive_put(int server_fd, struct sockaddr_in *client,
                socklen_t client_addr_len, tftp_packet *request);

int send_get(int server_fd, struct sockaddr_in *client,
                socklen_t client_addr_len, tftp_packet *request);

int validate_file(char *filename);

size_t process_data(char *block, size_t bytes_read, int MODE_BETWEEN);

#endif