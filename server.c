/* Copyright 2021 Blotiu Mihnea-Andrei */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

// Functie pentru initializarea unui server
server_memory* init_server_memory() {
	server_memory *new = (server_memory *)malloc(sizeof(server_memory));
	DIE(!new, "Eroare alocare memorie");

	new->server_ht = ht_create(10, hash_function_string, compare_function_strings);
	return new;
}

// Functie pentru adaugarea unei valori pe un server
void server_store(server_memory* server, char* key, char* value) {
	ht_put(server->server_ht, key, (strlen(key) + 1) * sizeof(char),
	value, (strlen(value) + 1) * sizeof(char));
}

// Functie pentru extragerea unei valori de pe un server
void server_remove(server_memory* server, char* key) {
	ht_remove_entry(server->server_ht, key);
}

// Functie pentru intoarcerea unei valori de pe un server
char* server_retrieve(server_memory* server, char* key) {
	char *result = (char *)ht_get(server->server_ht, key);
	return result;
}

// Functie pentru eliberarea memoriei asociata unui server
void free_server_memory(server_memory* server) {
	ht_free(server->server_ht);
	free(server);
}
