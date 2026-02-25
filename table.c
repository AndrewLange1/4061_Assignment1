#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./include/table.h"

bucket_t *bucket_init(const char ip[IP_LEN]) {
    if(ip == NULL){
        return NULL;
    }
    bucket_t *b = malloc(sizeof(bucket_t));
    if(b == NULL){
        return NULL;
    }
    b->next = NULL;
    strncpy(b->ip, ip, IP_LEN);
    b->ip[IP_LEN -1] = '\0';
    b->requests = 1;
    return b;
}

table_t *table_init() {
    table_t *t = calloc(1, sizeof(table_t));
    if (t == NULL) {
        return NULL;
    }
    return t;
}

void table_print(const table_t *table) {
    if(table == NULL){
        return;
    }
    for(int i = 0; i < TABLE_LEN; i++){
        bucket_t *curr = table->buckets[i];
        while(curr != NULL){
            printf("%s - %d\n", curr->ip, curr->requests);
            curr = curr->next;
        }
    }
    return;
}

void table_free(table_t *table) {
    if(table == NULL){
        return;
    }
    for(int i = 0; i < TABLE_LEN; i++){
        bucket_t *curr = table->buckets[i];
        while(curr != NULL){
            bucket_t *next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(table);
    return;
}

int table_add(table_t *table, bucket_t *bucket) {
    if(table == NULL || bucket == NULL || bucket->ip[0] == '\0'){
        return -1;
    }
    int idx = hash_ip(bucket->ip);
    if(idx == -1){
        return -1;
    }
    bucket->next = table->buckets[idx];
    table->buckets[idx] = bucket;
    return 0;
}

bucket_t *table_get(table_t *table, const char ip[IP_LEN]) {
    if(table == NULL || ip == NULL){
        return NULL;
    }
    int idx = hash_ip(ip);
    if(idx == -1){
        return NULL;
    }
    bucket_t *curr = table->buckets[idx];
    while(curr != NULL){
        if(strcmp(curr->ip, ip) == 0){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

int hash_ip(const char ip[IP_LEN]) {
    if(ip == NULL){
        return -1;
    }
    int sum = 0;
    int i = 0;
    while(ip[i] != '\0'){
        sum += ip[i];
        i++;
    }
    return sum % TABLE_LEN;
}

int table_to_file(table_t *table, const char out_file[MAX_PATH]) {
    if(table == NULL || out_file == NULL){
        return -1;
    }
    FILE *f = fopen(out_file, "wb");
    if(f == NULL){
        perror("fopen");
        return -1;
    }
    for(int i = 0; i < TABLE_LEN; i++){
        bucket_t *curr = table->buckets[i];
        while(curr != NULL){
            if(fwrite(curr, sizeof(bucket_t), 1, f) != 1){
                perror("fwrite");
                fclose(f);
                return -1;
            }
            curr = curr->next;
        }
    }
    fclose(f);
    return 0;
}

table_t *table_from_file(const char in_file[MAX_PATH]) {
    if(in_file == NULL){
        return NULL;
    }

    FILE *f = fopen(in_file, "rb");
    if(f == NULL){
        perror("fopen");
        return NULL;
    }

    table_t *table = table_init();
    if(table == NULL){
        fclose(f);
        return NULL;
    }

    bucket_t temp;
    while(fread(&temp, sizeof(bucket_t), 1, f) == 1){
        bucket_t *b = malloc(sizeof(bucket_t));
        if(b == NULL){
            table_free(table);
            fclose(f);
            return NULL;
        }
        b->next = NULL;
        strncpy(b->ip, temp.ip, IP_LEN);
        b->ip[IP_LEN -1] = '\0';
        b->requests = temp.requests;
        if(table_add(table, b) == -1){
            free(b);
            table_free(table);
            fclose(f);
            return NULL;
        }
    }

    if(ferror(f)){
        table_free(table);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return table;
}
