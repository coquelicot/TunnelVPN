#include "logger.h"
#include "DNSTunnel.h"
#include "utils.h"

#include <cstring>
#include <sstream>

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

using std::stringstream;

// DNSQuery
DNSQuery::DNSQuery(const strings &_name, uint16_t _type, uint16_t _cls) :
    name(_name), type(_type), cls(_cls)
{
    if (name.size() > 0) {
        logDebug("Create query: %s." ,getLogForm().c_str());
    }
}

string DNSQuery::getLogForm() const
{
    stringstream result;
    result << join(name, ".") << ", " << type << ", " << cls;
    return result.str();
}

ostream& operator<<(ostream &sink, const DNSQuery &query)
{
    logDebug("Write query: %s." ,query.getLogForm().c_str());
    return sink << query.name <= query.type <= query.cls;
}

istream& operator>>(istream &source, DNSQuery &query)
{
    source >> query.name >= query.type >= query.cls;
    logDebug("Read query: %s.", query.getLogForm().c_str());
    return source;
}

// DNSRecord
DNSRecord::DNSRecord(const strings &_name, uint16_t _type, uint16_t _cls, uint32_t _ttl, string _rdata) :
    name(_name), type(_type), cls(_cls), ttl(_ttl), rdata(_rdata)
{
    if (name.size() > 0) {
        logDebug("Create record: %s." ,getLogForm().c_str());
    }
}

string DNSRecord::getLogForm() const
{
    stringstream result;
    result << join(name, ".") << ", " << type << ", " << cls << ", " << ttl << ", " << hexForm(rdata);
    return result.str();
}

ostream& operator<<(ostream &sink, const DNSRecord &record)
{
    logDebug("Write record: %s.", record.getLogForm().c_str());
    return (sink << record.name <= record.type <= record.cls <= record.ttl <= (uint16_t)record.rdata.length()) << record.rdata;
}

istream& operator>>(istream &source, DNSRecord &record)
{
    uint16_t rdataLen;
    source >> record.name >= record.type >= record.cls >= record.ttl >= rdataLen;

    char buf[rdataLen + 1];
    source.read(buf, rdataLen);
    record.rdata.assign(buf, buf + rdataLen);

    logDebug("Read record: %s.", record.getLogForm().c_str());
    return source;
}

// DNSPacket
DNSPacket::DNSPacket(uint16_t _id, uint16_t _flags, Queries _queries, DNSRecords _answers, DNSRecords _authors, DNSRecords _additions) :
    id(_id), flags(_flags), queries(_queries), answers(_answers), authors(_authors), additions(_additions)
{
    logDebug("Create DNSPacket: %s.", getLogForm().c_str());
}

string DNSPacket::getLogForm() const
{
    stringstream ss;
    ss << id << ", ";
    for (int i = 15; i >= 0; i--) ss << (((flags >> i) & 1) ? '1' : '0');

    ss << ", [";
    for (auto it : queries) ss << "(" << it.getLogForm() << ")";
    ss << "], [";
    for (auto it : answers) ss << "(" << it.getLogForm() << ")";
    ss << "], [";
    for (auto it : authors) ss << "(" << it.getLogForm() << ")";
    ss << "], [";
    for (auto it : additions) ss << "(" << it.getLogForm() << ")";
    ss << "]";

    return ss.str();
}

ostream& operator<<(ostream &sink, const DNSPacket &packet)
{
    logDebug("Write DNSPacket: %s.", packet.getLogForm().c_str());
    sink <= packet.id <= packet.flags
         <= (uint16_t)packet.queries.size() <= (uint16_t)packet.answers.size()
         <= (uint16_t)packet.authors.size() <= (uint16_t)packet.additions.size();
    for (auto it : packet.queries) sink << it;
    for (auto it : packet.answers) sink << it;
    for (auto it : packet.authors) sink << it;
    for (auto it : packet.additions) sink << it;
    return sink;
}

istream& operator>>(istream &source, DNSPacket &packet)
{
    uint16_t qrySize, ansSize, authSize, addiSize;
    source >= packet.id >= packet.flags >= qrySize >= ansSize >= authSize >= addiSize;

    packet.queries.resize(qrySize);
    packet.answers.resize(ansSize);
    packet.authors.resize(authSize);
    packet.additions.resize(addiSize);

    for (auto &it : packet.queries) source >> it;
    for (auto &it : packet.answers) source >> it;
    for (auto &it : packet.authors) source >> it;
    for (auto &it : packet.additions) source >> it;

    logDebug("Read DNSPacket: %s.", packet.getLogForm().c_str());
    return source;
}

DNSTunnel::DNSTunnel(int _sockfd, uint16_t _pkgid, bool _doResp, sockaddr_in _objAddr, bool _strict) :
    sockfd(_sockfd), pkgid(_pkgid), doResp(_doResp), objAddr(_objAddr), strict(_strict)
{
    logInfo("Create tunnel: -> %s:%d.", inet_ntoa(objAddr.sin_addr), ntohs(objAddr.sin_port));
}

DNSTunnel::~DNSTunnel() {}

Message DNSTunnel::recv()
{
    DNSPacket packet;
    while (que.empty()) {

        sockaddr_in rmtAddr;
        socklen_t addrLen = sizeof(rmtAddr);
        memset(&rmtAddr, 0, sizeof(rmtAddr));

        char buf[65536];
        int ret = recvfrom(sockfd, buf, sizeof(buf), 0, (sockaddr*)&rmtAddr, &addrLen);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                logWarning("Working in non-blocking mode?");
                usleep(500000);
                continue;
            } else {
                logError("Can't recvFrom: %s.", strerror(errno));
            }
        } else if (ret == 0) {
            logError("Socket closed?");
        }

        if (strict && objAddr.sin_port != rmtAddr.sin_port && objAddr.sin_addr.s_addr != rmtAddr.sin_addr.s_addr) {
            logWarning("objAddr and rmtAddr mismatch?");
            continue;
        }

        Message msg;
        stringstream(string(buf, buf + ret)) >> packet;
        for (auto record : packet.additions) {
            if (record.type != FLG_TYPE_TXT) continue;
            stringstream(record.rdata) >> msg;
            if (!strict) {
                msg.extra = string((char*)&rmtAddr, (char*)&rmtAddr + sizeof(rmtAddr));
                msg.extra += string((char*)&packet.id, (char*)&packet.id + sizeof(packet.id));
            }
            que.push(msg);
        }

    }

    Message ret = que.front(); que.pop();
    return ret;
}

void DNSTunnel::send(const Message &msg)
{
    stringstream tmp; tmp << msg;
    DNSRecord addi({}, FLG_TYPE_TXT, FLG_CLS_IN, 0, tmp.str());

    stringstream ss;
    if (doResp) {
        ss << DNSPacket(pkgid, FLG_QR | FLG_OP_QRY | FLG_AA | FLG_RSP_SUC, {_query}, {_response}, {}, {addi});
    } else {
        ss << DNSPacket(pkgid, FLG_OP_QRY, {_query}, {}, {}, {addi});
    }
    if (sendto(sockfd, ss.str().c_str(), ss.str().length(), 0, (sockaddr*)&objAddr, sizeof(objAddr)) != (ssize_t)ss.str().length()) {
        logError("Can't send all data QQ?");
    }
}

const DNSQuery DNSTunnel::_query({"tun", "worm"}, FLG_TYPE_A, FLG_CLS_IN);
const DNSRecord DNSTunnel::_response({"tun", "worm"}, FLG_TYPE_A, FLG_CLS_IN, 0, {'\127', '\0', '\0', '\1'});
