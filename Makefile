CC=g++
FLAG=-O2 -std=c++11 -Wall -Wextra -Wshadow

OBJECTS=logger.o utils.o Tunnel.o NetDev.o DNSTunnel.o 
PROGS=vpnServer vpnClient

.SUFFIXES:

all: $(PROGS)

vpnServer: vpnServer.cpp $(OBJECTS)
	$(CC) $(FLAG) $(OBJECTS) vpnServer.cpp -o vpnServer

vpnClient: vpnClient.cpp $(OBJECTS)
	$(CC) $(FLAG) $(OBJECTS) vpnClient.cpp -o vpnClient

%.o: %.cpp %.h logger.h utils.h utils.cpp
	$(CC) $(FLAG) -c $< -o $@

clean:
	rm -f $(PROGS) $(OBJECTS)
