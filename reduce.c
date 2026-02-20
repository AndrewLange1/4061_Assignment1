#include "./include/reduce.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>   
#include <errno.h>

#include "./include/table.h"

int main(int argc, char *argv[]) {
    if(argc != 5){
        printf("Usage: reduce <read dir> <out file> <start ip> <end ip>\n");
        return 1;
    }

    char *read_dir = argv[1];
    char *out_file = argv[2];
    char *start_str = argv[3];
    char *end_str = argv[4];
    char *endptr;

    long start = strtol(start_str, &endptr, 10);
    if(*start_str == '\0' || *endptr != '\0'){
        printf("reduce: invalid IP range\n");
        return 1;
    }

    long end = strtol(end_str, &endptr, 10);
    if(*end_str == '\0' || *endptr != '\0'){
        printf("reduce: invalid IP range\n");
        return 1;
    }

    if(start < 0 || start >= 256 || end < 0 || end > 256 || start >= end){
        printf("reduce: invalid IP range\n");
        return 1;
    }

    table_t *master = table_init();
    if(master == NULL){
        return 1;
    }

    DIR *d = opendir(read_dir);
    if(d == NULL){
        perror("opendir");
        table_free(master);
        return 1;
    }

    struct dirent *ent;
    while((ent = readdir(d)) != NULL){
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0){
            continue;
        }
        char path[MAX_PATH];
        snprintf(path, MAX_PATH, "%s/%s", read_dir, ent->d_name);

        if(reduce_file(master, path, (int)start, (int)end) != 0){
            table_free(master);
            closedir(d);
            return 1;
        }
    }
    closedir(d);

    if(table_to_file(master, out_file) != 0){
        table_free(master);
        return 1;
    }

    table_free(master);
    return 0;
}

int reduce_file(table_t *table, const char file_path[MAX_PATH], const int start_ip,
                const int end_ip) {
    if(table == NULL || file_path == NULL){
        return 1;
    }
    table_t *temp = table_from_file(file_path);
    if(temp == NULL){
        return 1;
    }
    for(int i = 0; i < TABLE_LEN; i++){
        bucket_t *curr = temp->buckets[i];
        while(curr != NULL){
            int ipByte; 
            if (sscanf(curr->ip, "%d", &ipByte) != 1) {
                curr = curr->next;
                continue;
            }
            if(ipByte >= start_ip && ipByte < end_ip){
                bucket_t *exists = table_get(table, curr->ip);
                if(exists != NULL){
                    exists->requests += curr->requests;
                }
                else{
                    bucket_t *b = malloc(sizeof(bucket_t));
                    if(b == NULL){
                        table_free(temp);
                        return 1;
                    }
                    b->next = NULL;
                    strncpy(b->ip, curr->ip, IP_LEN);
                    b->ip[IP_LEN -1] = '\0';
                    b->requests = curr->requests;
                    if(table_add(table, b) == -1){
                        free(b);
                        table_free(temp);
                        return 1;
                    }
                }
            }
            curr = curr->next;
        }
    }
    table_free(temp);
    return 0;
}
