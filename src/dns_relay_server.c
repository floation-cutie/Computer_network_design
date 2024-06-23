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

unsigned char *find_ip_in_cache(const unsigned char *domain, int *ip_type) {
    unsigned char ipAddr[16];
    unsigned char *ipAddress = NULL;
    // 先在缓存表中查找,找到返回
    if (findEntry(cache, domain, ipAddr, TYPE_A)) {
        printf("Search successfully in cache table!\n,\
                domain: %s,\
                IPv4 address: %d.%d.%d.%d\n",
               domain, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
        ipAddress = (unsigned char *)malloc(sizeof(unsigned char) * 4);
        *ip_type = TYPE_A;
        memcpy(ipAddress, ipAddr, sizeof(unsigned char) * 4);
    } else if (findEntry(cache, domain, ipAddr, TYPE_AAAA)) {
        // 16字节地址
        printf("Search successfully in cache table!\n,\
                domain: %s,\
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
            addEntry(cache, domain, ipAddr, 1, CACHE_TTL);
            printf("Search successfully in local dictionary tree!\n,\
                    domain: %s,\
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
void send_dns_response(RAII_Socket sock, DNS_MSG *msg, address_t clientAddr) {
    unsigned char *bytestream = dnsmsg_to_bytestream(msg);
    int len;
    // 计算bytestream的长度: 原因是动态分配的内存无直接计算长度的方法
    DNS_MSG *temp = bytestream_to_dnsmsg(bytestream, (unsigned short *)(&len));
    if (debug_mode) {
        puts("------- SENDING DNS response --------");
    }
    if (socket_send(sock, &clientAddr, bytestream, len) != 0) {
        return;
    } else {
        printf("SEND DNS response to client successfully\n");
        puts("---------------------");
    }
    releaseMsg(temp);
    free(bytestream);
}

/**
 * @brief
 * 首先查找本地是否有缓存，如果有则直接返回，
 * 否则构造ID映射表，转发请求到远程DNS服务器
 * @param sock 服务器socket
 * @param clientAddr 客户端地址
 * @param msg DNS消息
 * @param len 消息长度
 * @return void
 */
void handle_client_request(RAII_Socket sock, address_t clientAddr, DNS_MSG *msg, int len) {
    unsigned char *ipAddress = NULL;
    // 将DNS消息中的域名转换为缓存对应格式
    unsigned char domain[256];
    getDomain(msg->question->qname, domain);
    printf("Received DNS request from client, domain: %s\n", domain);
    int ip_type = -1;
    if ((ipAddress = find_ip_in_cache(domain, &ip_type)) != NULL && (ip_type == msg->question->qtype)) {
        // 如果缓存内容与查询要求版本一致
        // 直接对DNS请求进行扩充
        puts("Find the domain in cache table, directly send the response to client");
        addAnswer(msg, ipAddress, ip_type, CACHE_TTL);
        send_dns_response(sock, msg, clientAddr);
        return;
    } else {
        // 如果缓存内容与查询要求版本不一致
        // 构造ID映射,转发请求到远程DNS服务器
        unsigned short original_id = msg->header->id;
        unsigned short relay_id = generate_unique_id();
        // 将ID映射关系添加到映射表中
        add_id_mapping(original_id, relay_id, clientAddr);
        // 修改DNS消息头部
        msg->header->id = htons(relay_id);
        // 将DNS消息转换为字节流
        unsigned char buf[1024];
        unsigned short offset = 0;
        dnsmsg_to_bytestream(msg, buf, &offset);
        // 转发DNS请求
        forward_dns_request(sock, buf, offset);
    }
}