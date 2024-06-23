#include "socket.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_MAPPING_SIZE 65536

/*
 * 用于存储原始ID和中继ID之间的映射关系
 * 由于ID是16位的，所以最多有2^16个ID
 * 为了简化实现，这里直接使用一个静态全局数组来存储映射关系
 */
typedef struct {
    unsigned short original_id;
    unsigned short relay_id;
    address_t client_addr; // 对应的客户端地址
    socklen_t addr_len;
} id_mapping_t;

pthread_mutex_t id_mapping_lock;
id_mapping_t id_mapping[MAX_ID_MAPPING_SIZE];

void init_id_mapping();
void cleanup_id_mapping();
unsigned short generate_unique_id();
void store_id_mapping(unsigned short original_id, unsigned short relay_id, struct sockaddr_in client_addr, socklen_t addr_len);
unsigned short get_original_id(unsigned short relay_id, struct sockaddr_in *client_addr, socklen_t *addr_len);