// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>
#include <sys/time.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "directory.h"
#include "pages.h"

#define DIR_PAGE 1
#define INODE_PAGE 2
#define DIRENT_MAX 64 



// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    int rv = 0;
    struct stat st;
    printf("in access\n");

    //rv = nufs_getattr(path, &st);
    //return rv;

    dirent_t *dirs = (dirent_t *)pages_get_page(DIR_PAGE);
    int i = directory_lookup(path);

    //if (i == DIRENT_MAX)
//	    return -ENOENT;
    printf("access(%s, %04o) -> %d\n", path, mask, rv);
    return rv;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    int rv = 0;
    printf("in getattr\n");
    dirent_t *dirs = (dirent_t *)pages_get_page(DIR_PAGE);
    inode *ix = (inode *)pages_get_page(INODE_PAGE);
    if (strcmp(path, "/") == 0 && strlen(path) == 1) {
        st->st_mode = 040755; // directory
        st->st_size = ix[0].size;
        st->st_uid = getuid();
    } else {

	int i = directory_lookup(path);

    	if (i == DIRENT_MAX) {
		rv = -ENOENT;
    	} else {
		inode node = ix[dirs[i].inum];
		//printf("getattr inum: %d\n", dirs[i].inum);
		
		st->st_mode = node.mode;
		st->st_size = node.size;
		st->st_uid = getuid();
		st->st_nlink = node.refs;
		st->st_atime = node.accesstime;
		st->st_mtime = node.modifytime;
    	}
    }

    printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
    return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;
    int rv;
    int i;
    dirent_t *dirs;

    dirs = (dirent_t *)pages_get_page(DIR_PAGE);
    //printf("size of dirent_t = %d\n", sizeof(dirent_t));
    
    rv = nufs_getattr("/", &st);
    assert(rv == 0);

    filler(buf, ".", &st, 0);
    
    
    printf("readdir here %08x\n", (unsigned long long)dirs);
    for (int i = 1; i < DIRENT_MAX; i++) {
	    //printf("%d\n", i);
	    char *n = (char *)&dirs[i].name;
	    //if (n[0] != '\0') {
	    char* token;
	    if (strlen(path) == 1) {
	    	token = strstr(n + strlen(path), "/");
	    } else {
		token = strstr(n + strlen(path) + 1, "/");
	    }
	    printf("tokens: %s, %s\n", n + strlen(path), token);
	    if (strncmp(path, n, strlen(path)) == 0 && strcmp(path, n) != 0
			    && token == NULL) {
		    printf("here %d\n", i);
		    printf("file %s\n", n);
		    rv = nufs_getattr(n, &st);
		    assert(rv == 0);

		    char* ncopy = strdup(n); 
		    token = strtok(ncopy, "/");
		    char* cutoffpath = strdup(token);

		    while(token != NULL) {
			    cutoffpath = strdup(token);
			    printf("cut off path = %s\n", cutoffpath);
			    token = strtok(NULL, "/");
			    
		    }

		    
		    
		    filler(buf, cutoffpath, &st, 0);
	    }
    }


    printf("readdir(%s) -> %d\n", path, rv);
    return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int rv = 0;
    // If dirent array is full, return -ENOSPC
    // Hack: Set inode number to the array index in dirs
    // Hack: pages_get_pages(inum) is your data block
    // Hack: inum starts at 1 (index + 1) (offbyon error alert)
    inode *ix = (inode *)pages_get_page(INODE_PAGE);
    dirent_t *dirs = (dirent_t *)pages_get_page(DIR_PAGE);
    int i;
    for (i = 1; i < DIRENT_MAX; i++) {
	    if (dirs[i].name[0] == '\0')
		    break;
    }
    printf("out of loop\n");

    if (i == DIRENT_MAX)
	    return -ENOSPC;

    
    //dirs[i].name = path;
    printf("got empty dirent\n");
    strcpy(dirs[i].name, path);
    dirs[i].inum = i;
    ix[i].refs = 1;
    ix[i].mode = mode;
    ix[i].size = 0;
    ix[i].ptrs[0] = i + 5;

    //ix[0].size++;


    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    printf("in mkdir\n");
    int rv = nufs_mknod(path, mode | 040000, 0);
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_unlink(const char *path)
{
    int rv = 0;

    inode *ix = (inode *)pages_get_page(INODE_PAGE);
    dirent_t *dirs = (dirent_t *)pages_get_page(DIR_PAGE);
    int i;
    for (i = 1; i < DIRENT_MAX; i++) {
	    if (strcmp(path, dirs[i].name) == 0)
		    break;
    }
    //printf("out of loop\n");
    //Free inode
    //Free direct blocks
    //Free indirect block (if any)
    //Free the dirent
    //If hardlinks, decrease the refcnt, only free if refcnt == 0
    //Return -EINVAL if it's a directory

    if (i == DIRENT_MAX)
	    return -ENOENT;
    
    dirs[i].name[0] = '\0';
    //ix[0].size--;
    
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    
    int rv = 0;

    printf("link(%s => %s) -> %d\n", from, to, rv);
    // Two cases:
    // hard link: ln src dst
    // 		This will increase refcnt on the inode
    // 		Separate dirent pointing to same inode
    // soft link: ln -s src dst
    // 		Something that contains the pathname to
    // 		the destination.  Sometimes stored in the inode as data
    // 		Probably also a special mode bit 020000?
    //
    dirent_t *dirs = (dirent_t *)pages_get_page(DIR_PAGE);
    inode *ix = (inode *)pages_get_page(INODE_PAGE);

    int f_index = directory_lookup(from);
    int t_index = directory_lookup(to);
    if (f_index == DIRENT_MAX)
	    return -ENOENT;
    dirent_t f = dirs[f_index];

    printf("here link 1\n");
    
    if (t_index == DIRENT_MAX)
	    nufs_mknod(to, ix[f.inum].mode, 0);

    t_index = directory_lookup(to);
    printf("here link 2\n");

   

    printf("before inum: %d\n", dirs[t_index].inum);
    dirs[t_index].inum = f.inum;
    printf("after inum: %d\n", dirs[t_index].inum);
    
    return rv;
}

