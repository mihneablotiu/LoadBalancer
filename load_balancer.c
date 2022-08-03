/* Copyright 2021 Blotiu Mihnea-Andrei */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "load_balancer.h"
#include "utils.h"
#include "Hashtable.h"

#define MAX_SERVERS 1024
#define MAX_OBJECTS 1024

// Retinem asocierea id_server -> hash_server
typedef struct {
    unsigned int id_server;
    unsigned int hash;
}servers_hash_t;

// Retinem asocierea id_server -> adresa_server
typedef struct {
    unsigned int id_server;
    server_memory *address;
}servers_address_t;

struct load_balancer {
	servers_hash_t *servers_hash;  // Array de asocieri id_server - hash
    servers_address_t *servers_address;  // Array de asocieri id_server - adresa
    int size;  // Numarul de servere existente
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

// Functie pentru alocarea memoriei pentru load-balancer
// si pentru vectorii de asocieri mentionati anterior
load_balancer* init_load_balancer() {
	struct load_balancer *new = (load_balancer *)malloc(sizeof(load_balancer));
    DIE(!new, "Eroare alocare memorie");

    new->servers_hash = (servers_hash_t *)
    calloc(3 * 99999, sizeof(servers_hash_t));

    DIE(!new->servers_hash, "Eroare alocare memorie");

    new->servers_address = (servers_address_t *)
    malloc(3 * 99999 * sizeof(servers_address_t));

    DIE(!new->servers_address, "Eroare alocare memorie");

    new->size = 0;
    return new;
}

// Functie care verifica si adauga o anumita cheie pe serverul corespunzator
void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
    unsigned int key_hash = hash_function_key(key);
    for (int i = 0; i < main->size; i++) {
        if (key_hash < main->servers_hash[i].hash) {
            *server_id = main->servers_hash[i].id_server % 100000;
            server_store(main->servers_address[i].address, key, value);
            return;
        }
    }

    *server_id = main->servers_hash[0].id_server % 100000;
    server_store(main->servers_address[0].address, key, value);
}

// Analog cu functia anterioara doar ca nu intoarcem si valoarea
// ci doar pozitia serverului
int server_find(load_balancer* main, unsigned int index, char* key) {
    unsigned int key_hash = hash_function_key(key);
    for (int i = 0; i < main->size; i++) {
        if (key_hash < main->servers_hash[i].hash &&
        main->servers_hash[i].hash != index) {
            return i;
        }
    }

    for (int i = 0; i < main->size; i++) {
        if (main->servers_hash[i].hash != index) {
            return i;
        }
    }

    return 0;
}

// Functie care intoarce valoarea de la o anumita cheie
char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	unsigned int key_hash = hash_function_key(key);

    for (int i = 0; i < main->size; i++) {
        if (key_hash < main->servers_hash[i].hash) {
            *server_id = main->servers_hash[i].id_server % 100000;
            return server_retrieve(main->servers_address[i].address, key);
        }
    }

    *server_id = main->servers_hash[0].id_server % 100000;
    return server_retrieve(main->servers_address[0].address, key);
}

// Functie care realizeaza deplasarea elementelor la fiecare add
// pentru a le pastra in ordine crescatoare
void add_server(load_balancer *main, int server_id, int i,
int *ok, int index, server_memory **server) {
    for (int j = main->size; j > i; j--) {
        main->servers_hash[j].id_server = main->servers_hash[j - 1].id_server;
        main->servers_hash[j].hash = main->servers_hash[j - 1].hash;
        (main->servers_address[j].id_server =
        main->servers_address[j - 1].id_server);
        main->servers_address[j].address = main->servers_address[j - 1].address;
    }

    main->servers_hash[i].hash = index;
    main->servers_hash[i].id_server = server_id;
    main->servers_address[i].id_server = server_id % 100000;
    main->servers_address[i].address = *server;
    main->size++;
    *ok = 1;
}

// Functia de redistribuire a valorilor dupa adaugarea unui server
void redistribution(load_balancer *main, unsigned int index)
{
    for (int i = 0; i < main->size; i++) {
        if (main->servers_hash[i].hash == index) {
            int j = 0;
            if (i == main->size - 1) {
                j = 0;
            } else {
                j = i + 1;
            }

            if (main->servers_address[j].id_server ==
            main->servers_address[i].id_server) {
                continue;
            }

            hashtable_t *curr = main->servers_address[j].address->server_ht;
            server_memory *curr_server = main->servers_address[j].address;

            for (unsigned int k = 0; k < curr->hmax; k++) {
                ll_node_t *node = curr->buckets[k]->head;
                while (node != NULL) {
                    if (server_find(main, index,
                    ((struct info *)node->data)->key) != j) {
                        node = node->next;
                        continue;
                    }

                    unsigned int key_hash =
                    hash_function_key(((struct info *)node->data)->key);

                    if (j == 0) {
                        if (key_hash < index &&
                        key_hash > main->servers_hash[0].hash) {
                            server_memory *new =
                            main->servers_address[i].address;
                            server_store(new, ((struct info *)node->data)->key,
                            ((struct info *)node->data)->value);

                            char *key = ((struct info *)node->data)->key;
                            node = node->next;

                            server_remove(curr_server, key);
                        } else {
                            node = node->next;
                        }

                    } else if (j != 1) {
                        if (key_hash < index) {
                            server_memory *new =
                            main->servers_address[i].address;
                            server_store(new, ((struct info *)node->data)->key,
                            ((struct info *)node->data)->value);

                            char *key = ((struct info *)node->data)->key;
                            node = node->next;

                            server_remove(curr_server, key);
                        } else {
                            node = node->next;
                        }
                    } else {
                        if (key_hash < index ||
                        key_hash > main->servers_hash[1].hash) {
                            server_memory *new =
                            main->servers_address[i].address;
                            server_store(new, ((struct info *)node->data)->key,
                            ((struct info *)node->data)->value);

                            char *key = ((struct info *)node->data)->key;
                            node = node->next;

                            server_remove(curr_server, key);
                        } else {
                            node = node->next;
                        }
                    }
                }
            }
        }
    }
}

