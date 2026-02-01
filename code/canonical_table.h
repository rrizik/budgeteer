#ifndef CANONICAL_TABLE_H
#define CANONICAL_TABLE_H

typedef struct CanonicalEntry{
    u32 count;
    String8 name;
    u32 merchant_id;
} CanonicalEntry;

typedef struct CanonicalNode{
    CanonicalNode* next;
    u64 hash;
    String8 key;
    CanonicalEntry* value;
} CanonicalNode;

#define TABLE_DEFAULT_COUNT 1024
typedef struct CanonicalTable{
    Arena* arena;
    u64 count;
    CanonicalNode** slots;
} CanonicalTable;

static CanonicalTable*
make_canonical_table(Arena* arena, u64 count=0){
    CanonicalTable* table = push_array(arena, CanonicalTable, 1);
    table->arena = arena;
    table->count = count == 0 ? TABLE_DEFAULT_COUNT : count;
    table->slots = push_array(arena, CanonicalNode*, table->count);
    return(table);
}

static u64 
hash_from_string(String8 string){
    // djb2 hash
    u64 result = 5381;
    for(u64 i = 0; i < string.count; ++i){
        result = ((result << 5) + result) + string.str[i];
    }
    return(result);
}

static CanonicalEntry*
canonical_table_lookup(CanonicalTable* table, String8 key){
    u64 hash = hash_from_string(key);
    u64 slot_idx = hash % table->count;

    for(CanonicalNode* n = table->slots[slot_idx]; n != 0; n = n->next){
        if(n->hash == hash && str8_compare(n->key, key)){
            return(n->value);
        }
    }

    return(0);
}

static void
canonical_table_insert(CanonicalTable* table, String8 key, CanonicalEntry* value){
    u64 hash = hash_from_string(key);
    u64 slot_idx = hash % table->count;

    for(CanonicalNode* n = table->slots[slot_idx]; n != 0; n = n->next){
        if(n->hash == hash && str8_compare(n->key, key)){
            n->value = value;
            return;
        }
    }

    CanonicalNode* node = push_struct(table->arena, CanonicalNode);
    node->value = value;
    node->hash = hash;
    node->key = key;
    node->next = table->slots[slot_idx];
    table->slots[slot_idx] = node;
}

#endif

