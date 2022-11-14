#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CAPACITY 50000 // Size of the Hash Table

char *sent_string;//String that will sent to client from server(based on clien's instriction)

char* concat(const char *s1, const char *s2)//concatenate two string
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

unsigned long hash_function(char* str)
{
    unsigned long i = 0;
    for (int j=0; str[j]; j++)
        i += str[j];
    return i % CAPACITY;
}

// Define the Hash Table Item here
typedef struct Ht_item Ht_item;
struct Ht_item
{
    char* key;
    char* value;
};

// Define the Linkedlist here
typedef struct LinkedList LinkedList;
struct LinkedList
{
    Ht_item* item;
    LinkedList* next;
};

// Define the Hash Table here
typedef struct HashTable HashTable;
struct HashTable
{
    // Contains an array of pointers
    // to items
    Ht_item** items;
    LinkedList** overflow_buckets;
    int size;
    int count;
};

static LinkedList* allocate_list ()// Allocates memory for a Linkedlist pointer
{
    LinkedList* list = (LinkedList*) malloc (sizeof(LinkedList));
    return list;
}

static LinkedList* linkedlist_insert(LinkedList* list, Ht_item* item)//Inserts the item onto the Linked List
{
    //Linked List is NULL
    if (!list)
    {
        LinkedList* head = allocate_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    }
    //Linked List is not NULL
    else if (list->next == NULL)
    {
        LinkedList* node = allocate_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }

    LinkedList* temp = list;
    while (temp->next->next)
    {
        temp = temp->next;
    }

    LinkedList* node = allocate_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;

    return list;
}

static void free_linkedlist(LinkedList* list)
{
    LinkedList* temp = list;
    while (list)
    {
        temp = list;
        list = list->next;
        free(temp->item->key);
        free(temp->item->value);
        free(temp->item);
        free(temp);
    }
}

static LinkedList** create_overflow_buckets(HashTable* table)// Create the overflow buckets;
{
    //Overflow buckets is an array of linkedlists
    LinkedList** buckets = (LinkedList**) calloc (table->size, sizeof(LinkedList*));
    for (int i=0; i<table->size; i++)
        buckets[i] = NULL;
    return buckets;
}

static void free_overflow_buckets(HashTable* table)
{
    // Free all the overflow bucket lists
    LinkedList** buckets = table->overflow_buckets;
    for (int i=0; i<table->size; i++)
        free_linkedlist(buckets[i]);
    free(buckets);
}

Ht_item* create_item(char* key, char* value)//create a hash table item
{
    // Creates a pointer to a new hash table item
    Ht_item* item = (Ht_item*) malloc (sizeof(Ht_item));
    item->key = (char*) malloc (strlen(key) + 1);
    item->value = (char*) malloc (strlen(value) + 1);

    strcpy(item->key, key);
    strcpy(item->value, value);

    return item;
}

