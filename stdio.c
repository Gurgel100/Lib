/*
 * stdio.c
 *
 *  Created on: 26.07.2012
 *      Author: pascal
 */

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdarg.h"
#include "string.h"
#include "userlib.h"

FILE* stderr;
FILE* stdin;
FILE* stdout;

char *itoa(int64_t x, char *s);		//Int nach String
char *utoa(uint64_t x, char *s);	//UInt nach String
char *ftoa(float x, char *s);		//Float nach String

//TODO: alle print-Funktionen fertigstellen
int fprintf(FILE *stream, const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	uint64_t pos = 0;
	char *buffer = malloc(64);	//Mehr wird momentan nicht benötigt
	if(stream == stdout)
	{
	for(; *format; format++)
	{
		switch(*format)
		{
			case '%':	//Formatieren?
				switch(*(++format))
				{
					case 'u':	//Unsigned int
						utoa(va_arg(arg, uint64_t), buffer);
						puts(buffer);
						pos += strlen(buffer);
					break;
					case 'i':	//Signed int
					case 'b':
						itoa(va_arg(arg, int64_t), buffer);
						puts(buffer);
						pos += strlen(buffer);
					break;
					case 'f':	//Float
						ftoa(va_arg(arg, double), buffer);
						puts(buffer);
						pos += strlen(buffer);
					break;
					case 'X':	//Hex 8
					break;
					case 'x':	//Hex 4
					break;
					case 'y':	//Hex 2
					break;
					case 's':	//String
					{
						char *temp = va_arg(arg, char*);
						puts(temp);
						pos += strlen(temp);
					}
					break;
					case 'c':	//Char
						putchar((char)va_arg(arg, int32_t));
						pos++;
					break;
					case '%':	//%
						putchar('%');
						pos++;
					break;
					default:	//Ansonsten ungültig
						format--;
						pos--;
					break;
				}
			break;
			default:	//Ansonsten schreibe das aktuelle Zeichen
				putchar(*format);
				pos++;
			break;
		}
	}
	}
	else
		pos = EOF;
	free(buffer);
	va_end(arg);
	return pos;
}
int printf(const char *format, ...)
{
	return fprintf(stdout, format);
}

int sprintf(char *str, const char *format, ...)
{
	return EOF;
}

int scanf(const char *format, ...)
{
	return EOF;
}

int sscanf(const char *str, const char *format, ...)
{
	return EOF;
}

int getc(FILE *stream)
{
	if(stream == stdin)
	{
		unsigned char zeichen;
		asm("int $0x30" :"=a"(zeichen) :"a"(20));
		return (int)zeichen;
	}
	else
		return EOF;
}

int getchar(void)
{
	return getc(stdin);
}

char *gets(char *str)
{
	char c;
	int i = 0;
	while((c = getc(stdin)) != '\n' && c != '\0')
	{
		switch(c)
		{
			case EOF:
				return NULL;
			case '\b':	//Backspace
				i--;
			break;
			default:
				str[i++] = c;
			break;
		}
	}
	str[i] = '\0';
	return str;
}


int putc(int zeichen, FILE *stream)
{
	if(stream == stdout)
	{
		asm("int $0x30" : : "a"(21), "b"(zeichen & 0xFF));
		return zeichen;
	}
	else
		return EOF;
}

int putchar(int zeichen)
{
	return putc(zeichen, stdout);
}

int puts(const char *str)
{
	unsigned int i;
	for(i = 0; str[i] != '\0'; i++)
	{
		if(putc(str[i], stdout) == EOF)
			return EOF;
	}
	return 1;
}

//Hilfsfunktionen
char *itoa(int64_t x, char *s)
{
	uint64_t i = 0;
	bool sign = x < 0;	//Ist true falls die Zahl ein Vorzeichen hat (x < 0)
	do
	{
		s[i++] = abs((x % 10)) + '0';
	}
	while((x /= 10) != 0);
	if(sign)
	{
		x = -x;
		s[i++] = '-';	//Das Minus kommt an den Anfang
	}
	s[i] = '\0';		//Ende des Strings markieren
	reverse(s);
	return s;
}

char *utoa(uint64_t x, char *s)
{
	uint64_t i = 0;
	do
	{
		s[i++] = (x % 10) + '0';
	}
	while((x /= 10) > 0);
	s[i] = '\0';		//Ende des Strings markieren
	reverse(s);
	return s;
}

char *ftoa(float x, char *s)
{
	char *buffer = s;
	itoa((int64_t)x, s);
	if(x < 0) x = -x;

	buffer += strlen(s);
	*buffer++ = '.';

	*buffer++ = ((uint64_t)(x * 10.0) % 10) + '0';
	*buffer++ = ((uint64_t)(x * 100.0) % 10) + '0';
	*buffer++ = ((uint64_t)(x * 1000.0) % 10) + '0';
	*buffer++ = ((uint64_t)(x * 10000.0) % 10) + '0';
	*buffer++ = ((uint64_t)(x * 100000.0) % 10) + '0';
	*buffer++ = ((uint64_t)(x * 1000000.0) % 10) + '0';
	*buffer = '\0';
	return s;
}
