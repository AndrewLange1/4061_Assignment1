#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./include/table.h"

#define MAX_FILES 4096
#define MAX_BUF 4096

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: mapreduce <directory> <n mappers> <n reducers>\n");
        return 1;
    }

    const char *dir = argv[1];
    int n_mappers = atoi(argv[2]);
    int n_reducers = atoi(argv[3]);

    if (n_mappers < 1 || n_reducers < 1) {
        printf("mapreduce: cannot have less than one mapper or reducer\n");
        return 1;
    }

    DIR *d = opendir(dir);
    if (d == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *ent;
    char *files[MAX_FILES];
    int filecount = 0;

    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        if (filecount >= MAX_FILES) {
            closedir(d);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }

        char *dup = strdup(ent->d_name);
        if (dup == NULL) {
            perror("strdup");
            closedir(d);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }

        files[filecount++] = dup;
    }

    closedir(d);

    pid_t *mapper_pids = (pid_t *)malloc(sizeof(pid_t) * n_mappers);
    if (mapper_pids == NULL) {
        perror("malloc");
        for (int k = 0; k < filecount; k++) {
            free(files[k]);
        }
        return 1;
    }

    int base = filecount / n_mappers;
    int rem = filecount % n_mappers;
    int idx = 0;

    for (int i = 0; i < n_mappers; i++) {
        int count = base;
        if (i < rem) {
            count = count + 1;
        } else {
            count = count;
        }
        int start = idx;
        int end = idx + count;
        idx = end;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(mapper_pids);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }

        if (pid == 0) {
            char outfile[MAX_BUF];
            char numbuf[16];

            sprintf(numbuf, "%d", i);
            strcpy(outfile, "./intermediate/");
            strcat(outfile, numbuf);
            strcat(outfile, ".tbl");

            int argc_map = count + 3;
            char **map_argv = (char **)malloc(sizeof(char *) * argc_map);
            if (map_argv == NULL) {
                perror("malloc");
                _exit(1);
            }

            map_argv[0] = (char *)"map";
            map_argv[1] = outfile;

            int pos = 2;
            for (int j = start; j < end; j++) {
                char *full = (char *)malloc(MAX_BUF);
                if (full == NULL) {
                    perror("malloc");
                    _exit(1);
                }
                strcpy(full, dir);
                strcat(full, "/");
                strcat(full, files[j]);
                map_argv[pos++] = full;
            }
            map_argv[pos] = NULL;

            execv("./map", map_argv);
            perror("execv");
            _exit(1);
        }

        mapper_pids[i] = pid;
    }

    for (int i = 0; i < n_mappers; i++) {
        int status = 0;
        if (waitpid(mapper_pids[i], &status, 0) < 0) {
            perror("waitpid");
            free(mapper_pids);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }
        if (status != 0) {
            free(mapper_pids);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }
    }

    free(mapper_pids);

    pid_t *reducer_pids = (pid_t *)malloc(sizeof(pid_t) * n_reducers);
    if (reducer_pids == NULL) {
        perror("malloc");
        for (int k = 0; k < filecount; k++) {
            free(files[k]);
        }
        return 1;
    }

    int ip_base = 256 / n_reducers;
    int ip_rem = 256 % n_reducers;
    int start_ip = 0;

    for (int i = 0; i < n_reducers; i++) {
        int span = ip_base;
        if (i < ip_rem) {
            span = span + 1;
        } else {
            span = span;
        }
        int end_ip = start_ip + span;

        char out_file[MAX_BUF];
        char numbuf[16];

        sprintf(numbuf, "%d", i);
        strcpy(out_file, "./out/");
        strcat(out_file, numbuf);
        strcat(out_file, ".tbl");

        char start_s[16], end_s[16];
        sprintf(start_s, "%d", start_ip);
        sprintf(end_s, "%d", end_ip);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(reducer_pids);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }

        if (pid == 0) {
            execl("./reduce", "reduce",
                  "./intermediate",
                  out_file,
                  start_s,
                  end_s,
                  (char *)NULL);
            perror("execl");
            _exit(1);
        }

        reducer_pids[i] = pid;
        start_ip = end_ip;
    }

    for (int i = 0; i < n_reducers; i++) {
        int status = 0;
        if (waitpid(reducer_pids[i], &status, 0) < 0) {
            perror("waitpid");
            free(reducer_pids);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }
        if (status != 0) {
            free(reducer_pids);
            for (int k = 0; k < filecount; k++) {
                free(files[k]);
            }
            return 1;
        }
    }

    free(reducer_pids);

    for (int k = 0; k < filecount; k++) {
        free(files[k]);
    }

    DIR *outd = opendir("./out");
    if (outd == NULL) {
        perror("opendir");
        return 1;
    }

    while ((ent = readdir(outd)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        char path[MAX_BUF];
        strcpy(path, "./out/");
        strcat(path, ent->d_name);

        table_t *t = table_from_file(path);
        if (t == NULL) {
            closedir(outd);
            return 1;
        }

        table_print(t);
        table_free(t);
    }

    closedir(outd);
    return 0;
}