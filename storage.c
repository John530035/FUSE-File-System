/**
 * @file storage.c
 * @author John Fahy and Kelvin Xu
 *
 * Implementation of a data block abstraction and related methods.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "storage.h"
#include "inode.h"
#include "directory.h"
#include "slist.h"
#include "blocks.h"

// Initializes the file system at the given path.
int storage_init(const char *path) {

    blocks_init(path); // initializes the file system

    if (bitmap_get(get_blocks_bitmap(), 1) == 0) {
        inode_table_init();                        // initialize the inode table
    }

    if (bitmap_get(get_inode_bitmap(), 2) == 0) {
        root_init();                               // initialize the root directory
    }

  return 0; // return 0 on success
}

// Fills in the stat struct for the given file.
int storage_stat(const char *path, struct stat *st) {
  int file_inum = tree_lookup(path); // get the inum of the file

  if (file_inum == -1) {
    return -1; // the file does not exist
  }

  inode_t* file_inode = get_inode(file_inum); // get the inode for the file

  st->st_ino = file_inum;          // set the fields of the stat structure
  st->st_nlink = file_inode->refs;
  st->st_mode = file_inode->mode;
  st->st_size = file_inode->size;

  return 0; // return 0 on success
}

// Read data from the given file.
int storage_read(const char *path, char *buf, size_t size, off_t offset) {
  int file_inum = tree_lookup(path); // retrieve the inum of the file
  
  if (file_inum == -1) {
    return -1; // if tree_lookup returns -1 the file does not exist
  }

  inode_t* file_inode = get_inode(file_inum);
  void* block = blocks_get_block(file_inode->block);
  void* offset_block = (void*) (((char*) block) + offset); // adjust the pointer by offset

  memcpy(buf, offset_block, size); // copy the memory from the file into the buffer

  return size;
}

// Write data to the given file.
int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
  assert(offset < 4096);

  int file_inum = tree_lookup(path);
  if (file_inum == -1) {
    return -1;    // if tree_lookup returns -1 the file does not exist
  }

  inode_t* file_inode = get_inode(file_inum);
  void* block = blocks_get_block(file_inode->block);
  void* offset_block = (void*) (((char*) block) + offset); // adjust the pointer by offset

  memcpy(offset_block, buf, size); // copy the memory from buffer into the file

  file_inode->size = file_inode->size + size; // update size of file
  assert(file_inode->size <= 4096);          // cannot write more than spcae of file

  int parent_inode = tree_lookup(get_dir_path(path));
  get_inode(parent_inode)->size += size;

  return size;
}

// Creates a new file at the given path.
int storage_mknod(const char *path, int mode) {

    //make sure the file does not already exist - if it does then returns -1 to indicate error
  if (tree_lookup(path) != -1) {
    return - 1;
  }

  int file_inum = alloc_inode(); // allocate an inode for the file
  assert(file_inum != -1);

  // initialize inode fields
  inode_t* inode = get_inode(file_inum);
  inode->refs = 1;
  inode->mode = mode;
  inode->size = 0;

  int file_bnum = alloc_block();
  assert(file_bnum != -1);
  inode->block = file_bnum;


  char* dir_path = get_dir_path(path);
  char* file_name = get_file_name(path);

  int dir_inum = tree_lookup(dir_path);
  assert(dir_inum != -1);

  // make a new dir entry for the new file in the directory it exists in
  directory_put(dir_inum, file_name, file_inum);

  return 0;
}

// Unlinks the given path name from the file.
int storage_unlink(const char *path) {
  int file_inum = tree_lookup(path); //retrieve the inum of the file

  // if tree_lookup returns -1 the file does not exist
  if (file_inum == -1) {
    return -1;
  }

  inode_t* file_inode = get_inode(file_inum);
  file_inode->refs = file_inode->refs - 1;    // decrement the number of references to the file

  // gets the path to the parent and the file name of the path entered
  char* dir_path = get_dir_path(path);
  char* file_name = get_file_name(path);
  int dir_inum = tree_lookup(dir_path);
  assert(dir_inum != -1);

  // go to the directory the file exists in and delete the entry for the file
  directory_delete(dir_inum, file_name);

  // if the file has no references, delete the file and set bitmaps to 0 - freed
  if (file_inode->refs == 0) {
    void* blocks_bitmap = get_blocks_bitmap();
    bitmap_put(blocks_bitmap, file_inode->block, 0);

    void* inode_bitmap = get_inode_bitmap();
    bitmap_put(inode_bitmap, file_inum, 0);
  }

  return 0; // return 0 on success
}

// Creates an alias for the from file.
int storage_link(const char *from, const char *to) {
  if (tree_lookup(to) != -1) return -1; // check to make sure the new file name does not already exist -

  int file_inum = tree_lookup(from);

  if (file_inum == -1) return -1; // check to make sure the file we are making an alias for exists

  // increment references for from inode
  inode_t* file_inode = get_inode(file_inum);
  file_inode->refs = file_inode->refs + 1;

  // reconstructs the parent path of to and get the file name
  char* parent_path = get_dir_path(to);
  char* file_name = get_file_name(to);
  int dir_inum = tree_lookup(parent_path);
  if (dir_inum == -1) return -1; // the given file path does not exist then method does nothing

  // make a new entry for the alias in the directory specified
  directory_put(dir_inum, file_name, file_inum);

  return 0; // return 0 on success
}

// Moves the file from the from path to the to path.
int storage_rename(const char *from, const char *to) {
  int file_inum = tree_lookup(from);

  // if the file we are renaming does not exist then this method does nothing
  if (file_inum == -1) {
   return -1;
  }

  // retrieve the inum for the new name of the file - doesnt have to exist
  int to_file_inum = tree_lookup(to);

  // if the new file path already exists
  if (to_file_inum != -1) {
    if (file_inum == to_file_inum) {
      return 0;
    }
    
    int ret = storage_unlink(to); // unlink the new name from whatever file it referred to
    assert(ret == 0);
  }

  char* from_dir_path = get_dir_path(from);
  int from_dir_inum = tree_lookup(from_dir_path);
  assert(from_dir_inum != -1);

  char* from_file_name = get_file_name(from);
  directory_delete(from_dir_inum, from_file_name); // delete the entry for old file path

  // make a new directory entry for the new name of the file. It should point to the file the old name pointed to
  char* to_dir_path = get_dir_path(to);
  int to_dir_inum = tree_lookup(to_dir_path);

  // if given an invalid file path then this method does nothing
  if (to_dir_inum == -1) {
    return -1;
  }

  char* to_file_name = get_file_name(to);
  directory_put(to_dir_inum, to_file_name, file_inum);
  return 0;
}

// Retrieve the file path of the directroy the given file exists in (whole path but file name).
char* get_dir_path(const char* file_path) {

  slist_t* path_split = s_explode(file_path, '/'); // split the path around /
  path_split = path_split->next;                   // go to the first string
  assert(path_split != NULL);

  char* build_path = (char*) malloc(sizeof(char) * 50);
  strcpy(build_path, "/"); // rebuild the path to include all but the last string (file name)

  // rebuilds file path up to parent directory of path
  while (path_split->next != NULL) {
    char* added_slash = strcat(path_split->data, "/");
    build_path = strcat(build_path, added_slash);
    path_split = path_split->next;
    free(added_slash);
  }

  return build_path;
}

//Given compelete file path, return the name of the file (last string).
char* get_file_name(const char* file_path) {

  slist_t* path_split = s_explode(file_path, '/'); // split the path around / char
  path_split = path_split->next;                   // go to first string

  while (path_split->next != NULL) { // get to the last string in path_split
    path_split = path_split->next;
  }

  return path_split->data;
}
