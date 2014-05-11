/*
 * ctype.c
 *
 *  Created on: 28.07.2012
 *      Author: pascal
 */

#include "ctype.h"

int islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

int isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}


int tolower(int c)
{
	return (isupper(c)) ? 'a' - 'A' + c : c;
}

int toupper(int c)
{
	return (islower(c)) ? 'A' - 'a' + c : c;
}
