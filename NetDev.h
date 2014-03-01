#ifndef __NetDev__
#define __NetDev__

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <string>
using std::string;

class NetDev {

    int tunfd;
    char addr[24], devName[IFNAMSIZ];

    void createNetDev(int flags);
    void setupNetDev();

    public:

    NetDev(string _addr="172.32.0.1/24", string _devName="tunDNS", int flags=IFF_TUN|IFF_NO_PI);
    ~NetDev();

    string getName() const;
    string recv();
    void send(string msg);

};

#endif
