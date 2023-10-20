#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

//++core parameter
typedef struct stream{
    int sk_fd;
    struct stream* next;
    struct hook* hook_addr;
    int hook_count;
}stream;

typedef struct hook{
    int sk_fd;
   // int state;
    struct hook* next;
    char hook_buff[256];
}hook;

stream* stream_head=NULL;

//++sock_init()
static int sock_int(int port){
    //socket_fd
    int sk_fd;
    if ((sk_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        perror("socket_init failed");
        return 0;
    } else{
        printf("create socket fd success!\nsocket_fd :%d\r\n",sk_fd);
    }
    //bind
    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(port);
    int addr_len=sizeof(bind_addr);
    if (bind(sk_fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr))<0) {
        perror("bind failed");
        return 0;
    } else{
        printf("bind socket fd success!");
    }
    //listen
    if (listen(sk_fd, 512)<0) {
        perror("listen failed");
        return 0;
    }
    return sk_fd;
}

//++add_stream
void add_stream(int sk_fd){
    if(sk_fd ==0 ){
        printf("add_stream_fail\r\n");
    } else{
        if (stream_head == NULL){
        stream_head=(stream*)malloc(sizeof(struct stream));
        stream_head->state=listening;
        stream_head->hook_addr=NULL;
        stream_head->hook_count=0;
        stream_head->sk_fd=sk_fd;
        stream_head->next=NULL;
        } else{
        stream* mov=(stream*)malloc(sizeof(stream));
        mov->next=stream_head;
        mov->state=listening;
        mov->hook_addr=NULL;
        mov->hook_count=0;
        mov->sk_fd=sk_fd;
        stream_head=mov;
            }
        }
    }

//++select_init
fd_set readfds;
fd_set writefds;
struct timeval  timeout;
void stream_select_init(){
    //timeout_set
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    //list_stream_loop then add select
    stream* mov ;
    int max_fd = 0;
    for(mov=stream_head;mov;mov=mov->next){
        FD_SET(mov->sk_fd,&readfds);
        FD_SET(mov->sk_fd,&writefds);
        //list_hook_loop then add select
        if(mov->hook_addr!=NULL){
            printf("###################################raw_socket_fd:%d\n\r",mov->sk_fd);
            hook* mov_hook=NULL;
            for(mov_hook=mov->hook_addr;mov_hook;mov_hook=mov_hook->next){
                    printf("###################################hook_fd:%d\n\r",mov_hook->sk_fd);
                    FD_SET(mov_hook->sk_fd,&readfds);
                    FD_SET(mov_hook->sk_fd,&writefds);
                     //get max—fd
                    if ((mov_hook->sk_fd) > max_fd){max_fd = mov_hook->sk_fd;}
                }
        }
        if ((mov->sk_fd) > max_fd){max_fd = mov->sk_fd;}  
    }
    printf("\r\nmax_fd is \n%d\n",max_fd);
    int result = select(max_fd + 1, &readfds, &writefds, NULL, &timeout);
    printf("\nselect_result\n:%d\n",result);
}

//++accept
void accept_raw_sk(stream** stream_in){
    socklen_t clilen;
    struct sockaddr_in  cli_addr;
    int newsockfd = accept((*stream_in)->sk_fd, (struct sockaddr *) &cli_addr, &clilen);
    if((*stream_in)->hook_addr==NULL){
        (*stream_in)->hook_addr=(hook*)malloc(sizeof(hook));
        ((*stream_in)->hook_addr)->sk_fd=newsockfd;
        ((*stream_in)->hook_addr)->next=NULL;
        //state
        }else{
            hook* new_hook=(hook*)malloc(sizeof(hook));
            new_hook->sk_fd=newsockfd;
            new_hook->next=(*stream_in)->hook_addr;
            (*stream_in)->hook_addr=new_hook;
            }
    printf("accept_then_new_sk_fd:%d\r\n",newsockfd);
}

//++read
void read_hook_then_send(hook** hook_in){
                    read((*hook_in)->sk_fd,&((*hook_in)->hook_buff),256);         
                    //get remote message
                    struct sockaddr_in clientAddr;
                    socklen_t addrLen = sizeof(clientAddr);
                    if (getpeername((*hook_in)->sk_fd, (struct sockaddr*)&clientAddr, &addrLen) == 0) {
                        // h to n
                        char clientIP[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
                        // get port 
                        int clientPort = ntohs(clientAddr.sin_port);
                        printf("对端客户端的 IP 地址是：%s\n", clientIP);
                        printf("对端客户端的端口号是：%d\n", clientPort);
                    } else {
                        perror("获取对端客户端地址信息失败");
                        exit(1);
                    }
                    printf("******************************************************\nget message:%s\n\n", (*hook_in)->hook_buff);            
                    getchar();
    }
int main(int argc, char *argv[])
{
    for(int i=1;i<argc;i++){
        int port=atoi(argv[i]);
        add_stream(sock_int(port));
        }
    while(1){
    stream_select_init();
    stream* mov=NULL;
    for(mov=stream_head;mov;mov=mov->next){
        if(mov->hook_addr!=NULL){
            hook* mov_hook=NULL;
            for(mov_hook=mov->hook_addr;mov_hook;mov_hook=mov_hook->next){
                if(FD_ISSET(mov_hook->sk_fd,&readfds)){
                    read_hook_then_send(&mov_hook);//e
                }
            }
        }   
        if(FD_ISSET(mov->sk_fd,&readfds)){
            accept_raw_sk(&mov);//e
            }
        }
    }
    return 0;
}

