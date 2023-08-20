#include "hash_table.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "log.h"
#include "object.h"
#include "type.h"

#include "value.h"

#include "memory.h"

#define INITIAL_SIZE 16
#define LOAD_FACTOR 0.75



static void *hashtable_constructor(void *self, va_list *ap);
static void *hashtable_destructor(void *self);
// static void hashtable_printer(const void *self);

static const struct Type _type_hashtable = {
	.size 			= sizeof(struct HashTable),
	.constructor 	= hashtable_constructor,
	.destructor 	= hashtable_destructor
	// , hashtable_printer
};

const void *HashTable = &_type_hashtable;


static void *hashtable_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("hashtable_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	self->capacity = INITIAL_SIZE;
	self->count = 0;

	self->entries = allocate(sizeof(struct HashTableEntry), self->capacity);

	#if defined(DEBUG)
		vi_log("ret hashtable_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

static void *hashtable_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("hashtable_destructor(self: %x)\n", _self);
	#endif // DEBUG


	struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	struct HashTableEntry *entry = NULL;

	for(uint32_t i = 0; i < self->capacity; i++) {
		entry = self->entries + i;

		if(NULL == entry->key) continue;

		free((char *) entry->key);

		if(NULL != entry->value)
			delete((void *) entry->value);
	}

	free(self->entries);

	#if defined(DEBUG)
		vi_log("ret hashtable_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


/*
void print_hashtable(const void *_self) {
	const struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	if(0 == self->count) {
		printf(
			"\nHashTable { address: %x, capacity: %d, count: %d, entries: [] }\n",
			self, self->capacity, self->count
		);
		return;
	}

	printf("\nHashTable {\n");
	printf("    address: %x,\n", self);
	printf("    capacity: %d, count: %d,\n", self->capacity, self->count);
	printf("    entries: [\n");

	struct HashTableEntry *entry = NULL;

	for(int i = 0; i < self->capacity; i++) {
		entry = self->entries + i;

		if(NULL == entry->key) continue;

		printf("        Entry {\n");
		printf("            address: %x,\n", entry);
		printf("            key: \"%s\", hash: %x,\n", entry->key, entry->hash);
		printf("            value: ");

		print_value(entry->value);

		printf("\n        }\n");
	}

	printf("    ]\n}\n");
}
// */






// FNV1
uint32_t get_hash(const char* key, int len) {
	uint32_t hash = 2166136261u;

	for (int i = 0; i < len; i++) {
		hash ^= (uint8_t) key[i];
		hash *= 16777619;
	}

	return hash;
}

struct HashTableEntry *find_entry(struct HashTableEntry *entries, uint32_t capacity, const char *key, uint32_t hash) {
	uint32_t index = hash % capacity;
	struct HashTableEntry *entry = NULL;
	struct HashTableEntry *deleted_entry = NULL;

	while(1) {
		entry = entries + index;

		if(entry->key == NULL) {
			if(entry->was_deleted) {
				deleted_entry = entry;
			} else {
				return (deleted_entry != NULL) ? deleted_entry : entry;
			}
		}
		else if(entry->key != NULL) {
			if(strcmp(entry->key, key) == 0) {
				return entry;
			}
		}

		index = (index + 1) % capacity;
	}
}



static void grow_entries(void *_self) {
	struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	uint32_t old_size = self->capacity;
	uint32_t new_count = 0;

	self->capacity *= 2;

	struct HashTableEntry *new_entries = calloc(self->capacity, sizeof(struct HashTableEntry));
	struct HashTableEntry *entry;
	struct HashTableEntry *new_entry;

	for(int i = 0; i < old_size; i++) {
		entry = self->entries + i;
		if(NULL == entry->key) continue;

		new_entry = find_entry(new_entries, self->capacity, entry->key, entry->hash);
		new_entry->hash = entry->hash;
		new_entry->key = entry->key;
		new_entry->value = entry->value;
		new_entry->was_deleted = false;

		new_count++;
	}

	memset(self->entries, 0, (self->count * sizeof(struct HashTableEntry)));
	free(self->entries);

	self->entries = new_entries;
	self->count = new_count;
}


bool hashtable_contains(const void *_self, const char *key) {
	const struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	uint32_t hash = get_hash(key, strlen(key));
	struct HashTableEntry *entry = find_entry(self->entries, self->capacity, key, hash);

	return (NULL == entry->key);
}


void hashtable_put(void *_self, const char *key, const void *value) {
	struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	if(self->count >= (self->capacity * LOAD_FACTOR))
		grow_entries(self);

	uint32_t hash = get_hash(key, strlen(key));
	struct HashTableEntry *entry = find_entry(self->entries, self->capacity, key, hash);

	if(entry->key != NULL) {
		// some element was already there

		free((char *) entry->key);

		if(NULL != entry->value)
			delete((void *) entry->value);

		entry->key = NULL;
		entry->value = NULL;

		if(self->count > 0) {
			--self->count;
		}
	}

	char *p = allocate(sizeof(char), strlen(key) + 1);
	strcpy(p, key);

	entry->key = p;
	entry->value = value;
	entry->hash = hash;
	entry->was_deleted = false;

	self->count++;
}


const void *hashtable_get(const void *_self, const char *key) {
	const struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	uint32_t hash = get_hash(key, strlen(key));
	
	struct HashTableEntry *entry = find_entry(self->entries, self->capacity, key, hash);

	if(NULL == entry->key)
		return NULL;

	return entry->value;
}


const void *hashtable_set(const void *_self, const char *key, const void *value) {
	const struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	uint32_t hash = get_hash(key, strlen(key));
	struct HashTableEntry *entry = find_entry(self->entries, self->capacity, key, hash);

	if(NULL == entry->key)
		return NULL;		// no entry found

	if(NULL == entry->value)
		delete((void *) entry->value);

	entry->value = value;

	return value;
}


void hashtable_delete(void *_self, const char *key) {
	const struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	uint32_t hash = get_hash(key, strlen(key));
	struct HashTableEntry *entry = find_entry(self->entries, self->capacity, key, hash);

	if(entry->key == NULL) return;

	free((char *) entry->key);

	entry->key = NULL;
	entry->value = NULL;
	entry->hash = 0;
	entry->was_deleted = true;
}



struct Entry *hashtable_get_entries(void *_self) {
	const struct HashTable *self = _self;
	assert(IS_HASH_TABLE(self));

	struct Entry *htentries = NULL;
	uint32_t entry_count = 0;

	struct HashTableEntry *entry = NULL;

	for(uint32_t i = 0; i < self->capacity; i++) {
		entry = self->entries + i;

		if(NULL == entry->key) continue;

		htentries = reallocate(htentries, sizeof(struct Entry), ++entry_count);
		(htentries + (entry_count - 1))->key = entry->key;
		(htentries + (entry_count - 1))->value = entry->value;
	}

	htentries = reallocate(htentries, sizeof(struct Entry), ++entry_count);
	(htentries + (entry_count - 1))->key = NULL;
	(htentries + (entry_count - 1))->value = NULL;

	return htentries;
}

