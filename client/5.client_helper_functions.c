#include "../header.h"
#include "1.client.h"

void menu_display(void)
{
    printf("1. Connect\n"
           "2. put (Send,    from client to server)\n"
           "3. get (Receive, from server to client)\n"
           "4. Select Mode\n"
           "5. Exit\n");
}

int validate_ip(char *ip)
{ 

    struct in_addr temp;

    /* Remove trailing newline */
    ip[strcspn(ip, "\n")] = '\0';


    /* inet_pton() is inbuilt function to validate IP address */
    int ret = inet_pton(AF_INET, ip, &temp); 
    if (ret == 1) 
    { 
        printf("Valid IPv4 address: %s\n", ip); 
        return SUCCESS; 
    } 

    return FAILURE;
}



int validate_file(char *filename)
{
    /* Validate filename */
    if (filename[0] == '\0')
    {
        printf("Filename cannot be empty\n");
        return FAILURE;
    }
    
    /* accepts only .txt, .bmp, .mp3 extentioned files */
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
        /* Windows to Linux */
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
        /* Windows to MacOS */
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
        /* Linux to Windows */
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
        /* Linux to MacOS */
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
        /* MacOS to Windows */
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
        /* MacOS to Linux */
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