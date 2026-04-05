#include <iostream>
#include <winsock2.h>

using namespace std;

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        error("ERROR opening socket");
    }

    struct hostent *server = gethostbyname("localhost");
    if (server == NULL) {
        error("ERROR, no such host");
    }

    struct sockaddr_in serv_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(8080);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }
    
    int n = send(sock, "Hello Server", 12, 0);
    if (n < 0) {
        error("ERROR writing to socket");
    }
    
    cout << "Sent: Hello Server";

    closesocket(sock);
    WSACleanup();
    return 0;
}
