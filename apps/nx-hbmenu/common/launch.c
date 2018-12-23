#include "common.h"

size_t launchAddArg(argData_s* ad, const char* arg) {
    size_t len = strlen(arg)+1;
    if ((ad->dst+len) >= (char*)(ad->buf + sizeof(ad->buf))) return len; // Overflow
    ad->buf[0]++;
    strcpy(ad->dst, arg);
    ad->dst += len;
    return len;
}

void launchAddArgsFromString(argData_s* ad, char* arg) {
    char c, *pstr, *str=arg, *endarg = arg+strlen(arg);

    do
    {
        do
        {
            c = *str++;
        } while ((c == ' ' || c == '\t') && str < endarg);

        pstr = str-1;

        if (c == '\"')
        {
            pstr++;
            while(*str++ != '\"' && str < endarg);
        }
        else if (c == '\'')
        {
            pstr++;
            while(*str++ != '\'' && str < endarg);
        }
        else
        {
            do
            {
                c = *str++;
            } while (c != ' ' && c != '\t' && str < endarg);
        }

        str--;

        if (str == (endarg - 1))
        {
            if(*str == '\"' || *str == '\'')
                *(str++) = 0;
            else
                str++;
        }
        else
        {
                *(str++) = '\0';
        }

        launchAddArg(ad, pstr);

    } while(str<endarg);
}