HashTable* create_table(int size)// Creates a new HashTable
{
    HashTable* table = (HashTable*) malloc (sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->items = (Ht_item**) calloc (table->size, sizeof(Ht_item*));
    for (int i=0; i<table->size; i++)
        table->items[i] = NULL;
    table->overflow_buckets = create_overflow_buckets(table);

    return table;
}

void free_item(Ht_item* item)
{
    // Frees an item
    free(item->key);
    free(item->value);
    free(item);
}

void free_table(HashTable* table)
{
    // Frees the table
    for (int i=0; i<table->size; i++)
    {
        Ht_item* item = table->items[i];
        if (item != NULL)
            free_item(item);
    }

    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void handle_collision(HashTable* table, unsigned long index, Ht_item* item)
{
    LinkedList* head = table->overflow_buckets[index];

    if (head == NULL)
    {
        // We need to create the list
        head = allocate_list();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    else
    {
        // Insert to the list
        table->overflow_buckets[index] = linkedlist_insert(head, item);
        return;
    }
}

void ht_insert(HashTable* table, char* key, char* value)
{
    // Create the item
    Ht_item* item = create_item(key, value);

    // Compute the index
    unsigned long index = hash_function(key);

    Ht_item* current_item = table->items[index];

    if (current_item == NULL)
    {
        // Key does not exist.
        if (table->count == table->size)
        {
            // Hash Table Full
            sent_string="Insert Error: Hash Table is full!\n";
            //printf("Insert Error: Hash Table is full\n");
            // Remove the create item
            free_item(item);
            return;
        }

        // Insert directly
        table->items[index] = item;
        table->count++;
        sent_string=concat("[OK] Key value pair {",key);
        sent_string=concat(sent_string,", ");
        sent_string=concat(sent_string,value);
        sent_string=concat(sent_string,"} is stored!\n");
        //printf("Key value pair {%s, %s} is stored!\n",key,value);
    }

    else
    {
        sent_string=concat("[OK] Key value pair {",key);
        sent_string=concat(sent_string,", ");
        sent_string=concat(sent_string,value);
        sent_string=concat(sent_string,"} is stored!\n");
        //printf("Key value pair {%s, %s} is stored!\n",key,value);
        // Scenario 1: We only need to update value
        if (strcmp(current_item->key, key) == 0)
        {
            strcpy(table->items[index]->value, value);
            return;
        }

        else
        {
            // Scenario 2: Collision
            handle_collision(table, index, item);
            return;
        }
    }
}

char* ht_search(HashTable* table, char* key)
{
    // Searches the key in the hashtable
    // and returns NULL if it doesn't exist
    int index = hash_function(key);
    Ht_item* item = table->items[index];
    LinkedList* head = table->overflow_buckets[index];

    // Ensure that we move to items which are not NULL
    while (item != NULL)
    {
        if (strcmp(item->key, key) == 0)
            return item->value;
        if (head == NULL)
            return NULL;
        item = head->item;
        head = head->next;
    }
    return NULL;
}

void ht_delete(HashTable* table, char* key)
{
    // Deletes an item from the table
    int index = hash_function(key);
    Ht_item* item = table->items[index];
    LinkedList* head = table->overflow_buckets[index];

    if (item == NULL)
    {
        sent_string=concat("[OK] Key \"", key);
        sent_string=concat(sent_string,"\" does not exist, nothing to removed!\n");
        return;
    }
    else
    {
        sent_string=concat("[OK] Key \"", key);
        sent_string=concat(sent_string,"\" is removed!\n");
        if (head == NULL && strcmp(item->key, key) == 0)
        {
            // No collision chain. Remove the item
            // and set table index to NULL
            table->items[index] = NULL;
            free_item(item);
            table->count--;
            return;
        }
        else if (head != NULL)
        {
            // Collision Chain exists
            if (strcmp(item->key, key) == 0)
            {
                // Remove this item and set the head of the list as the new item
                free_item(item);
                LinkedList* node = head;
                head = head->next;
                node->next = NULL;
                table->items[index] = create_item(node->item->key, node->item->value);
                free_linkedlist(node);
                table->overflow_buckets[index] = head;
                return;
            }

            LinkedList* curr = head;
            LinkedList* prev = NULL;

            while (curr)
            {
                if (strcmp(curr->item->key, key) == 0)
                {
                    if (prev == NULL)
                    {
                        // First element of the chain. Remove the chain
                        free_linkedlist(head);
                        table->overflow_buckets[index] = NULL;
                        return;
                    }
                    else
                    {
                        // This is somewhere in the chain
                        prev->next = curr->next;
                        curr->next = NULL;
                        free_linkedlist(curr);
                        table->overflow_buckets[index] = head;
                        return;
                    }
                }
                curr = curr->next;
                prev = curr;
            }

        }
    }
}

void print_search(HashTable* table, char* key)
{
    char* val;
    if ((val = ht_search(table, key)) == NULL)
    {
        sent_string=concat("[OK] Key \"", key);
        sent_string=concat(sent_string,"\" does not exist!\n");
        return;
    }
    else
    {
        sent_string=concat("[OK] The value of \"", key);
        sent_string=concat(sent_string,"\" is ");
        sent_string=concat(sent_string,val);
        sent_string=concat(sent_string,"!\n");
    }
}




