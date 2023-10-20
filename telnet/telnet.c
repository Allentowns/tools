#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

//++core parameter
typedef struct stream {
    int sk_fd;
    struct stream* next;
    struct hook* hook_addr;
    int hook_count;
} stream;

typedef struct hook {
    int sk_fd;
    struct hook* next;
    char hook_buff[256];
} hook;

stream* stream_head = NULL;

//++sock_init()
static int sock_int(int port) {
    //socket_fd
    int sk_fd;
    if ((sk_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("socket_init failed");
        return 0;
    } else {
        printf("create socket fd success!\nsocket_fd: %d\r\n", sk_fd);
    }
    //bind
    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(port);
    int addr_len = sizeof(bind_addr);
    if (bind(sk_fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        perror("bind failed");
        return 0;
    } else {
        printf("bind socket fd success!");
    }
    //listen
    if (listen(sk_fd, 512) < 0) {
        perror("listen failed");
        return 0;
    }
    return sk_fd;
}

//++add_stream
void add_stream(int sk_fd) {
    if (sk_fd == 0) {
        printf("add_stream_fail\r\n");
    } else {
        if (stream_head == NULL) {
            stream_head = (stream *)malloc(sizeof(struct stream));
            stream_head->state = listening;
            stream_head->hook_addr = NULL;
            stream_head->hook_count = 0;
            stream_head->sk_fd = sk_fd;
            stream_head->next = NULL;
        } else {
            stream* mov = (stream *)malloc(sizeof(stream));
            mov->next = stream_head;
            mov->state = listening;
            mov->hook_addr = NULL;
            mov->hook_count = 0;
            mov->sk_fd = sk_fd;
            stream_head = mov;
        }
    }
}

//++select_init
fd_set readfds;
fd_set writefds;
struct timeval timeout;
void stream_select_init() {
    //timeout_set
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_ZERO
