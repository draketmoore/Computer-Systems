// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME 48

#include "slist.h"
#include "pages.h"
#include "inode.h"

typedef struct my_dirent {
    char name[DIR_NAME];
    int  inum;
    char _reserved[12];
} dirent_t;

void directory_init();
int directory_lookup(const char* name);
int get_directory_inum(const char* name);
int tree_lookup(const char* path);
int directory_put(inode* dd, const char* name, int inum);
int directory_delete(inode* dd, const char* name);
slist* directory_list(const char* path);
void print_directory(inode* dd);

#endif

