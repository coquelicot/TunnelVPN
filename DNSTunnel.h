#ifndef __DNSPacket__
#define __DNSPacket__

#include "Tunnel.h"

#include <queue>
#include <vector>
#include <string>
#include <istream>
#include <ostream>
#include <cstdint>

#include <arpa/inet.h>
#include <sys/socket.h>

using std::queue;
using std::string;
using std::vector;
using std::ostream;
using std::istream;
typedef vector<string> strings;

static const int FLG_CLS_IN   = 1;
static const int FLG_TYPE_A   = 1;
static const int FLG_TYPE_NS  = 2;
static const int FLG_TYPE_CN  = 5;
static const int FLG_TYPE_SOA = 6;
static const int FLG_TYPE_PTR = 12;
static const int FLG_TYPE_MX  = 15;
static const int FLG_TYPE_TXT = 16;
static const int FLG_TYPE_A6  = 38;
static const int FLG_TYPE_ANY = 255;

static const int FLG_QR       = 1 << 15;
static const int FLG_OP_QRY   = 0 << 11;
static const int FLG_OP_INV   = 1 << 11;
static const int FLG_OP_STA   = 2 << 11;
static const int FLG_AA       = 1 << 10;
static const int FLG_TC       = 1 << 9;
static const int FLG_RD       = 1 << 8;
static const int FLG_RA       = 1 << 7;
static const int FLG_RSP_SUC  = 0 << 0;
static const int FLG_RSP_ERR  = 1 << 0; // format error
static const int FLG_RSP_FAIL = 2 << 0; // server failure

struct DNSQuery
{
    strings name;
    uint16_t type, cls;

    DNSQuery(const strings &_name={}, uint16_t _type=FLG_TYPE_ANY, uint16_t _cls=FLG_CLS_IN);
    string getLogForm() const;

    friend ostream& operator<<(ostream &sink, const DNSQuery &query);
    friend istream& operator>>(istream &source, DNSQuery &query);
};

struct DNSRecord
{
    strings name;
    uint16_t type, cls;
    uint32_t ttl;
    string rdata;

    DNSRecord(const strings &_name={}, uint16_t _type=FLG_TYPE_A, uint16_t _cls=FLG_CLS_IN, uint32_t _ttl=0, string _rdata="");
    string getLogForm() const;

    friend ostream& operator<<(ostream &sink, const DNSRecord &record);
    friend istream& operator>>(istream &source, DNSRecord &record);
};

struct DNSPacket
{
    typedef vector<DNSQuery> Queries;
    typedef vector<DNSRecord> DNSRecords;

    // header
    uint16_t id;
    uint16_t flags; // QR, OP, AA, TC, RD, RA rCode

    DNSPacket(uint16_t _id=0, uint16_t _flags=0, Queries _queries={}, DNSRecords _answers={}, DNSRecords _authors={}, DNSRecords _additions={});
    string getLogForm() const;

    // datas
    Queries queries;
    DNSRecords answers, authors, additions;

    friend ostream& operator<<(ostream &sink, const DNSPacket &packet);
    friend istream& operator>>(istream &source, DNSPacket &packet);
};

struct DNSTunnel : public Tunnel
{
    static const DNSQuery _query;
    static const DNSRecord _response;

    int sockfd;
    uint16_t pkgid;
    bool doResp;
    sockaddr_in objAddr;
    bool strict;
    queue<Message> que;

    DNSTunnel(int _sockfd, uint16_t _pkgid, bool _doResp, sockaddr_in _objAddr, bool _strict=true);
    ~DNSTunnel();

    Message recv();
    void send(const Message &msg);
};

#endif
