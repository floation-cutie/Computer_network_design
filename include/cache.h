#pragma once

#include "hash.h"
#include <time.h>

#define CACHE_SIZE 1031 // 缓存大小,质数

// 联合体,用于存储IPv4和IPv6地址
typedef union {
    unsigned char ipAddr4[16]; // IPv4 地址
    unsigned char ipAddr6[40]; // IPv6 地址
} IPAddress;

// 缓存项结构体,双向链表实现(非循环) LRU算法
struct CacheEntry {
    unsigned char domain[512]; // 域名,作为键
    IPAddress ipAddr;          // IP地址,作为值
    time_t expireTime;         // 过期时间,类比TTL
    struct CacheEntry *prev;   // 前驱指针
    struct CacheEntry *next;   // 后继指针
};

// 缓存结构体,这里的头尾指针均不是dummy节点
struct Cache {
    struct CacheEntry *table[CACHE_SIZE]; // 哈希表
    struct CacheEntry *head;              // 链表头指针
    struct CacheEntry *tail;              // 链表尾指针
};

// 拉链法内部
struct CacheNode {
    /* data */
};

// 初始化缓存
void initCache(struct Cache *cache);

// 计算哈希值
unsigned int hashCode(const unsigned char *domain);

// 查找缓存项
int findEntry(struct Cache *cache, const unsigned char *domain, unsigned char *ipAddr, int ipVersion);

// 添加缓存项
void addEntry(struct Cache *cache, const unsigned char *domain, const unsigned char *ipAddr, int ipVersion, time_t ttl);

// 删除缓存项
void removeExpiredEntries(struct Cache *cache);

// 清空缓存
void clearCache(struct Cache *cache);
