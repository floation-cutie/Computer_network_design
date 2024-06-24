#pragma once

#include "dns_msg.h"
#include "hash.h"
#include <time.h>

#define CACHE_SIZE 1031 // 缓存大小,质数

// 联合体,用于存储IPv4和IPv6地址
// ??
typedef union {
    unsigned char ipAddr4[16]; // IPv4 地址
    unsigned char ipAddr6[16]; // IPv6 地址
} IPAddress;

/*
  ------------- CACHE TABLE --------------------
0 | Entry_x1 -> Entry_x2 -> Entry_x3 -> ... -> NULL
        ^
        |<-----------|
                     |
                     |
1 | Entry_y1 -> Entry_y2 -> Entry_y3 -> ... -> NULL
2 | ....
3 |
. |
. | ....
*/

// 缓存项结构体,双向链表实现(非循环) LRU算法
struct CacheEntry {
    unsigned char domain[512];   // 域名,作为键
    IPAddress ipAddr;            // IP地址,作为值
    time_t expireTime;           // 过期时间,类比TTL
    struct CacheEntry *prev;     // 前驱指针
    struct CacheEntry *next;     // 后继指针
    struct CacheEntry *hashNext; // 拉链法内部指针
};

// 缓存结构体,这里的头尾指针均不是dummy节点
struct Cache {
    struct CacheEntry *table[CACHE_SIZE]; // 哈希表
    struct CacheEntry *head;              // LRU链表头指针
    struct CacheEntry *tail;              // LRU链表尾指针
};

void initCache(struct Cache *cache);

// 计算哈希值
unsigned int hashCode(const unsigned char *domain);

int findEntry(struct Cache *cache, const unsigned char *domain, unsigned char *ipAddr, int ipVersion);

void addEntry(struct Cache *cache, const unsigned char *domain, const unsigned char *ipAddr, int ipVersion, time_t ttl);

void removeExpiredEntries(struct Cache *cache);

void clearCache(struct Cache *cache);
