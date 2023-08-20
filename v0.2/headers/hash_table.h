#if !defined(hash_table_h)
	#define hash_table_h

	#include <stdbool.h>
	#include <stdint.h>

	// the list of entries which hashtable_get_entries returns 
	struct Entry {
		const char *key;
		const void *value;
	};


	// entry within the hashtable
	struct HashTableEntry {
		const char *key;

		const void *value;

		uint32_t hash;

		bool was_deleted;
	};

	struct HashTable {
		const void *type;

		uint32_t capacity;

		uint32_t count;

		struct HashTableEntry *entries;
	};

	extern const void *HashTable;

	#define IS_HASH_TABLE(ptr) ((ptr) && (((struct  HashTable *) ptr)->type == HashTable))


	// void print_hashtable(const void *hashtable);


	const void *hashtable_get(const void *hashtable, const char *key);
	void hashtable_put(void *hashtable, const char *key, const void *value);

	bool hashtable_contains(const void *hashtable, const char *key);

	// void hashtable_set(const void *hashtable, const char *key, const void *value);
	const void *hashtable_set(const void *hashtable, const char *key, const void *value);
	void hashtable_delete(void *hashtable, const char *key);


	struct Entry *hashtable_get_entries(void *hashtable);


#endif // hash_table_h
