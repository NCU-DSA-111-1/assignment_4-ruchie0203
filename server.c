#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define IP "127.0.0.1"
#define SPORT 8266

int socketFD = -1;
int client_count = 0;
enum flag{
    LOGIN,QUIT,MESSAGE,START
};
struct clients{
    int client_num;
    char client_ip[32];
    unsigned short client_port;
    char client_name[16];
    struct clients* next;
};
typedef struct clients the_client;
typedef struct clients* the_clientptr;

typedef struct client_message{
    enum flag FLAG;
    int client_num;
    int direction;
    // char client_msg[1024];
}cmessage;

void sendmessage(cmessage *cmsg);
void addNewClient(char* ip,unsigned short port);
void freestruct();
void deleteClient(int deletenum);
int searchclient(char* ip,unsigned short port);

/* inet_ntoa 用來把字串IP換成用點分開的數字 */
/* noth 將網路字串轉換為本機字串 */

the_clientptr ClientData;
the_clientptr ClientData_Head;

/* To receive the message from clients */
void *clientmessage(void* arg){
    int recv_ret;
    struct sockaddr_in recv_addr;
    cmessage cmsg;
    socklen_t recvlen = sizeof(recv_addr);
    char ip[32];
    unsigned port;
    int quitnum;
    while(1){
        memset(ip,'\0',sizeof(ip));
        unsigned port=0;
        recv_ret = recvfrom(socketFD,&cmsg,sizeof(cmsg),0,(struct sockaddr*)&recv_addr,&recvlen);
        strcpy(ip,inet_ntoa(recv_addr.sin_addr));
        port = ntohs(recv_addr.sin_port);
        if(recv_ret > 0){
            // printf("[receive from %s:%d]%s\n",ip,port,cmsg.client_msg);
            switch(cmsg.FLAG){
                case LOGIN:
                    addNewClient(ip,port);
                    break;
                case QUIT:
                    quitnum = searchclient(ip,port);
                    if(quitnum>0)
                        deleteClient(quitnum);
                    else
                        printf("cant find!\n");
                    break;
                case MESSAGE:
                    // strcpy(msg,cmsg.client_msg);
                    // printf("Send!\n");
                    sendmessage(&cmsg);
                    // printf("Sended\n");
                    break;
            }   
        }
    }
}
void freestruct(){
    if(ClientData_Head!=NULL)
        free(ClientData_Head);
    if(ClientData!=NULL)
        free(ClientData);
}
void deleteClient(int deletenum){
    the_clientptr deleteptr = ClientData_Head;
    the_clientptr tempptr = (the_clientptr) malloc(sizeof(the_client));
    for(int i=0;i<deletenum-1;i++){
        deleteptr=deleteptr->next;
    }
    printf("\nClient %d Logout\n- Port:%hu\n- IP:%s\n",deleteptr->next->client_num,deleteptr->next->client_port,deleteptr->next->client_ip);
    if(deleteptr->next->next != NULL){
        tempptr=deleteptr->next;
        deleteptr->next=deleteptr->next->next;
        
        free(tempptr);
        deleteptr=deleteptr->next;
        while(deleteptr!=NULL){
            deleteptr->client_num--;
            deleteptr=deleteptr->next;
                               
        }
    }
    else{
        tempptr=deleteptr->next;
        free(tempptr);
        deleteptr->next=NULL;
        ClientData=deleteptr;
    }
    client_count--;
    
}
/* Search if it is the new client */
int searchclient(char* ip,unsigned short port){
    the_clientptr searchptr = ClientData_Head;
    
    while(searchptr != NULL){
        if((strcmp(searchptr->client_ip,ip)==0) && searchptr->client_port==port){
            return (searchptr->client_num);
        }
        searchptr=searchptr->next;   
    }
    return 0;
}
/* Send message to all clients */
void sendmessage(cmessage *cmsg){
    the_clientptr msgptr = ClientData_Head;
    struct sockaddr_in client_addr;
    int send_ret = 0;
    socklen_t caddrlen = sizeof(client_addr);
    int size = sizeof(cmessage);
    while(msgptr->next!=NULL){
        msgptr=msgptr->next;
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr=inet_addr(msgptr->client_ip);
        client_addr.sin_port = htons(msgptr->client_port);
        send_ret = sendto(socketFD,cmsg,size,0,(struct sockaddr*)&client_addr,caddrlen);
        if(send_ret == -1)
            perror("Send failed");
        else
            printf("Send to client %d msg:%d port: %hu IP: %s\n",msgptr->client_num,cmsg->direction,msgptr->client_port,msgptr->client_ip);
    }
}
/* Add a new client using linked list */
void addNewClient(char* ip,unsigned short port){
    int num;
    struct sockaddr_in client_addr;
    socklen_t caddrlen = sizeof(client_addr);
    cmessage a;
    a.client_num=0;
    a.direction=0;
    client_count++;
    the_clientptr new = (the_clientptr) malloc(sizeof(the_client));
    new->client_num=client_count;
    strcpy(new->client_ip,ip);
    new->client_port=port;
    ClientData->next=new;
    new->next=NULL;
    num=new->client_num;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr=inet_addr(new->client_ip);
    client_addr.sin_port = htons(new->client_port);
    sendto(socketFD,&num,sizeof(num),0,(struct sockaddr*)&client_addr,caddrlen);
    ClientData=ClientData->next;
    // printf("New Client Login\n- Name: %s\n",ClientData->client_name);
    printf("\nNew Client Login\n- IP:%s\n- Port:%hu\n- Num:%d\n",ClientData->client_ip,ClientData->client_port,ClientData->client_num);
    if(client_count==1){
        a.FLAG = START;
        sendmessage(&a);
    }
    return;
}
void main(){
    int bind_ret;
    socketFD = socket(AF_INET,SOCK_DGRAM,0);
    /* Add a server socket*/
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SPORT);
    server_addr.sin_addr.s_addr = inet_addr(IP);
    socklen_t saddr_len = sizeof(server_addr);
    
    /* Initialize the structure */
    ClientData = (the_clientptr) malloc(sizeof(the_client));
    ClientData->next=NULL;
    ClientData_Head = ClientData;

    bind_ret = bind(socketFD,(struct sockaddr*)&server_addr,saddr_len);
    if(bind_ret < 0){
        printf("Bind failed\n");
        close(socketFD);
        return;
    }
    pthread_t t;
    pthread_create(&t,NULL,clientmessage,NULL);
    pthread_detach(t);

    system("clear");
    printf("Waiting for data...\n");
    char msg[1024];
    while(1){
        memset(msg,'\0',sizeof(msg));
        scanf("%s",msg);
        if(strcmp(msg,"q")==0)
            break;
        // sendmessage(&msg);
    }
    close(socketFD);
    freestruct();
    return;
}