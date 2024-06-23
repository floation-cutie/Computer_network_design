#include "dns_relay_server.h"
#include "config.h"
void forward_dns_request(RAII_Socket sock, unsigned char *buf, int len) {

    int offset = 0;

    // 转发DNS请求
    address_t remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(remote_dns); // 远程DNS服务器地址
    remote_addr.sin_port = htons(DNS_PORT);              // DNS服务器端口号

    if (socket_send(sock, &remote_addr, buf, len) != 0) {
        return;
    }
    printf("Forwarded DNS request to remote server\n");
}

void forward_dns_response(RAII_Socket sock, unsigned char *buf, int len, address_t clientAddr) {
    int addr_len = sizeof(address_t);
    if (socket_send(sock, &clientAddr, buf, len) != 0) {
        return;
    }
}

void receive_from_client() {
    int nbytes = -1;
    memset(buffer, 0, sizeof(buffer));
    if (socket_recv(server_sock, &client_addr, buffer, sizeof(buffer), &nbytes) != 0) {
        return;
    }
    printf("Received %d bytes from client\n", nbytes);
    unsigned short offset = 0;
    // 解析DNS消息
    DNS_MSG *msg = bytestream_to_dnsmsg(buffer, &offset);
    if (msg == NULL) {
        printf("Failed to parse DNS message\n");
        return;
    }
    debug_time();
    debug_dns_msg(msg);
    // 转发DNS请求
    if (msg->header->qr == HEADER_QR_QUERY) {
        forward_dns_request(server_sock, buffer, nbytes);
    }
}

void receive_from_server() {
    int nbytes = -1;
    memset(buffer, 0, sizeof(buffer));
    address_t remote_addr;
    if (socket_recv(server_sock, &remote_addr, buffer, sizeof(buffer), &nbytes) != 0) {
        return;
    }
    printf("Received %d bytes from server\n", nbytes);
    unsigned short offset = 0;
    // 解析DNS消息
    DNS_MSG *msg = bytestream_to_dnsmsg(buffer, &offset);
    if (msg == NULL) {
        printf("Failed to parse DNS message\n");
        return;
    }
    debug_time();
    debug_dns_msg(msg);
    // 转发DNS响应
    if (msg->header->qr == HEADER_QR_ANSWER && remote_addr.sin_addr.s_addr == inet_addr(remote_dns)) {
        forward_dns_response(server_sock, buffer, nbytes, client_addr);
    }
}