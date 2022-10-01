#include "entry.h"

struct entry_t *entry_create(char *key, struct data_t *data)
{
}

void entry_destroy(struct entry_t *entry)
{
}

struct entry_t *entry_dup(struct entry_t *entry)
{
}

void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value)
{
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2)
{
}