int
nufs_rmdir(const char *path)
{
    int rv = nufs_unlink(path);
    // If the directory has not dirents remaining, remove the directiry
    // Otherwise, reutrn -ENOTEMPTY
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    int rv = 0;
    // Changes the filename in the dirent
    //

    int f_index = directory_lookup(from);
    if (f_index == DIRENT_MAX)
	    return -ENOENT;
    dirent_t *dirs = (dirent_t *)pages_get_page(DIR_PAGE);

    strcpy(dirs[f_index].name, to); 

    printf("rename(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    int rv = -1;
    //Updates the mode
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int
nufs_truncate(const char *path, off_t size)
{
    int rv = -1;
    // Frees all blocks after size (bitmap operation)
    // Don't forget to set size in the inode
    // I'd be surprised if you have to do this
    printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
    return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    int rv = 0;
    // Should make sure the file exists?
    // I think the upper layers call access(), so maybe nothing here
    printf("open(%s) -> %d\n", path, rv);
    return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = size;
    int i = get_directory_inum(path);
    // Make sure you follow direct and indirect block pointers
    // Edge case: trying to read past end of file (size)
     

    inode* ix = (inode *)pages_get_page(INODE_PAGE);
    void* pageloc = (void*)(offset + pages_get_page(ix[i].ptrs[0]));
    strncpy(buf, (char*) pageloc, size);
    

    ix[i].accesstime = time(&(ix[i].accesstime)); 
    
    printf("read here %08x\n", (unsigned long long)pageloc); 

    //strcpy(buf, "hello\n");
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = size;
    int i = get_directory_inum(path);
    // Allocate space in the bitmap (if needed)
    // Handle overwrites
    // Handle indirect blocks
    // Edge case: overwrite and adding data
    // Edge case: offset + size > inode.size
    // I don't think you have to worry about sparse files
    // Zero fill newly allocated blocks :)
    

    inode* ix = (inode *)pages_get_page(INODE_PAGE);
    void* pageloc = (void*)(offset + pages_get_page(ix[i].ptrs[0]));
    strncpy((char*) pageloc, buf, size);

    ix[i].modifytime = time(&(ix[i].modifytime));

    printf("write here %08x\n", (unsigned long long)pageloc); 
    ix[i].size += size;
    //strcpy(buf, "hello\n");
    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = 0;
    // Just updated the gunk in the inode
    // Maybe after getattr?
    // mtime - time the file was last modified (write, chmod, etc.)
    // ctime - time the file was created
    // atime - time the file was last accessed (don't worry about this)
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
           unsigned int flags, void* data)
{
    int rv = -1;
    // Not supported = -ENOTSUP
    printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
    return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->link     = nufs_link;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->ioctl    = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    printf("TODO: mount %s as data file\n", argv[--argc]);
    //storage_init(argv[--argc]);
    pages_init(argv[argc]);

    inode *ix = (inode *)pages_get_page(INODE_PAGE);
    ix[0].size = 0;

    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

