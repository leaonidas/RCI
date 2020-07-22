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




int tcpsockClient(char *rsaddr, char *rsport, struct addrinfo *hints, struct addrinfo **res)
{
    int fd, i;

    memset(hints, 0, sizeof *hints);
    hints->ai_family=AF_INET;
    hints->ai_socktype=SOCK_STREAM;
    hints->ai_flags=AI_NUMERICSERV;
    
    
    if(getaddrinfo(rsaddr, rsport, hints, res)!=0) exit(0);
    
    fd=socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if(fd==-1) exit(0);
    
    if(connect(fd, (*res)->ai_addr, (*res)->ai_addrlen)==-1) exit(0);

    return(fd);

}

int tcpsockServer(char *rsport, struct addrinfo *hints, struct addrinfo **res){

    int fd, i;

    memset(hints, 0, sizeof *hints);
    hints->ai_family=AF_INET;
    hints->ai_socktype=SOCK_STREAM;
    hints->ai_flags=AI_PASSIVE|AI_NUMERICSERV;

    if(getaddrinfo(NULL, rsport, hints, res)!=0) exit(0);

    fd=socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if(fd==-1) exit(0);

    if(bind(fd, (*res)->ai_addr, (*res)->ai_addrlen)==-1) exit(0);

    if(listen(fd, 5)==-1) exit(0);

    return(fd);

}


int tcpread(messages **m, int fd){

    const char s[2] = "\n", z[2] = "\0";
    char *buffer;
    char rcv[128];
    char *piece;
    char temp[128];
    int i=0, nread;
    messages *current=*m;
    /*reads the message*/
    nread=read(fd, rcv, 128);
    if(nread==0 || nread==-1){
        return (nread);
    }
    rcv[nread]='\0';
    buffer=(char *)malloc(sizeof(char)*(nread+1));
    strcpy(buffer, rcv);

    /*if the last message is incomplete increments the flag*/
    if(buffer[nread-1]!='\n')
        i++;

    /*first piece on the message*/
    piece=strtok(buffer, s);
    //strcpy(temp, piece);

    /*there's an incomplete message in the struct*/
    while(current!=NULL){
        if(fd==current->fd){
            strcat((*m)->buffer,piece);
            strcat((*m)->buffer, s);
            strcat((*m)->buffer, z);
            (*m)->fd=0;
            current=current->next;
        }
    }
    if ((*m)==NULL){
        printf("FIRST PIECE: %s FIM\n", piece);
        addmsg(m, piece, 0);
        strcat((*m)->buffer, s);
        strcat((*m)->buffer, z);
    }
    /*gets other pieces on the message and stores in the struct*/
    while(piece!=NULL){
        piece=strtok(NULL,s);
        if (piece!=NULL){
            printf("PIECE: %s FIM\n", piece);
            //strcpy(temp, piece);
            addmsg(m, piece, 0);
            strcat((*m)->buffer, s);
            strcat((*m)->buffer, z);
        }
    }
    /*if the buffer doesn't end in \n the message is set incomplete*/
    if(i==1){
        (*m)->fd=fd;
    }

    free(buffer);

    return (nread);
}




int tcpwrite(int fd, char *message){

    extern int debug;
    ssize_t nwrite, nleft;
    char *ptr;

    ptr=message;

    nleft=strlen(message);

    while(nleft>0)
    {
        nwrite=write(fd, ptr, nleft);
        if(nwrite<=0) exit(0);
        nleft-=nwrite;
        ptr+=nwrite;
    }

    if (debug==1) printf("Message sent: \n%s\n", message);

    return(nwrite);
}


void welcome(int fd, char *streamid){

    char wlcm[128];
    sprintf(wlcm, "WE %s\n", streamid);
    tcpwrite(fd, wlcm);

}
int brokenstream(int fd){

    char bstream[128];
    sprintf(bstream, "BS\n");
    tcpwrite(fd, bstream);

    return(0);
}

int streamflow(int fd){

    char sflow[128];
    sprintf(sflow, "BS\n");
    tcpwrite(fd, sflow);

    return(1);
}

void redirect(int fd, char *message, char *ipaddr, char *tport, int mode){

    char redi[128];
    
    if (mode==0){
        sprintf(redi, "RE %s:%s\n", ipaddr, tport);
        tcpwrite(fd, redi);
    }
    if (mode==1){
        sscanf(message, "RE %[^:]:%s\n", ipaddr, tport);
        
    }
    
}

void newpop(int fd, char *message, char *ipaddr, char *tport, int mode){

    char npop[128], aux[2], ipread[32], portread[32];
     /*ou aqui ou vem lá de fora*/

    if(mode == 0){
        sprintf(npop, "NP %s:%s\n", ipaddr, tport);
        tcpwrite(fd, npop);
    }

    if(mode == 1){
        sscanf(message, "NP %[^:]:%s\n", ipaddr, tport);//depois vai ser o vector de cenas
    }
}

void popquery (char *message, aspoints *sons, aspoints *query, int mode){
    char popquery[128];
    aspoints *current=sons;
    
    if(mode==0){
        printf("%s\n%d\n",query->buffer, query->avails);
        sprintf(popquery, "PQ %s:%d\n", query->buffer, query->avails);
        
        current=sons;
        while(current!=NULL){
            tcpwrite(current->fd, popquery);
            current=current->next;
        }
    }
    if(mode==1){
        sscanf(message, "PQ %[^:]:%d\n", query->buffer, &query->avails);
    }
}


void popreply(int fd, aspoints *query, char *ipaddr, char *tport, int bp){
    char reply[128];
    
    sprintf(reply, "PR %s %s:%s %d\n", query->buffer, ipaddr, tport, bp);
    tcpwrite(fd, reply);
    
    
}

void prrecv(char *message, char *queryid, char *ip, char *port, int *avails){
    
    sscanf(message, "PR %s %[^:]:%s %d\n", queryid, ip, port, avails);
}


void treequery(int fd, char *ipaddr, char *tport, int mode){
    char query[128];

    sprintf(query, "TQ %s:%s\n", ipaddr, tport);
    tcpwrite(fd, query);
}


void treereply(int fd, char *buffer, char *ipaddr, char *tport, int tcpsessions, aspoints *sons){
    char reply[128], ip[128], port[128], aux[128];
    aspoints *current=sons;

    sscanf(buffer, "TQ %[^:]:%s\n", ip, port);

    //if the ip and port that came in the message are mine
    if(strcmp(ipaddr, ip)==0 && strcmp(tport, port)==0){
        sprintf(reply, "TR %s:%s %d\n", ipaddr, tport, tcpsessions);
        //I'll store my sons' stuff in a message (128 bits little?)
        while(current!=NULL){
            if(current->next!=NULL){//if the element is not the last only one \n
                sprintf(aux, "%s:%s\n", current->ip, current->tport);
                strcat(reply, aux);
                aux[0]='\0';
            }else{//if the element is the last one two \n
                sprintf(aux, "%s:%s\n\n", current->ip, current->tport);
                strcat(reply, aux);
                aux[0]='\0';
            }
            current=current->next;
        }//if there's no element send only the first line
        tcpwrite(fd, reply);
    }
    else{//if the ip and port are not mine further the message to my sons
        while(current!=NULL){
            tcpwrite(current->fd, buffer);
            current=current->next;
        }
    }
}







