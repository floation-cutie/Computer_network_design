#include "debug_info.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// 其中debug的信息需要进一步修饰，使其更加易读

void debug_time() {
    // 根据时钟周期计算时间
    static clock_t start = 0;
    if (start == 0) {
        start = clock();
    }
    clock_t end = clock();
    printf("Time: %.3f\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void debug_header(DNS_MSG *msg) {
    puts("------------------------HEADER--------------------------------");
    printf("ID: %d ", msg->header->id);
    printf("QR: %d ", msg->header->qr);
    printf("Opcode: %d ", msg->header->opcode);
    printf("AA: %d ", msg->header->aa);
    printf("TC: %d ", msg->header->tc);
    printf("RD: %d ", msg->header->rd);
    printf("RA: %d ", msg->header->ra);
    printf("Z: %d ", msg->header->z);
    printf("RCODE: %d ", msg->header->rcode);
    printf("QDCOUNT: %d ", msg->header->qdcount);
    printf("ANCOUNT: %d ", msg->header->ancount);
    printf("NSCOUNT: %d ", msg->header->nscount);
    printf("ARCOUNT: %d\n", msg->header->arcount);
}

void debug_question(DNS_MSG *msg) {
    puts("------------------------QUESTION------------------------");
    DNS_Question *q = msg->question;
    while (q) {
        unsigned char name[UDP_MAX];
        getDomain(q->qname, name);
        printf("QNAME: %20s ", name);
        printf("QTYPE: %2d ", q->qtype);
        printf("QCLASS: %2d\n", q->qclass);
        q = q->next;
    }
}

void RR_info(DNS_RR *rr) {
    unsigned char name[UDP_MAX];
    getDomain(rr->name, name);
    printf("NAME:%20s  ", name);
    printf("TYPE: %d ", rr->type);
    printf("CLASS: %d ", rr->_class);
    printf("TTL: %d ", rr->ttl);
    printf("RDLENGTH: %d ", rr->rdlength);
    // 根据不同的类型码输出不同的信息
    switch (rr->type) {
    case TYPE_A: {
        unsigned char IPv4[20];
        memset(IPv4, 0, sizeof(IPv4));
        getIPv4(rr->rdata, IPv4);
        printf("Address Resource Record:%20s", IPv4);
        break;
    }
    case TYPE_NS:
    case TYPE_CNAME:
    case TYPE_PTR:
        printf("RDATA: %s\n", rr->rdata);
        break;
    case TYPE_MX:
        printf("PREFERENCE: %d ", rr->rdata[0]);
        printf("EXCHANGE: %s\n", rr->rdata + 1);
        break;
    case TYPE_AAAA: {
        unsigned char IPv6[40];
        getIPv6(rr->rdata, IPv6);
        printf("IPv6 Address Resource Record: %s\n", IPv6);
        break;
    }
    case TYPE_SOA:
        printf("MNAME: %s ", rr->rdata);
        printf("RNAME: %s\n", rr->rdata + strlen(rr->rdata) + 1);
        break;
    case TYPE_TXT:
        printf("TXT RDATA: %s\n", rr->rdata);
        break;
    default:
        printf("RDATA: %s\n", rr->rdata);
        break;
    }
}

void debug_RR(DNS_MSG *msg) {
    DNS_RR *rr = msg->RRs;
    if (msg->header->ancount)
        puts("------------------------ANSWER------------------------");
    for (int i = 0; i < msg->header->ancount; i++) {
        printf("RR %d\n", i + 1);
        RR_info(rr);
        rr = rr->next;
    }

    if (msg->header->nscount)
        puts("-----------------------AUTHORITY----------------------");
    for (int i = 0; i < msg->header->nscount; i++) {
        printf("RR %d\n", i + 1);
        RR_info(rr);
        rr = rr->next;
    }

    if (msg->header->arcount)
        puts("----------------------ADDITIONAL----------------------\n");
    for (int i = 0; i < msg->header->arcount; i++) {
        printf("RR %d\n", i + 1);
        RR_info(rr);
        rr = rr->next;
    }

    puts("------------------------------------------------------");
}

void debug_bytestream(unsigned char *bytestream) {
    unsigned short offset = 0;
    DNS_MSG *msg = bytestream_to_dnsmsg(bytestream, &offset);
    // 打印接收到的DNS查询报文
    for (int i = 0; i < offset; i++) {
        // 格式控制: 以十六进制输出,2为指定的输出字段的宽度.如果位数小于2,则左端补0
        printf("%02x ", bytestream[i]);
        if ((i + 1) % 16 == 0) {
            puts("");
        }
    }
    puts("");
    releaseMsg(msg);
}

void debug_dns_msg(DNS_MSG *msg) {
    debug_header(msg);
    debug_question(msg);
    debug_RR(msg);
}