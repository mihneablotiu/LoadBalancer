/* Copyright 2021 Blotiu Mihnea-Andrei */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"

#define MAX_BUCKET_SIZE 64


// Functie de comparare a cheilor:
int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

// Functie de hash-ing
unsigned int
hash_function_string(void *a)
{
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}


// Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	hashtable_t *new = (hashtable_t *)malloc(sizeof(hashtable_t));
	DIE(!new, "Eroare alocare memorie hash");
	new->size = 0;
	new->hmax = hmax;
	new->hash_function = hash_function;
	new->compare_function = compare_function;
	new->buckets = (linked_list_t **)malloc(new->hmax * sizeof(linked_list_t *));
	DIE(!new->buckets, "Eroare alocare memorie lista");
	for (unsigned int i = 0; i < new->hmax; i++) {
		new->buckets[i] = ll_create(sizeof(struct info));
	}
	return new;
}

// Functie pentru adaugarea unui element intr-un hashtable
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	struct info data;
	data.key = malloc(key_size);
	DIE(!data.key, "Eroare alocare memorie");
	memcpy(data.key, key, key_size);
	data.value = malloc(value_size);
	DIE(!data.value, "Eroare alocare memorie");
	memcpy(data.value, value, value_size);
	ll_node_t *curr = ht->buckets[index]->head;
	for(unsigned int i = 0; i < ht->buckets[index]->size; i++) {
		if (ht->compare_function(data.key, ((struct info *)(curr->data))->key) == 0) {
			memcpy(((struct info *)(curr->data))->value, data.value, value_size);
			free(data.value);
			free(data.key);
			return;
		}
		curr = curr->next;
	}
	ll_add_nth_node(ht->buckets[index], ht->buckets[index]->size, &data);
	ht->size++;
}

// Functie pentru extragerea unui element din hashtable
void *
ht_get(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *curr = ht->buckets[index]->head;
	for(unsigned int i = 0; i < ht->buckets[index]->size; i++) {
		if (ht->compare_function(key, ((struct info *)(curr->data))->key) == 0) {
			return ((struct info *)(curr->data))->value;
		}

		curr = curr->next;
	}
	return NULL;
}

// Functie pentru eliminarea unui element dintr-un hashtable
void
ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *curr = ht->buckets[index]->head;
	ll_node_t *removed;
	for(unsigned int i = 0; i < ht->buckets[index]->size; i++) {
		if (ht->compare_function(key, ((struct info *)(curr->data))->key) == 0) {
			removed = ll_remove_nth_node(ht->buckets[index], i);
			break;
		}

		curr = curr->next;
	}
	if (curr != NULL) {
		free(((struct info *)(removed->data))->value);
		free(((struct info *)(removed->data))->key);
		free(removed->data);
		free(removed);
		ht->size--;
	}
}

// Functia de free pentru un hashtable
void
ht_free(hashtable_t *ht)
{
	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t *curr = ht->buckets[i]->head;
		while(curr != NULL) {
			free(((struct info *)(curr->data))->value);
			free(((struct info *)(curr->data))->key);
			curr = curr->next;
		}
		ll_free(&ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}
