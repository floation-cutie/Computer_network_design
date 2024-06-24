#include "dns_msg.h"
#include <stdlib.h>
// 用于解析 DNS 报文中的域名字段
void getName(unsigned char *qname, const unsigned char *bytestream,
             unsigned short *offset) {
    // 压缩标签常出现在Answer和Authority部分，用于指向Question部分的域名
    // 主要可能这个压缩标签会出现在不只是头部，还可能出现在中间各个位置
    unsigned short initial_offset = *offset; // 保存初始偏移量
    int count = 0;
    while (*(bytestream + *offset) != 0) {
        if (((*(bytestream + *offset) >> 6) & 3) == 3) { // 检测压缩标签（前两位为11）
            unsigned short new_offset =
                ntohs(*(unsigned short *)(bytestream + *offset)) & 0x3fff; // 获取新的偏移量
            getName(qname, bytestream, &new_offset);                       // 递归解析压缩标签指向的部分
            (*offset) += 2;                                                // 跳过压缩标签的两个字节
            return;
        }
        *qname = *(bytestream + *offset);
        qname++;
        (*offset)++;
    }
    (*offset)++;
    *qname = '\0'; // 终止字符串
}
// 获得点分十进制形式的IPv4地址 4字节
void getIPv4(unsigned char *original, unsigned char *IPv4) {
    sprintf((char *)(IPv4), "%d.%d.%d.%d", original[0], original[1], original[2], original[3]);
}

// 获得冒分十六进制形式的IPv6地址 16字节
void getIPv6(unsigned char *original, unsigned char *IPv6) {
    sprintf((char *)(IPv6), "%x:%x:%x:%x:%x:%x:%x:%x", ntohs(*(unsigned short *)(original)), ntohs(*(unsigned short *)(original + 2)),
            ntohs(*(unsigned short *)(original + 4)), ntohs(*(unsigned short *)(original + 6)), ntohs(*(unsigned short *)(original + 8)),
            ntohs(*(unsigned short *)(original + 10)), ntohs(*(unsigned short *)(original + 12)), ntohs(*(unsigned short *)(original + 14)));
}

void getDomain(unsigned char *original, unsigned char *domain) {
    while (*original != 0) {
        unsigned short len = *original;
        original++;
        memcpy(domain, original, len);
        original += len;
        domain += len;
        *domain = '.';
        domain++;
    }
    *(domain - 1) = '\0';
}

void getHeader(DNS_Header *header, const unsigned char *bytestream) {
    header->id = ntohs(*(unsigned short *)bytestream);
    header->qr = (bytestream[2] >> 7) & 1;
    header->opcode = (bytestream[2] >> 3) & 0x0f;
    header->aa = (bytestream[2] >> 2) & 1;
    header->tc = (bytestream[2] >> 1) & 1;
    header->rd = (bytestream[2]) & 1;
    header->ra = (bytestream[3] >> 7) & 1;
    header->z = (bytestream[3] >> 4) & 0x07;
    header->rcode = (bytestream[3]) & 0x0f;
    header->qdcount = ntohs(*(unsigned short *)(bytestream + 4));
    header->ancount = ntohs(*(unsigned short *)(bytestream + 6));
    header->nscount = ntohs(*(unsigned short *)(bytestream + 8));
    header->arcount = ntohs(*(unsigned short *)(bytestream + 10));
}

// 从字节流中提取出question的内容
void getQuestion(DNS_Question *quesiton, const unsigned char *bytestream, unsigned short *offset) {
    quesiton->qname = (unsigned char *)malloc(sizeof(unsigned char) * UDP_MAX);
    if (!quesiton->qname) {
        puts("动态分配内存失败");
        exit(1);
    }
    getName(quesiton->qname, bytestream, offset);
    quesiton->qtype = ntohs(*(unsigned short *)(bytestream + *offset));
    (*offset) += 2;
    quesiton->qclass = ntohs(*(unsigned short *)(bytestream + *offset));
    (*offset) += 2;
}

// 从字节流中提取RR的内容
void getRR(DNS_RR *RR, const unsigned char *bytestream, unsigned short *offset) {
    RR->name = (unsigned char *)malloc(256); // 假定最大域名长度为256字节
    if (!RR->name) {
        puts("动态分配内存失败");
        exit(1);
    }
    getName(RR->name, bytestream, offset);
    RR->type = ntohs(*(unsigned short *)(bytestream + *offset));
    (*offset) += 2;
    RR->_class = ntohs(*(unsigned short *)(bytestream + *offset));
    (*offset) += 2;
    RR->ttl = ntohl(*(unsigned int *)(bytestream + *offset));
    (*offset) += 4;
    RR->rdlength = ntohs(*(unsigned short *)(bytestream + *offset));
    (*offset) += 2;
    RR->rdata = (unsigned char *)malloc(RR->rdlength + 1); // 为rdata分配内存
    if (!RR->rdata) {
        puts("动态分配内存失败");
        exit(1);
    }
    memcpy(RR->rdata, bytestream + *offset, RR->rdlength);
    RR->rdata[RR->rdlength] = '\0';
    (*offset) += RR->rdlength;
}

