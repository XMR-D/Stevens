#pragma once   /* PORTABILITY_H */

/* Feature test macros - MUST come before any includes */
#if defined(__sun__) || defined(__sun)  /* Solaris/OmniOS */

#define __EXTENSIONS__    /* Enable Solaris extensions */
#define _XOPEN_SOURCE 600 /* X/Open 6, POSIX 2004 */
#define _POSIX_C_SOURCE 200112L /* POSIX.1-2001 for setenv */

#elif defined(__linux__)

#define _GNU_SOURCE       /* GNU extensions */
#define _POSIX_C_SOURCE 200809L /* POSIX 2008 */

#elif defined(__NetBSD__)
#endif


/* !PORTABILITY_H */
