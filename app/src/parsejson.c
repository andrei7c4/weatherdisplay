#include <os_type.h>
#include <mem.h>
#include <string.h>
#include "parsejson.h"
#include "typedefs.h"


LOCAL void pathAlloc(Path *path, size_t size);
LOCAL void pathPush(Path *path, const char *str);
LOCAL const char* pathGetLast(Path *path);
LOCAL void pathReplaceLast(Path *path, const char *str);
LOCAL void pathPop(Path *path);
LOCAL int pathEqual(const Path *p1, const Path *p2);

void ICACHE_FLASH_ATTR parsejson(const char *json, int jsonLen, PathCbPair *callbacks, size_t callbacksSize, void *object)
{
    int type;
    int curDepth = 0;

    Path path;
    pathAlloc(&path, JSONPARSE_MAX_DEPTH);

    struct jsonparse_state state;
    jsonparse_setup(&state, json, jsonLen);

    while ((type = jsonparse_next(&state)) != 0)
    {
        if (state.depth > curDepth)
        {
            char *str = os_malloc(2);
            str[0] = (char)type;
            str[1] = '\0';
            pathPush(&path, str);
        }
        else if (state.depth < curDepth)
        {
            pathPop(&path);
        }
        curDepth = state.depth;

        if (type == JSON_TYPE_PAIR_NAME)
        {
            size_t strLen = jsonparse_get_len(&state);
            char *str = os_malloc(strLen+1);
            jsonparse_copy_value(&state, str, strLen+1);
            pathReplaceLast(&path, str);

            // search matching callbacks for current path
            size_t i;
            for (i = 0; i < callbacksSize; i++)
            {
                PathCbPair *pathCb = &callbacks[i];
                if (pathEqual(&path, &pathCb->path) && pathCb->callback)
                {
                    type = jsonparse_next(&state);
                    if (type)
                    {
                        strLen = jsonparse_get_len(&state);
                        str = os_malloc(strLen+1);
                        jsonparse_copy_value(&state, str, strLen+1);
                        pathCb->callback(str, strLen, type, object);
                        os_free(str);
                    }
                    else
                    {
                        goto parseout;
                    }
                    break;
                }
            }
        }
        else if (type == JSON_TYPE_OBJECT)
        {
            size_t i;
            for (i = 0; i < callbacksSize; i++)
            {
                PathCbPair *pathCb = &callbacks[i];
                if (pathEqual(&path, &pathCb->path) && pathCb->callback)
                {
                    pathCb->callback(NULL, 0, type, object);
                    break;
                }
            }
        }
    }
parseout:
    pathFree(&path, TRUE);
}


void ICACHE_FLASH_ATTR pathInit_(Path *path, const char *str1, ...)
{
    va_list args;

    // get arguments count
    va_start(args, str1);
    const char *str = str1;
    size_t count = 0;
    while(str)
    {
        count++;
        str = va_arg(args, char*);
    }
    va_end(args);

    // allocate space and copy pointers to strings
    pathAlloc(path, count);
    va_start(args, str1);
    str = str1;
    while(str)
    {
        pathPush(path, str);
        str = va_arg(args, char*);
    }
    va_end(args);
}

void ICACHE_FLASH_ATTR pathFree(Path *path, int freeSegments)
{
    if (freeSegments)
    {
        while(path->count)
        {
            path->count--;
            os_free(path->segments[path->count]);
        }
    }
    os_free(path->segments);
    path->capacity = 0;
    path->count = 0;
}

LOCAL void ICACHE_FLASH_ATTR pathAlloc(Path *path, size_t size)
{
    path->segments = os_malloc(size * sizeof(char*));
    path->capacity = size;
    path->count = 0;
}

LOCAL void ICACHE_FLASH_ATTR pathPush(Path *path, const char *str)
{
    if (path->count < path->capacity)
    {
        path->segments[path->count] = (char*)str;
        path->count++;
    }
}

LOCAL ICACHE_FLASH_ATTR const char* pathGetLast(Path *path)
{
    if (path->count)
    {
        return path->segments[path->count - 1];
    }
    return NULL;
}

LOCAL void ICACHE_FLASH_ATTR pathReplaceLast(Path *path, const char *str)
{
    if (path->count)
    {
        os_free(path->segments[path->count - 1]);
        path->segments[path->count - 1] = (char*)str;
    }
}

LOCAL void ICACHE_FLASH_ATTR pathPop(Path *path)
{
    if (path->count)
    {
        path->count--;
        os_free(path->segments[path->count]);
        path->segments[path->count] = NULL;
    }
}

LOCAL int ICACHE_FLASH_ATTR pathEqual(const Path *p1, const Path *p2)
{
    if (p1->count != p2->count)
    {
        return FALSE;
    }
    size_t i;
    for(i = 0; i < p1->count; i++)
    {
        if (strcmp(p1->segments[i], p2->segments[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}
