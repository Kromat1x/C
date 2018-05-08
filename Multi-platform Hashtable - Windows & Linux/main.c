#include "tema1.h"

void execute_commands(Table **my_table, char *line, char *number)
{
	/*Functie ce executa o comanda folosind functiile*/
	/*particulare din tema1.c. Primeste o linie citita*/
	/*fie dintr-un fisier, fie de la stdin si il imparte*/
	/*in tokene si apeleaza functia corespunzatoare*/
	char *token = NULL;
	int found = 0;
	unsigned int i = 0;
	char *word_to_find = NULL;

	if (strcmp(line, "\n") != 0) {
		token = strtok(line, "\n ");
		if (strcmp(token, "add") == 0) {
			token = strtok(NULL, "\n ");
			DIE(token == NULL, "ADDWORD: Invalid add use");
			addWord((*my_table), token);
		} else if (strcmp(token, "print") == 0) {
			token = strtok(NULL, "\n ");
			if (token == NULL)
				print((*my_table), NULL);
			else
				print((*my_table), token);
		} else if (strcmp(token, "remove") == 0) {
			token = strtok(NULL, "\n ");
			DIE(token == NULL, "REMOVEWORD: Invalid remove use");
			removeWord((*my_table), token);
		} else if (strcmp(token, "find") == 0) {
			token = strtok(NULL, "\n ");
			DIE(token == NULL, "FIND: Invalid find use");
			word_to_find = malloc(sizeof(char) * strlen(token) + 1);
			strcpy(word_to_find, token);
			token = strtok(NULL, "\n ");
			if (token != NULL) {
				find((*my_table), word_to_find, token);
			} else {
				found = find((*my_table), word_to_find, NULL);
				if (found == 1)
					printf("True\n");
				else
					printf("False\n");
			}
			free(word_to_find);
		} else if (strcmp(token, "clear") == 0)
			clear(*my_table);
		else if (strcmp(token, "resize") == 0) {
			token = strtok(NULL, "\n ");
			if (strcmp(token, "double") == 0)
				*my_table = resize_double(my_table);
			else if (strcmp(token, "halve") == 0)
				*my_table = resize_halve(my_table);
			else
				DIE(1, "RESIZE: Invalid resize use");
		} else if (strcmp(token, "print_bucket") == 0) {
			token = strtok(NULL, "\n ");
			number = malloc(sizeof(char) * strlen(token) + 1);
			strcpy(number, token);
			for (i = 0; i < strlen(number); i++)
				DIE(!isdigit(number[i]), "PB: format");
			token = strtok(NULL, "\n ");
			print_bucket((*my_table), atoi(number), token);
			free(number);
		} else
			DIE(1, "FILE: Invalid file command");
	}
}


int main(int argc, char *argv[])
{
	Table *my_table;
	unsigned int i = 0;
	int fileno = 2;
	FILE *fd;
	char line[20000];
	char *number = NULL;

	DIE(argc < 2, "Need more arguements");
	number = malloc(sizeof(char) * strlen(argv[1]) + 1);
	strcpy(number, argv[1]);
	for (i = 0; i < strlen(argv[1]); i++)
		DIE(!isdigit(number[i]), "INIT: Invalid number format");
	my_table = initHash(atoi(argv[1]));
	/*Citirea din fisier/e sau de la stdin si*/
	/*apelul functiei de executie a comenzii*/
	if (argc > 2) {
		for (fileno = 2; fileno < argc; fileno++) {
			fd = fopen(argv[fileno], "r");
			while (fgets(line, 20000, fd) != NULL)
				execute_commands(&my_table, line, number);
			fclose(fd);
		}
	} else
		while (fgets(line, 20000, stdin) != NULL)
			execute_commands(&my_table, line, number);

	free(number);
	clear(my_table);
	free(my_table->buckets);
	free(my_table);
	return 0;
}
