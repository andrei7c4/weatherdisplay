#ifndef INCLUDE_PARSEJSON_H_
#define INCLUDE_PARSEJSON_H_

#include <stddef.h>
#include <stdarg.h>
#include "contikijson/jsonparse.h"

typedef void (*PathCb)(const char*, int, int, void*);

typedef struct
{
    char **segments;
    size_t capacity;
    size_t count;
}Path;

typedef struct
{
    Path path;
    PathCb callback;
}PathCbPair;

#define bindPathCb(pair, cb, ...) pair.callback = cb; pathInit(&pair.path, __VA_ARGS__)

void parsejson(const char *json, int jsonLen, PathCbPair *callbacks, size_t callbacksSize, void *object);

#define pathInit(...) pathInit_(__VA_ARGS__, NULL)
void pathInit_(Path *path, const char *str1, ...);

void pathFree(Path *path, int freeSegments);


#endif /* INCLUDE_PARSEJSON_H_ */
