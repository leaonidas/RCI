#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include "list.h"
#include "udplib.h"
#include "tcplib.h"
#include "utils.h"



#define STDIN 0

/*global variable: debug prints*/
int debug=0, display=0, ascii=1, flow=1;

int main (int argc, char *argv[]) 
{
    /*arguments*/
    char streamid[63], ipaddr[128], rsaddr[128]="193.136.138.142"; 
    char tport[6]="58000", uport[6]="58000", rsport[6]="59000";
    int tcpsessions=1, bestpops=1, tsecs=5;
    int i, j, k;
    int data=1;
    int root, aux, check=0;
    int state=0;
    int nread=0, sonsnread=0;
    

    /*select*/
    fd_set rfds;
    int maxfd=0;
    char stdinbuffer[128], cmd[128];
    
    
    /*sockets*/
    char asaddr[128], asport[128];
    char ipconnect[128], portconnect[128];

    /*access serv udp*/
    int asfd;
    struct addrinfo ashints, *asres;
    //struct sockaddr_in aaddr;
    aspoints * access=NULL;
    int available;

    
    /*stream*/
    int srcfd;
    struct addrinfo srchints, *srcres;
    struct sockaddr_in srcaddr_;
    char srcaddr[128], srcport[128], streamname[128];
    char piece[128], message[128];
    char piece1[128], message1[128];

    int *tcpvec;
    aspoints *sons=NULL;
    aspoints *current=NULL;
    aspoints *current1=NULL, *current2=NULL;
    aspoints *query=NULL;
    messages *msgs=NULL, *removeme=NULL, *srcremove=NULL;
    int count=1;
    int qaux;
    //char ***apoints;

    /*cenas*/
    aspoints *this;

    /*socket tcp for listen*/
    int lstfd, auxfd=0;
    struct addrinfo lsthints, *lstres;
    struct sockaddr_in lstaddr;
    socklen_t addrlen;
    addrlen = sizeof(lstaddr);
    
    
    /*messages*/
    char buffer[128];
    
    char auxqid[128], auxip[128], auxport[128]; int auxav;
    
    
    char errormsg[128];



    
    /*arguments*/
    if(argc<2) exit(0);
    if (argv[1][0]=='-')
    {
        if (strcmp(argv[1], "-h")==0){
            helpinfo();
            exit(0);
        }
        else{
            removemsg(rsaddr, rsport, streamid);
            dumpmsg(rsaddr, rsport);
            exit(0);
        }
    }
    else
    {
        sscanf(argv[1], "%[^;]", streamid);
    }
    for (i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-i")==0)
            sscanf(argv[i+1],"%[^;]",ipaddr);
        
        if (strcmp(argv[i], "-t")==0)
            sscanf(argv[i+1],"%[^;]",tport);
        
        if (strcmp(argv[i], "-u")==0)
            sscanf(argv[i+1],"%[^;]",uport);
        
        if (strcmp(argv[i], "-s")==0)
            sscanf(argv[i+1],"%[^;]:%[^;]",rsaddr, rsport);
        
        if (strcmp(argv[i], "-p")==0)
            sscanf(argv[i+1],"%d",&tcpsessions);
        
        if (strcmp(argv[i], "-n")==0)
            sscanf(argv[i+1],"%d",&bestpops);
        
        if (strcmp(argv[i], "-x")==0)
            sscanf(argv[i+1],"%d",&tsecs);
        
        if (strcmp(argv[i], "-d")==0)
            debug=1;
        
        if (strcmp(argv[i], "-b")==0)
            data=0;
        
        if (strcmp(argv[i], "-h")==0)
        {
            helpinfo();
            exit(0);
        }
        
    }
    
    /*struct to store son's info*/
    sons=malloc(sizeof(aspoints));
    sons=NULL;
    /*struct to store queries info*/
    query=malloc(sizeof(aspoints));
    query=NULL;

    msgs=malloc(sizeof(messages));
    msgs=NULL;


    lstfd=tcpsockServer(tport, &lsthints, &lstres);
    if(lstfd==-1){
        printf("Socket for listening failed!\n");
        exit(0);
    }
    
    
    
    
    printf("\n");
    root=whoisrootmsg(rsaddr, rsport, streamid, ipaddr, uport, asaddr, asport);
    
    /*root server communications*/
    if(root==1) /*if we are root*/
    {
        sscanf(streamid, "%[^:]:%[^:]:%s\n", streamname, srcaddr, srcport);
        
        srcfd=tcpsockClient(srcaddr, srcport, &srchints, &srcres);
        printf("TCP socket to source set\n");
        
        /*acess server socket*/
        asfd=udpsock(NULL, uport, &ashints, &asres, 1);
        printf("Access server UDP socket set\n");
        
        /*structure with the access points available*/
        access=malloc(sizeof(aspoints));
        access=NULL;
        
        /*fill with as many sessions available*/
        addaspoint(&access, 0, ipaddr, tport, tcpsessions);
        
        printaccess(access);
        printf("elementos: %d\n", countlist(access));
        
    }
    else if(root==-1) 
    {
        exit(0);
    }
    else if(root==0) /*if some one else is root*/
    {   
        popreqsend(asaddr, asport, streamid, ipconnect, portconnect);

        srcfd=tcpsockClient(ipconnect, portconnect, &srchints, &srcres);
        printf("up TCP socket set\n");
        
        /*MECANISMOS DE ADESÃO A ARVORE*/
    }
      
    
    /*application cycle - interface*/
    while(1)
    {


        FD_ZERO(&rfds);
        
        FD_SET(STDIN, &rfds); //STDIN
        if(root==1) FD_SET(asfd, &rfds); //UDP socket to communicate when accessserv
        FD_SET(lstfd, &rfds); //TCP socket to listen to incoming connection
        FD_SET(srcfd, &rfds); //TCP socket to establish connections and get data from stream        
        current=sons;
        while(current!=NULL){
            FD_SET(current->fd, &rfds);
            current=current->next;
        }


        printf("im in!!!\n");
        
        maxfd = max(asfd, srcfd, lstfd, sons, tcpsessions);
        printf("%d\n", maxfd);
        
        if (select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL)<=0) exit(0);
        
        /*stdin socket*/
        if(FD_ISSET(STDIN, &rfds))
        {
            printf("\n");
            
            fgets(cmd, 128, stdin);
            
            if(cmd[strlen(cmd)-1]=='\n') cmd[strlen(cmd)-1]='\0';
            
            
            /*verify upper case letters*/
            for(i=0; i<strlen(cmd); i++)
            {
                cmd[i]=(tolower(cmd[i]));
            }
            
            
            /*commands*/
            if (strcmp(cmd, "streams")==0)
            {
                dumpmsg(rsaddr, rsport);
            }
            
            else if (strcmp(cmd, "status")==0)
            {
                printf("%s\n", streamid);
                
                if(flow==1) printf("Stream flowing!\n\n");
                else if(flow==0) printf("Stream broken!\n\n");

                if(root==0){
                  printf("I am not root!\n");
                  printf("Upstream IP: %s\nUpstream port TCP port: %s\n\n", ipconnect, portconnect);  
                } 
                else if(root==1){
                  printf("I am Groot!\n");
                  printf("Access server IP: %s\nAccess server UDP port: %s\n\n", asaddr, asport);
                  if(debug==1){
                    printf("WE ARE GROOT!\n");
                  } 
                }
                printf("My IP: %s\nMy TCP port: %s\n\n", ipaddr, tport);

                printf("%d TCP connections are possible\n TCP connections are available\n\n", tcpsessions);

                printf("Sons:\n");//this criado na linha 69 :)
                this=sons;
                while(this!=NULL){
                    printf("%s %s\n", this->ip, this->tport);
                }printf("\n");


            }
            
            else if (strcmp(cmd, "display on")==0)
            {
                display=1;//quando fizermos função para a data se o display for 1 imprime
            }
            
            else if (strcmp(cmd, "display off")==0)
            {
                display=0;//quando fizermos função para a data se o display for 0 não imprime   
            }
            
            else if (strcmp(cmd, "format ascii")==0)
            {
                ascii=1;//quando fizermos função para a data, if display==1 && ascii==1 imprime normal
            }
            
            else if (strcmp(cmd, "format hex")==0)
            {
                ascii=0;//quando fizermos função para a data, if display==1 && ascii==0 imprime com o %X   
            }
            
            else if (strcmp(cmd, "debug on")==0)
            {
                debug=1;
                printf("Debug mode active\n");
            }
            
            else if (strcmp(cmd, "debug off")==0)
            {
                debug=0;
                printf("Debug mode inactive\n");
            }
            
            else if (strcmp(cmd, "tree")==0)
            {
                
            }
            
            else if(strcmp(cmd, "exit")==0) 
            {
                /*PROCEDIMENTOS ABANDONO DA ARVORE*/
                if(root==1) {
                    removemsg(rsaddr, rsport, streamid);
                    close(asfd);
                }
                close(srcfd);
                close(lstfd);
                while(sons!=NULL){
                    close(sons->fd);
                    sons=sons->next;
                }

                    
                exit(1);
            }
            
            else printf("Invalid command\n");
        }
        if(FD_ISSET(asfd, &rfds)){
            
            popreqrcv(asfd, asres, streamid, access);
            /*send pop query to all sons*/


        }
        if(FD_ISSET(lstfd, &rfds)){

            auxfd=accept(lstfd, (struct sockaddr*)&lstaddr, &addrlen);
            
            if(countlist(sons)==tcpsessions){
                redirect(auxfd, "", sons->ip, sons->tport, 0);
                close(auxfd);
            }
            else{
                addaspoint(&sons, auxfd, "", "", 0);
                printsons(sons);
                welcome(sons->fd, streamid);

                current=sons;
                while(current!=NULL){
                    addfirst(&query, 0, "", "", bestpops);
                    hexa(count, query->buffer);
                    count++;
                    popquery(" ", sons, query, 0);
                    current=current->next;
                }

            }
        }
        
        
        if(FD_ISSET(srcfd, &rfds)){
            
            nread=tcpread(&msgs, srcfd);

            //if (debug==1) printf("Message recieved: \n%s\n", message);
            

            if (nread==-1 || nread==0){
                current=sons;//acho que aqui não vê sons. 
                //cada um tem um srcfd e fica independente quando o parent sai
                while(current!=NULL){
                    flow=brokenstream(current->fd);
                    current=current->next;
                }

                exit(0);
                
                /*METODOS DE ADESÃO A ARVORE*/
            }

            printmsg(msgs);

            srcremove=msgs;
            while(srcremove!=NULL){
                aux=headercheck(srcremove->buffer);//checks the header
                
                if (aux==0){
                    newpop(srcfd, " ", ipaddr, tport, 0);
                }
                else if(aux==2){
                    redirect(0, msgs->buffer, ipconnect, portconnect, 1);
                    srcfd=tcpsockClient(ipconnect, portconnect, &srchints, &srcres);    
                }
                else if(aux==3){
                    
                    addaspoint(&query, 0, "", "", 0);
                    popquery(msgs->buffer, sons, query, 1);
                    /*case there's no access point available*/
                    if(tcpsessions-countlist(sons)==0){
                        popquery("", sons, query, 0);
                        
                    }
                    /*case there's all point available*/
                    else if((tcpsessions-countlist(sons))>=query->avails){
                        popreply(srcfd, query, ipaddr, tport, query->avails);
                        removebyid(query, query->buffer);
                    }
                    /*case there's not enough points available*/
                    else if((tcpsessions-countlist(sons))<query->avails){
                        query->avails-=(tcpsessions-countlist(sons));
                        popquery("", sons, query, 0);
                        popreply(srcfd, query, ipaddr, tport, (tcpsessions-countlist(sons)));
                    }
                }srcremove=srcremove->next;
            } 
            rmvmsg(&msgs);
        }

        /*current iterates sons file descriptors*/
        current=sons;
        removeme=msgs;
        while(current!=NULL){
            if(FD_ISSET(current->fd, &rfds)){
                
                //nread=read(current->fd, current->buffer, 128);
                //while(tcpread(nread, current->buffer, )==1)
                //if (debug==1) printf("Message recieved: \n%s\n", current->buffer);
                /*vai ler e guardar as mensagens na estrutura*/
                sonsnread = tcpread(&msgs ,current->fd);
                printmsg(msgs);
                /*crashou bazou*/
                if(sonsnread==-1 || sonsnread==0){
                    removebyfd(sons, current->fd);
                }
                /*vai percorrer as mensagens*/
                while(removeme!=NULL){
                    if(removeme->fd==0){//se o fd for zero faz cenas com essa mensagem, falta alterar isso removeme->buffer
                
                        aux=headercheck(removeme->buffer);
                        
                        if (aux==1){
                            newpop(0, removeme->buffer, current->ip, current->tport, 1);
                            //printf("guardou as vairaveis NP\n");
                            printsons(sons);
                        }
                        /*receives PReply*/ 
                        else if(aux==4){
                            prrecv(removeme->buffer, auxqid, auxip, auxport, &auxav);
                            qaux=0;
                            /*if we are rute*/
                            if(root==1){
                                /*current1 iterates the queries requests of the son
                                to see if theres a query for the reply received*/
                                current1=query;
                                while(current1!=NULL){
                                    /*if there is a query*/
                                    if ((strcmp(current1->buffer, auxqid)==0)){
                                        /*if the bestpops asked are all received*/
                                        if(current1->avails==auxav)
                                            /*revomes the query*/
                                            removebyid(query, auxqid);
                                        /*if the bestpops received are not enough*/
                                        else if(current1->avails>auxav)
                                            /*decrements the bestpops of the query*/
                                            current1->avails-=auxav;
                                        /*as we are root, we have to update the access points list*/
                                        current2=access;
                                        while(current2!=NULL){
                                            /*if we already have this par's ip and tport in the access list*/
                                            if ((strcmp(current2->ip, auxip)==0) && (strcmp(current2->tport, auxport)==0)){
                                                /*if the number os bestpops recieved if higher than what exists*/
                                                if(current2->avails<auxav)
                                                    /*update*/
                                                    current2->avails=auxav;
                                                qaux=1; /*sets aux variable to 1*/
                                            }
                                            current2=current2->next;
                                        }
                                        /*if the aux variable is 0 there is no data of this par in the list*/
                                        if (qaux==0)
                                            /*add access point to the list*/
                                            addaspoint(&access, 0, auxip, auxport, auxav);
                                    }
                                    current1=current1->next;
                                }
                            }
                            /*if we are not rute*/
                            else{
                                /*current1 iterates the pop queries*/
                                current1=query;
                                while(current1!=NULL){
                                    /*if there's a query to the reply received*/
                                    if ((strcmp(current1->buffer, auxqid)==0)){
                                        /*send the reply up*/
                                        popreply(srcfd, current1, auxqid, auxport, auxav);
                                        /*updates the bestpops of the query*/
                                        current1->avails-=aux;
                                        /*if this bestpops is zero all the queries have been replied*/
                                        if(current1->avails==0)
                                            /*removes the query of the list*/
                                            removebyid(query, auxqid);
                                    }
                                    current1=current1->next;
                                }
                            }
                        }removeme=removeme->next;
                        //rmvmsg(&removeme);/*removes the message of the list*/
                    }
                    else{//*if fd=0 the message is not complete*/
                        removeme=removeme->next;
                    }
                }
                rmvmsg(&msgs);
            } current=current->next;
        }
    }
    
    
    
    exit(1);
}



