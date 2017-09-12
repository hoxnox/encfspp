#include <string.h>

char testpass[] = {'t', 'e', 's', 't', 'p', 'a', 's', 's', '\0'};

char*
readpassphrase(const char *prompt, char *buf, size_t bufSize, int flags)
{
    return testpass;
}

