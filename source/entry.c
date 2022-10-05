/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct entry_t *entry_create(char *key, struct data_t *data)
{
    struct entry_t *entry = malloc(sizeof(struct entry_t));

    if (entry == NULL)
    {
        perror("entry_create");
        exit(1);
    }

    entry->key = key;
    entry->value = data;
    return entry;
}

void entry_destroy(struct entry_t *entry)
{
    if (entry == NULL)
        return;

    data_destroy(entry->value);
    free(entry->key);
    free(entry);
}

struct entry_t *entry_dup(struct entry_t *entry)
{
    if (entry == NULL)
        return NULL;
    struct entry_t *dup = malloc(sizeof(struct entry_t));

    if (dup == NULL)
    {
        perror("entry_dup");
        exit(1);
    }
    dup->key = malloc(strlen(entry->key) + 1);

    if (dup->key == NULL)
    {
        perror("entry_dup");
        exit(1);
    }
    strcpy(dup->key, entry->key);

    dup->value = data_dup(entry->value);

    return dup;
}

void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value)
{
    if (entry == NULL)
        return;

    free(entry->key);
    data_destroy(entry->value);
    entry->key = new_key;
    entry->value = new_value;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2)
{
    if (entry1 == NULL || entry2 == NULL || entry1->key == NULL || entry2->key == NULL)
    {
        perror("entry_compare");
        exit(-1);
    }

    int comp = strcmp(entry1->key, entry2->key);

    if (comp == 0)
    {
        return 0;
    }
    else if (comp < 0)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}
