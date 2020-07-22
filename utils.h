#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


void helpinfo();

int headercheck(char *buffread);

int checkenter(char *ptr, int nread);

int max (int udpas, int tcpups, int tcplst, aspoints *sons, int tcpsessions);

int checkpoints(int *v, int tcpsessions);

void hexa (int count, char *queryid);