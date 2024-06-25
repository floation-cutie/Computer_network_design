#include "..\include\debug_info.h"
#include "..\include\msg_convert.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 打印程序执行时间
void printTime() {
    static clock_t start_time = 0;
    if (start_time == 0) {
        start_time = clock();
    }
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    log_message("%.3f:", elapsed_time);
}

// 输出resource record
void RRInfo(Dns_RR *rr) {
    unsigned char name[UDP_MAX];
    transDN(rr->name, name);
    log_message("NAME:%20s  ", name);
    log_message("TYPE:%2d  ", rr->type);
    log_message("CLASS:%2d", rr->_class);
    log_message("TTL:%2d  ", rr->ttl);
    log_message("RDLENGTH:%2d  ", rr->rdlength);
    if (rr->type == TYPE_A) {
        unsigned char IPv4[20];
        transIPv4(rr->rdata, IPv4);
        log_message("RDATA:%20s", IPv4);
    }
    if (rr->type == TYPE_AAAA) {
        unsigned char IPv6[40];
        transIPv6(rr->rdata, IPv6);
        log_message("RDATA:%20s", IPv6);
    }
    log_message("");
}

// 输出调试信息
void debug(Dns_Msg *msg) {
    printTime();
    log_message("------------------------HEADER------------------------");
    log_message("ID:%2d  ", msg->header->id);
    log_message("QR:%2d  ", msg->header->qr);
    log_message("Opcode:%2d  ", msg->header->opcode);
    log_message("AA:%2d  ", msg->header->aa);
    log_message("TC:%2d  ", msg->header->tc);
    log_message("RD:%2d  ", msg->header->rd);
    log_message("RA:%2d", msg->header->ra);
    log_message("RCODE:%2d  ", msg->header->rcode);
    log_message("QDCOUNT:%2d  ", msg->header->qdcount);
    log_message("ANCOUNT:%2d  ", msg->header->ancount);
    log_message("NSCOUNT:%2d  ", msg->header->nscount);
    log_message("ARCOUNT:%2d", msg->header->arcount);

    Dns_Question *current_que = msg->question;
    log_message("-----------------------QUESTION-----------------------");
    for (int i = 0; i < msg->header->qdcount; i++) {
        log_message("QUESTION %d", i + 1);
        unsigned char name[512];
        transDN(current_que->qname, name);
        log_message("QNAME:%20s  ", name);
        log_message("QTYPE:%2d  ", current_que->qtype);
        log_message("QCLASS:%2d", current_que->qclass);
        current_que = current_que->next;
    }

    Dns_RR *rr = msg->RRs;
    if (msg->header->ancount) {
        log_message("------------------------ANSWER------------------------");
    }
    for (int i = 0; i < msg->header->ancount; i++) {
        log_message("RR %d", i + 1);
        RRInfo(rr);
        rr = rr->next;
    }

    if (msg->header->nscount) {
        log_message("-----------------------AUTHORITY----------------------");
    }
    for (int i = 0; i < msg->header->nscount; i++) {
        log_message("RR %d", i + 1);
        RRInfo(rr);
        rr = rr->next;
    }

    if (msg->header->arcount) {
        log_message("----------------------ADDITIONAL----------------------");
    }
    for (int i = 0; i < msg->header->arcount; i++) {
        log_message("RR %d", i + 1);
        RRInfo(rr);
        rr = rr->next;
    }

    log_message("------------------------------------------------------");
}

// 输出16进制字节流
void bytestreamInfo(unsigned char *bytestream) {
    unsigned short offset;
    Dns_Msg *msg = bytestream_to_dnsmsg(bytestream, &offset);
    for (int i = 0; i < (int)(offset); i += 16) {
        log_message("%04lx: ", i);
        for (int j = i; j < i + 16 && j < (int)(offset); j++) {
            log_message("%02x ", (unsigned char)bytestream[j]);
        }
        log_message("");
    }
    releaseMsg(msg);
}
