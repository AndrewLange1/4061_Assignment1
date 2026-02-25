#include "./include/map.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./include/table.h"

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Usage: map <outfile> <infiles...>\n");
        return 1;
    }
    table_t *table = table_init();
    if (table == NULL) {
        fprintf(stderr, "Table_init failed\n");
        return 1;
    }
    for(int i = 2; i < argc; i++){
        int log = map_log(table, argv[i]);
        if (log != 0) {
            table_free(table);
            return 1;
        }
    }
    int ttf = table_to_file(table, argv[1]);
    if (ttf != 0) {
        table_free(table);
        return 1;
    }
    table_free(table);
    return 0;
}

int map_log(table_t *table, const char file_path[MAX_PATH]) {
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        perror("fopen");
        return 1;
    }

    char line[256];
    char timestamp[20];
    char ip[16];
    char method[8];
    char path[37];
    char status[4];

    while(fgets(line, sizeof(line), f) != NULL) {
        int match = sscanf(line, "%19[^,],%15[^,],%7[^,],%36[^,],%3s", timestamp, ip, method, path, status);
        if (match == 5) {
            bucket_t *bucket = table_get(table, ip);
            if (bucket != NULL) {bucket->requests++;}
            else {
                bucket_t *new_bucket = bucket_init(ip);
                if (new_bucket == NULL) {
                    fclose(f);
                    return 1;
                }
                if (table_add(table, new_bucket) != 0) {
                    free(new_bucket);
                    fclose(f);
                    return 1;
                }
            }
        }
    }

    fclose(f);
    return 0;
}
