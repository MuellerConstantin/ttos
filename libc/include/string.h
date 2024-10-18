#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Determines the length of a string.
 * 
 * @param str The string to determine the length of.
 * @return The length of the string.
 */
size_t strlen(const char*);

/**
 * Reverses a string.
 * 
 * @param str The string to reverse.
 * @return The reversed string.
 */
char *strrev(char *str);

/**
 * Copies the string pointed to by src, including the null-terminator,
 * to the buffer pointed to by dest.
 * 
 * @param dest The destination to copy the string to.
 * @param src The source to copy the string from.
 * @return A pointer to the destination.
 */
char* strcpy(char* dest, const char* src);

/**
 * Compares two strings.
 * 
 * @param str1 The first string to compare.
 * @param str2 The second string to compare.
 * @return An integer less than, equal to, or greater than zero if str1 is found,
 *        respectively, to be less than, to match, or be greater than str2.
 */
int strcmp(const char* str1, const char* str2);

/**
 * Appends the string pointed to by src to the end of the string pointed to by dest.
 * 
 * @param dest The destination to append the string to.
 * @param src The source to append the string from.
 * @return A pointer to the destination.
 */
char* strcat(char* dest, const char* src);

/**
 * Copies the given value into each of the first n characters
 * of the object pointed to by dest.
 * 
 * @param dest The destination to copy the value to.
 * @param ch The value to copy.
 * @param n The number of characters to copy.
 * @return A pointer to the destination.
 */
void *memset(void *dest, uint8_t ch, size_t n);

/**
 * Copies the given value into each of the first n characters
 * of the object pointed to by dest.
 * 
 * @param dest The destination to copy the value to.
 * @param value The value to copy.
 * @param n The number of characters to copy.
 * @return A pointer to the destination.
 */
void *memsetw(void *dest, uint16_t value, size_t n);

/**
 * Copies n characters from the object pointed to by src into
 * the object pointed to by dest.
 * 
 * @param dest The destination to copy the characters to.
 * @param src The source to copy the characters from.
 * @param n The number of characters to copy.
 * @return A pointer to the destination.
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * Compares the first n bytes of the objects pointed to by s1 and s2.
 * 
 * @param s1 The first object to compare.
 * @param s2 The second object to compare.
 * @param n The number of bytes to compare.
 * @return An integer less than, equal to, or greater than zero if the first n bytes of s1 is found,
 *         respectively, to be less than, to match, or be greater than the first n bytes of s2.
 */
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * Separates the string into tokens.
 * 
 * @param stringp The string to separate.
 * @param delim The delimiter to separate the string by.
 * @return The next token from the string.
 */
char *strsep(char **stringp, const char *delim);

/**
 * Locates the first occurrence in the string s of any character in the string accept.
 * 
 * @param s The string to search.
 * @param accept The characters to search for.
 * @return A pointer to the character in s that matches one of the characters in accept,
 *         or NULL if no such character is found.
 */
char *strpbrk(const char *s, const char *accept);

/**
 * Compares the first n bytes of str1 and str2.
 * 
 * @param str1 The first string to compare.
 * @param str2 The second string to compare.
 * @param n The number of bytes to compare.
 * @return An integer less than, equal to, or greater than zero if the first n bytes of str1 is found,
 *        respectively, to be less than, to match, or be greater than the first n bytes of str2.
 */
int strncmp(const char *str1, const char *str2, size_t n);

#ifdef __cplusplus
}
#endif

#endif // _STRING_H
