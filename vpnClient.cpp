#include "logger.h"
#include "NetDev.h"
#include "DNSTunnel.h"

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>

#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

Tunnel* createTunnel(char *objHost)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) logError("Can't create socket: %s.", strerror(errno));

    int port = rand() % 10000 + 10000;
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (sockaddr*)&addr, addrLen) < 0) logError("Can't bind: %s." ,strerror(errno));
    logInfo("Bind UDP at port %d.", port);

    addr.sin_port = htons(53);
    inet_aton(objHost, &addr.sin_addr);
    return new DNSTunnel(sockfd, rand() % 50000 + 10000, false, addr);
}

NetDev* createNetDev(Tunnel *tun)
{
    logInfo("Asking for ip/mask..");
    Message getip(CMD_GET_IP);
    for (int i = 0; i < 3; i++) {
        tun->send(getip);
        usleep(100000);
    }
    while (true) {
        Message msg = tun->recv();
        if (msg.cmd != CMD_SET_IP) tun->send(getip);
        else return new NetDev(msg.data);
    }
}

pid_t pid;
void cleanup()
{
    kill(pid, SIGKILL);
    wait(NULL);
}

int main(int argc, char *args[])
{
    //logLevel = LOG_LV_DEBUG;
    if (argc < 2) logError("No enough argument, usage: %s <IP>", args[0]);
    logInfo("Start client (%d)", getpid());
    srand(getpid());

    Tunnel *tun = createTunnel(args[1]);
    NetDev *net = createNetDev(tun);

    pid = fork();
    if (pid < 0) logError("Can't fork: %s.", strerror(errno));

    if (pid == 0) {
        while (true) tun->send(Message(CMD_MSG, net->recv()));
    } else {
        atexit(cleanup);
        while (true) {
            Message msg = tun->recv();
            if (msg.cmd == CMD_MSG) net->send(msg.data);
        }
    }

}

