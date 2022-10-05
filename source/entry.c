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
    struct entry_t *entry_dup = malloc(sizeof(struct entry_t));

    if (entry_dup == NULL)
    {
        perror("entry_dup");
        exit(1);
    }
    entry_dup->key = malloc(strlen(entry->key) + 1);

    if (entry_dup->key == NULL)
    {
        perror("entry_dup");
        exit(1);
    }
    strcpy(entry_dup->key, entry->key);

    entry_dup->value = data_dup(entry->value);

    return entry_dup;
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
