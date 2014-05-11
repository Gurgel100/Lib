/*
 * stdarg.h
 *
 *  Created on: 22.10.2012
 *      Author: pascal
 */

#ifndef STDARG_H_
#define STDARG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _VA_LIST
#define _VA_LIST
typedef void *va_list;
#endif

#if (defined(__BUILTIN_VA_ARG_INCR) || defined(sparc) || defined(__sparc) || \
    defined(i386) || defined(__i386)) && !(defined(lint) || defined(__lint))

#define	va_start(list, name) (void) (list = (va_list) &__builtin_va_alist)
#define	va_arg(list, mode) ((mode *)__builtin_va_arg_incr((mode *)list))[0]

#else

#if __STDC__ != 0	/* -Xc compilation */
#define	va_start(list, name) (void) (list = (void *)((char *)&name + \
	((sizeof (name) + (sizeof (int) - 1)) & ~(sizeof (int) - 1))))
#else
#define	va_start(list, name) (void) (list = (void *)((char *)&...))
#endif	/* __STDC__ != 0 */
#define	va_arg(list, mode) ((mode *)(list = (char *)list + sizeof (mode)))[-1]

#endif	/* (defined(__BUILTIN_VA_ARG_INCR) || ... */

extern void va_end(va_list);
#define va_end(list)			(void)0

#ifdef __cplusplus
}
#endif

#endif /* STDARG_H_ */
