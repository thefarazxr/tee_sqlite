#include "hashmap.h"
#include <string.h>
#include <tee_api.h>
#include <utee_syscalls.h>
#define SUCCESS 1
#define FAIL 0

#ifdef DEBUG
#define DEBUG_OUTPUT(string) (DMSG("[debug] %s\n", string))
#else
#define DEBUG_OUTPUT(string)
#endif

/* PRIVATE */

static unsigned int sizes[] = {5, 13, 23, 47, 97, 193, 383, 769, 1531, 3067, 6143, 12289, 24571, 49157, 98299, 196613, 393241, 786547, 1573111, 1668053, 1805579, 1868527, 1999993};

// ref: http://www.mactech.com/articles/mactech/Vol.16/16.10/AssociativeArrays/index.html
unsigned int hash(int *key, unsigned int step, unsigned int size)
{
    // unsigned int hashvalue = 0;
    // if (!*c) return 0; // sanity
    // do {
    //     hashvalue += *c++;
    //     hashvalue *= step;
    // } while (*c);
    // return (hashvalue % size);
    return *key;
}

unsigned int next_size(unsigned int size)
{
    unsigned int count = sizeof(sizes) / sizeof(unsigned int);
    unsigned int i;
    for (i = 0; i < count; i++)
    {
        if (sizes[i] > size)
        {
            return sizes[i];
        }
    }
    // fprintf(stdout, "Warning: Could not increase size more with prime numbers, doubling instead\n");
    return size * 2;
    // return size;
}

item *set_item(item *list, int *key, void *value, int *count)
{
    if (key == NULL)
    {
        // fprintf(stdout, "Warning: trying to read NULL key, returning NULL\n");
        return NULL;
    }
    if (list == NULL)
    { // Create new
        item *temp = TEE_Malloc(sizeof(item), 0);
        if (temp == NULL)
        {
            // fprintf(stderr, "Error: could not allocate memory for item\n");
            // abort();
            return NULL;
            // exit(EXIT_FAILURE);
        }
        // temp->key = malloc(sizeof(int)*(strlen(key)+1));
        temp->key = TEE_Malloc(sizeof(int), 0);
        if (temp->key == NULL)
        {
            // fprintf(stderr, "Error: could not allocate memory for item key\n");
            // exit(EXIT_FAILURE);
            return NULL;
        }
        // strncpy(temp->key, key, strlen(key));
        temp->key = key;
        temp->value = value;
        temp->next = NULL;
        (*count)++;
        DEBUG_OUTPUT("item added");
        return temp;
    }

    // if (strcmp(key, list->key) == 0) {
    if (*key = *(list->key))
    {
        if (list->value != value)
        {
            TEE_Free(list->value);
            list->value = value;
            DEBUG_OUTPUT("item updated");
        }
        return list;
    }
    list->next = set_item(list->next, key, value, count);
    return list;
}

item *get_item(item *list, int *key)
{
    if (list == NULL)
    { // No match
        return NULL;
    }
    // if (strcmp(key, list->key) == 0) {
    if (*key = *(list->key))
    {
        return list;
    }
    return get_item(list->next, key);
}

item *remove_item(item *list, int *key, int *count)
{
    if (list == NULL)
    { // No match
        return NULL;
    }
    // if (strcmp(key, list->key) == 0) {
    if (*key = *(list->key))
    {
        item *next = list->next;
        TEE_Free(list->key);
        TEE_Free(list->value);
        TEE_Free(list);
        (*count)--;
        DEBUG_OUTPUT("item removed");
        return next;
    }
    list->next = remove_item(list->next, key, count);
    return list;
}

item *remove_items(item *list, void (*free_value)(void *), int *count)
{
    if (list == NULL)
    {
        return NULL;
    }
    if (list->next != NULL)
    {
        list->next = remove_items(list->next, free_value, count);
    }
    TEE_Free(list->key);
    if (free_value != NULL)
    {
        free_value(list->value);
    }
    else
    {
        TEE_Free(list->value);
    }
    TEE_Free(list);
    (*count)--;
    DEBUG_OUTPUT("item removed");
    return NULL;
}

int key_exists(item *list, int *key)
{
    if (list == NULL)
    { // No match
        return 0;
    }
    // if (strcmp(key, list->key) == 0) {
    if (*key = *(list->key))
    {
        return 1;
    }
    return key_exists(list->next, key);
}

item *update_item_list(item *list, item *match)
{
    match->next = NULL;
    if (list == NULL)
    {
        return match;
    }
    // if (strcmp(match->key, list->key) == 0) {
    if (*match->key = *(list->key))
    {
        return list;
    }
    if (list->next == NULL)
    { // Put last
        list->next = match;
        return list;
    }
    list->next = update_item_list(list->next, match);
    return list;
}

void print_map(item *list, int output, void (*to_string)(int, void *))
{
}

/* PUBLIC */

