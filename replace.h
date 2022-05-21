#ifndef REPLACE_H
#define REPLACE_H

#define TALLOC_BUILD_VERSION_MAJOR 2
#define TALLOC_BUILD_VERSION_MINOR 3
#define TALLOC_BUILD_VERSION_RELEASE 3

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define HAVE_CONSTRUCTOR_ATTRIBUTE
#define HAVE_VA_COPY

#endif
