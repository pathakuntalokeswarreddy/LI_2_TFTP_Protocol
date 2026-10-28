#include "../header.h"
#include "1.client.h"

int get(int *server_fd, struct sockaddr_in *server, int MODE, int MODE_BETWEEN)
{ 
    char filename[50];

    /* clearing leftover data in stdin stream, if exist */
    __fpurge(stdin);

    printf("Enter text file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL)
    {
        printf("Error reading filename\n");
        return FAILURE;
    }

    /* Remove trailing newline */
    filename[strcspn(filename, "\n")] = '\0';

    if( validate_file(filename) == FAILURE )
        return FAILURE;

    

    /* Prepare RRQ packet */
    tftp_packet client_st;
    
    memset(&client_st, 0, sizeof(client_st));

    client_st.opcode = RRQ;

    strncpy(client_st.body.request.filename, filename,
            sizeof(client_st.body.request.filename) - 1);

    client_st.body.request.mode = MODE;
    client_st.body.request.mode_between = MODE_BETWEEN;
    
    
    /* Send GET request */
    if (sendto(*server_fd, &client_st, sizeof(client_st), 0,
               (struct sockaddr *)server, sizeof(*server)) == -1)
    {
        perror("sendto RRQ failed");
        return FAILURE;
    }
    printf("GET request sent: %s\n", filename);



    /* Wait for server's ready ACK */
    tftp_packet response = {0};

    int recv_len = recvfrom(*server_fd, &response, sizeof(response), 0, NULL, NULL);

    if (recv_len == -1)
    {
        perror("recvfrom failed");
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
        return FAILURE;
    }

    printf("Server is ready. Receiving data...\n");


    /* Open existing file and truncate it, or create a new file. */
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    uint16_t expected_packet = 1;

    while (1)
    {
        // sleep(1);
        tftp_packet data = {0};

        int recv_len = recvfrom(*server_fd, &data, sizeof(data), 0, NULL, NULL);

        if (recv_len == -1)
        {
            perror("recvfrom DATA failed");
            close(fd);
            return FAILURE;
        }

        uint16_t data_size = ntohs(data.body.data_packet.size);
        if (data.opcode == DATA && data_size == 0)
        {
            uint16_t packet_no = ntohs(data.body.data_packet.packet_no);
            uint16_t data_size = ntohs(data.body.data_packet.size);

            tftp_packet complete = {0};

            complete.opcode = ACK;
            complete.body.ack_packet.packet_no = htons(packet_no);
            complete.body.ack_packet.ack_mode = htons(ACK_COMPLETE);

            sendto(*server_fd, &complete, sizeof(complete), 0,
                   (struct sockaddr *)server, sizeof(*server));
            
            // printf("C Sent P No: %d %d\n",data.body.data_packet.packet_no, packet_no);
            // printf("C Sent A Sz: %d %d\n",data.body.data_packet.size, data_size);

            
            printf("\nPackets Received: %d\n", expected_packet-1);
            printf("File transfer completed: %s\n\n", filename);

            close(fd);
            return SUCCESS;
        }

        if( data.opcode == DATA )
        {
            /*
             * A DATA packet must contain the opcode, packet number,
             * and size. Its payload may contain 0 to 512 bytes.
             */
            size_t header_size = sizeof(data.opcode) +
                                 sizeof(data.body.data_packet.packet_no) +
                                 sizeof(data.body.data_packet.size);

            if ((size_t)recv_len < header_size)
            {
                printf("Invalid DATA packet received\n");
                continue;
            }

            uint16_t packet_no = ntohs(data.body.data_packet.packet_no);
            uint16_t data_size = ntohs(data.body.data_packet.size);

            /* Verify actual received packet size */
            size_t received_data_size = data_size; // strlen(data.body.data_packet.data) ;  

            // printf("packet_no: %d\n",packet_no);
            // printf("data_size: %d\n",data_size);
            // printf("RCV PY LD: %ld\n",received_data_size);

            if (data_size == 0)
            {
                goto retrasmit;
            }

            if ( !(received_data_size == data_size) )
            {
                printf("Packet size mismatch. Expected packet %u\n", expected_packet);
                goto retrasmit;
            }

            /* Do not write duplicate or out-of-order packets */
            if (packet_no != expected_packet)
            {
                printf("Unexpected packet number %u; expected %u\n",
                    packet_no, expected_packet);

                goto retrasmit;
            }

            /*
             * Write the complete payload.
             * write() may write fewer bytes than requested, so loop.
             */
            size_t total_written = 0;
            while (total_written < data_size)
            {
                ssize_t written = write(fd,
                                        data.body.data_packet.data + total_written,
                                        data_size - total_written);

                if (written == -1)
                {
                    perror("write failed");
                    close(fd);
                    return FAILURE;
                }

                if (written == 0)
                {
                    printf("Write returned zero bytes\n");
                    close(fd);
                    return FAILURE;
                }

                total_written += (size_t)written;
            }

            /* Acknowledge the accepted packet */
            tftp_packet success_ack = {0};

            success_ack.opcode = ACK;
            success_ack.body.ack_packet.packet_no = htons(packet_no);
            success_ack.body.ack_packet.data_size = htons(data_size);
            success_ack.body.ack_packet.ack_mode  = htons(ACK_SUCCESS);

            if (sendto(*server_fd, &success_ack, sizeof(success_ack), 0,
                      (struct sockaddr *)server, sizeof(*server)) == -1)
            {
                perror("sendto DATA ACK failed");
                close(fd);
                return FAILURE;
            }

            /* Current packet accepted */
            // printf("Packet %u received: %u bytes\n", packet_no, data_size);

            expected_packet++;
            continue;
            
            /* goto lable for re-transmission ACK */
            retrasmit:
                tftp_packet retry = {0};

                retry.opcode = ACK;
                retry.body.ack_packet.packet_no = htons(expected_packet);
                retry.body.ack_packet.data_size = htons(received_data_size);
                retry.body.ack_packet.ack_mode = htons(ACK_DATA_RETRANSMIT);

                sendto(*server_fd, &retry, sizeof(retry), 0,
                        (struct sockaddr *)server, sizeof(*server));
                
                printf("Asking to retransmit\n");
        }
        else
            printf("Invalid DATA packet received\n");
    }
}