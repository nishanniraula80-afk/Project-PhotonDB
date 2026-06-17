#pragma once

#include <stddef.h>
#include <stdint.h>

// Hashtable node structure
struct HNode {
    HNode *next = nullptr;
    uint32_t hcode = 0;
};

// Fixed-size or resizable bucket array
struct HTab {
    HNode **tab = nullptr;
    size_t mask = 0;
    size_t size = 0;
};

// Managing wrapper for progressive rehashing
struct HashMap {
    HTab ht1;
    HTab ht2;
    size_t rehash_idx = 0;
};

// Function declarations we need to implement
void hm_insert(HashMap *hm, HNode *node);
HNode *hm_lookup(HashMap *hm, HNode *key, bool (*cmp)(HNode *, HNode *));
HNode *hm_detach(HashMap *hm, HNode *key, bool (*cmp)(HNode *, HNode *));
void hm_destroy(HashMap *hm);