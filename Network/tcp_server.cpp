#include <iostream>
#include <winsock2.h>

using namespace std;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error("ERROR opening socket");
    }
    
    struct sockaddr_in address;
    memset((char *) &address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("ERROR on binding");
    }

    listen(server_fd, 3);
    cout << "Listening on port 8080: " << endl;

    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);
    SOCKET new_socket = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
    if (new_socket < 0) {
        error("ERROR on accept");
    }

    char buffer[1024];
    memset(buffer, 0, 1024);
    
    int n = recv(new_socket, buffer, 1024, 0);
    if (n < 0) {
        error("ERROR reading from socket");
    }
    cout << buffer << endl;

    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
