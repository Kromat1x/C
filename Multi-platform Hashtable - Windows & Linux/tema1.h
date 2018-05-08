#include "hash.h"
#include "utils.h"
#include <string.h>
#include <ctype.h>

typedef struct Word {
	char *value;
	struct Word *next;

} Word;

typedef struct Table {
	int table_size;

	Word **buckets;

} Table;

Table *initHash(unsigned int table_size);
void addWord(Table *table, char *word);
void removeWord(Table *table, char *word);
int find(Table *table, char *word, char *filename);
void clear(Table *table);
void print_bucket(Table *table, unsigned int index_bucket, char *filename);
void print(Table *table, char *filename);
Table *resize_double(Table **table);
Table *resize_halve(Table **table);
