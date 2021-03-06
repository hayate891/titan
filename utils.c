/*
 * Copyright (C) 2017 Niko Rosvall <niko@byteptr.com>
 */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include "utils.h"
#include "crypto.h"

bool file_exists(const char *path)
{
    struct stat buf;

    if(stat(path, &buf) != 0)
        return false;
    
    return true;
}

/* Function checks that we have a valid path
 * in our lock file and if the database is not
 * encrypted.
 */
bool has_active_database()
{
    char *path = NULL;

    path = get_lockfile_path();

    if(!path)
        return false;

    struct stat buf;

    if(stat(path, &buf) != 0)
    {
        free(path);
        return false;
    }

    //If the database is encrypted, it's not active so return false
    if(is_file_encrypted(path))
    {
        free(path);
        return false;
    }

    free(path);

    return true;
}

/* Returns the path of ~/.titan.lock file.
 * Caller must free the return value */
char *get_lockfile_path()
{
    char *home = NULL;
    char *path = NULL;

    home = getenv("HOME");

    if(!home)
        return NULL;

    /* /home/user/.titan.lock */
    path = tmalloc(sizeof(char) * (strlen(home) + 13));

    strcpy(path, home);
    strcat(path, "/.titan.lock");

    return path;
}

/* Reads and returns the path of currently decrypted
 * database. Caller must free the return value */
char *read_active_database_path()
{
    FILE *fp = NULL;
    char *path = NULL;
    char *lockpath = NULL;
    size_t len;

    path = get_lockfile_path();

    if(!path)
        return NULL;

    fp = fopen(path, "r");

    if(!fp)
    {
        free(path);
        return NULL;
    }

    /* We only need the first line from the file */

    if(getline(&lockpath, &len, fp) < 0)
    {
        if(lockpath)
            free(lockpath);

        fclose(fp);
        free(path);

        return NULL;
    }

    fclose(fp);
    free(path);

    return lockpath;
}

void write_active_database_path(const char *db_path)
{
    FILE *fp = NULL;
    char *path = NULL;

    path = get_lockfile_path();

    if(!path)
        return;

    fp = fopen(path, "w");

    if(!fp)
    {
        fprintf(stderr, "Error creating lock file\n");
        free(path);
        return;
    }

    fprintf(fp, "%s", db_path);
    fclose(fp);

    free(path);
}

//Simple malloc wrapper to prevent enormous error
//checking every where in the code
void *tmalloc(size_t size)
{
    void *data = NULL;

    data = malloc(size);

    if(data == NULL)
    {
        fprintf(stderr, "Malloc failed. Abort.\n");
        abort();
    }

    return data;
}
