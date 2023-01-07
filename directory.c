/**
 * @file directory.c
 * @author John Fahy and Kelvin Xu
 *
 * Implementation of a directory abstraction and related operations.
 */
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "directory.h"
#include "blocks.h"
#include "inode.h"
#include "storage.h"
#include "bitmap.h"
#include "slist.h"

const int DIR_SIZE = 4096;                       // the size of a directory in bytes
const int DIRENT_SIZE = sizeof(dirent_t);        // the size of a directory entry
const int DIRENT_COUNT = DIR_SIZE / DIRENT_SIZE; // the number of directory entries that fit in a directory

// Creates a new directory at the given path with the given mode.
int directory_init(const char* path, mode_t mode) {

    // if the given directory already exists, then this method does nothing
    if (tree_lookup(path) != -1) {
      return -1;
    }

    // allocate an inode for the directory and initialize as directory inode
    int dir_inum = alloc_inode();
    assert(dir_inum != -1);
    inode_t* inode = get_inode(dir_inum);
    inode->refs = 1;
    inode->mode = mode;
    inode->size = 0;

    int dir_bnum = alloc_block();
    assert(dir_bnum != -1);
    inode->block = dir_bnum;

    // get the parent directory and name of the new directory
    char* upper_dir_path = get_dir_path(path);
    int upper_dir_inum = tree_lookup(upper_dir_path);
    if (upper_dir_inum == -1) return -1;
    char* dir_name = get_file_name(path);

    // create a new directory entry in the directory the new directory is in
    int res = directory_put(upper_dir_inum, dir_name, dir_inum);
    assert(res == 0);

    // create a new directory entry in the new directory - refers to itself
    int res2 = directory_put(dir_inum, ".", dir_inum);
    assert(res2 == 0);

    // create a new directory entry to parent directory
    int res3 = directory_put(dir_inum, "..", upper_dir_inum);
    assert(res3 == 0);

    return 0; // return 0 on success
}

// Initializes the root directory.
int root_init() {
    // root has inum 2 and mark bitmap as allocated
    int dir_inum = 2;
    void* inode_bitmap = get_inode_bitmap();
    bitmap_put(inode_bitmap, 2, 1);

    // initialize inode values
    inode_t* inode = get_inode(dir_inum);
    inode->refs = 1;
    inode->mode = 040000;
    inode->size = 0;

    int dir_bnum = alloc_block();
    assert(dir_bnum != -1);
    inode->block = dir_bnum;

    // create a new directory entry in the root - refers to itself
    int res2 = directory_put(dir_inum, ".", dir_inum);
    assert(res2 == 0);

    return 0;
}

// Checks if there is a directory entry with the given name in the directory
// with the given inum.
int directory_lookup(int inum, const char *name) {
    // retrieve the first directory entry
    inode_t* dir_inode = get_inode(inum);
    int dir_block = dir_inode->block;
    dirent_t* dir_entry = (dirent_t *) blocks_get_block(dir_block);

    // check if the given file exists
    for (int i = 0; i < DIRENT_COUNT; i ++) {

        if (dir_entry->name == NULL) {
	  return -1;
	}

        if (dir_entry->free == 1 && strcmp(dir_entry->name, name) == 0) {
	  return dir_entry->inum;                                         // found the entry with the matching name, return the inum
	}

        dir_entry = dir_entry + 1;
    }

    return -1; // return -1 if did not find a directory entry with the given name in the specified directory
}

// Returns the inum of the specified directory/file in the given path.
int tree_lookup(const char *path) {

    if (strcmp(path, "/") == 0) {
      return 2; // if looking for the root directory then return its inum
    }

    // determine whether every parent has the child set in path
    slist_t* path_split = s_explode(path, '/');
    path_split = path_split->next;

    int current_inum = 2; // always start search from root directory
    int res = 0;

    while(path_split != NULL) {
        res = directory_lookup(current_inum, path_split->data); // search for path split in the current directory

        if (res == -1) {
	  return -1;                                             // path_split does not exist, path is invalid, return -1
	}

        current_inum = res;
        path_split = path_split->next;
    }

    return current_inum; // found the inum of the last directory/file in the path
}

// Creates a new directory entry in the directory specified by the given
// dir_inum with the given name and entry_inum.
int directory_put(int dir_inum, const char *name, int entry_inum) {
    // get the directory entries for the directory
    inode_t* dir_inode = get_inode(dir_inum);
    int dir_block = dir_inode->block;
    dirent_t* dir_entry = (dirent_t *) blocks_get_block(dir_block);

    // iterate through all of the directroy entries in the directory
    // and check for a free spot and put the entry there
    for (int i = 0; i < DIRENT_COUNT; i ++) {
        if (dir_entry->name == NULL || dir_entry->free == 0) {
            strcpy(dir_entry->name, name);
            dir_entry->inum = entry_inum;
            dir_entry->free = 1;
            return 0;                                          // 0 signals success
        }

        dir_entry = dir_entry + 1; // if the current directroy is not free then iterate to next
    }

    return -1; // have not made the entry, -1 signals failure
}

// Delete the directory entry with the given entry_name from the directory
// specified by the given dir_inum.
int directory_delete(int dir_inum, const char *entry_name) {

    // the entry we are trying to delete does not exist in the current directory
    if (directory_lookup(dir_inum, entry_name) == -1) {
      return -1;
    }

    // get the directory entries of the directory we are deleting from
    inode_t* dir_inode = get_inode(dir_inum);
    int dir_block = dir_inode->block;
    dirent_t* dir_entry = (dirent_t *) blocks_get_block(dir_block);

    // iterate through directory entries and delete the given entry_name when found
    for (int i = 0; i < DIRENT_COUNT; i ++) {
        if (strcmp(dir_entry->name, entry_name) == 0) {
            dir_entry->free = 0;
            int file_num = directory_lookup(dir_inum, entry_name);
            dir_inode->size -= get_inode(file_num)->size;
            return 0;
        }

        dir_entry = dir_entry + 1;
    }

    return -1; // return -1 if entry_name not found
}

// Lists the contents of the directory with the given path.
slist_t *directory_list(const char *path) {
    int dir_inum = tree_lookup(path);
    inode_t* dir_inode = get_inode(dir_inum);

    int dir_block = dir_inode->block;
    dirent_t* dir_entry = (dirent_t*) blocks_get_block(dir_block);
    assert(dir_entry->name != NULL);

    slist_t* list = s_cons(dir_entry->name, NULL); // add the name of the first entry to the list
    dir_entry = dir_entry + 1;

    // iterate through directory entries of the given path and add them to slist
    for(int i = 0; i < DIRENT_COUNT; i ++) {
        if (dir_entry->free == 1) {
            list = s_cons(dir_entry->name, list); // append the name of the entry to the front of the list
        }

        dir_entry = dir_entry + 1;
    }

    return list;
}
