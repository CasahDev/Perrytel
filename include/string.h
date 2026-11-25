#pragma once
#include <stdbool.h>
#include <kernel/memory.h>

unsigned long strlen(const char *str);

/**
 * Check if the two given strings are equal
 * @param str1 The first string to compare
 * @param str2 The second string
 * @return if both strings are equal
 */
bool streq(const char *str1, const char *str2);

/**
 * Copy a string into another char pointer
 * @param dest The destination
 * @param src The sourcr
 */
void strcpy(char *dest, const char *src);

/**
 * Convert an integer into it's string equivalent
 * @param n The input int
 * @param dist The output string
 */
void itoa(int n, char *dist);

/**
 * Split a string into a string buffer based on the given spliter
 * and the output buffer max size
 * @param str The input string
 * @param out The output string buffer
 * @param max_args The maximum number of strings the buffer
 * @param splitter The separator
 * @return The number of strings in the byffer
 */
int strsplit(char *str, char **out, uint32_t max_args, char splitter);

/**
 * Concat 2 strings into a third
 * @param str The output string
 * @param str1 First input string
 * @param str2 Second input string
 * @return 0 if went well
 */
int strcat(char *str, const char *str1, const char *str2);

/**
 * Lower a given char
 * @param c The char to lower
 * @return The lowered char
 */
char to_lower(char c);

/**
 * Convert a string into it's lower case equivalent
 * @param dest The destination string
 * @param src The source string
 */
void strlow(char *dest, const char *src);

/**
 * Compare 2 strings, if a difference is detected, returns the ascii
 * difference between them and stops the function
 * @param str1 The first string to compare
 * @param str2 The second string to compare
 * @param len The length to compare
 * @return The difference between 2 chars of each strings
 */
int strncmp(const char *str1, const char *str2, int len);

/**
 * Convert a string into it's equivalent in long
 *
 * If a char different from a digit is present, the function will end and
 *
 * Only works with base 0 for the moment
 * endptr will point at the last checked char
 * @param str The string to convert
 * @param endptr The last char analyzed by the function
 * @param base The base of the number (2, 10, 16...)
 * @return The number itself
 */
long strtol(const char *str, char **endptr, int base);
