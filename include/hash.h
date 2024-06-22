#pragma once

#include <stdint.h>
/**
 * @brief
 * Key (数据)：要进行哈希的数据，可以是任意长度的字节序列。
    Length (长度)：数据的字节长度。
    Seed (种子值)：一个初始值，用于混合哈希值，提供额外的随机性
    将输入数据转化为一个32位的整数哈希值。
 */

uint32_t murmurHash32(const void *key, size_t len, uint32_t seed);
