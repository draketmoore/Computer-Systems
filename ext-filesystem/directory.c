// based on cs3650 starter code


#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <bsd/string.h>
#include "directory.h" 
#include "pages.h"

#define DIR_PAGE 1
#define DIRENT_MAX 64
/*
typedef struct my_dirent {
    char name[DIR_NAME];
    int  inum;
    char _reserved[12];
} dirent_t;
*/
void directory_init();

int directory_lookup(const char* name) {
    dirent_t *dirs = (dirent_t *)pages_get_page(1);
    for (int i = 0; i < DIRENT_MAX; i++) {
	    if (strcmp(name, dirs[i].name) == 0)
		    return i;
    }
    return DIRENT_MAX; 

}

int get_directory_inum(const char* name) {
    dirent_t *dirs = (dirent_t *)pages_get_page(1);
    return dirs[directory_lookup(name)].inum;
}

int tree_lookup(const char* path);

int directory_put(inode* dd, const char* name, int inum);

int directory_delete(inode* dd, const char* name);

slist* directory_list(const char* path);

void print_directory(inode* dd);

