#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

char *util_getcwd_alloc(void);
int util_parse_positive_int(const char *text, int *value_out);

#endif
