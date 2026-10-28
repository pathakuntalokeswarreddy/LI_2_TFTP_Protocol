#include "../header.h"
#include "1.client.h"

int put(int *server_fd, struct sockaddr_in *server, int MODE, int MODE_BETWEEN)
{
    char filename[50];

    /* clear leftover input */
    __fpurge(stdin);

    printf("Enter text file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL)
    {
        printf("Error reading filename\n");
        return FAILURE;
    }

    /* Remove trailing newline */
    filename[strcspn(filename, "\n")] = '\0';

    /* Name Validation of file*/
    if( validate_file(filename) == FAILURE )
        return FAILURE;

    /* Open source file */
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Unable to open file");
        return FAILURE;
    }

    /* Check whether file is empty */
    if (fgetc(fp) == EOF)
    {
        printf("File is empty or cannot be read\n");
        fclose(fp);
        fp = NULL;
        return FAILURE;
    }

    /* Reset file position to the beginning */
    rewind(fp);



    /* Prepare WRQ packet */
    tftp_packet client_st;
    
    memset(&client_st, 0, sizeof(client_st));

    client_st.opcode = WRQ;

    strncpy(client_st.body.request.filename, filename,
            sizeof(client_st.body.request.filename) - 1);
    
    client_st.body.request.mode = MODE;
    client_st.body.request.mode_between = MODE_BETWEEN;


    
    /* Send PUT request */
    if (sendto(*server_fd, &client_st, sizeof(client_st), 0,
               (struct sockaddr *)server, sizeof(*server)) == -1)
    {
        perror("sendto WRQ failed");
        fclose(fp);
        return FAILURE;
    }
    printf("PUT request sent: %s\n", filename);
    


    /* Wait for server's ready ACK */
    tftp_packet response = {0};

    int recv_len = recvfrom(*server_fd, &response, sizeof(response), 0, NULL, NULL);

    if (recv_len == -1)
    {
        perror("recvfrom failed");
        fclose(fp);
        return FAILURE;
    } 

    if( recv_len < (int)sizeof(response.opcode) || 
        response.opcode == ERROR)
    {
        printf("Server sent Ack: %s\n",response.body.error_packet.error_msg);
        return FAILURE;
    }

    uint16_t ack_mode = ntohs(response.body.ack_packet.ack_mode);
    if (response.opcode != ACK ||
        response.body.ack_packet.packet_no != 0 ||
        ack_mode != ACK_READY)
    {
        printf("Server did not acknowledge the PUT request\n");
        fclose(fp);
        return FAILURE;
    }

    printf("Server is ready. Sending data...\n");


    int block_size;
    if(MODE == 1 || MODE==3) block_size = 512;
    else if(MODE == 2)       block_size = 1;


    uint16_t packet_number = 0;
    char block[block_size];
    size_t bytes_read;
    while(1)
    {
        // sleep(1);
        memset(block,0,sizeof(block));
        
        bytes_read = fread(block, 1, sizeof(block), fp);

        if(MODE == NETASCII)
        {
            bytes_read = process_data(block, bytes_read, MODE_BETWEEN);
        }
                 
        /*Formatting extracted data, if MODE is NETASCII*/
        if (bytes_read == 0 && ferror(fp))
        {
            perror("File read failed");
            fclose(fp);
            return FAILURE;
        }

        tftp_packet data = {0};

        /* Packing information to be sent */
        data.opcode = DATA;
        data.body.data_packet.packet_no = htons(packet_number + 1);
        data.body.data_packet.size = htons((uint16_t)bytes_read);
        if (bytes_read > 0)
            memcpy(data.body.data_packet.data, block, bytes_read);


        uint16_t current_packet = packet_number + 1;

        while (1)
        {
            /* Send the current packet */
            if (sendto(*server_fd, &data, sizeof(data), 0,
                       (struct sockaddr *)server, sizeof(*server)) == -1)
            {
                perror("sendto DATA failed");
                fclose(fp);
                return FAILURE;
            }
            // printf("Sent packet_number No: %d\n",packet_number+1);
            // printf("Sent bytes_read      : %ld\n",bytes_read);



            /* wait loop for acknowledgement */
            while(1)
            {
                /* Wait for ACK */
                tftp_packet ack = {0};

                recv_len = recvfrom(*server_fd, &ack, sizeof(ack), 0, NULL, NULL);
                if (recv_len == -1)
                {
                    perror("recvfrom ACK failed");
                    fclose(fp);
                    return FAILURE;
                }

                if (recv_len < (int)sizeof(ack.opcode) || ack.opcode != ACK)
                {
                    printf("Invalid ACK received. Waiting for a valid response.\n");
                    continue;
                }

                uint16_t packet_no = ntohs(ack.body.ack_packet.packet_no);
                uint16_t data_size = ntohs(ack.body.ack_packet.data_size);
                uint16_t ack_mode = ntohs(ack.body.ack_packet.ack_mode);

                // printf("Recv P No: %d\n",packet_no);
                // printf("Recv D Sz: %d\n",data_size);
                // printf("Recv A Md: %d\n",ack_mode);

                if(ack_mode == ACK_COMPLETE)
                {
                    printf("Total %d packets sent\n",packet_number);
                    printf("Data Transfer Completed\n");
                    fclose(fp);
                    return SUCCESS;
                }

                if (packet_no != current_packet)
                {
                    printf("ACK packet number mismatch. Resending packet %u\n", current_packet);
                    break;
                }
                
                /* Validate accepted data size */
                if (ack_mode == ACK_DATA_RETRANSMIT)
                {
                    printf("Server requests retransmission of packet %u\n", current_packet);
                    break;
                }

                if (ack_mode != ACK_SUCCESS || data_size != bytes_read)
                {
                    printf("Invalid ACK or size mismatch. Resending packet %u\n", current_packet);
                    break;
                }

                /* Current packet accepted */
                // printf("Packet %u acknowledged: %zu bytes\n", current_packet, bytes_read);

                packet_number++;
                break;
            }
            if( packet_number ==  current_packet)
                break;
        }
        if (bytes_read == 0)
            break;
    }
    printf("Total %d packets sent\n",packet_number);
    fclose(fp);
    return SUCCESS;
}