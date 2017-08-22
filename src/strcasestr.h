#pragma once
#include <string.h>

#if (! defined __linux__) || (defined FORCE_STRCASESTR)

#ifdef __cplusplus
extern "C"
#endif
char *strcasestr(const char *, const char *);

#endif
