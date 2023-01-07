/**
 * @file nufs.c
 * @author John Fahy and Kelvin Xu
 *
 * Implementation of fuse functions.
 *
 * Based on cs3650 starter code.
 */
#include <assert.h>
#include <bsd/string.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "directory.h"
#include "storage.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

// Implementation for: man 2 access
// Checks if the file with the given path exists.
int nufs_access(const char *path, int mask) {
  int rv = 0;
  int inum = tree_lookup(path); // get the inum of the given directory/file

  if (inum != -1) {
    rv = 0;         // the given directory/file exists
  } 
  else {        
    rv = -ENOENT;   // the given directory/file does not exist
  }

  printf("access(%s, %04o) -> %d\n", path, mask, inum);
  return rv;
}

// Implementation for: man 2 stat
// Gets the attributes of the file at the given path (type, permissions, size, etc).
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;
  int inum = tree_lookup(path); // get the inum of the directory/file

  if (inum == -1)  {
    rv = -ENOENT; // if the directory/file does not exist then error is returned
  }
  else {
    rv = storage_stat(path, st); // fill up the stat struct for the given path
    assert(rv == 0);
  } 

  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, inum, st->st_mode,
         st->st_size);
  return rv;
}

// Implementation for: man 2 readdir
// Lists the contents of the directory with the given path.
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  struct stat st;
  int rv = nufs_getattr(path, &st); // fill up the stat struct for the directory
  assert(rv == 0);

  slist_t* dir_contents = directory_list(path); // get a list of the contents of the directory
  assert(dir_contents != NULL);

  while(dir_contents != NULL) {                         // iterate through directory entries
    char* path_copy = (char*) malloc(strlen(path + 1)); // copy of the directory path to build file path of each entry
    strcpy(path_copy, path);

    if (strcmp(path_copy, "/") != 0) {
      strcat(path_copy, "/");
    }

    char* file_path = strcat(path_copy, dir_contents->data); // absolute file path of each directory entry
    nufs_getattr(file_path, &st);                            // fill up the stat struct for each directory entry

    filler(buf, dir_contents->data, &st, 0);                 // fill the buffer with the current directory entry

    dir_contents = dir_contents->next;
    
    free(path_copy);                                         // free the copy of the file path
  }

  printf("readdir(%s) -> %d\n", path, rv);
  return rv;
}

// called for: man 2 open, man 2 link
// mknod makes a filesystem object like a file or directory
// Creates a directory or normal file with the given path and mode
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = 0;

  if (S_ISREG(mode)) {	  
    rv = storage_mknod(path, mode);  // create the file
    assert(rv == 0);
  }
  else if (S_ISDIR(mode)) {
    printf("GOOD\n");
    rv = directory_init(path, mode); // create the directory
    assert(rv == 0);
  }
  else {
    rv = -ENOENT;                    // not making file or directory
  }

  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// another system call; see section 2 of the manual
// Makes a directory with the given path and mode.
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = -ENOENT;

  rv = nufs_mknod(path, S_IFDIR | mode, 0); // create the directory
  assert(rv == 0);

  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

// Unlinks the given file name from the file.
int nufs_unlink(const char *path) {
  int rv = -ENOENT;

  rv = storage_unlink(path); // unlink the path from the file
  assert(rv == 0);

  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

// Adds an alias for the from file.
int nufs_link(const char *from, const char *to) {
  int rv = -ENOENT;

  rv = storage_link(from, to); // link the files
  assert(rv == 0);

  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// Deletes the directory at the given path.
int nufs_rmdir(const char *path) {
  int rv = 0;

  rv = storage_unlink(path);
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// Moves the file to a different path in the file system.
int nufs_rename(const char *from, const char *to) {
  int rv = -ENOENT;

  rv = storage_rename(from, to); // rename the file
  assert(rv == 0);

  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// Changes the permissions of the file at the given path.
int nufs_chmod(const char *path, mode_t mode) {
  int rv = 0;

  // get the inode for the file and change its permissions
  int inum = tree_lookup(path);
  inode_t* inode = get_inode(inum);
  inode->mode = mode;

  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

//Truncates the given file to the given size.
int nufs_truncate(const char *path, off_t size) {
  int rv = -1;

  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = -ENOENT;

  rv = nufs_access(path, 0); // check if the file exists 
  assert(rv == 0);

  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Reads size bytes from the file at the given path, starting at the
// given offset.
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = -ENOENT;

  rv = storage_read(path, buf, size, offset); // read the data
  assert(rv != -1);

  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Writes size bytes from the file at the given path, starting at the
// given offset.
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {	
  int rv = -ENOENT;

  rv = storage_write(path, buf, size, offset); // write the data
  assert(rv != -1);

  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = 0;

  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;
}

// Extended operations.
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = 0;

  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

// Initialze fuse operations to nufs implementations.
void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

// Atruct containing fuse operations to implement.
struct fuse_operations nufs_ops;

// Initiales fuse operations and intializes 
// the file system.
int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  printf("TODO: mount %s as data file\n", argv[--argc]);

  int rv = storage_init(argv[argc]);                     // initialize the file system
  assert(rv == 0);
  nufs_init_ops(&nufs_ops);                              // set up fuse operations
 
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
