#include "utils.h"

string join(strings strs, string glue)
{
    string result;
    for (auto it : strs) {
        if (it != *strs.begin()) result += glue;
        result += it;
    }
    return result;
}

string hexForm(string str)
{
    if (str.length() == 0) return "(NULL)";

    string result = "0x";
    for (uint8_t ch : str) {
        uint8_t chH = ch >> 4, chL = ch & 15;
        result += chH + (chH >= 10 ? 'a' - 10 : '0');
        result += chL + (chL >= 10 ? 'a' - 10 : '0');
    }
    return result;
}

ostream& operator<<(ostream &sink, const strings strs)
{
    for (auto it : strs) {
        sink.put((uint8_t)it.length()) << it;
    }
    return sink.put('\0');
}

istream& operator>>(istream &source, strings &result)
{
    char buf[256];
    uint8_t length;

    result.clear();
    while (source.get((char&)length) && length > 0) {
        source.read(buf, length);
        buf[length] = '\0';
        result.push_back(buf);
    }
    return source;
}
