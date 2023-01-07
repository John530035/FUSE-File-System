/**
 * @file slist.h
 * @author CS3650 staff
 *
 * A simple linked list of strings.
 *
 * Useful for directory listings and for manipulating paths.
 */
#ifndef SLIST_H
#define SLIST_H

// struct representing one strin in a linked
// list of strings.
typedef struct slist {
  char *data;         // the string
  int refs;           // the number of references to the string
  struct slist *next; // the next string in the linked list
} slist_t;

/**
 * Cons a string to a string list.
 *
 * @param text The text of the new string.
 * @param rest The rest of the list of strings.
 *
 * @return New list of strings with the given string
 * 	   in front.
 */
slist_t *s_cons(const char *text, slist_t *rest);

/**
 * Free the given string list.
 *
 * @param xs The list of strings to free.
 */
void s_free(slist_t *xs);

/**
 * Split the given on the given delimiter into a list of strings.
 *
 * @param text The string to explode.
 * @param delim The delimiter to explode the string about.
 *
 * @return The list of strings exploded about the given delimeter.
 */
slist_t *s_explode(const char *text, char delim);

#endif
