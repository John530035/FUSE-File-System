/**
 * @file bitmap.h
 * @author CS3650 staff
 *
 * A bitmap interface.
 */
#ifndef BITMAP_H
#define BITMAP_H

/**
 * Get the given bit from the bitmap.
 *
 * @param bm Pointer to the start of the bitmap.
 * @param i The bit index.
 *
 * @return The state of the given bit (0 or 1).
 */
int bitmap_get(void *bm, int i);

/**
 * Set the given bit in the bitmap to the given value.
 *
 * @param bm Pointer to the start of the bitmap.
 * @param i Bit index.
 * @param v Value the bit should be set to (0 or 1).
 */
void bitmap_put(void *bm, int i, int v);

/**
 * Pretty-print a bitmap. 
 *
 * @param bm Pointer to the bitmap.
 * @param size The number of bits to print.
 */
void bitmap_print(void *bm, int size);

#endif
