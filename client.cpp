#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <strings.h>
#include <math.h>

#define BUFFER_SIZE 9000

int main(int argc, char * argv[]) {
    int tcpSocket, connectResult;
    struct sockaddr_in serverAddress;
    struct hostent *localhost;

    char buffer[BUFFER_SIZE];

    // Open a TCP socket and connect it to the edge server
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    // If an error occurred when open a socket, print the error and exit
    if (tcpSocket < 0) {
        perror("ERROR: Can't open socket\n");
        return 0;
    }

    localhost = gethostbyname("localhost");
    // If an error occurred when find the localhost, print the error and exit
    if (localhost == 0) {
        perror("ERROR: can't find host\n");
        return 0;
    }

    // Set the server address
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    memcpy(localhost->h_addr, &serverAddress.sin_addr.s_addr, localhost->h_length);
    serverAddress.sin_port = 23784;

    // Connect the TCP socket
    connectResult = connect(tcpSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if(connectResult < 0) {
        perror("ERROR: connection error");
        return 0;
    }

    printf("The client is up and running.\n");

    // Read the input file
    FILE *file;
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("ERROR: Failed to open file");
    }

    // Send the file
    bzero(buffer, BUFFER_SIZE);
    fread(buffer, sizeof(char), BUFFER_SIZE, file);
    buffer[BUFFER_SIZE - 1] = '\n';
    if (send(tcpSocket, buffer, BUFFER_SIZE, 0) < 0) {
        printf("ERROR: Send File Failed!");
    }
    bzero(buffer, sizeof(buffer));

    // counting the number of lines in the file
    rewind(file);
    int ch = 0;
    int numLineSum = 0;
    while (EOF != (ch=getc(file)))
        if (',' == ch)
            numLineSum++;

    fclose(file);

    printf("The client has successfully finished sending %d lines to the edge server.\n", numLineSum / 2);

    // Receive result
    printf("The client has successfully finished receiving all computation results from the edge server.\n");
    printf("The final computation results are:\n");
    bzero(buffer, BUFFER_SIZE);
    int receiveResult = 0;
    receiveResult = recv(tcpSocket, buffer, BUFFER_SIZE, 0);
    if(receiveResult < 0) {
        printf("ERROR: receive failed");
        return 0;
    }
    // Print out the result.
    printf("%s", buffer);

    return 0;
}