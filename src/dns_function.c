#include "dns_function.h"
#include "msg_convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 添加answer字段
void addAnswer(Dns_Msg *msg, const unsigned char *IP, unsigned int _ttl, unsigned short _type) {
    // 不良网站拦截
    if (_type == TYPE_A && *(unsigned int *)(IP) == 0) {
        msg->header->rcode = HEADER_RCODE_NAME_ERROR;
    } else {
        msg->header->rcode = HEADER_RCODE_NO_ERROR;
    }
    // 设置header字段的值
    msg->header->qr = HEADER_QR_ANSWER;
    msg->header->ancount++;

    // 为answer字段添加一个Resource Record
    Dns_RR *rr = msg->RRs;
    Dns_RR *prev = NULL;
    while (rr) {
        prev = rr;
        rr = rr->next;
    }
    if (prev) {
        rr = (Dns_RR *)malloc(sizeof(Dns_RR));
        if (!rr) {
            puts("Allocate memory failed");
            exit(1);
        }
        prev->next = rr;
        rr->next = NULL;
    } else {
        msg->RRs = (Dns_RR *)malloc(sizeof(Dns_RR));
        msg->RRs->next = NULL;
        rr = msg->RRs;
    }
    rr->name = (unsigned char *)malloc(sizeof(unsigned char) * UDP_MAX);
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

// 从外部DNS的回复报文中提取域名和IP地址
void getDN_IP(const unsigned char *bytestream, unsigned char *DN, unsigned char *IP, unsigned int *_ttl, unsigned short *_type) {
    unsigned short offset;
    // 此时的IP占据了CNAME的位置
    Dns_Msg *msg = bytestream_to_dnsmsg(bytestream, &offset);
    offset = 0;
    *_type = -1; // 不存在的值
    getCNAME(bytestream, &offset, IP);
    transDN(msg->question->qname, DN);
    Dns_RR *rr = msg->RRs;
    while (rr) {
        if (rr->type == TYPE_A || rr->type == TYPE_AAAA) {
            *_type = rr->type;
            memcpy(IP, rr->rdata, rr->rdlength);
            *_ttl = rr->ttl;
            break;
        }
        *_type = rr->type;
        *_ttl = rr->ttl;
        rr = rr->next;
    }
    releaseMsg(msg);
}