// 字节流转换为dns报文结构体
DNS_MSG *bytestream_to_dnsmsg(const unsigned char *bytestream, unsigned short *offset) {
    DNS_MSG *msg = (DNS_MSG *)malloc(sizeof(DNS_MSG));
    if (!msg) {
        puts("动态分配内存失败");
        exit(1);
    }

    // 转换header部分
    msg->header = (DNS_Header *)malloc(sizeof(DNS_Header));
    if (!msg->header) {
        puts("动态分配内存失败");
        exit(1);
    }
    getHeader(msg->header, bytestream);

    *offset = 12;
    // 转换question部分
    msg->question = NULL;
    // 尾插法
    DNS_Question *question_tail = NULL;
    for (int i = 0; i < msg->header->qdcount; i++) {
        DNS_Question *current = (DNS_Question *)malloc(sizeof(DNS_Question));
        if (!current) {
            puts("动态分配内存失败");
            exit(1);
        }
        if (!question_tail) {
            msg->question = current;
            current->next = NULL;
        } else {
            question_tail->next = current;
            current->next = NULL;
        }
        question_tail = current;
        getQuestion(current, bytestream, offset);
    }

    // 转换answer、authority、 additional部分
    unsigned short total_length = msg->header->ancount + msg->header->nscount + msg->header->arcount;
    msg->RRs = NULL;
    DNS_RR *RRs_tail = NULL;
    for (int i = 0; i < total_length; i++) {
        DNS_RR *current = (DNS_RR *)malloc(sizeof(DNS_RR));
        if (!current) {
            puts("动态分配内存失败");
            exit(1);
        }
        if (!RRs_tail) {
            msg->RRs = current;
            current->next = NULL;
        } else {
            RRs_tail->next = current;
            current->next = NULL;
        }
        RRs_tail = current;
        getRR(current, bytestream, offset);
    }

    return msg;
}

// 将header填入字节流
void putHeader(const DNS_Header *header, unsigned char *bytestream) {
    bytestream[0] = header->id >> 8;
    bytestream[1] = header->id;
    bytestream[2] = 0;
    bytestream[2] |= header->qr << 7;
    bytestream[2] |= header->opcode << 3;
    bytestream[2] |= header->aa << 2;
    bytestream[2] |= header->tc << 1;
    bytestream[2] |= header->rd;
    bytestream[3] = 0;
    bytestream[3] |= header->ra << 7;
    bytestream[3] |= header->z << 4;
    bytestream[3] |= header->rcode;
    bytestream[4] = header->qdcount >> 8;
    bytestream[5] = header->qdcount;
    bytestream[6] = header->ancount >> 8;
    bytestream[7] = header->ancount;
    bytestream[8] = header->nscount >> 8;
    bytestream[9] = header->nscount;
    bytestream[10] = header->arcount >> 8;
    bytestream[11] = header->arcount;
}

// 将question填入字节流
void putQuestion(const DNS_Question *que, unsigned char *bytestream, unsigned short *offset) {
    memcpy(bytestream + *offset, que->qname, strlen((char *)(que->qname)) + 1);
    (*offset) += strlen((char *)(que->qname)) + 1;
    *(bytestream + *offset) = que->qtype >> 8;
    (*offset)++;
    *(bytestream + *offset) = que->qtype;
    (*offset)++;
    *(bytestream + *offset) = que->qclass >> 8;
    (*offset)++;
    *(bytestream + *offset) = que->qclass;
    (*offset)++;
}

// 将RR填入字节流
void putRR(const DNS_RR *rr, unsigned char *bytestream, unsigned short *offset) {
    size_t name_len = strlen((char *)(rr->name)) + 1; // 包括终止符

    // 将name复制到字节流中
    memcpy(bytestream + *offset, rr->name, name_len);
    (*offset) += name_len;

    // 将type转换为网络字节序并写入字节流
    *(unsigned short *)(bytestream + *offset) = htons(rr->type);
    (*offset) += 2;

    // 将class转换为网络字节序并写入字节流
    *(unsigned short *)(bytestream + *offset) = htons(rr->_class);
    (*offset) += 2;

    // 将ttl转换为网络字节序并写入字节流
    *(unsigned int *)(bytestream + *offset) = htonl(rr->ttl);
    (*offset) += 4;

    // 将rdlength转换为网络字节序并写入字节流
    *(unsigned short *)(bytestream + *offset) = htons(rr->rdlength);
    (*offset) += 2;

    // 将rdata复制到字节流中
    memcpy(bytestream + *offset, rr->rdata, rr->rdlength);
    (*offset) += rr->rdlength;
}

