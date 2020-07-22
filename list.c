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



/*add point in the beginning of the list*/
void addaspoint (aspoints **top, int fd, char *ip, char *tport, int avails){
    aspoints *new;
    
    new=malloc(sizeof(aspoints));
    
    new->fd=fd;
    strcpy(new->ip, ip);
    strcpy(new->tport, tport);
    new->avails=avails;
    
    new->next=*top;
    *top=new;
}


void addfirst(aspoints **top, int fd, char *ip, char *tport, int avails){
    aspoints *new;
    new=malloc(sizeof(aspoints));
    
    new->fd=fd;
    strcpy(new->ip, ip);
    strcpy(new->tport, tport);
    new->avails=avails;
    
    new->next=NULL;
    *top=new;
}


/*remove the last item of the list
and returns number os elements*/
int removeaspoint (aspoints *top){
    aspoints *next_top=NULL;
    aspoints *current=top;
    int n=1;
    
    if(top->next==NULL){
        free(top);
        return 0;
    }
    
    while (current->next->next!=NULL){
        current=current->next;
        n++;
    }
    
    free(current->next);
    current->next=NULL;
    
    return n;
}


/*remove a specific item by fd*/
void removebyfd(aspoints *top, int fd){
    aspoints *current=top;
    aspoints *temp=NULL;

    while(current!=NULL){
        if (current->fd==fd){
            top=current->next;
            free(current);
            return;
        }
        if(current->next->fd==fd){
            temp=current->next;
            current->next=temp->next;
            free(temp);
            return;
        }
        current=current->next;
    }
}

void removebyid(aspoints *top, char *id){
    aspoints *current=top;
    aspoints *temp=NULL;

    while(current!=NULL){
        if (current->buffer==id){
            top=current->next;
            free(current);
            return;
        }
        if(strcmp(current->next->buffer, id)==0){
            temp=current->next;
            current->next=temp->next;
            free(temp);
            return;
        }
        current=current->next;
    }
}


void printaccess(aspoints *top){
    aspoints *current=top;
    int n=0;
    printf("\naccess points\n");
    
    while(current!=NULL){
        printf("%d\n", n);
        printf("%s\n%s\navail:%d\n", current->ip, current->tport, current->avails);
        current=current->next;
        n++;
    }
    printf("\n");
}


int countlist (aspoints *top){
    int n=0;
    aspoints *current;
    current=top;
    
    while(current!=NULL){
        n++;
        current=current->next;
    }
    return n;
}


void printsons(aspoints *top){
    aspoints *current=top;
    int n=0;
    printf("\nsons\n");
    
    while(current!=NULL){
        printf("%d\n", n);
        printf("%d\n%s\n%s\n", current->fd, current->ip, current->tport);
        current=current->next;
        n++;
    }
    printf("\n");
}

void printquery(aspoints *top){
    aspoints *current=top;
    int n=0;
    printf("\nquery\n");
    
    while(current!=NULL){
        printf("%d\n", n);
        printf("%d\n%s\n%s\n", current->avails, current->ip, current->tport);
        current=current->next;
        n++;
    }
    printf("\n");
}


void addmsg(messages **top, char *message, int fd){
    messages *current = *top;
    messages *new;
    new=malloc(sizeof(messages));


    if(current!=NULL){
        while(current->next!=NULL){
            current=current->next;
        }
    
        current->next = malloc(sizeof(messages));
        strcpy(current->next->buffer, message);
        current->next->fd=fd;
        current->next->next=NULL;
    }
    else{
        strcpy(new->buffer, message);
        new->fd=fd;
        new->next=NULL;
        (*top)=new;
    }

}

int rmvmsg(messages **m){
    

    messages *current=NULL;
    messages *rmv=NULL;

    current=*m;
    while(current!=NULL){

        if((*m)->fd==0){
            current=(*m)->next;
            free(*m);
            *m=current;
            continue;
        }
        else if(current->next!=NULL && current->next->fd==0){

                rmv=current->next->next;
                free(current->next);
                current->next=rmv;
            
        }
        current=current->next;
    }
    return(1);
}


    /*messages *current = NULL;

    current=(*m)->next;
    free(*m);
    *m=current;*/

    



void printmsg(messages *top){
    messages *current=top;
    int n=0;
    printf("\nMessages received:\n");
    
    while(current!=NULL){
        printf("%d\n", n);
        printf("%s", current->buffer);
        current=current->next;
        n++;
    }
    printf("FIM\n");
}







    
