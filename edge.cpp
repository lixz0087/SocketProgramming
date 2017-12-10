#include <string>
#include <sstream>
#include <vector>
#include <iostream>
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
using namespace std;
#define BUFFER_SIZE 9000

// make each line of input become a int vector
vector<int> strToVector(string s) {
    vector<int> vector1;
    std::stringstream ss(s);
    string tmp;

    getline(ss, tmp, ',');

    if (tmp == "and") {
        vector1.push_back(1);
    } else {
        vector1.push_back(0);
    }

    while(getline(ss, tmp, ',')) {
        int tmpInt = atoi(tmp.c_str());
        vector1.push_back(tmpInt);
    }

    return vector1;
}

int main() {
    // TCP params
    int tcpSocket, udpSocket, newSocket, length;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddress, serverOR, serverAND;
    struct addrinfo tmp, *addrinfo1;
    struct hostent *localhost;
    string s;
    string result, resultPrint;

    // Open TCP socket
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpSocket < 0) {
        perror("ERROR: can't open TCP socket");
        exit(1);
    }
    // Set TCP address
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = 23784;

    // Bind TCP socket
    if (bind(tcpSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    // Listen to client connection
    listen(tcpSocket, 5);

    // Open UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpSocket < 0) {
        perror("ERROR: can't open UDP socket");
        exit(1);
    }

    memset(&tmp, '\0', sizeof(tmp));
    tmp.ai_family = AF_INET;
    tmp.ai_socktype = SOCK_DGRAM;
    tmp.ai_flags = 0;
    getaddrinfo(NULL, "24784", &tmp, &addrinfo1);

    //Bind UDP socket with its IP addrss and port number
    bind(udpSocket, addrinfo1->ai_addr, addrinfo1->ai_addrlen);


    localhost = gethostbyname("localhost");
    if (localhost == 0) {
        perror("ERROR: unknown host");
        exit(1);
    }

    // Set the address of server_and and server_or
    serverOR.sin_family = AF_INET;
    serverAND.sin_family = AF_INET;

    bcopy(localhost -> h_addr, (char *)&serverOR.sin_addr, localhost->h_length);
    serverOR.sin_port = 21784;

    bcopy(localhost -> h_addr, (char *)&serverAND.sin_addr, localhost->h_length);
    serverAND.sin_port = 27784;

    length = sizeof (struct sockaddr_in);

    printf("The edge server is up and running.\n");

    while (1) {
        // Receive the file from client.
        newSocket = accept(tcpSocket, NULL, NULL);
        if (newSocket < 0) {
            perror ("ERROR on accept");
        }

        bzero(buffer, BUFFER_SIZE);
        if(recv(newSocket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("ERROR: can't get file");
        }

        // Read and sent each line to back server.
        string tmpstr(buffer);
        istringstream is(tmpstr);

        string line;
        vector<int> tmpV;
        int andLineNum, orLineNum;
        andLineNum = 0;
        orLineNum = 0;
        while (getline(is,line)) {
            tmpV = strToVector(line);
            // to server_and and receive the result from server_and
            if(tmpV.at(0) == 1) {
                andLineNum = andLineNum + 1;
                string s1 = to_string(tmpV[1]);
                string s2 = to_string(tmpV[2]);
                string theString = "0," + s1 + "," + s2;
                char bufferTmp[BUFFER_SIZE];
                bzero(bufferTmp, sizeof(bufferTmp));
                strcpy(bufferTmp, theString.c_str());
                if(sendto(udpSocket, bufferTmp, BUFFER_SIZE, 0, (struct sockaddr*) &serverAND, length) < 0) {
                    perror("ERROR: can't send to server AND");
                    return 0;
                }
                bzero(bufferTmp, sizeof(bufferTmp));
                recvfrom(udpSocket, bufferTmp, BUFFER_SIZE, 0, (struct sockaddr*) &serverAND, (socklen_t *) &length);
                resultPrint = resultPrint + s1 + " and " + s2 + " = " + bufferTmp + "\n";
                result = result + bufferTmp + "\n";

            // to server_or and receive the result from server_or
            } else {
                orLineNum = orLineNum + 1;
                string s1 = to_string(tmpV[1]);
                string s2 = to_string(tmpV[2]);
                string theString = "0," + s1 + "," + s2;
                char bufferTmp[BUFFER_SIZE];
                bzero(bufferTmp, sizeof(bufferTmp));
                strcpy(bufferTmp, theString.c_str());
                if(sendto(udpSocket, bufferTmp, BUFFER_SIZE, 0, (struct sockaddr*) &serverOR, length) < 0) {
                    perror("ERROR: can't send to server OR");
                    return 0;
                }
                bzero(bufferTmp, sizeof(bufferTmp));
                recvfrom(udpSocket, bufferTmp, BUFFER_SIZE, 0, (struct sockaddr*) &serverOR, (socklen_t *) &length);
                resultPrint = resultPrint + s1 + " or " + s2 + " = " + bufferTmp + "\n";
                result = result + bufferTmp + "\n";
            }
        }

        // tell the number of lines send to server_and.
        string theString = "1," + to_string(andLineNum);
        char bufferTmp[BUFFER_SIZE];
        bzero(bufferTmp, sizeof(bufferTmp));
        strcpy(bufferTmp, theString.c_str());
        if(sendto(udpSocket, bufferTmp, BUFFER_SIZE, 0, (struct sockaddr*) &serverAND, length) < 0) {
            perror("ERROR: can't send to server AND");
            return 0;
        }

        // tell the number of lines send to server_or.
        theString = "1," + to_string(orLineNum);
        bzero(bufferTmp, sizeof(bufferTmp));
        strcpy(bufferTmp, theString.c_str());
        if(sendto(udpSocket, bufferTmp, BUFFER_SIZE, 0, (struct sockaddr*) &serverOR, length) < 0) {
            perror("ERROR: can't send to server OR");
            return 0;
        }

        printf("The edge server has received %d lines from the client using TCP over port 23319\n", andLineNum + orLineNum);
        printf("The edge server has successfully sent %d lines to Backend-Server OR.\n", orLineNum);
        printf("The edge server has successfully sent %d lines to Backend-Server AND.\n", andLineNum);
        printf("The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP port 24784.\n");
        printf("The computation results are:\n");

        // Print out the result.
        printf("%s", resultPrint.c_str());

        // Send result to client.
        bzero(bufferTmp, sizeof(bufferTmp));
        strcpy(bufferTmp, result.c_str());
        if(send(newSocket, bufferTmp, BUFFER_SIZE, 0) < 0) {
            perror("ERROR: can't send result to client");
            return 0;
        }

        resultPrint = "";
        result = "";

        printf("The edge server has successfully finished receiving all computation results from the Backend-Server OR and Backend-Server AND.\n");
        printf("The edge server has successfully finished sending all computation results to the client.\n");
    }
}



