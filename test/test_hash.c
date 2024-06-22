#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_SIZE 100

typedef struct Entry {
    char key[256];
    char value[256];
} Entry;

typedef struct ListNode {
    Entry *entry;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *table[CACHE_SIZE];
} HashTable;

unsigned int hash(const char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + *key++;
    }
    return hash % CACHE_SIZE;
}

void put(HashTable *hashTable, const char *key, const char *value) {
    unsigned int index = hash(key);

    ListNode *node = hashTable->table[index];
    while (node) {
        if (strcmp(node->entry->key, key) == 0) {
            // update
            strcpy(node->entry->value, value);
            return;
        }
        node = node->next;
    }

    Entry *entry = (Entry *)malloc(sizeof(Entry));
    strcpy(entry->key, key);
    strcpy(entry->value, value);

    node = (ListNode *)malloc(sizeof(ListNode));
    node->entry = entry;
    // insert at head
    node->next = hashTable->table[index];
    hashTable->table[index] = node;
}

char *get(HashTable *hashTable, const char *key) {
    unsigned int index = hash(key);

    ListNode *node = hashTable->table[index];
    while (node) {
        if (strcmp(node->entry->key, key) == 0) {
            return node->entry->value;
        }
        node = node->next;
    }

    return NULL;
}

void delete(HashTable *hashTable, const char *key) {
    unsigned int index = hash(key);

    ListNode *node = hashTable->table[index];
    ListNode *prev = NULL;
    while (node) {
        if (strcmp(node->entry->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                hashTable->table[index] = node->next;
            }
            free(node->entry);
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void init(HashTable *hashTable) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        hashTable->table[i] = NULL;
    }
}

int main() {
    HashTable hashTable;
    init(&hashTable);
    put(&hashTable, "key1", "value1");
    put(&hashTable, "key2", "value2");
    put(&hashTable, "key3", "value3");

    printf("%s\n", get(&hashTable, "key2"));

    // delete (&hashTable, "key2");

    return 0;
}