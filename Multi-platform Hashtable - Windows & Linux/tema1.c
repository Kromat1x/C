#include "tema1.h"

Table *initHash(unsigned int table_size)
{
	/*Initializarea unui tabel nou*/
	unsigned int i = 0;
	Table *table = (Table *)malloc(sizeof(Table));

	DIE(table == NULL, "INIT: Malloc table");
	/*Alocarea bucketurilor din tabel*/
	table->buckets = (Word **)malloc(sizeof(Word *)*table_size);
	DIE(table->buckets == NULL, "INIT: Malloc buckets");
	table->table_size = table_size;
	for (i = 0; i < table_size; i++)
		table->buckets[i] = NULL;
	return table;
}

void addWord(Table *table, char *word)
{
	/*Adaugarea unui cuvant intr-un tabel*/
	int key = hash(word, table->table_size);
	int duplicate = 0;
	Word *iterator = table->buckets[key];
	Word *prev = NULL;
	Word *newWord = (Word *) malloc(sizeof(Word));

	DIE(newWord == NULL, "ADD: Malloc newWord");
	newWord->value = malloc(sizeof(char) * strlen(word) + 1);
	strcpy(newWord->value, word);
	DIE(word == NULL, "ADD: Word is NULL");
	/*Se itereaza prin cuvintele aflate*/
	/*in bucketul cu cheia pe care o are*/
	/*si cuvantul pe care vrem sa il adaugam*/
	while (iterator != NULL) {
		if (strcmp(iterator->value, word) == 0)
			duplicate = 1;
		prev = iterator;
		iterator = iterator->next;
	}
	/*Daca nu exista deja, atunci adaugam cuvantul nou*/
	if (duplicate == 0) {
		if (prev == NULL)
			table->buckets[key] = newWord;
		else if (prev != NULL)
			prev->next = newWord;
		newWord->next = NULL;
	} else {
		/*Daca exista deja, se dezaloca newWord alocat mai sus*/
		free(newWord->value);
		free(newWord);
	}
}

void removeWord(Table *table, char *word)
{
	/*Stergerea unui cuvant din tabel*/
	int key = hash(word, table->table_size);
	int found = 0;
	Word *iterator = table->buckets[key];
	Word *prev = NULL;

	DIE(word == NULL, "REMOVE: Word is NULL");

	/*Caut prin bucketul cu cheia pe care o genereaza cuvantul primit*/
	if (iterator == NULL)
		return;

	/*Iterez prin cuvinte si caut cuvantul dat*/
	/*Daca gasesc cuvantul, actualizez legaturile*/
	/*si eliberez memoria*/
	if (strcmp(iterator->value, word) == 0) {
		table->buckets[key] = iterator->next;
		free(iterator->value);
		free(iterator);
	} else {
		while (iterator != NULL) {
			if (strcmp(iterator->value, word) == 0) {
				found = 1;
				break;
			}
			prev = iterator;
			iterator = iterator->next;
		}
		if (found == 1) {
			if (prev != NULL)
				prev->next = iterator->next;
			free(iterator->value);
			free(iterator);
		}
	}
}

int find(Table *table, char *word, char *filename)
{
	/*Caut un cuvant intr-un tabel*/
	int key = hash(word, table->table_size);
	Word *iterator = table->buckets[key];
	FILE *fd;
	int found = 0;

	DIE(word == NULL, "FIND: Word is NULL");

	/*Iterez prin cuvintele din bucketul de la cheia respectiva*/
	/*Daca am gasit cuvantul fac flagul found = 1*/
	while (iterator != NULL) {
		if (strcmp(iterator->value, word) == 0)
			found = 1;
		iterator = iterator->next;
	}
	/*Daca filename e NULL inseamna ca vreau output*/
	/*la stdout, in caz contrar scriu in fisier*/
	if (filename != NULL) {
		fd = fopen(filename, "a");
		DIE(fd < 0, "FIND: opening file");
		if (found == 1)
			fprintf(fd, "True\n");
		else
			fprintf(fd, "False\n");
		fclose(fd);
	}
	return found;
}

void clear(Table *table)
{
	/*Goleste tabela*/
	/*Se iau bucketurile la rand si elibereaza toate cuvintele*/
	/*Se pune capatul de linie al tabelului buckets[i] la NULL*/
	int i = 0;
	Word *iterator = NULL;
	Word *prev = NULL;

	for (i = 0; i < table->table_size; i++) {
		iterator = table->buckets[i];
		while (iterator != NULL) {
			prev = iterator;
			iterator = iterator->next;
			prev->next = NULL;
			free(prev->value);
			free(prev);
		}
		table->buckets[i] = NULL;
	}
}

Table *resize_double(Table **table)
{
	/*Redimensionarea tabelei*/
	/*Se creeaza un tabel nou care urmeaza a fi returnat*/
	/*Se citeste din vechiul tabel si se adauga in cel nou*/
	/*In final tabelul vechi este eliberat*/
	int new_size = 2 * (*table)->table_size;
	Table *new_table = initHash(new_size);
	Word *iterator;
	int i = 0;
	char *word;

	for (i = 0; i < (*table)->table_size; i++) {
		iterator = (*table)->buckets[i];
		while (iterator != NULL) {
			word = iterator->value;
			addWord(new_table, word);
			iterator = iterator->next;
		}
	}
	clear(*table);
	free((*table)->buckets);
	free(*table);

	return new_table;
}

Table *resize_halve(Table **table)
{
	/*Redimensionarea tabelei*/
	/*Se creeaza un tabel nou care urmeaza a fi returnat*/
	/*Se citeste din vechiul tabel si se adauga in cel nou*/
	/*In final tabelul vechi este eliberat*/
	int new_size = (*table)->table_size / 2;
	Table *new_table = initHash(new_size);
	Word *iterator;
	int i = 0;
	char *word;

	for (i = 0; i < (*table)->table_size; i++) {
		iterator = (*table)->buckets[i];
		while (iterator != NULL) {
			word = iterator->value;
			addWord(new_table, word);
			iterator = iterator->next;
		}
	}
	clear(*table);
	free((*table)->buckets);
	free(*table);

	return new_table;
}

void print_bucket(Table *table, unsigned int index_bucket, char *filename)
{
	/*Printarea unui anumit bucket*/
	/*Se itereaza prin cuvintele din bucket si se*/
	/*printeaza fiecare cuvant separat de spatiu*/
	/*pe aceeasi linie. Dupa caz in fisier sau*/
	/*la stdout*/
	FILE *fd;
	Word *iterator;
	int print = 0;

	if (filename == NULL) {
		iterator = table->buckets[index_bucket];
		if (iterator == NULL)
			print = 1;
		while (iterator != NULL) {
			printf("%s ", iterator->value);
			iterator = iterator->next;
		}
		if (print == 0)
			printf("\n");
		print = 0;
	} else {
		fd = fopen(filename, "a");
		DIE(fd < 0, "PRINT_BUCKET: opening file");

		iterator = table->buckets[index_bucket];
		while (iterator != NULL) {
			fprintf(fd, "%s ", iterator->value);
			iterator = iterator->next;
		}
		fprintf(fd, "\n");
		fclose(fd);
	}
}

void print(Table *table, char *filename)
{
	/*Printarea intregului tabel*/
	/*Se itereaza prin bucketuri si pentru fiecare se*/
	/*apeleaza functia print_bucket de mai sus*/
	int i = 0;

	for (i = 0; i < table->table_size; i++)
		print_bucket(table, i, filename);
}
