#ifndef __utils__
#define __utils__

#include "DNSTunnel.h"

string join(strings strs, string glue);
string hexForm(string str);

ostream& operator<<(ostream &sink, const strings strs);
istream& operator>>(istream &source, strings &result);

template <typename T>
ostream& operator<=(ostream &sink, T num)
{
    const char *ary = (const char*)&num;
    int size = sizeof(num) / sizeof(char);

    // FIXME: need support for big endian
    for (int i = size - 1; i >= 0; i--) sink.put(ary[i]);
    return sink;
}

template <typename T>
istream& operator>=(istream &source, T &num)
{
    char *ary = (char*)&num;
    int size = sizeof(num) / sizeof(char);

    // FIXME: need support for big endian
    for (int i = size - 1; i >= 0; i--) source.get(ary[i]);
    return source;
}

#endif
