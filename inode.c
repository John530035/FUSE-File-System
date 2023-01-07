/**
 * @file inode.c
 * @author John Fahy and Kelvin Xu
 *
 * Implementation of an inode abstraction and its related methods.
 */

#include "inode.h"

const int INODE_COUNT = 250; // 256 total blocks - 1 block for bitmaps - 5 for inode table = 250 data blocks

// COME BACK
void print_inode(inode_t *node) {}

// Initializes the inode table, reserves blocks 1, 2, 3, 4, 5
// for the inode table.
void inode_table_init() {
  void *blocks_bitmap = get_blocks_bitmap(); // retrieve blocks bitmap

  bitmap_put(blocks_bitmap, 1, 1);           // indicate that blocks 1-5 are being used
  bitmap_put(blocks_bitmap, 2, 1);
  bitmap_put(blocks_bitmap, 3, 1);
  bitmap_put(blocks_bitmap, 4, 1);
  bitmap_put(blocks_bitmap, 5, 1);
}

// Returns the inode with the given inum.
inode_t *get_inode(int inum) {
  void* start = blocks_get_block(1);   // pointer to beginning of inode table
  int offset = sizeof(inode_t) * inum; // offset to the inode of interest

  return (inode_t *) ((char *)start + offset); // return the inode of interest
}

// Allocates a new inode, reserves the new inode
// in the inode bitmap.
int alloc_inode() {
  void *inode_bitmap = get_inode_bitmap();

  //iterate through the inode bitmap and return the index
  //of the first inode that is free
   for (int ii = 0; ii < INODE_COUNT; ii++) {
     if (!bitmap_get(inode_bitmap, ii)) {
       bitmap_put(inode_bitmap, ii, 1);
       printf("+ alloc_inode() -> %d\n", ii);

       return ii;
     }
   }

   return -1; // return -1 if did not find any free inode
}

// Frees the inode with the given inum in the inode bitmap.
void free_inode(int inum) {
  void *inode_bitmap = get_inode_bitmap(); // get the inode bitmap
  bitmap_put(inode_bitmap, inum, 0);       // mark that the given inode is free

  printf("+ free_inode(%d)\n", inum);
}

