#ifndef TFTP
#define TFTP

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio_ext.h>   //for __fpurge()

#define RRQ 0
#define WRQ 1

#define SUCCESS 0
#define FAILURE 1

#define CONNECTION_REQUEST 3
#define CONNECTION_RESPONSE 4

/* Opcode Macros */
#define DATA 5
#define ACK 6
#define ERROR 7

/* ACK status values */
#define ACK_READY 0
#define ACK_SUCCESS 1
#define ACK_DATA_RETRANSMIT 2
#define ACK_COMPLETE 3


/* Modes for Transferring data */
#define NORMAL 1
#define OCTET 2
#define NETASCII 3


/* Modes to select OS */
enum { 
W_TO_L=1,   /* W - WINDOWS */
W_TO_M,     /* L - LINUX   */
            /* M - MACOS   */
L_TO_W,
L_TO_M,

M_TO_W,
M_TO_L
};


// TFTP Packet Structure
typedef struct {
    uint16_t opcode; // Operation code (RRQ/WRQ/DATA/ACK/ERROR)
    union 
    {
        struct 
        {
            char filename [256];
            uint16_t mode; // Typically "Normal"
            uint16_t mode_between;
        } request; // RRQ and WRQ

        struct
        {
            uint16_t packet_no;
            uint16_t size;
            char data [512];
        } data_packet; // DATA

        struct
        {
            uint16_t ack_mode;
            uint16_t data_size;
            uint16_t packet_no;
        }ack_packet; // ACK

        struct
        {
            uint16_t error_code;
            char error_msg[512];
        } error_packet; // ERROR
    } body;
}tftp_packet;


#endif