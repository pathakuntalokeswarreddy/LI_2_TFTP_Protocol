#include "../header.h"
#include "1.client.h"



int connect_to_server(int *server_fd, struct sockaddr_in *server)
{
    char ip[50];   /* Loop Back IP Address:"127.0.0.1" */
    __fpurge(stdin);
    printf("Enter an IP address: ");
    if (fgets(ip, sizeof(ip), stdin) == NULL)
        return 1;

    if( validate_ip(ip) == FAILURE )
    {
        printf("Invalid IP Address\n");
        return FAILURE;
    }
    
    int port;
    printf("Enter Port Number: ");
    if( scanf("%d",&port) != 1 || !(port>=1024 && port<=65535) )
    {
        printf("Invalid Port Number\n");
        return FAILURE;
    }
    else
        printf("Entered: %d\n",port);


    *server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*server_fd == -1)
    {
        perror("Error:");
        return FAILURE;
    }

    server->sin_family = AF_INET;
    server->sin_port = htons(5001);
    server->sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Initialize connection request packet */
    tftp_packet packet = {0};
    packet.opcode = CONNECTION_REQUEST;

    if (sendto(*server_fd, &packet, sizeof(packet), 0, (struct sockaddr *)server, sizeof(*server)) == -1)
    {
        perror("sendto failed");
        close(*server_fd);
        *server_fd = -1;
        return FAILURE;
    }

    printf("CONNECTION REQUEST SENT TO SERVER\n");

    /* Receive connection response */
    tftp_packet response = {0};

    int n = recvfrom(*server_fd, &response, sizeof(response), 0, NULL, NULL);
    if (n == -1)
    {
        perror("recvfrom failed");
        close(*server_fd);
        *server_fd = -1;
        return FAILURE;
    }
    else if(n>=0)
    {        
        if( response.opcode == CONNECTION_RESPONSE )
        {
            printf("Connection response received from server\n");
            printf("Server and client are Connected\n\n");
            return SUCCESS;
        }
    }
    else if( n==0)
    {
        printf("Connection closed\n");
        close(*server_fd);
        return FAILURE; 
    }   
}