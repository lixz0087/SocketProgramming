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
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

// make each line of input become a int vector
vector<int> strToVector(string s) {
    vector<int> vector1;
    std::stringstream ss(s);
    string tmp;

    while(getline(ss, tmp, ',')) {
        int tmpInt = atoi(tmp.c_str());
        vector1.push_back(tmpInt);
    }

    return vector1;
}

// calculate the result of AND operation.
string andR(string s1, string s2) {
    string res, fin;
    int lenMin = min((int) s1.length(), (int) s2.length());
    int lenMax = max((int) s1.length(), (int) s2.length());

    if((int) s1.length() > (int) s2.length()) {
        int len = lenMax - lenMin;
        for (int i = 0; i < len; i = i + 1) {
            s2 = "0" + s2;
        }
    } else {
        int len = lenMax - lenMin;
        for (int i = 0; i < len; i = i + 1) {
            s1 = "0" + s1;
        }
    }

    for (int i = lenMax - 1; i >= 0; i = i - 1) {
        if(s1[i] == '1' && s2[i] == '1') {
            res = "1" + res;
        } else {
            res = "0" + res;
        }
    }

    int lenRes = (int) res.length();
    for (int i = 0; i < lenRes; i = i + 1) {
        if(res[i] == '0') {
            if(i == lenRes - 1) {
                return "0";
            }
            continue;
        } else {
            while (i < lenRes) {
                fin = fin + res[i];
                i = i + 1;
            }
            break;
        }
    }
    return fin;
}

int main() {
    int udpSocket, length, n;
    struct sockaddr_in serverAddress;
    struct sockaddr_in from;
    vector<int> twoNumbers;
    char buffer[100];

    //establish UDP socket in server_and
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        perror("ERROR: can't open socket");
        return 0;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = 27784;

    //bind socket
    if (bind (udpSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("ERROR: can't binding");
        return 0;
    }
    length = sizeof(struct sockaddr_in);

    printf("The Server AND is up and running using UDP on port 27784.\n");

    printf("The Server AND start receiving lines from the edge server for AND computation.");
    printf("The computation results are:\n");

    while (1) {
        // Receive data from edge server.
        bzero(buffer, sizeof(buffer));
        n = recvfrom(udpSocket, buffer, 100, 0, (struct sockaddr *)&from, (socklen_t *) &length);
        if (n < 0) {
            perror("ERROR: can't receive from udp");
            return 0;
        }
        string tmpS(buffer);
        vector<int> tmpV = strToVector(tmpS);
        if (tmpV[0] == 1) {
            printf("The Server AND has successfully received %d lines from the edge server and finished all AND computations.\n", tmpV[1]);
            printf("The Server AND has successfully finished sending all computation results to the edge server\n");
            continue;
        }
        string res = andR(to_string(tmpV[1]).c_str(), to_string(tmpV[2]).c_str());
        printf("%d and %d = %s\n", tmpV[1], tmpV[2], res.c_str());

        // Send data to the edge server.
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, res.c_str());
        n = sendto(udpSocket, buffer, 100 , 0, (struct sockaddr *)&from, length);
        if (n < 0) {
            perror("ERROR: can't send result");
            return 0;
        }

    }
}

