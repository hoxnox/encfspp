#include <sys/types.h>
#include <memory.h>
#include <errno.h>
#include <string>
#include <EncfsMounter.hpp>

extern "C"
char*
readpassphrase(const char *prompt, char *buf, size_t bufSize, int)
{
    std::string pass = encfspp::EncfsMounter::readpassphrase(prompt);
    if (bufSize < pass.size())
    {
        errno = EINVAL;
        return NULL;
    }
    memcpy(buf, pass.c_str(), pass.length() + 1);
    return buf;
}

