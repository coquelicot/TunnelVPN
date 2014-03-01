#include "Tunnel.h"
#include "utils.h"
#include "logger.h"

Message::Message(uint8_t _cmd, string _data, string _extra) :
    cmd(_cmd), data(_data), extra(_extra)
{
    if (data.length() >= 65536) logError("Message too long.");
    logDebug("Create message: %u, %u.", (uint32_t)cmd, (uint32_t)data.length());
}

ostream& operator<<(ostream& sink, const Message &msg)
{
    logDebug("Write message: %u, %u.", (uint32_t)msg.cmd, (uint32_t)msg.data.length());
    return (sink <= msg.cmd <= (uint16_t)msg.data.length()) << msg.data;
}

istream& operator>>(istream& source, Message &msg)
{
    uint16_t length;
    source >= msg.cmd >= length;
    msg.extra = "";

    char buf[length];
    source.read(buf, length);
    msg.data.assign(buf, buf + length);
    logDebug("Read message: %u, %u.", (uint32_t)msg.cmd, (uint32_t)msg.data.length());
    return source;
}
