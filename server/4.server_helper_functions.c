#include "../header.h"
#include "1.server.h"

#include <ctype.h>


int connect_to_server(int *server_fd, struct sockaddr_in *server)
{
    *server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*server_fd == -1)
    {
        perror("Error:");
        return FAILURE;
    }

    server->sin_family = AF_INET;
    server->sin_port = htons(5001);
    server->sin_addr.s_addr = inet_addr("127.0.0.1");

    if( bind(*server_fd, (struct sockaddr *)server, sizeof(*server)) == -1)
    {
        perror("Error in bind():");
        return FAILURE;
    }

    printf("Server is Connected to port 5001\n");
    return SUCCESS;
}

int validate_file(char *filename)
{
    /* Validate filename */
    if (filename[0] == '\0')
    {
        printf("Filename cannot be empty\n");
        return FAILURE;
    }
    
    char *temp = strstr(filename, ".txt");
    if(temp == NULL)
    {
        temp = strstr(filename, ".bmp");
        if(temp == NULL)
        {
            temp = strstr(filename, ".mp3");
            if(temp == NULL)
            {
                printf("File should have either .txt or .bmp or .mp3 extension\n");
                return FAILURE;
            }
        }
    }

    if(temp == filename)
    {
        printf("File should not start with dot(.)\n");
        return FAILURE;
    }

    if( *(temp+4) != '\0')
    {
        printf("No characters should exist after .txt extenstion\n");
        return FAILURE;
    }

    /* File is ready to read */
    printf("File validated successfully: %s\n", filename);
    return SUCCESS;
}

size_t process_data(char *block, size_t bytes_read, int MODE_BETWEEN)
{
    char temp_block[bytes_read];
    memmove(temp_block,block,bytes_read);
    memset(block,0,bytes_read);

    int i=0,j=0;

    switch(MODE_BETWEEN)
    {
        case W_TO_L :
        {
            while(i<bytes_read)
            {
                if(temp_block[i] == '\r')
                    i++;
                else
                    block[j++]=temp_block[i++];
            }
            break;
        }
        case W_TO_M :
        {
            while(i<bytes_read)
            {
                if(temp_block[i] == '\n')
                    i++;
                else
                    block[j++]=temp_block[i++];
            }
            break;
        }
        case L_TO_W :
        {
            while(i<bytes_read)
            {
                if(temp_block[i] == '\n')
                {
                    block[j++]='\r';
                    block[j++]=temp_block[i++];
                }
                else
                    block[j++]=temp_block[i++];
            }
            break;
        }
        case L_TO_M :
        {
            while(i<bytes_read)
            {
                if(temp_block[i] == '\n')
                {
                    block[j++]='\r';
                    i++;
                }
                else
                    block[j++]=temp_block[i++];
            }
            break;
        }
        case M_TO_W :
        {
            while(i<bytes_read)
            {
                if(temp_block[i] == '\r')
                {
                    block[j++]=temp_block[i++];
                    block[j++]='\n';
                }    
                else
                    block[j++]=temp_block[i++];
            }
            break;
        }
        case M_TO_L :
        {
            while(i<bytes_read)
            {
                if(temp_block[i] == '\r')
                {
                    block[j++]='\n';
                    i++;
                }
                else
                    block[j++]=temp_block[i++];
            }
            break;
        }
    }
    return j;
}