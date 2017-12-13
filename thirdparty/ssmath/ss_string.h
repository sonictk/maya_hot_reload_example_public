#ifndef SS_STRING_H
#define SS_STRING_H

#include <string.h>


/**
 * This function replaces the given ``token`` in the ``input`` string.
 *
 * @param input		The input string to replace characters in.
 * @param output		A buffer that will have the new string written to. Should
 * 					be the ``size`` specified.
 * @param token		The search character to replace.
 * @param replace		The character to replace it with.
 * @param size			The number of characters to write to the new string.
 *
 * @return				The number of times the ``token`` was replaced. Returns a
 * 					negative value on error.
 */
inline int stringReplace(const char *input,
						 char *output,
						 const char token,
						 const char replace,
						 unsigned int size)
{
	sizet len = strlen(input);
	if (len <= 0) {
		return 0;
	}
	int replaced = 0;
	unsigned int i = 0;
	for (; i < size && i < (len + 1) && input[i] != '\0'; ++i) {
		if (input[i] == token) {
			output[i] = replace;
			replaced++;
		} else {
			output[i] = input[i];
		}
	}
	output[i] = '\0';

	return replaced;
}


#endif /* SS_STRING_H */
