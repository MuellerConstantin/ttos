#ifndef _CTYPE_H
#define _CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Checks if the given character is an alphabetic character.
 * 
 * @param argument The character to check.
 * @return Non-zero if the character is an alphabetic character, zero otherwise.
 */
int isalpha(int argument);

#ifdef __cplusplus
}
#endif

#endif // _CTYPE_H
