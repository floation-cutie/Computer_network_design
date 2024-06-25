#include "debug_info.h"
#include "log.h" // 添加这个包含
#include <stdio.h>
#include <string.h>
#include <time.h>

void debug_time() {
    static clock_t start = 0;
    if (start == 0) {
        start = clock();
    }
    clock_t end = clock();
    log_message("Time: %.3f", (double)(end - start) / CLOCKS_PER_SEC);
}

void debug_header(DNS_MSG *msg) {
    log_message("------------------------HEADER--------------------------------");
    log_message("ID: %d ", msg->header->id);
    log_message("QR: %d ", msg->header->qr);
    log_message("Opcode: %d ", msg->header->opcode);
    log_message("AA: %d ", msg->header->aa);
    log_message("TC: %d ", msg->header->tc);
    log_message("RD: %d ", msg->header->rd);
    log_message("RA: %d ", msg->header->ra);
    log_message("Z: %d ", msg->header->z);
    log_message("RCODE: %d ", msg->header->rcode);
    log_message("QDCOUNT: %d ", msg->header->qdcount);
    log_message("ANCOUNT: %d ", msg->header->ancount);
    log_message("NSCOUNT: %d ", msg->header->nscount);
    log_message("ARCOUNT: %d", msg->header->arcount);
}

void debug_question(DNS_MSG *msg) {
    log_message("------------------------QUESTION------------------------");
    DNS_Question *q = msg->question;
    while (q) {
        unsigned char name[UDP_MAX];
        getDomain(q->qname, name);
        log_message("QNAME: %20s ", name);
        log_message("QTYPE: %2d ", q->qtype);
        log_message("QCLASS: %2d", q->qclass);
        q = q->next;
    }
}

void RR_info(DNS_RR *rr) {
    unsigned char name[UDP_MAX];
    getDomain(rr->name, name);
    log_message("NAME:%20s  ", name);
    log_message("TYPE: %d ", rr->type);
    log_message("CLASS: %d ", rr->_class);
    log_message("TTL: %d ", rr->ttl);
    log_message("RDLENGTH: %d ", rr->rdlength);
    switch (rr->type) {
    case TYPE_A: {
        unsigned char IPv4[20];
        memset(IPv4, 0, sizeof(IPv4));
        getIPv4(rr->rdata, IPv4);
        log_message("Address Resource Record:%20s", IPv4);
        break;
    }
    case TYPE_NS:
    case TYPE_CNAME:
    case TYPE_PTR:
        log_message("RDATA: %s", rr->rdata);
        break;
    case TYPE_MX:
        log_message("PREFERENCE: %d ", rr->rdata[0]);
        log_message("EXCHANGE: %s", rr->rdata + 1);
        break;
    case TYPE_AAAA: {
        unsigned char IPv6[40];
        getIPv6(rr->rdata, IPv6);
        log_message("IPv6 Address Resource Record: %s", IPv6);
        break;
    }
    case TYPE_SOA:
        log_message("MNAME: %s ", rr->rdata);
        log_message("RNAME: %s", rr->rdata + strlen(rr->rdata) + 1);
        break;
    case TYPE_TXT:
        log_message("TXT RDATA: %s", rr->rdata);
        break;
    default:
        log_message("RDATA: %s", rr->rdata);
        break;
    }
}

void debug_RR(DNS_MSG *msg) {
    DNS_RR *rr = msg->RRs;
    if (msg->header->ancount)
        log_message("------------------------ANSWER------------------------");
    for (int i = 0; i < msg->header->ancount; i++) {
        log_message("RR %d", i + 1);
        RR_info(rr);
        rr = rr->next;
    }

    if (msg->header->nscount)
        log_message("-----------------------AUTHORITY----------------------");
    for (int i = 0; i < msg->header->nscount; i++) {
        log_message("RR %d", i + 1);
        RR_info(rr);
        rr = rr->next;
    }

    if (msg->header->arcount)
        log_message("----------------------ADDITIONAL----------------------");
    for (int i = 0; i < msg->header->arcount; i++) {
        log_message("RR %d", i + 1);
        RR_info(rr);
        rr = rr->next;
    }

    log_message("------------------------------------------------------");
}

void debug_bytestream(unsigned char *bytestream) {
    unsigned short offset = 0;
    DNS_MSG *msg = bytestream_to_dnsmsg(bytestream, &offset);
    for (int i = 0; i < offset; i++) {
        log_message("%02x ", bytestream[i]);
        if ((i + 1) % 16 == 0) {
            log_message("");
        }
    }
    log_message("");
    releaseMsg(msg);
}

void debug_dns_msg_by_bytestream(unsigned char *bytestream) {
    unsigned short offset = 0;
    DNS_MSG *msg = bytestream_to_dnsmsg(bytestream, &offset);
    debug_dns_msg(msg);
    releaseMsg(msg);
}

void debug_dns_msg(DNS_MSG *msg) {
    debug_header(msg);
    debug_question(msg);
    debug_RR(msg);
}

void debug_cache(struct Cache *cache) {
    log_message("------------------------CACHE------------------------");
    time_t now = time(NULL);
    for (int i = 0; i < CACHE_SIZE; i++) {
        log_message("Bucket %d", i);
        struct CacheEntry *entry = cache->table[i];
        if (entry) {
            log_message("Domain: %s", entry->domain);
            if (entry->expireTime < now) {
                log_message("Expired");
            } else {
                struct tm *local_time = localtime(&now);
                char time_str[20];
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                log_message("Expire time: %s", time_str);
            }
            log_message("");
        }
    }
    log_message("----------------------------------------------------");
}
