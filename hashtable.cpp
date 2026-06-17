#include "hashtable.h"
#include <stdlib.h>
#include <assert.h>

// Initialize a table with a power-of-2 size
static void h_init(HTab *htab, size_t n) {
    assert(((n - 1) & n) == 0 && "Size must be a power of 2");
    htab->tab = (HNode **)calloc(n, sizeof(HNode *));
    htab->mask = n - 1;
    htab->size = 0;
}

// Insert a node into a specific table bucket slot
static void h_insert(HTab *htab, HNode *node) {
    size_t pos = node->hcode & htab->mask;
    HNode *next = htab->tab[pos];
    node->next = next;
    htab->tab[pos] = node;
    htab->size++;
}

// Lookup logic for a single table list chain
static HNode **h_lookup(HTab *htab, HNode *key, bool (*cmp)(HNode *, HNode *)) {
    if (!htab->tab) return nullptr;

    size_t pos = key->hcode & htab->mask;
    HNode **from = &htab->tab[pos];
    while (*from) {
        if ((*from)->hcode == key->hcode && cmp(*from, key)) {
            return from;
        }
        from = &(*from)->next;
    }
    return from;
}

// Help migrate buckets step-by-step
static void hm_help_resizing(HashMap *hm) {
    if (hm->ht2.tab == nullptr) return; // Not resizing right now

    size_t n_work = 0;
    while (n_work < 128 && hm->ht1.size > 0) {
        // Find a non-empty bucket
        while (hm->ht1.tab[hm->rehash_idx] == nullptr) {
            hm->rehash_idx++;
        }

        // Migrate all nodes from this bucket to ht2
        HNode **from = &hm->ht1.tab[hm->rehash_idx];
        while (*from) {
            HNode *next = (*from)->next;
            h_insert(&hm->ht2, *from);
            *from = next;
            hm->ht1.size--;
            n_work++;
        }
    }

    if (hm->ht1.size == 0) {
        // Resizing fully done! Swap tables
        free(hm->ht1.tab);
        hm->ht1 = hm->ht2;
        hm->ht2 = HTab();
        hm->rehash_idx = 0;
    }
}

static void hm_start_resizing(HashMap *hm) {
    assert(hm->ht2.tab == nullptr);
    // Double the current size (default to 4 if empty)
    size_t num_buckets = hm->ht1.tab ? (hm->ht1.mask + 1) * 2 : 4;
    h_init(&hm->ht2, num_buckets);
    hm->rehash_idx = 0;
}

void hm_insert(HashMap *hm, HNode *node) {
    if (!hm->ht1.tab) {
        h_init(&hm->ht1, 4);
    }
    h_insert(&hm->ht1, node);

    if (!hm->ht2.tab) {
        // Trigger resize if load factor exceeds 1.0
        size_t load_factor = hm->ht1.size / (hm->ht1.mask + 1);
        if (load_factor >= 1) {
            hm_start_resizing(hm);
        }
    }
    hm_help_resizing(hm); // Do a step of work
}

HNode *hm_lookup(HashMap *hm, HNode *key, bool (*cmp)(HNode *, HNode *)) {
    hm_help_resizing(hm);
    HNode **from = h_lookup(&hm->ht1, key, cmp);
    if (from && *from) return *from;

    // Check secondary table if resizing is active
    from = h_lookup(&hm->ht2, key, cmp);
    if (from && *from) return *from;
    
    return nullptr;
}

HNode *hm_detach(HashMap *hm, HNode *key, bool (*cmp)(HNode *, HNode *)) {
    hm_help_resizing(hm);
    HNode **from = h_lookup(&hm->ht1, key, cmp);
    if (from && *from) {
        HNode *node = *from;
        *from = node->next;
        hm->ht1.size--;
        return node;
    }

    from = h_lookup(&hm->ht2, key, cmp);
    if (from && *from) {
        HNode *node = *from;
        *from = node->next;
        hm->ht2.size--;
        return node;
    }

    return nullptr;
}

void hm_destroy(HashMap *hm) {
    free(hm->ht1.tab);
    free(hm->ht2.tab);
    *hm = HashMap();
}