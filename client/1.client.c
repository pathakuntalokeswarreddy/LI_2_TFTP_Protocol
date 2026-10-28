#include "../header.h"
#include "1.client.h"


int main(void)
{
    int MODE = NORMAL;
    int MODE_BETWEEN = 0;
    int server_fd;
    struct sockaddr_in server ={0};

    int connect_flag=0;
    
    while(1)
    {
        printf("\n===============================================\n");
        menu_display();
        printf("===============================================\n");

        int choice;
        printf("Enter your choice: ");
        __fpurge(stdin);
  
        while( scanf("%d",&choice) != 1 || !(choice>0 && choice<6) )
        {
            __fpurge(stdin);
            printf("Invalid entry, Enter again : ");
        }

        switch( choice )
        {
            case 1:
            {
                /* Connect client to server */
                if( connect_flag == 0 )
                {
                    if( connect_to_server(&server_fd, &server) == SUCCESS )
                        connect_flag = 1;
                }
                else
                    printf("Server and Client are already Connected\n");
                break;                
            }
            case 2:
            {
                /* Data transfer from client to server */
                if(connect_flag == 1)
                    put(&server_fd, &server, MODE, MODE_BETWEEN);
                else
                    printf("Server and Client are not connected yet\n");  
                break;
            }
            case 3:
            {
                /* Data transfer from server to client */
                if(connect_flag == 1)
                    get(&server_fd, &server, MODE, MODE_BETWEEN);
                else
                    printf("Server and Client are not connected yet\n");  
                break;
            }
            case 4:
            {
                /* Select Mode */
                select_mode(&MODE, &MODE_BETWEEN);
                break;
            }
            case 5:
            {
                printf("Exiting....\n");
                return 0;
            }
        }
    }
}