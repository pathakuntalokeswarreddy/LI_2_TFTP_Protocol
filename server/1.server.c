#include "../header.h"
#include "1.server.h"


int main()
{
    int server_fd;
    struct sockaddr_in server ={0};
    struct sockaddr_in client={0};

    socklen_t client_addr_len = sizeof(client); 

    if( connect_to_server(&server_fd, &server) == FAILURE )
    {
        perror("Connection failed :");
        return FAILURE;
    }

    char buff[512];
    while(1)
    {
        tftp_packet request = {0};

        int recv_len = recvfrom(server_fd, &request, sizeof(request), 0, 
                                (struct sockaddr *)&client, &client_addr_len);
        if (recv_len == -1)
        {
            perror("recvfrom failed");
            continue;
        }
        if(recv_len == 0)
        {
            printf("Reveived Empty Datagram\n");
            continue;
        }

        else if(recv_len > 0)
        {
            /* Check for Connection Request */
            if( request.opcode == CONNECTION_REQUEST )
            {       
                printf("Connection request received from client\n");

                //sending connection Acknowledgement
                tftp_packet response = {0};
                response.opcode = CONNECTION_RESPONSE;

                if (sendto(server_fd, &response, sizeof(response), 0,
                        (struct sockaddr *)&client, client_addr_len) == -1)
                {
                    perror("sendto failed");
                    continue;
                }
                printf("Connection response sent to client\n\n");
            }
            else if( request.opcode == WRQ )
            {
                if( receive_put(server_fd, &client, client_addr_len, &request) == FAILURE)
                    printf("PUT failed\n\n");
            }
            else if( request.opcode == RRQ )
            {
                if( send_get(server_fd, &client, client_addr_len, &request) == FAILURE)
                    printf("GET failed\n\n");
            }
        }
    }
}