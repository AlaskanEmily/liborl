#pragma once
#include <string.h>

#if (! defined __linux__) || (defined FORCE_STRCASESTR)

#ifdef __cplusplus
extern "C"
#endif
int strcasecmp(const char *, const char *);

#endif
