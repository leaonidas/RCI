#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "list.h"
#include "udplib.h"
#include "tcplib.h"
#include "utils.h"


int udpsock(char *rsaddr, char *rsport, struct addrinfo *hints, struct addrinfo **res, int listenUDP){

	int fd;

	memset(hints, 0, sizeof *hints);
	hints->ai_family=AF_INET;
	hints->ai_socktype=SOCK_DGRAM;
    if(listenUDP==1){
        hints->ai_flags=AI_PASSIVE|AI_NUMERICSERV;
    }else{
        hints->ai_flags=AI_NUMERICSERV;
    }
    
	if(getaddrinfo(rsaddr, rsport, hints, res)!=0) exit(0);

	fd=socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
	if(fd==-1) exit(0);


    if(listenUDP==1){
        if(bind(fd, (*res)->ai_addr, (*res)->ai_addrlen)==-1) exit(0);
    }

	return fd;
}

int udpclient (int fd, char *buffer, char *message, struct addrinfo *res, struct sockaddr_in addr, int smode, int rmode)
{
    extern int debug;
    socklen_t addrlen;
    int n;

    addrlen=sizeof(addr);

    if (smode==1)
    {
        if(sendto(fd, message, 128, 0, res->ai_addr, res->ai_addrlen)==-1) exit(0);
        if (debug==1) printf("Message sent:\n%s\n", message);
    }
    if (rmode==1)
    {
        if(recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen)==-1) exit(0);
        
        for (n=0;n<128; n++){
            if (buffer[n]=='\n')
                buffer[n+1]='\0';
        }
        
        if (debug==1) printf("Message received:\n%s\n", buffer);
    }
    return(0);
}

int udpserver (int fd, char *message, struct sockaddr_in *addr, int mode)
{
    extern int debug;
    socklen_t addrlen;
    int n;
    addrlen=sizeof(*addr);

    if(mode==0){
        if(recvfrom(fd, message, 128, 0, (struct sockaddr*)addr, &addrlen)==-1) exit(0);
        if (debug==1) printf("Message received:\n%s\n", message);
        
    }
    if(mode==1){
        n=sendto(fd, message, 128, 0, (struct sockaddr*)addr, addrlen);
        if (debug==1) printf("Message sent:\n%s\n", message);
    }
    
    
    return(0);
}

int whoisrootmsg (char *rsaddr, char *rsport, char *streamid, char *ipaddr, char *uport, char *asaddr, char *asport)
{
    /*sockets*/
    /*rootserv udp*/
    int rsfd;
    struct addrinfo rshints, *rsres;
    struct sockaddr_in raddr;

    /*messages*/
    char buffer[128];
    char whoisroot[128], urroot[128], rootis[128], error[128];
    //char errormsg[128];

    

    /*fill messages*/
    sprintf(whoisroot, "WHOISROOT %s %s:%s\n", streamid, ipaddr, uport);
    sprintf(urroot, "URROOT %s\n", streamid);

    
    /*create rootserv udp socket*/
    rsfd=udpsock(rsaddr, rsport, &rshints, &rsres, 0);
    
    
    /*send whoisroot message*/
    udpclient(rsfd, buffer, whoisroot, rsres, raddr, 1, 1);

    
    /*root server communications*/
    
    if(strcmp(buffer, urroot)==0) /*if we are root*/
    {
        freeaddrinfo(rsres);
        close(rsfd);
        return 1;
    }
    /*else if(strcmp(&buffer[0], "E")) 
    {
        printf("%s\n", buffer);
        freeaddrinfo(rsres);
        close(rsfd);
        return -1;
    }*/
    else
    {
        sscanf(buffer, "ROOTIS %s %[^:]:%s\n", streamid, asaddr, asport);
        freeaddrinfo(rsres);
        close(rsfd);
        return 0;
    }
    
    

}

void dumpmsg (char *rsaddr, char *rsport)
{
    /*sockets*/
    /*rootserv udp*/
    int rsfd;
    struct addrinfo rshints, *rsres;
    struct sockaddr_in raddr;
    
    char buffer[128], dump[128];
    sprintf(dump, "DUMP\n");
    
    /*create rootserv udp socket*/
    rsfd=udpsock(rsaddr, rsport, &rshints, &rsres, 0);
    
    /*send message*/
    udpclient(rsfd, buffer, dump, rsres, raddr, 1, 1);
    
    freeaddrinfo(rsres);
    close(rsfd);

}

void removemsg (char *rsaddr, char *rsport, char *streamid)
{
    /*sockets*/
    /*rootserv udp*/
    int rsfd;
    struct addrinfo rshints, *rsres;
    struct sockaddr_in raddr;
    
    char remove[128];
    
    /*create rootserv udp socket*/
    rsfd=udpsock(rsaddr, rsport, &rshints, &rsres, 0);
    
    sprintf(remove, "REMOVE %s\n", streamid);
    /*send message*/
    udpclient(rsfd, " ", remove, rsres, raddr, 1, 0);
    
    freeaddrinfo(rsres);
    close(rsfd);

}


void popreqsend (char *asaddr, char *asport, char *streamid, char *ipaddr, char *tport)
{
    /*access serv udp*/
    int asfd;
    struct addrinfo ashints, *asres;
    struct sockaddr_in aaddr;
    
    char popreq[128], popresp[128];
    char buffer[128];
    
    /*create rootserv udp socket*/
    asfd=udpsock(asaddr, asport, &ashints, &asres, 0);
    
    sprintf(popreq, "POPREQ\n");
    
    udpclient(asfd, buffer, popreq, asres, aaddr, 1, 1);

    sscanf(buffer, "POPRESP %s %[^:]:%s\n", streamid, ipaddr, tport);
    
    freeaddrinfo(asres);
    close(asfd);
    
}


/*verifies the POPREQ message
sends the POPRESP message with data of available access point
removes that access point from the linked list*/
void popreqrcv (int asfd, struct addrinfo *asres, char *streamid, aspoints *access)
{
    aspoints *current=access;
    char popreq[128], popresp[128];
    struct sockaddr_in aaddr;
    int n;
    
    udpserver(asfd, popreq, &aaddr, 0);

    if(strcmp(popreq, "POPREQ\n")==0)
    {
        
        printf("current ip%s\nport%s\n", current->ip, current->tport);
        
        
        if (current->next==NULL){
            sprintf(popresp, "POPRESP %s %s:%s\n", streamid, current->ip, current->tport);
        }
        else{
            while(current->next!=NULL){
                current=current->next;
            }
            sprintf(popresp, "POPRESP %s %s:%s\n", streamid, current->ip, current->tport);
        }
        
        if(current->avails==1){
            n=removeaspoint(access);            
        }
        else{
            current->avails--;
        }
        
        printaccess(access);

        udpserver(asfd, popresp, &aaddr, 1);
    }
    else{
      printf("Wrong message received in Access Server!\n");  
    }
}




