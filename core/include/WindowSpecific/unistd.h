#ifndef _UNISTD_H
#define _UNISTD_H        1

/* This file intended to serve as a drop-in replacement for 
 *  unistd.h on Windows
 *  Please add functionality as neeeded 
 */

#include <stdlib.h>
#include <io.h>

#define srandom srand
#define random rand

#define W_OK 2
#define R_OK 4

#define access _access
#define ftruncate _chsize

#define ssize_t int
#define usleep(val) ::Sleep(val/1000)
#endif /* unistd.h  */