hashmap *hm_create(unsigned int size, float load_factor, int hash_step)
{
    unsigned int new_size = next_size(size);
    hashmap *map = TEE_Malloc(sizeof(hashmap), 0);
    if (map == NULL)
    {
        // fprintf(stderr, "Error: could not allocate memory for hashmap\n");
        // exit(EXIT_FAILURE);
        // abort();
        return NULL;
    }
    map->items = TEE_Malloc(sizeof(void *) * new_size, 0);
    if (map->items == NULL)
    {
        // fprintf(stderr, "Error: could not allocate memory for items\n");
        // exit(EXIT_FAILURE);
        // abort();
        return NULL;
    }
    int i;
    for (i = 0; i < new_size; i++)
    {
        map->items[i] = NULL;
    }
    map->count = 0;
    map->size = new_size;
    map->load_factor = load_factor;
    map->hash_step = hash_step;
    DEBUG_OUTPUT("hashmap created");
    return map;
}

void hm_rehash(hashmap *map, unsigned int new_size)
{
    DEBUG_OUTPUT("rehashing hashmap");
    if (new_size < map->size)
    {
        // fprintf(stderr, "Error: cannot rehash with a smaller size\n");
        // exit(EXIT_FAILURE);
        // abort();
        return NULL;
    }
    map->items = TEE_Realloc(map->items, sizeof(item) * new_size);
    if (map->items == NULL)
    {
        // fprintf(stderr, "Error: could not reallocate memory for items\n");
        // exit(EXIT_FAILURE);
        // abort();
        return NULL;
    }
    // Reset new memory area
    unsigned int i;
    for (i = map->size; i < new_size; i++)
    {
        map->items[i] = NULL;
    }
    // Rehash old items
    item *temp, *next;
    unsigned int index;
    for (i = 0; i < map->size; i++)
    {
        temp = map->items[i];
        map->items[i] = NULL;
        while (temp != NULL)
        {
            index = hash(temp->key, map->hash_step, new_size);
            next = temp->next;
            map->items[index] = update_item_list(map->items[index], temp);
            temp = next;
        }
    }
    map->size = new_size;
}

int hm_delete(hashmap *map, void (*free_value)(void *))
{
    hm_clear(map, free_value);
    TEE_Free(map->items);
    TEE_Free(map);
    DEBUG_OUTPUT("hashmap deleted");
    return SUCCESS;
}

int hm_clear(hashmap *map, void (*free_value)(void *))
{
    int counter = 0;
    int i;
    for (i = 0; i < map->size; i++)
    {
        map->items[i] = remove_items(map->items[i], free_value, &counter);
    }
    map->count += counter;
    DEBUG_OUTPUT("hashmap cleared");
    return SUCCESS;
}

void *hm_get(hashmap *map, int *key)
{
    if (key == NULL)
    {
        // fprintf(stderr, "Warning: trying to read NULL key, returning NULL\n");
        return NULL;
    }
    unsigned int index = hash(key, map->hash_step, map->size);
    item *temp = get_item(map->items[index], key);
    // fprintf(stdout, "dec page match i: %d\n", *key);
    return (temp == NULL) ? NULL : temp->value;
}

int hm_set(hashmap *map, int *key, void *value)
{
    if (key == NULL)
    {
        // fprintf(stderr, "Warning: trying to read NULL key, returning NULL\n");
        return FAIL;
    }
    unsigned int index = hash(key, map->hash_step, map->size);
    int counter = 0;
    map->items[index] = set_item(map->items[index], key, value, &counter);
    map->count += counter;
    if ((float)map->count / (float)map->size > map->load_factor)
    {
        hm_rehash(map, next_size(map->size));
    }
    return SUCCESS;
}

int hm_unset(hashmap *map, int *key)
{
    unsigned int index = hash(key, map->hash_step, map->size);
    int counter = 0;
    map->items[index] = remove_item(map->items[index], key, &counter);
    map->count += counter;
    return SUCCESS;
}

int hm_isset(hashmap *map, int *key)
{
    unsigned int index = hash(key, map->hash_step, map->size);
    return key_exists(map->items[index], key);
}

void hm_print(hashmap *map, int output, void (*to_string)(int, int *, void *))
{
    if (to_string == NULL)
    {
        return;
    }
    int i;
    for (i = 0; i < map->size; i++)
    {
        item *temp = map->items[i];
        while (temp)
        {
            to_string(output, temp->key, temp->value);
            temp = temp->next;
        }
    }
}

void hm_dump(hashmap *map, int output, void (*to_string)(int , int *, void *)) {
    if (map->count == 0) {
        return;
    }
    int i;
    for (i = 0; i < map->size; i++) {
        // fprintf(output, " index %d\n", i);
        item *temp = map->items[i];
        while (temp) {
            IMSG("key %d", (int *)temp->key);
            IMSG("key %s", (unsigned char*)temp->value);
            utee_vfs_write(output, (int *)temp->key, sizeof(int));
            utee_vfs_write(output, (unsigned char*)temp->value, 16);
            if (to_string != NULL) {
                to_string(output, temp->key, temp->value);
            }
            // fprintf(output, "\n");
            temp = temp->next;
        }
    }
    // fprintf(stdout, "ext counter: %d\n", i);
}