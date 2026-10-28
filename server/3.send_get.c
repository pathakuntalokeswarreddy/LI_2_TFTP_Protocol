#include "../header.h"
#include "1.server.h"

int send_get(int server_fd, struct sockaddr_in *client,
                socklen_t client_addr_len, tftp_packet *request)
{
    char filename[256]={0};

    strncpy(filename, request->body.request.filename,sizeof(filename) - 1);

    filename[strlen(filename)] = '\0';

    if (filename[0] == '\0')
    {
        printf("Invalid filename\n");
        return FAILURE;
    }
    printf("GET request received for: %s\n", filename);

    if( validate_file(filename) == FAILURE )
        return FAILURE;

    /* Open source file */
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Unable to open file");
        
        tftp_packet error = {0};
        error.opcode = ERROR;
        
        error.body.error_packet.error_code = htons(1);

        strncpy(error.body.error_packet.error_msg, "Destination file open failed",
                sizeof(error.body.error_packet.error_msg) - 1);

        sendto(server_fd, &error, sizeof(error), 0,
               (struct sockaddr *)client, client_addr_len);

        return FAILURE;
    }
        

    /* Check whether file is empty */
    if (fgetc(fp) == EOF)
    {
        printf("File is empty or cannot be read\n");
        fclose(fp);
        fp = NULL;
        tftp_packet error = {0};
        error.opcode = ERROR;
        
        error.body.error_packet.error_code = htons(1);

        strncpy(error.body.error_packet.error_msg, "Destination file open failed",
                sizeof(error.body.error_packet.error_msg) - 1);

        sendto(server_fd, &error, sizeof(error), 0,
               (struct sockaddr *)client, client_addr_len);

        return FAILURE;
    }

    /* Reset file position to the beginning */
    rewind(fp);


    /* Tell client that the server is ready */
    tftp_packet ack = {0};

    ack.opcode = ACK;
    ack.body.ack_packet.packet_no = htons(0);
    ack.body.ack_packet.ack_mode = htons(ACK_READY);

    if (sendto(server_fd, &ack, sizeof(ack), 0,
               (struct sockaddr *)client, client_addr_len) == -1)
    {
        perror("sendto ready ACK failed");
        fclose(fp);
        return FAILURE;
    }

    uint16_t MODE = request->body.request.mode;
    uint16_t MODE_BETWEEN = request->body.request.mode_between;
    
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
            if (sendto(server_fd, &data, sizeof(data), 0,
                       (struct sockaddr *)client, client_addr_len) == -1)
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

                int recv_len = recvfrom(server_fd, &ack, sizeof(ack), 0, 
                                        (struct sockaddr *)client, &client_addr_len);
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
                    printf("\nPackets Received: %d\n", packet_number);
                    printf("File transfer completed: %s\n\n", filename);
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
