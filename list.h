#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

typedef struct as_points{
    
    int fd;
    char ip[128];
    char tport[6];
    char buffer[128];
    int avails;
    struct as_points *next;
    
} aspoints;

typedef struct messages_{

	char buffer[128];
	int fd;
	struct messages_ *next;

} messages;


void addaspoint (aspoints **top, int fd, char *ip, char *tport, int avails);

void addfirst(aspoints **top, int fd, char *ip, char *tport, int avails);

int removeaspoint (aspoints *top);

void removebyfd(aspoints *top, int fd);

void removebyid(aspoints *top, char *id);

void printaccess(aspoints *top);

void printsons(aspoints *top);

void printquery(aspoints *top);

int countlist (aspoints *top);

void addmsg(messages **top, char *message, int fd);

int rmvmsg(messages **m);

void printmsg(messages *top);