// Adaugarea efectiva a serverelor
void loader_add_server(load_balancer* main, int server_id) {
	int eticheta1 = 1 * pow(10, 5) + server_id;
    int eticheta2 = 2 * pow(10, 5) + server_id;
    unsigned int index = hash_function_servers(&server_id);
    unsigned int index1 = hash_function_servers(&eticheta1);
    unsigned int index2 = hash_function_servers(&eticheta2);

    int ok = 0, ok1 = 0, ok2 = 0;
    server_memory *server = init_server_memory();

    for (int i = 0; i <= main->size; i++) {
        if (i == main->size) {
            if (ok == 0) {
                add_server(main, server_id, i, &ok, index, &server);
                redistribution(main, index);
                i--;
            } else if (ok1 == 0) {
                add_server(main, eticheta1, i, &ok1, index1, &server);
                redistribution(main, index1);
                i--;
            } else if (ok2 == 0) {
                add_server(main, eticheta2, i, &ok2, index2, &server);
                redistribution(main, index2);
                i--;
            }
        } else {
            if (index < main->servers_hash[i].hash && ok == 0) {
                add_server(main, server_id, i, &ok, index, &server);
                redistribution(main, index);
                i--;
            } else if (index1 < main->servers_hash[i].hash && ok1 == 0) {
                add_server(main, eticheta1, i, &ok1, index1, &server);
                redistribution(main, index1);
                i--;
            } else if (index2 < main->servers_hash[i].hash && ok2 == 0) {
                add_server(main, eticheta2, i, &ok2, index2, &server);
                redistribution(main, index2);
                i--;
            }
        }

        if (ok == 1 && ok1 == 1 && ok2 == 1) {
            break;
        }
    }
}

// Functie de deplasare a elementelor in cazul in care se sterge un server
void left_shift(load_balancer *main, int pos) {
    for (int i = pos; i < main->size - 1; i++) {
        main->servers_address[i].address = main->servers_address[i + 1].address;
        (main->servers_address[i].id_server =
        main->servers_address[i + 1].id_server);
        main->servers_hash[i].id_server = main->servers_hash[i + 1].id_server;
        main->servers_hash[i].hash = main->servers_hash[i + 1].hash;
    }
    main->size--;
}

// Functia de redistribuire a valorilor dupa stergerea unui server
void remove_redistribution(load_balancer *main,
unsigned int index, int *server1) {
    for (int i = 0; i < main->size; i++) {
        if (index == main->servers_hash[i].hash) {
            *server1 = i;

            hashtable_t *curr = main->servers_address[i].address->server_ht;
            server_memory *curr_server = main->servers_address[i].address;

            int pos;
            for (pos = 0; pos < main->size; pos++) {
                    if (main->servers_address[pos].id_server
                    != main->servers_address[i].id_server) {
                        break;
                }
            }

            for (unsigned int k = 0; k < curr->hmax; k++) {
                ll_node_t *node = curr->buckets[k]->head;
                while (node != NULL) {
                    unsigned int key_hash =
                    hash_function_key(((struct info *)node->data)->key);
                    int j = pos;

                    for (int p = 0; p < main->size; p++) {
                        if (key_hash < main->servers_hash[p].hash &&
                        main->servers_address[p].id_server !=
                        main->servers_address[i].id_server) {
                            j = p;
                            break;
                        }
                    }

                    server_memory *new = main->servers_address[j].address;
                    server_store(new, ((struct info *)node->data)->key,
                    ((struct info *)node->data)->value);

                    char *key = (char *)((struct info *)node->data)->key;
                    node = node->next;
                    server_remove(curr_server, key);
                }
            }
        }
    }
}

// Stergerea efectiva a serverelor
void loader_remove_server(load_balancer* main, int server_id) {
	int eticheta1 = 1 * pow(10, 5) + server_id;
    int eticheta2 = 2 * pow(10, 5) + server_id;
    unsigned int index = hash_function_servers(&server_id);
    unsigned int index1 = hash_function_servers(&eticheta1);
    unsigned int index2 = hash_function_servers(&eticheta2);

    int server1;
    remove_redistribution(main, index, &server1);
    free_server_memory(main->servers_address[server1].address);

    for (int i = 0; i < main->size; i++) {
        if (main->servers_hash[i].hash == index) {
            left_shift(main, i);
            break;
        }
    }

    for (int i = 0; i < main->size; i++) {
        if (main->servers_hash[i].hash == index1) {
            left_shift(main, i);
            break;
        }
    }

    for (int i = 0; i < main->size; i++) {
        if (main->servers_hash[i].hash == index2) {
            left_shift(main, i);
            break;
        }
    }
}

// Functei de free a intregului load-balancer
void free_load_balancer(load_balancer* main) {
    for (int i = 0; i < main->size; i++) {
        for (int j = i + 1; j < main->size; j++) {
            if (main->servers_address[j].id_server ==
            main->servers_address[i].id_server) {
                left_shift(main, j);
                j--;
            }
        }
        free_server_memory(main->servers_address[i].address);
        left_shift(main, i);
        i--;
    }

    free(main->servers_address);
    free(main->servers_hash);
    free(main);
}
