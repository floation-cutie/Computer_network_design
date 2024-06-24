#include "socket.h"

#pragma comment(lib, "Ws2_32.lib")

// 初始化Winsock
void initialize_winsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

// 初始化RAII_Socket
RAII_Socket create_socket() {
    RAII_Socket rs;
    rs.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (rs.sockfd == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        cleanup_socket(&rs);
        exit(EXIT_FAILURE);
    }

    rs.addr.sin_family = AF_INET;
    rs.addr.sin_addr.s_addr = INADDR_ANY;
    rs.addr.sin_port = htons(DNS_PORT);

    if (bind(rs.sockfd, (struct sockaddr *)&rs.addr, sizeof(rs.addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        cleanup_socket(&rs);
        exit(EXIT_FAILURE);
    }
    printf("Socket created and bound to port %d\n", DNS_PORT);

    return rs;
}

// 清理RAII_Socket
void cleanup_socket(RAII_Socket *rs) {
    closesocket(rs->sockfd);
    WSACleanup();
}

int socket_recv(RAII_Socket s, address_t *from, void *buffer, int maxlength, int *len) {
    int fromlen = sizeof(address_t);
    *len = recvfrom(s.sockfd, buffer, maxlength, 0, (struct sockaddr *)from, &fromlen);
    if (*len == SOCKET_ERROR) {
        printf("recvfrom() failed. Error Code: %d\n", WSAGetLastError());

        if (WSAGetLastError() != WSAECONNRESET) {
            cleanup_socket(&s);
            return -1;
        }
    }
    return 0;
}

int socket_send(RAII_Socket s, const address_t *to, const void *buffer, int len) {
    if (sendto(s.sockfd, buffer, len, 0, (struct sockaddr *)to, sizeof(*to)) == SOCKET_ERROR) {
        printf("sendto() failed. Error Code: %d\n", WSAGetLastError());
        cleanup_socket(&s);
        return -1;
    }
    return 0;
}