#include <sys/types.h>
#include <memory.h>
#include <errno.h>
#include <string>

extern "C"
char*
readpassphrase(const char *prompt, char *buf, size_t bufSize, int flags)
{
    if (bufSize < 9)
    {
        errno = EINVAL;
        return NULL;
    }
    std::string pass = "testpass";
    memcpy(buf, pass.c_str(), pass.length() + 1);
    return buf;
}

