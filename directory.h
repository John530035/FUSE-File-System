/**
 * @file directory.h
 * @author John Fahy and Kelvin Xu
 *
 * A directory abastraction. Contains methods for directory
 * manipulation.
 *
 * Based on cs3650 starter code.
 */
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "blocks.h"
#include "inode.h"
#include "storage.h"
#include "slist.h"

#define DIR_NAME_LENGTH 10 // the length of the name of a directory entry is at most 10 characters

/**
 * Represents a directory entry.
 * A directory entry can be a normal file or can be another directory
 */
typedef struct dirents {
  char name[DIR_NAME_LENGTH]; // the name of the directory entry
  int inum;                   // the inum of the directory entry
  int free;                   // 1 if the directory entry is in use, 0 if it has been deleted
  char reserved[6];           // rounds out the size of the dirents struct
} dirent_t;

extern const int DIR_SIZE;     // the size of a directory in bytes
extern const int DIRENT_SIZE;  // the size of a directory entry
extern const int DIRENT_COUNT; // the number of directory entries that can fit in a directory

/**
 * Create a new directory with the given path and mode.
 *
 * @param path The absolute file path of the new directory.
 * @param mode The mode of the directory - including file type and permissions.
 *
 * @return 0 if the new directory was created successfully and -1 if otherwise.
 */
int directory_init(const char* path, mode_t mode);

/**
 * Create the root directory.
 *
 * @return 0 if the root directory was created successfully and -1 otherwise.
 */
int root_init();

/**
 * Searches in the directory with the given inum for the directory entry with the
 * given name.
 *
 * @param inum The inum of the directory we are seraching in.
 * @param name The name of the directory entry we are searching for.
 *
 * @return The inum of the entry with the given name if found, -1 if the entry is
 * 	   not found.
 */
int directory_lookup(int inum, const char *name);

/**
 * Returns the inum of the directory or path specified by the given path.
 * Designed to work with nested directories.
 *
 * @param The absolute path of the directory or file we are locating.
 *
 * @return The inum of the directory or file at the given path, -1 if the
 * 	   given directory or file does not exist.
 */
int tree_lookup(const char *path);

/**
 * Makes a new directory entry with the given name and entry_inum in the directory
 * with the given dir_inum.
 *
 * @param dir_inum The inum of the directory we are making a new entry into.
 * @param name The name of the new directory entry.
 * @param entry_inum The inum of the new directory entry.
 *
 * @return 0 if the new entry was made successfully, -1 if there is no room in the
 * 	   given directory for a new entry.
 */
int directory_put(int dir_inum, const char *name, int entry_inum);

/**
 * Deletes the entry with the given entry_name from the directory with the given
 * dir_inum.
 *
 * @param dir_inum The inum of the directory we are deleting an entry from.
 * @param entry_name The name of the entry we are deleting from the directory.
 *
 * @return 0 if the given entry was successfully deleted from the directory, -1
 * 	   if the given entry could not be found in the given directory.
 */
int directory_delete(int dir_inum, const char *entry_name);

/**
 * List the entry names of the directory entries of the directory at the 
 * given absolute path.
 *
 * @param The absolute path o the directory we are listing the contents of.
 *
 * @return A list of strings containing the names of the entries in the given
 * 	   directory.
 */
slist_t *directory_list(const char *path);

#endif
