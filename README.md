# TFTP Protocol Implementation in C

A client-server file transfer application implemented in **C using UDP sockets**, enabling file transfers between different systems over a network.

The project implements core file transfer operations inspired by the **Trivial File Transfer Protocol (TFTP)**, including request handling, data packet transmission, acknowledgements, and retransmission handling.

## Features

* **Client-Server Communication:** Establishes communication between a client and server using UDP sockets.
* **File Upload (PUT):** Transfers files from the client to the server.
* **File Download (GET):** Retrieves files from the server to the client.
* **File Validation:** Validates filenames and checks file availability before transfer.
* **Packet-Based Transfer:** Divides file contents into data blocks for transmission.
* **Acknowledgement Mechanism:** Uses ACK packets to track successful data reception.
* **Retransmission Handling:** Requests retransmission when a received data packet is invalid or unexpected.
* **Transfer Modes:** Supports transfer-mode handling, including binary (`octet`) and text-oriented (`netascii`) modes.
* **Cross-Platform Testing:** Successfully tested file transfers between Windows, Linux, and macOS.

## Supported File Types Tested

The following file types have been tested successfully:

| File Extension | File Type      |
| -------------- | -------------- |
| `.txt`         | Text documents |
| `.bmp`         | Bitmap images  |
| `.mp3`         | Audio files    |

The application transfers file contents as data blocks, allowing it to handle both text and binary files.

## Technologies Used

* **Programming Language:** C
* **Networking:** UDP sockets
* **System Interfaces:** Socket API and file I/O operations
* **Data Handling:** Structures, unions, buffers, and packet-based communication
* **Development Environment:** GCC and Linux-based development tools

## Protocol Workflow

### 1. Connection Establishment

The client sends a connection request to the server. The server responds to establish communication.

### 2. File Upload — PUT

1. The client accepts a filename and validates the input.
2. The client sends a Write Request (WRQ) to the server.
3. The server checks the request and prepares to receive the file.
4. The client divides the file into data blocks and transmits them.
5. The server receives the data blocks and acknowledges successful reception.
6. The transfer completes after the final data packet is acknowledged.

### 3. File Download — GET

1. The client accepts a filename and validates the input.
2. The client sends a Read Request (RRQ) to the server.
3. The server checks whether the requested file is available.
4. The server reads the file in blocks and sends DATA packets to the client.
5. The client receives the blocks, writes the contents to a local file, and acknowledges successful reception.
6. The transfer completes after the final data packet is acknowledged.

### 4. Error Handling

The application handles invalid requests, unavailable files, packet-number mismatches, and unsuccessful socket or file operations.

## Concepts Demonstrated

This project provides practical experience with:

* UDP socket programming using `socket()`, `bind()`, `sendto()`, and `recvfrom()`
* Client-server communication
* Network byte order using `htons()` and `ntohs()`
* Packet structures and data serialization
* File handling using standard C I/O and file descriptors
* Data block sequencing and acknowledgements
* Error handling and retransmission logic
* Cross-platform network communication

## Cross-Platform Testing

File transfers were tested between systems running:

* Windows
* Linux
* macOS

Successful transfers were verified using `.txt`, `.bmp`, and `.mp3` files.

Testing across different operating systems helped validate communication between independent systems connected through a network.

## Learning Outcomes

* Gained practical experience implementing a file transfer protocol in C.
* Understood how UDP sockets support application-level communication.
* Learned to design request, data, acknowledgement, and error packets.
* Implemented file transfer operations for both text and binary data.
* Explored packet sequencing, transfer completion, and retransmission handling.
* Tested network communication between different operating systems.

## Limitations and Security

This is an educational project, not a production-ready file transfer service.

* UDP does not guarantee packet delivery, ordering, or duplicate protection by itself; the application implements its own acknowledgement and retransmission logic.
* Transfer reliability depends on the implemented protocol and its error-handling behavior.
* The application does not provide encryption or authentication. Use it only on trusted networks.
* The implementation should not be assumed to be fully interoperable with standard TFTP clients and servers.

## Repository

**Project:** [LI_2_TFTP_Protocol](https://github.com/pathakuntalalokeswarreddy/LI_2_TFTP_Protocol)

---

*Developed as a hands-on project to understand network programming, UDP communication, and file transfer protocol implementation in C.*
