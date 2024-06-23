#include "id_convert.h"

void init_id_mapping() {
    pthread_mutex_init(&id_mapping_lock, NULL);
    memset(id_mapping, 0, sizeof(id_mapping));
}

void cleanup_id_mapping() {
    pthread_mutex_destroy(&id_mapping_lock);
}

// 原版基础上进行了修改
unsigned short generate_unique_id() {
    static unsigned short current_id = 0;
    unsigned short unique_id;
    pthread_mutex_lock(&id_mapping_lock);
    current_id = (current_id + 1) % MAX_ID_MAPPING_SIZE;
    unique_id = current_id; // 在持有锁的情况下保存当前的id到unique_id
    pthread_mutex_unlock(&id_mapping_lock);
    return unique_id; // 返回保存的unique_id
}

void store_id_mapping(unsigned short original_id, unsigned short relay_id, struct sockaddr_in client_addr, socklen_t addr_len) {
    pthread_mutex_lock(&id_mapping_lock);
    id_mapping[relay_id].original_id = original_id;
    id_mapping[relay_id].relay_id = relay_id;
    id_mapping[relay_id].client_addr = client_addr;
    id_mapping[relay_id].addr_len = addr_len;
    pthread_mutex_unlock(&id_mapping_lock);
}

unsigned short get_original_id(unsigned short relay_id, struct sockaddr_in *client_addr, socklen_t *addr_len) {
    pthread_mutex_lock(&id_mapping_lock);
    unsigned short original_id = id_mapping[relay_id].original_id;
    *client_addr = id_mapping[relay_id].client_addr;
    *addr_len = id_mapping[relay_id].addr_len;
    pthread_mutex_unlock(&id_mapping_lock);
    return original_id;
}
