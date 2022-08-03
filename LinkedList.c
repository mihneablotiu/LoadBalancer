/* Copyright 2021 Blotiu Mihnea-Andrei */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LinkedList.h"
#include "utils.h"


// Functia de creare a listei
linked_list_t*
ll_create(unsigned int data_size)
{
    linked_list_t *list = (linked_list_t *)malloc(sizeof(linked_list_t));
    if (!list) {
        fprintf(stderr, "%s\n", "Eroare alocare memorie");
        exit(1);
    }
    list->head = NULL;
    list->data_size = data_size;
    list->size = 0;
    return list;
}

void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    // Daca vrem sa adaugam fara ca lista sa existe
    if (list == NULL)
        return;

    // Adaugam la final
    if (list->size > 0 && n > list->size - 1) {
        ll_node_t *new, *curr;
        new = (ll_node_t *)malloc(sizeof(ll_node_t));
        new->data = malloc(list->data_size);

        memcpy(new->data, new_data, list->data_size);
        new->next = NULL;
        curr = list->head;

        while (curr->next != NULL)
            curr = curr->next;
        curr->next = new;
        list->size++;

    } else if (list->size == 0 || n == 0) {  // Adaugam la inceput
        ll_node_t *new;
        new = (ll_node_t *)malloc(sizeof(ll_node_t));
        new->data = malloc(list->data_size);

        memcpy(new->data, new_data, list->data_size);
        new->next = list->head;

        list->head = new;
        list->size++;
    } else if (list->size > 0 && n > 0
    && n <= list->size - 1) {  // Adaugam pe pozitia n (la mijloc)
        ll_node_t *new, *prev;
        new = (ll_node_t *)malloc(sizeof(ll_node_t));
        prev = list->head;

        for (unsigned int i = 0; i < n-1; i++)
            prev = prev->next;

        new->data = malloc(list->data_size);
        memcpy(new->data, new_data, list->data_size);

        new->next = prev->next;
        prev->next = new;
        list->size++;
    }
}

// Functie pentru eliminarea unui nod de la pozitia n
ll_node_t*
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *prev, *curr;

    // Lista este goala sau nu are niciun element
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    // Eliminarea nodului de la finalul listei
    if (n > list->size - 1) {
        n = list->size - 1;
    }

    curr = list->head;
    prev = NULL;

    // Mergem la pozitia la care ne intereseaza
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        n--;
    }

    if (prev == NULL) {
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }

    list->size--;
    return curr;
}

// Functie pentru eliberarea memoriei
void
ll_free(linked_list_t** pp_list)
{
    ll_node_t* curr;

    if (pp_list == NULL || *pp_list == NULL) {
        return;
    }

    while ((*pp_list)->size > 0) {
        curr = ll_remove_nth_node(*pp_list, 0);
        free(curr->data);
        free(curr);
    }

    free(*pp_list);
    *pp_list = NULL;
}
