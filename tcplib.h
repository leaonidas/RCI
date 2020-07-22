#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>


int tcpsockClient(char *rsaddr, char *rsport, struct addrinfo *hints, struct addrinfo **res);
int tcpsockServer(char *rsport, struct addrinfo *hints, struct addrinfo **res);


int tcpread(messages **m, int fd);


void welcome(int fd, char *streamid);
void redirect(int fd, char *message, char *ipaddr, char *tport, int mode);
void newpop(int fd, char *message, char *ipaddr, char *tport, int mode);
//void popquery (int fd, char *message, aspoints *query, int mode);
//void popreply(int fd, aspoints *query);
void popquery (char *message, aspoints *sons, aspoints *query, int mode);
void popreply(int fd, aspoints *query, char *ipaddr, char *tport, int bp);
void prrecv(char *message, char *queryid, char *ip, char *port, int *avails);


int brokenstream(int fd);
int streamflow(int fd);

void treequery(int fd, char *ipaddr, char *tport, int mode);
void treereply(int fd, char *buffer, char *ipaddr, char *tport, int tcpsessions, aspoints *sons);