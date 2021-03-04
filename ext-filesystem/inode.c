// based on cs3650 starter code

#include <stdint.h>
#include <assert.h>
#include "pages.h"
#include "inode.h"
#include "util.h"

const int INODE_PAGE = 2;
const int INODE_COUNT = 64;


void print_inode(inode* node);

inode* 
get_inode(int inum) 
{
    assert(inum < INODE_COUNT);
    inode* nodes = pages_get_page(INODE_PAGE);
    return &(nodes[inum]);
}

int alloc_inode() {
    for (int ii = 0; ii < INODE_COUNT; ++ii) {
	    inode* node = get_inode(ii);
	    if (node->mode == 0) {
		    memset(node, 0, sizeof(inode));
		    node->mode = 010644;
		    node->size = 0;
		    node->refs = 0;
		    node->ptrs[0] = alloc_page();
		    node->iptr = 0;
		    printf("alloc_inode() = %d\n", ii);
		    return ii;
	    }
    }
    return -1;

}


void free_inode();

int grow_inode(inode* node, int size);

int shrink_inode(inode* node, int size);

int inode_get_pnum(inode* node, int fpn);

