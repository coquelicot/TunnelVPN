#include "logger.h"
#include "utils.h"
#include "DNSTunnel.h"

#include <cstring>
#include <map>
#include <sstream>
#include <utility>

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using std::map;
using std::pair;
using std::make_pair;
using std::stringstream;

DNSTunnel* createTunnel()
{
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) logError("Can't create socket: %s.", strerror(errno));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        logError("Can't bind: %s.", strerror(errno));
    }

    return new DNSTunnel(sockfd, 0, false, addr, false);
}

bool operator<(const sockaddr_in &a, const sockaddr_in &b)
{
    if (a.sin_addr.s_addr != b.sin_addr.s_addr) {
        return a.sin_addr.s_addr < b.sin_addr.s_addr;
    } else {
        return a.sin_port < b.sin_port;
    }
}

int main()
{
    logLevel = LOG_LV_DEBUG;
    logInfo("Start server (%d)", getpid());

    DNSTunnel *source = createTunnel();

    std::map<sockaddr_in, pair<int, DNSTunnel*>> tunMap;
    while (true) {

        uint16_t pkgid;
        sockaddr_in cliAddr;
        Message msg = source->recv();
        memcpy((char*)&cliAddr, msg.extra.c_str(), sizeof(cliAddr));
        memcpy((char*)&pkgid, msg.extra.c_str() + sizeof(cliAddr), sizeof(pkgid));

        if (tunMap.count(cliAddr) == 0) {
            logInfo("Accepting new client.");
            int idx = tunMap.size() + 1;
            tunMap[cliAddr] = make_pair(idx, new DNSTunnel(source->sockfd, pkgid, true, cliAddr, true));
        }

        if (msg.cmd == CMD_MSG) {
            for (auto it : tunMap) {
                if (it.first < cliAddr || cliAddr < it.first) it.second.second->send(msg);
            }
        } else if (msg.cmd == CMD_GET_IP) {
            char buf[32];
            sprintf(buf, "172.32.0.%d/24", tunMap[cliAddr].first);
            tunMap[cliAddr].second->send(Message(CMD_SET_IP, buf));
        }

    }

}
