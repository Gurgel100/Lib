/*
 * string.c
 *
 *  Created on: 27.07.2012
 *      Author: pascal
 */

#include "string.h"
#include "stdlib.h"

int strcmp(const char *str1, const char *str2)
{
	register int i;
	for(i = 0; str1[i] != '\0' && str2[i] != '\0'; i++)
	{
		if(str1[i] == str2[i])
			continue;
		return str1[i] - str2[i];
	}
	if(str1[i] == str2[i])
		return 0;
	return str1[i] - str2[i];
}

size_t strlen(const char *cs)
{
	register size_t i;
	for(i = 0; cs[i] != '\0'; i++);
	return i;
}

void *memset(void *block, int c, size_t n)
{
	register unsigned char *i;
	for(i = block; (void*)i < block + n; i++)
		*i = (unsigned char)c;
	return block;
}

void *memmove(void *to, const void *from, size_t size)
{
	size_t i;
	char *dest = to;
	char *src = (char*)from;
	char *tmp;
	if(from + size > to)
	{
		tmp = malloc(size);
		for(i = 0; i < size; i++)
		{
			tmp[i] = src[i];
		}
		for(i = 0; i < size; i++)
		{
			dest[i] = tmp[i];
		}
		free(tmp);
	}
	else
	{
		for(i = 0; i < size; i++)
		{
			dest[i] = src[i];
		}
	}
	return to;
}
