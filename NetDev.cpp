#include "NetDev.h"
#include "logger.h"

#include <cstring>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stropts.h>
#include <asm-generic/ioctl.h>

NetDev::NetDev(string _addr, string _devName, int flags)
{
    if (_addr.length() > sizeof(addr)) logWarning("Address truncated.");
    strncpy(addr, _addr.c_str(), sizeof(addr));
    if (_devName.length() > sizeof(devName)) logWarning("DevName truncated.");
    strncpy(devName, _devName.c_str(), sizeof(devName));

    logInfo("Creating device..");
    createNetDev(flags);
    logInfo("Setting up device..");
    setupNetDev();
    logInfo("NetDev %s created, addr=%s.", devName, addr);
}

NetDev::~NetDev()
{
    close(tunfd);
}

string NetDev::getName() const
{
    return string(devName);
}

void NetDev::createNetDev(int flags)
{
    static const char *cloneDev = "/dev/net/tun";

    if ((tunfd = open(cloneDev, O_RDWR)) < 0) logError("Can't open cloneDev.");

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = flags;
    strncpy(ifr.ifr_name, devName, IFNAMSIZ);
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) logError("Can't ioctl: %s.", strerror(errno));

    strncpy(devName, ifr.ifr_name, IFNAMSIZ);
}

void NetDev::setupNetDev()
{
    int ret;
    char buf[1024];

    sprintf(buf, "ip link set dev %s up", devName);
    if ((ret = system(buf))) logError("Can't bring up %s, return=%d.", devName, ret);

    sprintf(buf, "ip addr add dev %s %s", devName, addr);
    if ((ret = system(buf))) logError("Can't set addr of %s, return=%d.", devName, ret);
}

string NetDev::recv()
{
    char buf[65536];
    for (;;) {
        int length = read(tunfd, buf, sizeof(buf));
        if (length < 0) {
            logWarning("Can't read? %s.", strerror(errno));
            usleep(100000);
        } else if (length == 0) {
            logWarning("read() returning zero OAO?");
        } else {
            // FIXME: strange 0x00000080
            return string(buf, buf + length);
        }
    }
}

void NetDev::send(string msg)
{
    const char *_msg = msg.c_str();
    int sent = 0, length = msg.length();
    while (sent < length) {
        int ret = write(tunfd, _msg + sent, length - sent);
        if (ret < 0) {
            logWarning("Can't write? %s.", strerror(errno));
            usleep(100000);
        } else if (length == 0) {
            logWarning("write() returning zero OAO?");
        } else {
            sent += ret;
        }
    }
}
