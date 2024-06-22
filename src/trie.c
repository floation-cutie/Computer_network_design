#include "trie.h"

#define MAX_LINE_LENGTH 512

void initTrie(struct Trie *trie) {
    memset(trie->tree, 0, sizeof(trie->tree));
    memset(trie->prefix, 0, sizeof(trie->prefix));
    memset(trie->isEnd, false, sizeof(trie->isEnd));
    memset(trie->toIp, 0, sizeof(trie->toIp));
    trie->size = 0;
}

void loadLocalTable(struct Trie *trie) {
    FILE *fp = fopen("dnsrelay.txt", "r");
    if (fp == NULL) {
        printf("Failed to open dnsrelay.txt\n");
        return;
    }

    // 读取文件中的每一行
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        // 域名和4个字节的IP地址
        char domain[MAX_LINE_LENGTH];
        unsigned char ip[4] = {0, 0, 0, 0};
        // 通过sscanf解析每一行
        // 格式化输出unsigned char
        if (sscanf(line, "%hhu.%hhu.%hhu.%hhu %s", &ip[0], &ip[1], &ip[2], &ip[3], domain) != 5) {
            printf("Invalid line in dnsrelay.txt: %s\n", line);
            continue;
        }
        insertNode(trie, domain, ip);
    }
    fclose(fp);
}

// 将域名简化,即将域名中的大写字母转换成小写字母
void simplifyDomain(char domain[]) {
    // 获取域名的长度
    int len = strlen(domain);
    int i;
    // 遍历域名的每个字符
    for (i = 0; i < len; i++) {
        // 如果当前字符不是.和-,将其转换为小写
        if (domain[i] != '.' && domain[i] != '-')
            domain[i] = tolower(domain[i]);
    }
    // 给域名加上结束符
    domain[i] = '\0';
}

// 在Trie树中插入一个域名和对应的IP地址
// eg:insertNode(trie, "www.baidu.com", "10.3.8.211")
void insertNode(struct Trie *trie, const char domain[], unsigned char ipAddr[4]) {
    int len = strlen(domain);
    if (len == 0)
        return;
    // 申请内存保存域名的副本
    char *temp = (char *)malloc(sizeof(char) * (len + 1));
    strcpy(temp, domain);
    simplifyDomain(temp);
    int parent = 0;
    // 遍历域名的每个字符
    for (int i = 0; i < len; i++) {
        int id;
        if (temp[i] >= '0' && temp[i] <= '9')
            id = temp[i] - '0';
        // 如果当前字符是小写字母,id为字符的值减去'a'加10
        else if (temp[i] >= 'a' && temp[i] <= 'z')
            id = temp[i] - 'a' + 10;
        else if (temp[i] == '-')
            id = 36;
        // 如果当前字符是'.',id为37
        else
            id = 37;

        // 如果不存在对应id的子节点,则创建一个新的节点
        if (!trie->tree[parent][id])
            trie->tree[parent][id] = ++trie->size;

        // 记录前缀
        trie->prefix[trie->tree[parent][id]] = parent;
        // 移动到下一个节点
        parent = trie->tree[parent][id];
    }
    // 将当前节点标记为结束节点
    trie->isEnd[parent] = true;
    // 复制IP地址
    memcpy(trie->toIp[parent], ipAddr, sizeof(unsigned char) * 4);
    // 释放临时内存
    free(temp);
}

// 查找一个域名在Trie树中对应的IP地址
// eg:findNode(trie, "www.baidu.com")
int findNode(struct Trie *trie, const unsigned char domain[]) {
    // 获取域名的长度
    int len = strlen((char *)domain);
    if (len == 0)
        return 0;
    // 申请内存保存域名的副本
    char *temp = (char *)malloc(sizeof(char) * (len + 1));
    // 复制域名
    strcpy(temp, (char *)domain);
    // 简化域名,将大写字母转换为小写字母
    simplifyDomain(temp);
    // 初始化根节点为0
    int parent = 0;
    // 遍历域名的每个字符
    for (int i = 0; i < len; i++) {
        int id;
        if (temp[i] >= '0' && temp[i] <= '9')
            id = temp[i] - '0';
        else if (temp[i] >= 'a' && temp[i] <= 'z')
            id = temp[i] - 'a' + 10;
        else if (temp[i] == '-')
            id = 36;
        else
            id = 37;
        // 如果不存在
        if (!trie->tree[parent][id]) {
            free(temp);
            return 0;
        }

        // 移动到下一个节点
        parent = trie->tree[parent][id];
    }
    // 如果找到的节点不是终止节点,说明域名不在Trie树中,返回0
    if (trie->isEnd[parent] == false) {
        free(temp);
        return 0;
    }
    // 释放临时内存
    free(temp);
    // 返回找到的节点
    return parent;
}

// 在Trie树中删除一个域名和对应的IP地址
// eg: deleteNode(trie, "www.baidu.com")
void deleteNode(struct Trie *trie, const unsigned char domain[]) {
    // 检查是否为空串
    int len = strlen((char *)domain);
    if (len == 0)
        return;

    // 查找这个域名是否存在
    int parent = findNode(trie, domain);
    if (parent == 0)
        return;

    // 将这个节点标记为非终止节点
    trie->isEnd[parent] = false;

    // 复制域名
    char *temp = (char *)malloc(sizeof(char) * (len + 1));
    strcpy(temp, (char *)domain);
    simplifyDomain(temp);

    // 如果这个节点不是根节点
    while (parent != 0) {
        int id;
        if (temp[len - 1] >= '0' && temp[len - 1] <= '9')
            id = temp[len - 1] - '0';
        else if (temp[len - 1] >= 'a' && temp[len - 1] <= 'z')
            id = temp[len - 1] - 'a' + 10;
        else if (temp[len - 1] == '-')
            id = 36;
        else // temp[len - 1] == '.'
            id = 37;

        bool haveChild = false;
        for (int i = 0; i < MAX_ALPHABET; i++) {
            // 如果这个节点的第i个子节点存在，就说明这个节点还有子节点
            if (trie->tree[parent][i]) {
                haveChild = true;
                break;
            }
        }
        if (haveChild)
            break;

        // 如果这个节点的所有子节点都被删除了，就删除这个节点
        int preNode = trie->prefix[parent];
        trie->tree[preNode][id] = 0;
        trie->prefix[parent] = 0;
        parent = preNode;
        len--;
    }

    // 释放临时内存
    free(temp);
}