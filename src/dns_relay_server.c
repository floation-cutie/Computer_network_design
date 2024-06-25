#include "dns_relay_server.h"
#include "config.h"
#define MAX_DNS_PACKET_SIZE 1024 // DNS请求报文最大长度

extern int debug_mode;

// 通过线程池并发处理DNS请求
void handle_dns_request(struct Trie *trie, struct Cache *cache, SOCKET sock, struct sockaddr_in clientAddr) {
    unsigned short offset = 0;
    // 解析DNS报文
    while (1) {
        unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DNS_PACKET_SIZE);
        // 接收来自用户端的DNS请求字节流
        int clientAddrLen = sizeof(clientAddr);
        int len = recvfrom(sock, (char *)buf, MAX_DNS_PACKET_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (len == SOCKET_ERROR) {
            if (WSAGetLastError() == 10054) {
                puts("the client offline\n");
            }
            continue;
        }
        Dns_Msg *msg = NULL;
        offset = 0;
        msg = bytestream_to_dnsmsg(buf, &offset);
        debug_mode == 1 ? bytestreamInfo(buf) : (void)0; // 打印DNS请求报文的字节流
        debug_mode == 1 ? debug(msg) : (void)0;          // 打印DNS请求报文的结构体

        if (msg->header->qr == HEADER_QR_QUERY && msg->header->opcode == HEADER_OPCODE_QUERY) // 只处理DNS请求报文
        {
            unsigned char *domain = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DOMAIN_LENGTH);
            transDN(msg->question->qname, domain); // 取出域名
            printf("Receving message from %s:%d,the length is %d bytes.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), len);
            int ip_type = -1;
            unsigned char *ipAddress = findIpAddress(trie, cache, domain, &ip_type); // 查找域名对应的IP地址
            if ((ipAddress != NULL && (ip_type == msg->question->qtype))
                || (ip_type == TYPE_A && (*(unsigned int *)ipAddress == 0))) // 如果找到了,则发送DNS响应报文
            {
                printf("Find in cache!\n");
                addAnswer(msg, ipAddress, CACHE_TTL, ip_type); // 将IP地址添加到DNS响应报文中
                printf("Sending DNS reponse for domain %s...\n", domain);
                if (debug_mode)
                    debug(msg);
                // puts("Always ready to send DNS response");
                free(domain);
                send_dns_response(sock, msg, buf, clientAddr); // 发送DNS响应报文
            } else                                             // 如果没找到,则转发DNS请求报文给远程DNS服务器
            {
                puts("The default entrance for DNS message");
                if (debug_mode)
                    debug(msg);
                // 将id和客户端绑定,产生新的id
                unsigned short newId = trans_port_id(msg->header->id, clientAddr);
                buf[0] = newId >> 8;
                buf[1] = newId;
                free(domain);
                forward_dns_request(sock, buf, len); // 转发DNS请求报文给远程DNS服务器
            }

        } else if (msg->header->qr == HEADER_QR_ANSWER) // 处理从远程DNS服务器返回的DNS响应报文
        {
            puts("Receving message from remote DNS server");
            unsigned char *domain = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DOMAIN_LENGTH);
            unsigned char *ipAddr = malloc(sizeof(unsigned char) * MAX_DOMAIN_LENGTH);
            unsigned int ttl;
            unsigned short type;
            getDN_IP(buf, domain, ipAddr, &ttl, &type);
            printf("The type is %d\nThe domain is %s\n", type, domain);
            if (type == TYPE_A) {
                printf("Receiving from remote DNS server about DNS message\n\
                domain: %s\
                IPv4 address: %d.%d.%d.%d\n",
                       domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
                addEntry(cache, domain, ipAddr, type, ttl);
            } else if (type == TYPE_AAAA) {
                printf("Receiving from remote DNS server about DNS message\n\
                domain: %s,\
                IPv6 address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                       domain, ipAddr[0], ipAddr[1], ipAddr[2],
                       ipAddr[3], ipAddr[4], ipAddr[5], ipAddr[6],
                       ipAddr[7], ipAddr[8], ipAddr[9], ipAddr[10],
                       ipAddr[11], ipAddr[12], ipAddr[13], ipAddr[14], ipAddr[15]);
                addEntry(cache, domain, ipAddr, type, ttl);
            }
            // 迭代查询
            if (type == TYPE_CNAME) {
                transDN(ipAddr, domain);
                printf("\nThe CNAME domain is %s\n", domain);
                unsigned short temp_offset = 0;
                Dns_Msg *temp = bytestream_to_dnsmsg(buf, &temp_offset);
                temp->header->qr = HEADER_QR_QUERY;
                temp->header->opcode = HEADER_OPCODE_QUERY;
                temp->header->qdcount = 1;
                temp->header->ancount = 0;
                temp->header->nscount = 0;
                temp->header->arcount = 0;
                memcpy(temp->question->qname, ipAddr, strlen(ipAddr) + 1);
                unsigned char *cname_buf = dnsmsg_to_bytestream(temp);
                unsigned short sendLen = 0;
                Dns_Msg *temp2 = bytestream_to_dnsmsg(cname_buf, &sendLen);
                // printf("The send about cname is %d length\n", sendLen);
                free(ipAddr);
                releaseMsg(temp);
                releaseMsg(temp2);
                // 构造新的转发包向服务器进行通信
                forward_dns_request(sock, cname_buf, sendLen);
                free(domain);
                free(cname_buf);
            } else {
                const struct sockaddr_in result = find_clientAddr(msg->header->id); // 通过id找到客户端地址
                unsigned short original_id = find_id(msg->header->id);              // 通过id找到原始id
                buf[0] = original_id >> 8;
                buf[1] = original_id;
                free(ipAddr);
                free(domain);
                forward_dns_response(sock, buf, len, result); // 转发DNS响应报文给用户端
            }
        } else // 直接转发DNS报文给远程DNS服务器
        {
            unsigned short newId = trans_port_id(msg->header->id, clientAddr);
            buf[0] = newId >> 8;
            buf[1] = newId;
            forward_dns_request(sock, buf, len); // 转发DNS请求报文给远程DNS服务器
        }

        removeExpiredEntries(cache); // 每次处理完一个DNS请求,删除过期的缓存记录
        releaseMsg(msg);             // 释放DNS报文
        free(buf);
    }
}

