#pragma once
#include <stdint.h>
#include <stdio.h>
/*
    本头文件专门用于存放DNS报文结构体Message的定义，以及一切有关DNS报文的操作
    DNS 报文格式如下：
    +---------------------+
    |        Header       | 报文头，固定12字节，由结构体DNS_Header存储
    +---------------------+
    |       Question      | 向域名服务器的查询请求，由结构体DNS_Question存储
    +---------------------+
    |        Answer       | 对于查询问题的回复
    +---------------------+
    |      Authority      | 指向授权域名服务器
    +---------------------+
    |      Additional     | 附加信息
    +---------------------+
    后面三个部分由结构体DNS_RR存储
*/

#define UDP_MAX 512

// QR字段的值定义
#define HEADER_QR_QUERY 0
#define HEADER_QR_ANSWER 1

// OPCODE字段的值定义
#define HEADER_OPCODE_QUERY 0
#define HEADER_OPCODE_IQUERY 1
#define HEADER_OPCODE_STATUS 2

// RCODE字段的值定义
#define HEADER_RCODE_NO_ERROR 0
#define HEADER_RCODE_NAME_ERROR 3

// TYPE字段的值定义
#define TYPE_A 1
#define TYPE_NS 2
#define TYPE_CNAME 5
#define TYPE_SOA 6
#define TYPE_PTR 12
#define TYPE_HINFO 13
#define TYPE_MINFO 14
#define TYPE_MX 15
#define TYPE_TXT 16
#define TYPE_AAAA 28

// CLASS字段的值定义
#define CLASS_IN 1
#define CLASS_NOT 254
#define CLASS_ALL 255

// DNS报文Header部分
// 在定义结构体时，位域字段的顺序与实际填充的顺序是相反的，
// 位域的填充是从低字节开始的
// 进行跨平台统一时，需要注意字节序的问题
typedef struct DNS_HEADER {
    uint16_t id;         // 标识符，一对DNS查询和恢复的ID相同
    unsigned rd : 1;     /* recursion desired */
    unsigned tc : 1;     /* truncated message */
    unsigned aa : 1;     /* authoritive answer */
    unsigned opcode : 4; /* purpose of message */
    unsigned qr : 1;     /* response flag */
    unsigned rcode : 4;  /* response code */
    unsigned cd : 1;     /* checking disabled by resolver */
    unsigned ad : 1;     /* authentic data from named */
    unsigned z : 1;      /* unused bits, must be ZERO */
    unsigned ra : 1;     /* recursion available */
    uint16_t qdcount;    /* number of question entries */
    uint16_t ancount;    /* number of answer entries */
    uint16_t nscount;    /* number of authority entries */
    uint16_t arcount;    /* number of resource entries */
} DNS_Header;

// DNS报文Question部分
typedef struct Question {
    unsigned char *qname; // 域名
    uint16_t qtype;       // 资源类型
    uint16_t qclass;      // 地址类型，通常为1 IN
    struct Question *next;
} DNS_Question;

// Resource Record
typedef struct RR {
    unsigned char *name;  // 名字
    uint16_t type;        // RR的类型码
    uint16_t _class;      // 通常为IN(1)，指Internet数据
    uint32_t ttl;         // 客户端DNS cache可以缓存该记录多长时间
    uint16_t rdlength;    // 资源数据的字节数
    unsigned char *rdata; // 资源数据
    struct RR *next;
} DNS_RR;

// DNS报文,后三段格式相同，每段都是由0~n个资源记录(Resource Record)构成
typedef struct DNS_MSG {
    DNS_Header *header;
    DNS_Question *question;
    DNS_RR *RRs;
} DNS_MSG;

// 从网络字节中获得域名
void getName(unsigned char *qname, const unsigned char *bytestream, unsigned short *offset);

void getHeader(DNS_Header *header, const unsigned char *bytestream);

void getQuestion(DNS_Question *question, const unsigned char *bytestream, unsigned short *offset);

void getRR(DNS_RR *rr, const unsigned char *bytestream, unsigned short *offset);

// 从网络字节中获得点分十进制形式的IPv4地址
void getIPv4(unsigned char *original, unsigned char *IPv4);

// 从网络字节中获得冒分十六进制形式的IPv6地址
void getIPv6(unsigned char *original, unsigned char *IPv6);

// 获取对应格式的域名
void getDomain(unsigned char *original, unsigned char *domain);

DNS_MSG *bytestream_to_dnsmsg(const unsigned char *bytestream, unsigned short *offset);

// 将header填入字节流
void putHeader(const DNS_Header *header, unsigned char *bytestream);

// 将question填入字节流
void putQuestion(const DNS_Question *que, unsigned char *bytestream, unsigned short *offset);

// 将RR填入字节流
void putRR(const DNS_RR *rr, unsigned char *bytestream, unsigned short *offset);

// dns报文结构体转换为字节流
unsigned char *dnsmsg_to_bytestream(const DNS_MSG *msg);

// 释放dns报文结构体
void releaseMsg(DNS_MSG *msg);