#pragma once

#include "dns_msg.h"

void debug_header(DNS_MSG *msg);

void debug_question(DNS_MSG *msg);

void RR_info(DNS_RR *rr);

void debug_RR(DNS_MSG *msg);

void debug_time(); // 输出程序运行时间

void debug_bytestream(unsigned char *bytestream);