unsigned char *findIpAddress(struct Trie *trie, struct Cache *cache, unsigned char domain[MAX_DOMAIN_LENGTH], int *ip_type) {
    unsigned char ipAddr[16];
    unsigned char *ipAddress = NULL;
    // 先在缓存表中查找,找到返回
    if (findEntry(cache, domain, ipAddr, TYPE_A)) {
        printf("Search successfully in cache table!\n\
                domain: %s\
                IPv4 address: %d.%d.%d.%d\n",
               domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
        ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 4);
        *ip_type = TYPE_A;
        memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 4);
    } else if (findEntry(cache, domain, ipAddr, TYPE_AAAA)) {
        // 16字节地址
        printf("Search successfully in cache table!\n\
                domain: %s\
                IPv6 address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               domain, ipAddr[0], ipAddr[1], ipAddr[2],
               ipAddr[3], ipAddr[4], ipAddr[5], ipAddr[6],
               ipAddr[7], ipAddr[8], ipAddr[9], ipAddr[10],
               ipAddr[11], ipAddr[12], ipAddr[13], ipAddr[14], ipAddr[15]);
        ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 16);
        *ip_type = TYPE_AAAA;
        memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 16);
    } else {
        // 如果在本地表中找到了记录,将其添加到缓存表中
        int node = findNode(trie, domain);
        if (node != 0) {
            memcpy(ipAddr, trie->toIp[node], sizeof(ipAddr));
            if (*(unsigned int *)ipAddr != 0) {
                // 如果字典树中的记录为0,则不缓存
                addEntry(cache, domain, ipAddr, TYPE_A, CACHE_TTL);
            }
            printf("Search successfully in local dictionary tree!\n\
                    domain: %s\
                    IPv4 address: %d.%d.%d.%d\n",
                   domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
            ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 4);
            *ip_type = TYPE_A;
            memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 4);
        } else {
            printf("Can't find the domain (%s) in cache table or local dictionary tree\n", domain);
            return NULL;
        }
    }
    return ipAddress;
}

// 向用户端发送DNS响应报文
void send_dns_response(int sock, Dns_Msg *msg, unsigned char *buf, struct sockaddr_in clientAddr) {
    unsigned char *bytestream = dnsmsg_to_bytestream(msg);
    unsigned short len = 0;
    // 计算bytestream的长度
    // 考虑到int 和 unsigned short 的长度不同，会出现相关问题
    Dns_Msg *temp = bytestream_to_dnsmsg(bytestream, (&len));
    // printf("The length of bytestream is %d\n", len);
    int ret = sendto(sock, (const char *)bytestream, len, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    if (ret == SOCKET_ERROR)
        printf("send failed with error: %d\n", WSAGetLastError());
    else {
        debug_mode == 1 ? bytestreamInfo(bytestream) : (void)0; // 打印bytestream信息
        debug_mode == 1 ? debug(msg) : (void)0;                 // 打印DNS报文信息
        puts("Send DNS response successfully");
    }
    releaseMsg(temp);
    free(bytestream);
}

// 转发DNS请求报文给远程DNS服务器
void forward_dns_request(int sock, unsigned char *buf, int len) {
    struct sockaddr_in remoteAddr;
    memset(&remoteAddr, 0, sizeof(struct sockaddr_in));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.s_addr = inet_addr(remote_dns); // 远程DNS服务器地址
    remoteAddr.sin_port = htons(53);                    // DNS服务器端口号

    // 向远程DNS服务器发送DNS请求报文
    int ret = sendto(sock, (char *)buf, len, 0, (struct sockaddr *)&remoteAddr, sizeof(struct sockaddr_in));
    if (ret == SOCKET_ERROR)
        printf("sendto failed with error: %d\n", WSAGetLastError());
    else {
        debug_mode == 1 ? bytestreamInfo(buf) : (void)0; // 打印bytestream信息
        puts("Forward DNS request to remote DNS server successfully");
    }
}

// 转发DNS响应报文给用户端
void forward_dns_response(int sock, unsigned char *buf, int len, struct sockaddr_in clientAddr) {
    int addrLen = sizeof(clientAddr);

    // 向用户端发送DNS响应报文
    int ret = sendto(sock, (const char *)buf, len, 0, (struct sockaddr *)&clientAddr, addrLen);
    if (ret == SOCKET_ERROR)
        printf("sendto failed with error: %d\n", WSAGetLastError());
    else {
        debug_mode == 1 ? bytestreamInfo(buf) : (void)0; // 打印bytestream信息
        unsigned short offset;
        Dns_Msg *msg = bytestream_to_dnsmsg(buf, &offset);
        debug_mode == 1 ? debug(msg) : (void)0; // 打印DNS报文信息
        releaseMsg(msg);
        puts("Forward DNS response to client successfully");
    }
}
