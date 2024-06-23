#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 初始化缓存
void initCache(struct Cache *cache) {
    memset(cache->table, 0, sizeof(cache->table)); // 将哈希表清零
    cache->head = NULL;                            // 链表头指针置空
    cache->tail = NULL;                            // 链表尾指针置空
}

// 计算哈希值
unsigned int hashCode(const unsigned char *domain) {
    uint32_t hashValue = murmurHash32(domain, strlen((const char *)domain), 0) % CACHE_SIZE; // 调用 MurmurHash 算法计算哈希值
    return (unsigned int)hashValue;                                                          // 返回哈希值
}

/**
 * @brief
 * 如果缓存项存在且未过期，将其移动到链表头部，并放入IP缓冲区，返回 1
 * 如果缓存项不存在或已过期，删除其对应的哈希表和链表项，返回 0
 * @param cache 缓存
 * @param domain 域名
 * @param ipAddr IP缓冲区
 * @param ipVersion IP版本
 */
int findEntry(struct Cache *cache, const unsigned char *domain, unsigned char *ipAddr, int ipVersion) {
    size_t domainLen = strlen((const char *)domain); // 获取域名长度
    unsigned int hash = hashCode(domain);            // 获取哈希值
    time_t now = time(NULL);                         // 获取当前时间

    if (cache->head == cache->tail) {
        removeExpiredEntries(cache);
        return 0;
    }
    // 遍历哈希表
    struct CacheEntry *entry = cache->table[hash];
    struct CacheEntry *preEntry = NULL;
    while (entry != NULL) {
        // 如果找到了对应的域名
        if (strcmp((const char *)entry->domain, (const char *)domain) == 0) {
            // 如果缓存项未过期
            if (entry->expireTime >= now) {
                // LRU策略，将命中的缓存移动到链表头部
                if (entry->prev != NULL) // 如果不是链表头部
                    entry->prev->next = entry->next;
                else // 如果是链表头部
                    cache->head = entry->next;
                if (entry->next != NULL) // 如果不是链表尾部
                    entry->next->prev = entry->prev;
                else // 如果是链表尾部
                    cache->tail = entry->prev;

                entry->expireTime = now + 60; // 更新过期时间
                entry->prev = NULL;
                entry->next = cache->head;
                cache->head = entry;

                // 设置 IP 地址
                if (ipVersion == TYPE_A)
                    memcpy(ipAddr, entry->ipAddr.ipAddr4, sizeof(entry->ipAddr.ipAddr4));
                else if (ipVersion == TYPE_AAAA)
                    memcpy(ipAddr, entry->ipAddr.ipAddr6, sizeof(entry->ipAddr.ipAddr6));

                return 1; // 返回成功
            }
            // 如果缓存项已过期,删除对应的哈希表和链表项
            else {
                // 从哈希表中删除过期的缓存项
                if (preEntry != NULL)
                    preEntry->hashNext = entry->hashNext;
                else
                    cache->table[hash] = entry->hashNext;
                // 从链表中删除过期的缓存项
                if (entry->prev != NULL)
                    entry->prev->next = entry->next;
                else
                    cache->head = entry->next;
                if (entry->next != NULL)
                    entry->next->prev = entry->prev;
                else
                    cache->tail = entry->prev;
                free(entry); // 释放内存
                return 0;    // 返回失败
            }
        }
        preEntry = entry;
        entry = entry->hashNext; // 移动指针
    }
    return 0; // 返回失败
}

// 添加缓存项(ttl根据DNS报文中的 TTL 字段设置)
// 此处不进行更新操作，只进行插入操作,降低复杂度

void addEntry(struct Cache *cache, const unsigned char *domain, const unsigned char *ipAddr, int ipVersion, time_t ttl) {
    size_t domainLen = strlen((const char *)domain);
    unsigned int hash = hashCode(domain);
    time_t now = time(NULL);

    // 新建缓存项
    struct CacheEntry *entry = (struct CacheEntry *)malloc(sizeof(struct CacheEntry));
    if (entry == NULL)
        return;

    // 复制域名
    memcpy(entry->domain, domain, domainLen + 1);
    entry->domain[domainLen] = '\0';

    // 复制IP地址
    if (ipVersion == TYPE_A)
        memcpy(entry->ipAddr.ipAddr4, (void *)&ipAddr, sizeof(entry->ipAddr.ipAddr4));
    else if (ipVersion == TYPE_AAAA)
        memcpy(entry->ipAddr.ipAddr6, (void *)&ipAddr, sizeof(entry->ipAddr.ipAddr6));

    // 设置过期时间
    entry->expireTime = now + ttl;

    // 添加到链表头部
    entry->prev = NULL;
    entry->next = cache->head;
    if (cache->head != NULL) // 如果链表不为空
        cache->head->prev = entry;
    cache->head = entry;     // 更新链表头指针
    if (cache->tail == NULL) // 如果链表为空
        cache->tail = entry;

    // 添加到哈希表,通过拉链法解决哈希冲突
    entry->hashNext = cache->table[hash];
    cache->table[hash] = entry;
    // 删除过期的缓存项
    removeExpiredEntries(cache);
}

void removeExpiredEntries(struct Cache *cache) {
    time_t now = time(NULL); // 获取当前时间
    struct CacheEntry *entry = cache->tail;
    while (entry != NULL && entry->expireTime < now) {
        struct CacheEntry *pre = entry->prev;

        // 从链表中删除过期的缓存项
        if (pre != NULL) {
            pre->next = NULL;
            cache->tail = pre;
        } else {
            cache->tail = NULL;
            cache->head = NULL;
        }
        // 从哈希表中删除过期的缓存项
        unsigned int hash = hashCode(entry->domain);
        struct CacheEntry *hashEntry = cache->table[hash];
        struct CacheEntry *preHashEntry = NULL;
        while (hashEntry != NULL) {
            if (strcmp((const char *)hashEntry->domain, (const char *)entry->domain) == 0) {
                if (preHashEntry != NULL)
                    preHashEntry->hashNext = hashEntry->hashNext;
                else
                    cache->table[hash] = hashEntry->hashNext;
                break;
            }
            preHashEntry = hashEntry;
            hashEntry = hashEntry->hashNext;
        }
        free(entry);         // 释放内存
        entry = cache->tail; // 移动指针
    }
}

void clearCache(struct Cache *cache) {
    // 遍历哈希表，释放缓存项内存
    // 不能只释放链表内容，因为哈希表中也有指向缓存项的指针
    for (int i = 0; i < CACHE_SIZE; i++) {
        struct CacheEntry *entry = cache->table[i];
        // 主体循环
        while (entry != NULL) {
            struct CacheEntry *next = entry->hashNext;
            free(entry);
            entry = next;
        }
        cache->table[i] = NULL;
    }
    // 头尾指针置空
    cache->head = NULL;
    cache->tail = NULL;
}
