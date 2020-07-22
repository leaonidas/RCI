#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>


int udpsock(char *rsaddr, char *rsport, struct addrinfo *hints, struct addrinfo **res, int listenUDP);
int udpclient (int fd, char *buffer, char *message, struct addrinfo *res, struct sockaddr_in addr, int smode, int rmode);
int udpserver (int fd, char *message, struct sockaddr_in *addr, int mode);


int whoisrootmsg (char *rsaddr, char *rsport, char *streamid, char *ipaddr, char *uport, char *asaddr, char *asport);
void dumpmsg (char *rsaddr, char *rsport);
void removemsg (char *rsaddr, char *rsport, char *streamid);


void popreqsend (char *asaddr, char *asport, char *streamid, char *ipaddr, char *tport);
void popreqrcv (int asfd, struct addrinfo *asres, char *streamid, aspoints *access);