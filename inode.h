/**
 * @file inode.h
 * @author John Fahy and Kelvin Xu
 *
 * An abstraction of an inode for a directory or a normal file.
 * Contains methods to manipulate an inode.
 *
 * Based on cs3650 starter code.
 */
#ifndef INODE_H
#define INODE_H

#include <sys/stat.h>

#include "blocks.h"

// struct representing an inode and its necessary fields
typedef struct inode {
  int refs;     // the numberof references to a file
  mode_t mode;  // permission & type of a file
  int size;     // size in bytes of a file
  int block;    // the index of the block containing a file's contents
} inode_t;

extern const int INODE_COUNT; // the number of inodes that fit in one data block

/**
 * Initialize the inode table.
 * Reserves data blocks 1 - 5 to hold only inodes.
 */
void inode_table_init();

/**
 * COME BACK
 */
void print_inode();

/**
 * Retrieves the inode with the given inum.
 *
 * @param inum The index of the inode we return.
 *
 * @return The inode with the given inum.
 */
inode_t* get_inode(int inum);

/**
 * Allocates a new inode. Searches in the inode
 * bitmap for the first free inode and reserves that inode.
 *
 * @return The index of the newly reserved inode.
 */
int alloc_inode();

/**
 * Frees the inode with the given inum. Indicates in the
 * inode bitmap that the inode with the given inum is now free.
 *
 * @param inum The index of the inode to free.
 */
void free_inode(int inum);

#endif
