/**
 * @file storage.h
 * @authors John Fahy and Kelvin Xu
 *
 * An abstraction of a data block. Contains data block manipulations.
 *
 * Based on cs3650 starter code.
 */
#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "slist.h"

/**
 * Initialzes a new file system at the given image file path.
 * Initializes the inode table.
 * Initialzes the root directory.
 *
 * @param The absolute path of the image file where we mount the
 * 	  file system.
 *
 * @return 0 on successful file system initialization.
 */
int storage_init(const char *path);

/**
 * Enters the attributes of the file at the given path into the
 * given stat struct.
 *
 * @param path The absolute path of the file we want the attributes of.
 * @param st The stat struct we enter the file attributes into.
 *
 * @return 0 on success, -1 if the file does not exist.
 */
int storage_stat(const char *path, struct stat *st);

/**
 * Reads data from the file at the given path.
 *
 * @param path The absolute path of the file we are reading data from.
 * @param buf Read the data from the file to the buffer.
 * @param size The number of bytes we read from the file at the given path.
 * @param offset The offset we start reading from the file at.
 *
 * @return The number of bytes read from the file, -1 if the file does not exist.
 */
int storage_read(const char *path, char *buf, size_t size, off_t offset);

/**
 * Writess data to the file at the given path.
 *
 * @param path The absolute path of the file we are writing data to.
 * @param buf Write the data to the file from the buffer.
 * @param size The number of bytes we write to the file at the given path.
 * @param offset The offset we start writing to the file at.
 *
 * @return The number of bytes written to the file, -1 if the file does not exist.
 */
int storage_write(const char *path, const char *buf, size_t size, off_t offset);

/**
 * Creates a new file at the given path with the given mode.
 *
 * @param path The absolute path where we create the new file.
 * @param mode The mode of the new file.
 *
 * @return 0 on success, -1 if the file already exists.
 */
int storage_mknod(const char *path, int mode);

/**
 * Unlinks the file name at the given path from the file.
 * If the file has 0 references after unlinking the given file name,
 * the file is deleted.
 *
 * @param path The name of the file we are unlinking from its file.
 * @return 0 on success, -1 if the file does not exist.
 */
int storage_unlink(const char *path);

/**
 * Create an alias for the file at from.
 *
 * @param from The file we are making an alias for.
 * @param to The new alias for the file at from.
 *
 * @return 0 on success, -1 if the file at from does not exist.
 */
int storage_link(const char *from, const char *to);

/**
 * Move the file at from to the path at to.
 *
 * @param from The original absolute path of the file.
 * @param to The new absolute path of the file.
 *
 * @return 0 on success, -1 if the file at from does not exist.
 */
int storage_rename(const char *from, const char *to);

/**
 * Get the path of the directory the file at the given path exists in.
 *
 * @param file_path The absolute path of the file.
 * 
 * @return The absolute path of the directory the file is in.
 */
char* get_dir_path(const char* file_path);

/**
 * Get the name of the file from the given file_path.
 *
 * @param file_path The absolute path of the file.
 *
 * @return The name of the file.
 */
char* get_file_name(const char* file_path);

#endif
