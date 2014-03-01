#ifndef __Tunnel__
#define __Tunnel__

#include <string>
#include <iostream>

#include <arpa/inet.h>

using std::string;
using std::ostream;
using std::istream;

static const int CMD_ALIVE  = 0;
static const int CMD_GET_IP = 1;
static const int CMD_SET_IP = 2;
static const int CMD_MSG    = 3;
static const int CMD_DIE    = 4;

struct Message
{
    uint8_t cmd;
    string data;
    string extra;

    Message(uint8_t _cmd=CMD_ALIVE, string _data="", string _extra="");

    friend ostream& operator<<(ostream& sink, const Message &msg);
    friend istream& operator>>(istream& source, Message &msg);
};

struct Tunnel
{
    ~Tunnel() {}

    virtual Message recv() = 0;
    virtual void send(const Message &msg) = 0;
};

#endif
