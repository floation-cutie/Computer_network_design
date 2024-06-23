#pragma once

#include "cache.h"
#include "dns_msg.h"

// 统计信息(后续)
static uint64_t query_count, cache_hit_count, cache_outdate_count;
static uint64_t remote_send_count, remote_recv_count;

void debug_header(DNS_MSG *msg);

void debug_question(DNS_MSG *msg);

void RR_info(DNS_RR *rr);

void debug_RR(DNS_MSG *msg);

void debug_time(); // 输出程序运行时间

void debug_dns_msg(DNS_MSG *msg);

void debug_bytestream(unsigned char *bytestream);

void debug_cache(struct Cache *cache);

void debug_dns_msg_by_bytestream(unsigned char *bytestream);