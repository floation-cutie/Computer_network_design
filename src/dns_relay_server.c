#include "dns_relay_server.h"
#include "config.h"

/**
 * @param sock 服务器socket
 * @param buf 转发使用的缓冲区
 * @param len 转发的最大长度
 */
void forward_dns_request(RAII_Socket sock, unsigned char *buf, int len) {
    // 转发DNS请求
    address_t remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(remote_dns); // 远程DNS服务器地址
    remote_addr.sin_port = htons(DNS_PORT);              // DNS服务器端口号
    if (debug_mode)
        puts("------- FORWARDING DNS request --------");
    if (socket_send(sock, &remote_addr, buf, len) != 0) {
        return;
    }
    if (debug_mode) {
        debug_dns_msg_by_bytestream(buf);
    }
    printf("Forwarded DNS request to remote server successfully\n");
    puts("---------------------");
}

void forward_dns_response(RAII_Socket sock, unsigned char *buf, int len, address_t clientAddr) {
    if (debug_mode) {
        puts("------- FORWARDING DNS response --------");
    }
    if (socket_send(sock, &clientAddr, buf, len) != 0) {
        return;
    } else {
        printf("Forwarded DNS response to client successfully\n");
        puts("---------------------");
    }
}

unsigned char *find_ip_in_cache(struct Trie *trie, struct Cache *cache, const unsigned char *domain) {
    unsigned char ipAddr[16];
    unsigned char *ipAddress = NULL;

    // 先在缓存表中查找,找到返回
    if (findEntry(cache, domain, ipAddr, TYPE_A)) {
        printf("Search successfully in cache table!\n,\
                domain: %s,\
                IPv4 address: %d.%d.%d.%d\n",
               domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
        ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 4);
        memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 4);
        ipAddress[4] = '\0';
    } else if (findEntry(cache, domain, ipAddr, TYPE_AAAA)) {
        printf("在缓存表查找成功,域名为%s,IPv6地址为%d.%d.%d.%d.%d.%d.%d.%d\n", domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], ipAddr[4], ipAddr[5], ipAddr[6], ipAddr[7]);
        ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 16);
        memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 16);
    } else {
        // 如果在本地表中找到了记录,将其添加到缓存表中
        int node = findNode(trie, domain);
        if (node != 0) {
            memcpy(ipAddr, trie->toIp[node], sizeof(ipAddr));
            addEntry(cache, domain, ipAddr, 1, CACHE_TTL);
            printf("在本地字典树查找成功,域名为%s,IP地址为%d.%d.%d.%d\n", domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
            ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 5);
            memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 5);
            ipAddress[4] = '\0';
        } else // 本地表和缓存表都没有找到,需要转发到远程DNS服务器
        {
            printf("本地表和缓存表都未查找到域名%s,需要访问远程DNS服务器\n", domain);
            return NULL;
        }
    }
    return ipAddress;
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