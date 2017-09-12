#include <sys/types.h>
#include <memory.h>
#include <errno.h>

char*
readpassphrase(const char *prompt, char *buf, size_t bufSize, int flags)
{
    if (bufSize < 9)
    {
        errno = EINVAL;
        return NULL;
    }
    memcpy(buf, "testpass\0", 9);
    return buf;
}

