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




void helpinfo(){
    
    printf ("\nhelp\n\n./iamroot [<streamID>][-i <ipaddr>] [-t <tport>] [-u <uport>]\n");
    printf("[-s <rsaddr>[:<rsport>]]\n");
    printf("[-p <tcpsessions>]\n");
    printf("[-n <bestpops>] [-x <tsecs>]\n");
    printf("[-b] [-d] [-h]\n");
    printf("\n");
    printf("<streamID> – identificação do stream (ASCII, alfanuméricos, sem espaços; máx 63 char); nome do stream, IP e porto TCP do servidor fonte, separados por ‘:’\n");
    printf("<ipaddr> <tport> - IP e porto TCP onde aceita sessões de pares a jusante [58000];\n");
    printf("<uport> - porto UDP do servidor de acesso, quando raíz [58000];\n");
    printf("<rsaddr> <rsport> - IP e porto UDP do servidor de raízes [193.136.138.142 e 59000];\n");
    printf("<tcpsessions> - número de sessões TCP que aceita para a ligação de pares a jusante [1];\n");
    printf("<bestpops> - número de pontos de acesso a recolher, quando é raiz, em descoberta [1];\n");
    printf("<tsecs> - quando raíz, é o períodode registo periódico no servidor de raízes [5 s];\n");
    printf("–b - desactiva a apresentação de dados do stream na interface [on];\n");
    printf("–d - “debug”: aplicação apresenta informação detalhada acerca do seu funcionamento;\n");
    printf("-h - “help”: apresenta sinopse da linha de comandos e termina.\n\n");
    
}

int headercheck(char *buffread){

    char header[3];

    sscanf(buffread, "%s", header);

    if(strcmp(header, "WE")==0){
        return(0);
    }
    else if(strcmp(header, "NP")==0){
        return(1);
    }
    else if(strcmp(header, "RE")==0){
        return(2);
    }
    else if(strcmp(header, "PQ")==0){
        return(3);
    }
    else if(strcmp(header, "PR")==0){
        return(4);
    }


    return(-1);

}


int max (int udpas, int tcpups, int tcplst, aspoints *sons, int tcpsessions){

    int size = 3, i, maxfd=0;
    int vector[3];
    aspoints *current;
    current=sons;

    vector[0]=udpas; vector[1]=tcpups; vector[2]=tcplst;

    for(i=0; i<size; i++){
        if(vector[i]>maxfd){
            maxfd=vector[i];
        }
    }
    while(current!=NULL){
        if(current->fd>maxfd)
            maxfd=current->fd;
        current=current->next;
    }
    return(maxfd);

}

int checkenter(char *ptr, int nread){

    int i;

    for(i=0; i<nread; i++){
        if (ptr[i]=='\n')
            return 1;
    }
    return 0;
}


int checkpoints(int *v, int tcpsessions){
    
    int i, n=0;
    
    for(i=0; i<tcpsessions; i++){
        if(v[i]!=-1)
            n++;
    }
    
    return n;
}

void hexa (int count, char *queryid){
    int n;
    char one[10]="0", two[10]="00", three[10]="000";

    sprintf(queryid, "%X", count);

    n=strlen(queryid);

    if(n==3){
        strcat(one, queryid);
        strcpy(queryid, one);
    }
    if(n==2){
        strcat(two, queryid);
        strcpy(queryid, two);
    }
    if(n==1){
        strcat(three, queryid);
        strcpy(queryid, three);
    }
}