// dns报文结构体转换为字节流,进行DNS报文的发送
unsigned char *dnsmsg_to_bytestream(const DNS_MSG *msg) {
    unsigned char *bytestream = (unsigned char *)malloc(sizeof(unsigned char) * UDP_MAX);
    if (!bytestream) {
        puts("动态分配内存失败");
        exit(1);
    }

    // 转换header部分
    putHeader(msg->header, bytestream);

    // 转换question部分
    unsigned short offset = 12;
    DNS_Question *que = msg->question;
    while (que) {
        putQuestion(que, bytestream, &offset);
        que = que->next;
    }

    // 转换answer、authority、 additional部分
    DNS_RR *rr = msg->RRs;
    while (rr) {
        putRR(rr, bytestream, &offset);
        rr = rr->next;
    }

    return bytestream;
}

// 释放dns报文结构体
void releaseMsg(DNS_MSG *msg) {
    if (!msg)
        return;
    free(msg->header);
    DNS_Question *pQue = msg->question;
    while (pQue) {
        if (pQue->qname)
            free(pQue->qname);
        DNS_Question *temp = pQue;
        pQue = pQue->next;
        free(temp);
    }
    DNS_RR *pRR = msg->RRs;
    while (pRR) {
        if (pRR->name)
            free(pRR->name);
        if (pRR->rdata)
            free(pRR->rdata);
        DNS_RR *temp = pRR;
        pRR = pRR->next;
        free(temp);
    }
    free(msg);
}

// 添加answer字段
void addAnswer(DNS_MSG *msg, const unsigned char *IP, unsigned int _ttl, unsigned short _type) {
    // 不良网站拦截的报文设置
    if (_type == TYPE_A && *(unsigned int *)(IP) == 0) {
        msg->header->rcode = HEADER_RCODE_NAME_ERROR;
    }

    // 设置header字段的值
    msg->header->qr = HEADER_QR_ANSWER;
    msg->header->rd = 1;
    msg->header->ra = 1;
    msg->header->ancount++;

    // 为answer字段添加一个Resource Record
    DNS_RR *rr = msg->RRs;
    DNS_RR *prev = NULL;
    while (rr) {
        prev = rr;
        rr = rr->next;
    }
    if (prev) {
        rr = (DNS_RR *)malloc(sizeof(DNS_RR));
        if (!rr) {
            puts("动态分配内存失败");
            exit(1);
        }
        prev->next = rr;
        rr->next = NULL;
    } else {
        msg->RRs = (DNS_RR *)malloc(sizeof(DNS_RR));
        msg->RRs->next = NULL;
        rr = msg->RRs;
    }
    rr->name = (unsigned char *)malloc(sizeof(unsigned char) * UDP_MAX);
    // 从question中复制域名,不使用压缩标签
    memcpy(rr->name, msg->question->qname, strlen((char *)(msg->question->qname)) + 1);
    rr->type = _type;
    rr->_class = CLASS_IN;
    rr->ttl = _ttl;
    if (_type == TYPE_A) {
        rr->rdlength = 4; // IPv4
    } else {
        rr->rdlength = 16; // IPv6
    }
    rr->rdata = (unsigned char *)malloc(sizeof(unsigned char) * rr->rdlength);
    memcpy(rr->rdata, IP, rr->rdlength);
}

/**
 * @brief
 * 从外部DNS服务器返回的报文中提取出域名、IP地址、TTL、类型
 */
void getInfoFromServer(const unsigned char *bytestream, unsigned char *DN, unsigned char *IP, unsigned int *_ttl, unsigned short *_type) {
    unsigned short offset;
    DNS_MSG *msg = bytestream_to_dnsmsg(bytestream, &offset);
    getDomain(msg->question->qname, DN);
    DNS_RR *rr = msg->RRs;
    while (rr) {
        if (rr->type == TYPE_A || rr->type == TYPE_AAAA) {
            *_type = rr->type;
            memcpy(IP, rr->rdata, rr->rdlength);
            *_ttl = rr->ttl;
            break;
        }
        rr = rr->next;
    }
    releaseMsg(msg);
}